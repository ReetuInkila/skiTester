//
//  SkiTesterAppApp.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//

import SwiftUI
import SwiftData

@main
struct SkiTesterAppApp: App {
    @StateObject private var store = AppStore()
    @StateObject private var ble = BLEManager()
    @State private var isInfoVisible = false

    @ViewBuilder
    private var rootView: some View {
        switch store.state.navigation {
        case .start:
            StartView(isInfoVisible: $isInfoVisible)
        case .settings:
            SettingsView()
        case .bleSetup:
            BLESetupView()
        case .measure:
            MeasurementView()
        case .results:
            ResultsView()
        }
    }

    var body: some Scene {
        WindowGroup {
            NavigationStack {
                rootView
                    .toolbar {
                        if store.state.navigation != .start {
                            ToolbarItem(placement: .navigationBarLeading) {
                                Button("Alkuun") {
                                    store.state.navigation = .start
                                }
                            }
                        }
                    }
            }
            .environmentObject(store)
            .environmentObject(ble)
            .preferredColorScheme(.light)
            .sheet(isPresented: $isInfoVisible) {
                InfoView(isPresented: $isInfoVisible)
            }
        }
    }
}
