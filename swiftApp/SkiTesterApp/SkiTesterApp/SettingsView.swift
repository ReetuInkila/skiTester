//
//  SettingsView.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 18.1.2026.
//


import SwiftUI

struct SettingsView: View {
    @EnvironmentObject var store: AppStore

    @State private var pairs: Int = 5
    @State private var rounds: Int = 5
    @State private var names: [String] = Array(repeating: "", count: 5)
    @State private var temperature: Int = 0
    @State private var snowQuality: String = ""
    @State private var baseHardness: String = ""
    @State private var isShowingScanner = false
    @State private var scanErrorMessage: String? = nil

    private var canStartMeasurement: Bool {
        // All names must be non-empty and represent a positive integer
        !names.isEmpty && names.allSatisfy { name in
            if let n = Int(name), n > 0 { return true }
            return false
        }
    }

    var body: some View {
        ScrollView {
            HStack(alignment: .top, spacing: 20) {

                // Vasen sarake
                VStack(alignment: .leading, spacing: 10) {

                    VStack(alignment: .leading) {
                        Text("Suksien lukumäärä")
                        TextField("", value: $pairs, format: .number)
                            .keyboardType(.numberPad)
                            .textFieldStyle(.roundedBorder)
                            .onChange(of: pairs) { newValue in
                                updateNames(newValue)
                            }
                    }

                    VStack(alignment: .leading) {
                        Text("Kierrosten lukumäärä")
                        TextField("", value: $rounds, format: .number)
                            .keyboardType(.numberPad)
                            .textFieldStyle(.roundedBorder)
                    }
                    VStack(alignment: .leading) {
                        Text("Lämpötila (°C)")
                        TextField("", value: $temperature, format: .number)
                            .keyboardType(.numbersAndPunctuation)
                            .textFieldStyle(.roundedBorder)
                    }
                    VStack(alignment: .leading) {
                        Text("Lumen laatu")
                        TextField("", text: $snowQuality)
                            .textFieldStyle(.roundedBorder)
                    }
                    VStack(alignment: .leading) {
                        Text("Pohjan kovuus")
                        TextField("", text: $baseHardness)
                            .textFieldStyle(.roundedBorder)
                    }
                }

                // Oikea sarake
                VStack(alignment: .leading, spacing: 10) {
                    Text("Sukset")
                        .font(.headline)

                    ForEach(names.indices, id: \.self) { index in
                        TextField("Pari \(index + 1)", text: $names[index])
                            .keyboardType(.numberPad)
                            .onChange(of: names[index]) { newValue in
                                // Strip non-digits
                                let filtered = newValue.filter { $0.isNumber }
                                // Remove leading zeros
                                let noLeadingZeros = filtered.drop { $0 == "0" }
                                let sanitized = noLeadingZeros.isEmpty ? "" : String(noLeadingZeros)
                                names[index] = sanitized
                            }
                            .textFieldStyle(.roundedBorder)
                    }
                }
            }
            Button("Aloita mittaus") {
                saveAndContinue()
            }
            .padding(.top, 16)
            .disabled(!canStartMeasurement)
            .opacity(canStartMeasurement ? 1 : 0.5)
            .padding()
        }
        .background(Color.white)
        .toolbar {
            ToolbarItem(placement: .navigationBarTrailing) {
                Button {
                    isShowingScanner = true
                } label: {
                    Image(systemName: "qrcode.viewfinder")
                        .font(.system(size: 22))
                }
                .accessibilityLabel("Skannaa QR-koodi")
            }
        }
        .sheet(isPresented: $isShowingScanner) {
            QRScannerView { result in
                switch result {
                case .success(let payload):
                    handleScannedPayload(payload)
                case .failure(let error):
                    scanErrorMessage = error.localizedDescription
                }
                isShowingScanner = false
            }
        }
        .alert("QR-virhe", isPresented: .constant(scanErrorMessage != nil), actions: {
            Button("OK") { scanErrorMessage = nil }
        }, message: {
            Text(scanErrorMessage ?? "Tuntematon virhe")
        })
    }

    // MARK: - Helpers

    private func updateNames(_ newValue: Int) {
        if newValue > names.count {
            names.append(contentsOf: Array(repeating: "", count: newValue - names.count))
        } else if newValue < names.count {
            names = Array(names.prefix(newValue))
        }
    }

    private func saveAndContinue() {
        store.state.results = []

        store.state.settings = SettingsData(
            pairs: pairs,
            rounds: rounds,
            names: names,
            temperature: temperature,
            snowQuality: snowQuality,
            baseHardness: baseHardness
        )

        store.state.order = buildOrder()
        store.state.navigation = .measure
    }

    private func buildOrder() -> [OrderItem] {
        var result: [OrderItem] = []

        for round in 1...rounds {
            let sequence: [Int] = round.isMultiple(of: 2)
                ? Array(names.indices.reversed())
                : Array(names.indices)

            for i in sequence {
                let nameString = names[i]
                let displayName: String
                if let n = Int(nameString), n > 0 {
                    displayName = String(n)
                } else {
                    displayName = "Pari \(i + 1)"
                }
                result.append(
                    OrderItem(
                        name: displayName,
                        round: round
                    )
                )
            }
        }
        return result
    }

    private func handleScannedPayload(_ payload: String) {
        if let parsed = parseSkisJSON(from: payload) {
            // Sanitize names like UI (digits only, no leading zeros, positive)
            let sanitized = parsed.names.map { raw -> String in
                let filtered = raw.filter { $0.isNumber }
                let noLeadingZeros = filtered.drop { $0 == "0" }
                let value = noLeadingZeros.isEmpty ? "" : String(noLeadingZeros)
                if let n = Int(value), n > 0 { return String(n) }
                return ""
            }.filter { !$0.isEmpty }
            guard !sanitized.isEmpty else { return }
            pairs = min(parsed.count, sanitized.count)
            updateNames(pairs)
            names = Array(sanitized.prefix(pairs))
            return
        }

        // Fallback to legacy loose number parsing
        let skis = parseSkis(from: payload)
        guard !skis.isEmpty else { return }
        pairs = skis.count
        updateNames(pairs)
        names = skis
    }

    // Accepts formats like "12, 34,56" or "[12,34,56]" or "12 34 56" or newline-separated. Non-digits are treated as separators.
    private func parseSkis(from payload: String) -> [String] {
        // Replace any non-digit with a space, then split
        let cleaned = payload.map { $0.isNumber ? $0 : " " }
        let parts = String(cleaned).split{ $0 == " " }
        // Remove leading zeros and empty parts, keep only positive integers
        let normalized: [String] = parts.compactMap { part in
            let noLeadingZeros = part.drop { $0 == "0" }
            let s = noLeadingZeros.isEmpty ? (part.contains("0") ? "0" : "") : String(noLeadingZeros)
            guard let n = Int(s), n > 0 else { return nil }
            return String(n)
        }
        return normalized
    }

    private func parseSkisJSON(from payload: String) -> (count: Int, names: [String])? {
        struct QRSkis: Decodable { let count: Int; let names: [String] }
        guard let data = payload.data(using: .utf8) else { return nil }
        do {
            let decoded = try JSONDecoder().decode(QRSkis.self, from: data)
            guard decoded.count > 0, !decoded.names.isEmpty else { return nil }
            return (decoded.count, decoded.names)
        } catch {
            return nil
        }
    }

}

