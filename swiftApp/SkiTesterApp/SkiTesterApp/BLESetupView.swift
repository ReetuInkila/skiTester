import SwiftUI
import CoreBluetooth

struct BLESetupView: View {
    @EnvironmentObject var store: AppStore
    @StateObject private var ble = BLEManager()
    @State private var savedSelectedID: UUID? = UserDefaults.standard.string(forKey: "selectedTesterDeviceID").flatMap(UUID.init)
    @State private var savedSelectedName: String? = UserDefaults.standard.string(forKey: "selectedTesterDeviceName")

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            HStack {
                Text("Valitse BLE-laite").font(.title2).bold()
                Spacer()
            }
            .padding(.horizontal)

            // Optional: Add basic filtering controls (name prefix / min RSSI) if desired.

            List {
                // Section for previously selected device
                if let selID = savedSelectedID, let selName = savedSelectedName {
                    Section("Valittu laite") {
                        HStack(spacing: 12) {
                            VStack(alignment: .leading, spacing: 2) {
                                Text(selName)
                                    .font(.body)
                                Text(selID.uuidString)
                                    .font(.caption2)
                                    .foregroundColor(.secondary)
                            }
                            Spacer()
                            // Live visibility indicator
                            let isVisible = ble.devices.contains(where: { $0.id == selID })
                            Text(isVisible ? "Yhdistettävissä" : "Ei näkyvissä juuri nyt")
                                .font(.caption)
                                .foregroundColor(isVisible ? .green : .gray)
                            Button {
                                // Remove selection
                                UserDefaults.standard.removeObject(forKey: "selectedTesterDeviceID")
                                UserDefaults.standard.removeObject(forKey: "selectedTesterDeviceName")
                                savedSelectedID = nil
                                savedSelectedName = nil
                            } label: {
                                Image(systemName: "trash")
                            }
                            .buttonStyle(.borderless)
                            .foregroundColor(.red)
                            .accessibilityLabel("Poista valinta")
                        }
                    }
                }

                // Live discovered devices list
                Section("Lähellä") {
                    ForEach(ble.devices) { device in
                        Button {
                            print("[BLE DEBUG] BLESetupView selecting device id=\(device.id.uuidString), name=\(device.name)")
                            ble.selectDevice(device)
                            savedSelectedID = device.id
                            savedSelectedName = device.name
                            print("[BLE DEBUG] BLESetupView saved selection ID=\(String(describing: savedSelectedID)), name=\(String(describing: savedSelectedName))")
                            ble.stop()
                            store.state.navigation = .start
                        } label: {
                            HStack(spacing: 12) {
                                VStack(alignment: .leading, spacing: 2) {
                                    let deviceName: String = {
                                        let name = device.name
                                        if name.isEmpty { return "<Tuntematon>" }
                                        return String(name)
                                    }()
                                    Text(deviceName)
                                        .font(.body)
                                    Text(device.id.uuidString)
                                        .font(.caption2)
                                        .foregroundColor(.secondary)
                                }
                                Spacer()
                                Text("RSSI \(device.rssi) dBm")
                                    .font(.caption)
                                    .foregroundColor(.secondary)
                                if device.id == savedSelectedID {
                                    Image(systemName: "checkmark.circle.fill")
                                        .foregroundColor(.green)
                                }
                            }
                            .padding(.vertical, 4)
                        }
                        .buttonStyle(.plain)
                        .listRowBackground(device.id == savedSelectedID ? Color.green.opacity(0.12) : Color.clear)
                    }
                }
            }
        }
        .background(Color.white)
        .onAppear {
            savedSelectedID = UserDefaults.standard.string(forKey: "selectedTesterDeviceID").flatMap(UUID.init)
            savedSelectedName = UserDefaults.standard.string(forKey: "selectedTesterDeviceName")
            print("[BLE DEBUG] BLESetupView onAppear read saved ID=\(String(describing: savedSelectedID)), name=\(String(describing: savedSelectedName))")
            ble.discoveryOnly = true
            ble.clearDiscoveredPeripherals()
            ble.start()
        }
        .onDisappear {
            ble.stop()
        }
        .toolbar {
            ToolbarItem(placement: .navigationBarTrailing) {
                Button("Valmis") {
                    store.state.navigation = .start
                }
            }
        }
        .navigationTitle("BLE Setup")
        .navigationBarTitleDisplayMode(.inline)
    }
}
