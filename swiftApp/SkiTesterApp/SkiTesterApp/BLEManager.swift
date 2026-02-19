//
//  BLEManager.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 21.1.2026.
//
//
//  BLEManager.swift
//  SkiTesterApp
//
//  CoreBluetooth client for your ESP32 NimBLE "UART-style" service.
//  - Subscribes to TX (notify) for JSON messages
//  - Writes ACK JSON {"id":...} to RX (write) after every received packet
//
//  UUIDs must match firmware:
//  SERVICE 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
//  RX      6E400002-B5A3-F393-E0A9-E50E24DCCA9E  (phone -> device write)
//  TX      6E400003-B5A3-F393-E0A9-E50E24DCCA9E  (device -> phone notify)
//
//  Supports background restoration via CBCentralManager restore identifier and willRestoreState.
//

import Foundation
import CoreBluetooth
import Combine

final class BLEManager: NSObject, ObservableObject {

    // MARK: - Public state

    @Published var stateText: String = "BLE: ei aloitettu"
    @Published var isConnected: Bool = false
    @Published var discoveredPeripherals: [CBPeripheral] = []
    @Published var discoveredNames: [String] = []

    var onTextMessage: ((String) -> Void)?

    // MARK: - UUIDs

    private let serviceUUID = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
    private let rxUUID      = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
    private let txUUID      = CBUUID(string: "6E400003-B5A3-F393-E0A9-E50E24DCCA9E")

    // MARK: - CoreBluetooth

    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var rxChar: CBCharacteristic?
    private var txChar: CBCharacteristic?

    private var pendingPeripheralNameMatches: [String] = ["SkiTester"]
    private var isConnecting: Bool = false


    override init() {
        super.init()
        central = CBCentralManager(delegate: self, queue: .main, options: [CBCentralManagerOptionRestoreIdentifierKey: "BLEManagerRestoreID"])
    }

    // MARK: - Control

    func start() {
        guard central.state == .poweredOn else {
            stateText = "BLE: Bluetooth ei päällä"
            return
        }

        // reset
        isConnecting = false
        peripheral = nil
        rxChar = nil
        txChar = nil
        isConnected = false

        print("SCAN START")
        central.stopScan()
        stateText = "BLE: skannataan"
        central.scanForPeripherals(withServices: nil,
                                   options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
    }

    func stop() {
        central.stopScan()
        if let p = peripheral {
            central.cancelPeripheralConnection(p)
        }
        peripheral = nil
        rxChar = nil
        txChar = nil
        isConnected = false
        stateText = "BLE: pysäytetty"
    }

    func sendAck(id: Any) {
        guard let p = peripheral, let rx = rxChar else { return }

        let ack: [String: Any] = ["id": id]
        guard let data = try? JSONSerialization.data(withJSONObject: ack) else { return }

        // Prefer .withResponse if supported; else fallback.
        let type: CBCharacteristicWriteType = rx.properties.contains(.write) ? .withResponse : .withoutResponse
        p.writeValue(data, for: rx, type: type)
    }

    func sendClear() {
        guard let p = peripheral, let rx = rxChar else { return }
        let cmd: [String: Any] = ["cmd": "clear"]
        guard let data = try? JSONSerialization.data(withJSONObject: cmd) else { return }
        let type: CBCharacteristicWriteType = rx.properties.contains(.write) ? .withResponse : .withoutResponse
        p.writeValue(data, for: rx, type: type)
    }

    func connect(to peripheral: CBPeripheral) {
        guard central.state == .poweredOn else { return }
        isConnecting = true
        self.peripheral = peripheral
        self.peripheral?.delegate = self
        stateText = "BLE: yhdistetään \(peripheral.name ?? peripheral.identifier.uuidString)"
        central.stopScan()
        central.connect(peripheral, options: nil)
    }

    func clearDiscoveredPeripherals() {
        discoveredPeripherals.removeAll()
        discoveredNames.removeAll()
    }
}

extension BLEManager: CBCentralManagerDelegate {

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        print("CB state", central.state.rawValue)

        switch central.state {
        case .poweredOn:
            stateText = "BLE: valmis"
            // Auto-start scanning when Bluetooth becomes ready
            if !isConnected && !isConnecting {
                start()
            }

        case .poweredOff:
            stateText = "BLE: Bluetooth pois"

        case .unauthorized:
            stateText = "BLE: ei oikeuksia"

        case .unsupported:
            stateText = "BLE: ei tuettu"

        case .resetting:
            stateText = "BLE: resetoidaan"

        case .unknown:
            fallthrough
        @unknown default:
            stateText = "BLE: tuntematon"
        }
    }


    func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String : Any],
                        rssi RSSI: NSNumber) {

        let advName = advertisementData[CBAdvertisementDataLocalNameKey] as? String
        print("DISCOVER", advName, peripheral.identifier.uuidString, RSSI)
        let name = advName ?? peripheral.name ?? ""

        guard name.contains("SkiTester") else { return }

        if !discoveredPeripherals.contains(where: { $0.identifier == peripheral.identifier }) {
            discoveredPeripherals.append(peripheral)
            discoveredNames.append(name)
        }

        guard !isConnecting && !isConnected else { return }
        isConnecting = true

        self.peripheral = peripheral
        self.peripheral?.delegate = self

        stateText = "BLE: yhdistetään \(name)"
        central.stopScan()
        central.connect(peripheral, options: nil)
    }



    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("CONNECTED", peripheral.identifier.uuidString)
        isConnected = true
        stateText = "BLE: yhdistetty"
        peripheral.discoverServices([serviceUUID])
    }

    func centralManager(_ central: CBCentralManager,
                        didFailToConnect peripheral: CBPeripheral,
                        error: Error?) {
        print("FAIL CONNECT", error?.localizedDescription ?? "nil")
        isConnected = false
        isConnecting = false
        stateText = "BLE: yhteys epäonnistui"
        self.peripheral = nil
        rxChar = nil
        txChar = nil
        start()
    }
    
    func centralManager(_ central: CBCentralManager,
                        didDisconnectPeripheral peripheral: CBPeripheral,
                        error: Error?) {
        print("DISCONNECTED", error?.localizedDescription ?? "nil")
        isConnected = false
        isConnecting = false
        stateText = "BLE: irti"
        self.peripheral = nil
        rxChar = nil
        txChar = nil
        start()
    }
    
    func centralManager(_ central: CBCentralManager, willRestoreState dict: [String : Any]) {
        print("BLE restoration: willRestoreState")
        if let peripherals = dict[CBCentralManagerRestoredStatePeripheralsKey] as? [CBPeripheral] {
            print("Restored peripherals: \(peripherals.map { $0.identifier.uuidString })")
            for peripheral in peripherals {
                self.peripheral = peripheral
                peripheral.delegate = self
                isConnected = (peripheral.state == .connected)
            }
        }
        if let scannedServices = dict[CBCentralManagerRestoredStateScanServicesKey] as? [CBUUID] {
            print("Restored scan services: \(scannedServices)")
        }
        if let connectedPeripherals = dict[CBCentralManagerRestoredStatePeripheralsKey] as? [CBPeripheral] {
            for p in connectedPeripherals {
                p.delegate = self
            }
        }
    }
}

extension BLEManager: CBPeripheralDelegate {

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        print("SERVICES", peripheral.services?.map{$0.uuid.uuidString} ?? [])
        guard error == nil else { return }
        guard let services = peripheral.services else { return }

        for s in services where s.uuid == serviceUUID {
            peripheral.discoverCharacteristics([rxUUID, txUUID], for: s)
            return
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didDiscoverCharacteristicsFor service: CBService,
                    error: Error?) {
        print("CHARS", service.characteristics?.map{$0.uuid.uuidString} ?? [])
        guard error == nil else { return }
        guard let chars = service.characteristics else { return }

        for c in chars {
            if c.uuid == rxUUID { rxChar = c }
            if c.uuid == txUUID { txChar = c }
        }

        if let tx = txChar {
            stateText = "BLE: kuunnellaan"
            peripheral.setNotifyValue(true, for: tx)
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didUpdateNotificationStateFor characteristic: CBCharacteristic,
                    error: Error?) {
        if let error = error {
            print("NOTIFY STATE ERROR", error.localizedDescription)
            return
        }
        print("NOTIFY STATE", characteristic.uuid.uuidString, "isNotifying:", characteristic.isNotifying)

        if characteristic.uuid == txUUID {
            stateText = characteristic.isNotifying ? "BLE: kuunnellaan" : "BLE: notify pois"
        }
    }

    func peripheral(_ peripheral: CBPeripheral,
                    didUpdateValueFor characteristic: CBCharacteristic,
                    error: Error?) {
        guard error == nil else { return }
        guard characteristic.uuid == txUUID else { return }
        guard let data = characteristic.value else { return }
        guard let text = String(data: data, encoding: .utf8) else { return }

        onTextMessage?(text)
    }
}

