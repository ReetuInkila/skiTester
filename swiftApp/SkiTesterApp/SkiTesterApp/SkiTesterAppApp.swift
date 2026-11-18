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
    @State private var splashVisible = true

    @ViewBuilder
    private var rootView: some View {
        switch store.state.navigation {
        case .start:
            StartView(isInfoVisible: $isInfoVisible)
        case .settings:
            SettingsView()
        case .measure:
            MeasurementView()
        case .results:
            ResultsView()
        default:
            StartView(isInfoVisible: $isInfoVisible)
        }
    }

    var body: some Scene {
        WindowGroup {
            ZStack {
                if splashVisible {
                    SplashView()
                        .preferredColorScheme(.light)
                } else {
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
            .onAppear {
                if splashVisible {
                    DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
                        splashVisible = false
                    }
                }
                store.state.navigation = .start
            }
        }
    }
}
