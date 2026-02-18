//
//  HomeView.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//

import SwiftUI

import UIKit

//
//  MeasurementView.swift
//  SkiTesterApp
//
//  WebSocket removed. Uses BLEManager.
//

import SwiftUI
import UIKit

struct MeasurementView: View {
    @EnvironmentObject var store: AppStore

    @StateObject private var ble = BLEManager()
    @State private var serverState: String = "Yhdistetään..."
    @State private var index: Int = 0

    private let generator = UINotificationFeedbackGenerator()

    var body: some View {
        VStack {

            VStack(alignment: .leading, spacing: 4) {
                Text("Seuraavaksi: \(currentOrder?.name ?? "N/A") kierros: \(currentOrder?.round ?? 0)")
                Text("Kierros \(currentRoundIndex) / \(totalRounds)")
                    .font(.subheadline)
                    .foregroundColor(.secondary)
            }
            .font(.headline)
            .padding()

            HStack {
                Text("Pari").bold()
                Spacer()
                Text("Kierros").bold()
                Spacer()
                Text("Kiihtyvyys").bold()
                Spacer()
                Text("Aika").bold()
            }
            .padding(.horizontal)

            ScrollView {
                ForEach(store.state.results.reversed().indices, id: \.self) { i in
                    let r = store.state.results.reversed()[i]
                    HStack {
                        Text(r.name)
                        Spacer()
                        Text("\(r.round)")
                        Spacer()
                        Text(r.mag_avg.formatted(.number.precision(.fractionLength(3))))
                        Spacer()
                        Text(r.time.formatted(.number.precision(.fractionLength(3))))
                    }
                    .padding(.horizontal)
                    .padding(.vertical, 4)
                }
            }

            Text("Yhteyden tila: \(serverState)")
                .frame(maxWidth: .infinity)
                .padding()
                .background(Color.gray.opacity(0.15))
        }
        .onAppear {
            UIApplication.shared.isIdleTimerDisabled = true
            generator.prepare()

            ble.onTextMessage = { text in
                handleMessage(text)
            }
            ble.start()
        }
        .onDisappear {
            ble.stop()
            UIApplication.shared.isIdleTimerDisabled = false
        }
        .onReceive(ble.$stateText) { t in
            serverState = t
        }
    }

    private var currentOrder: OrderItem? {
        guard index < store.state.order.count else { return nil }
        return store.state.order[index]
    }

    private var totalRounds: Int { store.state.order.count }
    private var currentRoundIndex: Int { min(index + 1, max(totalRounds, 1)) }

    private func handleMessage(_ text: String) {
        guard
            let data = text.data(using: .utf8),
            let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
            let statusRaw = json["status"] as? Int,
            let status = StatusCode(rawValue: statusRaw)
        else { return }

        // ACK via BLE write to RX
        if let id = json["id"] {
            ble.sendAck(id: id)
        }

        DispatchQueue.main.async {
            switch status {

            case .idle:
                serverState = "Valmiustila"
                generator.notificationOccurred(.warning)

            case .start:
                serverState = "Mittaus käynnissä"
                generator.notificationOccurred(.error)

            case .result:
                generator.notificationOccurred(.error)
                guard index < store.state.order.count else { return }

                let orderItem = store.state.order[index]
                let mag = json["mag_avg"] as? Double ?? 0
                let time = json["time"] as? Double ?? 0

                let result = ResultModel(
                    name: orderItem.name,
                    round: orderItem.round,
                    mag_avg: mag,
                    time: time
                )

                store.state.results.append(result)
                index += 1
                Storage.save(store.state)

                serverState = "Mittaus valmis"

                if index == store.state.order.count {
                    index = 0
                    store.state.navigation = .results
                }

            case .error:
                let msg = json["message"] as? String ?? "Tuntematon virhe"
                serverState = "Virhe: \(msg)"
                generator.notificationOccurred(.error)

            case .imuStatus:
                let cal = json["imu_cal"] as? Int ?? 0
                serverState = "IMU kalibraatio: \(cal)"
                generator.notificationOccurred(.warning)
            }
        }
    }
}
