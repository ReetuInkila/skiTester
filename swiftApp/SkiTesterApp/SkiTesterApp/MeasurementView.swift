//
//  HomeView.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//

import SwiftUI

struct MeasurementView: View {
    @EnvironmentObject var store: AppStore

    @State private var serverState: String = "Yhdistetään..."
    @State private var socket: URLSessionWebSocketTask?
    @State private var index: Int = 0
    
    
    var body: some View {
        VStack {

            // Status
            Text(
                "Seuraavaksi: \(currentOrder?.name ?? "N/A") kierros: \(currentOrder?.round ?? 0)"
            )
            .font(.headline)
            .padding()

            // Table header
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

            // Results list
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

            // Footer
            Text("Yhteyden tila: \(serverState)")
                .frame(maxWidth: .infinity)
                .padding()
                .background(Color.gray.opacity(0.15))
        }
        .onAppear {
            connect()
            UIApplication.shared.isIdleTimerDisabled = true
        }
        .onDisappear {
            socket?.cancel()
            UIApplication.shared.isIdleTimerDisabled = false
        }
    }

    // MARK: - Helpers

    private var currentOrder: OrderItem? {
        guard index < store.state.order.count else { return nil }
        return store.state.order[index]
    }

    private func connect() {
        guard socket == nil else { return }

        let url = URL(string: "ws://192.168.4.1/ws")!
        let session = URLSession(configuration: .default)
        socket = session.webSocketTask(with: url)
        socket?.resume()

        serverState = "Yhdistetty"
        listen()
    }

    private func listen() {
        socket?.receive { result in
            switch result {
            case .failure:
                serverState = "Ei yhteyttä."
                socket = nil

            case .success(let message):
                if case let .string(text) = message {
                    handleMessage(text)
                }
                listen()
            }
        }
    }

    private func handleMessage(_ text: String) {
        guard
            let data = text.data(using: .utf8),
            let json = try? JSONSerialization.jsonObject(with: data) as? [String: Any],
            let statusRaw = json["status"] as? Int,
            let status = StatusCode(rawValue: statusRaw)
        else { return }

        // ACK
        if let id = json["id"] {
            let ack: [String: Any] = ["id": id]
            if let ackData = try? JSONSerialization.data(withJSONObject: ack),
               let ackString = String(data: ackData, encoding: .utf8) {
                socket?.send(.string(ackString)) { _ in }
            }
        }

        DispatchQueue.main.async {
            switch status {

            case .idle:
                serverState = "Valmiustila"

            case .start:
                serverState = "Mittaus käynnissä"

            case .result:
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

            case .imuStatus:
                let cal = json["imu_cal"] as? Int ?? 0
                serverState = "IMU kalibraatio: \(cal)"
            }
        }
    }

}
