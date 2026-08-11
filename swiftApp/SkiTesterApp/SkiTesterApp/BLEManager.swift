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
    @Published var devices: [BLEDevice] = []

    var onTextMessage: ((String) -> Void)?

    private static let udSelectedDeviceIDKey = "selectedTesterDeviceID"
    private static let udSelectedDeviceNameKey = "selectedTesterDeviceName"

    // MARK: - UUIDs

    private let serviceUUID = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
    private let rxUUID      = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
    private let txUUID      = CBUUID(string: "6E400003-B5A3-F393-E0A9-E50E24DCCA9E")

    // MARK: - CoreBluetooth

    private var central: CBCentralManager!
    private var peripheral: CBPeripheral?
    private var rxChar: CBCharacteristic?
    private var txChar: CBCharacteristic?

    private var isConnecting: Bool = false
    var discoveryOnly: Bool = false


    override init() {
        super.init()
        central = CBCentralManager(delegate: self, queue: .main, options: [CBCentralManagerOptionRestoreIdentifierKey: "BLEManagerRestoreID"])
    }

    // MARK: - Control

    func start(withServices services: [CBUUID]? = nil) {
        guard central.state == .poweredOn else {
            stateText = "BLE: Bluetooth ei päällä"
            return
        }

        log("start() instance=\(ObjectIdentifier(self)) discoveryOnly=\(discoveryOnly)")

        isConnecting = false
        resetConnectionState()

        central.stopScan()
        stateText = "BLE: skannataan"
        central.scanForPeripherals(withServices: nil,
                                   options: [CBCentralManagerScanOptionAllowDuplicatesKey: true])

        // Include already-connected peripherals so they appear even if not advertising
        includeAlreadyConnectedPeripherals()
    }

    func stop() {
        if central.state == .poweredOn {
            central.stopScan()
            if let p = peripheral {
                central.cancelPeripheralConnection(p)
            }
        }
        resetConnectionState()
        stateText = "BLE: pysäytetty"
    }

    func sendAck(id: Any) {
        write(["id": id])
    }

    func sendClear() {
        write(["cmd": "clear"])
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

    func selectDevice(_ device: BLEDevice) {
        log("selectDevice saving ID=\(device.id.uuidString), name=\(device.name)")
        UserDefaults.standard.set(device.id.uuidString, forKey: Self.udSelectedDeviceIDKey)
        UserDefaults.standard.set(device.name, forKey: Self.udSelectedDeviceNameKey)
        connect(to: device.peripheral)
    }

    func clearDiscoveredPeripherals() {
        devices.removeAll()
    }

    // MARK: - Private helpers

    /// Case-insensitive match against the expected peripheral name ("SkiTester...").
    private func matchesTargetName(_ name: String?) -> Bool {
        name?.localizedCaseInsensitiveContains("SkiTester") ?? false
    }

    /// Insert or update a device in `devices`, preserving its RSSI when `rssi` is nil.
    private func upsertDevice(id: UUID, name: String, rssi: Int?, peripheral: CBPeripheral) {
        if let idx = devices.firstIndex(where: { $0.id == id }) {
            devices[idx] = BLEDevice(id: id, name: name, rssi: rssi ?? devices[idx].rssi, peripheral: peripheral)
        } else {
            devices.append(BLEDevice(id: id, name: name, rssi: rssi ?? 0, peripheral: peripheral))
        }
    }

    private func includeAlreadyConnectedPeripherals() {
        let connected = central.retrieveConnectedPeripherals(withServices: [serviceUUID])
        let savedIDString = UserDefaults.standard.string(forKey: Self.udSelectedDeviceIDKey)
        let savedName = UserDefaults.standard.string(forKey: Self.udSelectedDeviceNameKey)

        for p in connected {
            let hasSavedMatch = (savedIDString != nil) && (UUID(uuidString: savedIDString!) == p.identifier)
            let nameCandidate = p.name ?? (hasSavedMatch ? savedName : nil)
            log("retrieveConnectedPeripherals found id=\(p.identifier.uuidString), name=\(nameCandidate ?? "<nil>") hasSavedMatch=\(hasSavedMatch)")

            // Apply same name filter as didDiscover, but always include if it is the saved selection
            guard matchesTargetName(nameCandidate) || hasSavedMatch else { continue }
            let finalName = nameCandidate ?? "SkiTester"
            upsertDevice(id: p.identifier, name: finalName, rssi: nil, peripheral: p)
        }
    }

    private func resetConnectionState() {
        peripheral = nil
        rxChar = nil
        txChar = nil
        isConnected = false
    }

    private func write(_ payload: [String: Any]) {
        guard let p = peripheral, let rx = rxChar else { return }
        guard let data = try? JSONSerialization.data(withJSONObject: payload) else { return }

        // Prefer .withResponse if supported; else fallback.
        let type: CBCharacteristicWriteType = rx.properties.contains(.write) ? .withResponse : .withoutResponse
        p.writeValue(data, for: rx, type: type)
    }

    private func log(_ message: String) {
        #if DEBUG
        print("[BLE] \(message)")
        #endif
    }
}

extension BLEManager: CBCentralManagerDelegate {

    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        log("central state update instance=\(ObjectIdentifier(self)) state=\(central.state.rawValue)")

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
        let baseName = advName ?? peripheral.name
        // Filter: only include devices whose name contains "SkiTester" (case-insensitive)
        guard let name = baseName, matchesTargetName(name) else { return }

        upsertDevice(id: peripheral.identifier, name: name, rssi: RSSI.intValue, peripheral: peripheral)

        // In discovery-only mode (BLE setup), do not auto-connect or stop scanning
        guard !discoveryOnly else { return }

        // Legacy behavior: auto-connect to known devices (kept for MeasurementView flow)
        guard !isConnecting && !isConnected else { return }
        connect(to: peripheral)
    }


    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        log("connected \(peripheral.identifier.uuidString)")
        isConnected = true
        stateText = "BLE: yhdistetty"
        peripheral.discoverServices([serviceUUID])
    }

    func centralManager(_ central: CBCentralManager,
                        didFailToConnect peripheral: CBPeripheral,
                        error: Error?) {
        log("fail connect \(error?.localizedDescription ?? "nil")")
        isConnecting = false
        stateText = "BLE: yhteys epäonnistui"
        resetConnectionState()
        start()
    }

    func centralManager(_ central: CBCentralManager,
                        didDisconnectPeripheral peripheral: CBPeripheral,
                        error: Error?) {
        log("disconnected \(error?.localizedDescription ?? "nil")")
        isConnecting = false
        stateText = "BLE: irti"
        resetConnectionState()
        start()
    }

    func centralManager(_ central: CBCentralManager, willRestoreState dict: [String : Any]) {
        log("willRestoreState")
        if let peripherals = dict[CBCentralManagerRestoredStatePeripheralsKey] as? [CBPeripheral] {
            log("restored peripherals: \(peripherals.map { $0.identifier.uuidString })")
            for peripheral in peripherals {
                self.peripheral = peripheral
                peripheral.delegate = self
                isConnected = (peripheral.state == .connected)
            }
        }
        if let scannedServices = dict[CBCentralManagerRestoredStateScanServicesKey] as? [CBUUID] {
            log("restored scan services: \(scannedServices)")
        }
    }
}

extension BLEManager: CBPeripheralDelegate {

    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        log("services \(peripheral.services?.map{$0.uuid.uuidString} ?? [])")
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
        log("characteristics \(service.characteristics?.map{$0.uuid.uuidString} ?? [])")
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
            log("notify state error \(error.localizedDescription)")
            return
        }
        log("notify state \(characteristic.uuid.uuidString) isNotifying=\(characteristic.isNotifying)")

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

struct BLEDevice: Identifiable, Equatable {
    let id: UUID
    let name: String
    var rssi: Int
    let peripheral: CBPeripheral

    static func == (lhs: BLEDevice, rhs: BLEDevice) -> Bool {
        lhs.id == rhs.id
    }
}
