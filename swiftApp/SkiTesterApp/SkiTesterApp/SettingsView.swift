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
                    labeledField(
                        title: "Suksien lukumäärä",
                        value: $pairs,
                        onChange: updateNames
                    )

                    labeledField(
                        title: "Kierrosten lukumäärä",
                        value: $rounds
                    )
                    labeledField(
                        title: "Lämpötila (°C)",
                        value: $temperature
                    )
                    textField("Lumen laatu", text: $snowQuality)
                    textField("Pohjan kovuus", text: $baseHardness)
                }

                // Oikea sarake
                VStack(alignment: .leading, spacing: 10) {
                    Text("Sukset")
                        .font(.headline)

                    ForEach(names.indices, id: \.self) { index in
                        TextField("Pari \(index + 1)", text: $names[index])
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
                result.append(
                    OrderItem(
                        name: names[i].isEmpty ? "Pari \(i + 1)" : names[i],
                        round: round
                    )
                )
            }
        }
        return result
    }


    private func labeledField(
        title: String,
        value: Binding<Int>,
        onChange: ((Int) -> Void)? = nil
    ) -> some View {
        VStack(alignment: .leading) {
            Text(title)
            TextField("", value: value, format: .number)
                .keyboardType(.numberPad)
                .textFieldStyle(.roundedBorder)
                .onChange(of: value.wrappedValue) { newValue in
                    onChange?(newValue)
                }
        }
    }

    private func textField(_ title: String, text: Binding<String>) -> some View {
        VStack(alignment: .leading) {
            Text(title)
            TextField("", text: text)
                .textFieldStyle(.roundedBorder)
        }
    }
}

