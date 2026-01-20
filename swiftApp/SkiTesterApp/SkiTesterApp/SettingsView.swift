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
            .padding()

            Button("Aloita mittaus") {
                saveAndContinue()
            }
            .padding()
        }
        .background(Color.white)
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

}

