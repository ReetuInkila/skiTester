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
import Combine

struct MeasurementView: View {
    @EnvironmentObject var store: AppStore

    @EnvironmentObject private var ble: BLEManager
    @State private var serverState: String = "Yhdistetään..."
    @State private var index: Int = 0
    @State private var showRetry: Bool = false
    @State private var disconnectedSince: Date? = nil

    private let generator = UINotificationFeedbackGenerator()

    var body: some View {
        ZStack {
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
            }
        }
        .toolbar {
            ToolbarItem(placement: .bottomBar) {
                HStack(spacing: 12) {
                    HStack(spacing: 8) {
                        Circle()
                            .fill(ble.isConnected ? Color.green : Color.red)
                            .frame(width: 8, height: 8)
                        Text(serverState)
                            .font(.footnote)
                            .foregroundColor(.primary)
                    }
                    Spacer()
                    if showRetry {
                        Button(action: {
                            ble.start()
                            showRetry = false
                            disconnectedSince = Date()
                        }) {
                            HStack(spacing: 6) {
                                Image(systemName: "arrow.clockwise")
                                Text("Yritä uudelleen")
                            }
                            .font(.footnote.weight(.semibold))
                            .padding(.horizontal, 12)
                            .padding(.vertical, 6)
                            .background(Color.accentColor.opacity(0.15))
                            .clipShape(Capsule())
                        }
                        .buttonStyle(.plain)
                    }
                }
            }
        }
        .onAppear {
            UIApplication.shared.isIdleTimerDisabled = true
            generator.prepare()

            ble.onTextMessage = { text in
                handleMessage(text)
            }
            // Do not start BLE here; it should be started at app launch so connection persists across views.

            // Track disconnection duration and show retry after timeout
            if !ble.isConnected { disconnectedSince = Date() }
        }
        .onDisappear {
            // Do not stop BLE here; keep connection persistent across the app lifecycle.
            UIApplication.shared.isIdleTimerDisabled = false
        }
        .onReceive(ble.$stateText) { t in
            serverState = t
        }
        .onReceive(Timer.publish(every: 1, on: .main, in: .common).autoconnect()) { _ in
            if ble.isConnected {
                showRetry = false
                disconnectedSince = nil
            } else {
                if disconnectedSince == nil { disconnectedSince = Date() }
                if let since = disconnectedSince, Date().timeIntervalSince(since) > 5 { // 5s timeout
                    showRetry = true
                }
            }
        }
        .onReceive(ble.$isConnected) { connected in
            if connected {
                showRetry = false
                disconnectedSince = nil
            } else {
                disconnectedSince = Date()
            }
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

