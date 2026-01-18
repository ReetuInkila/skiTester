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
}
    }

    @StateObject private var store = AppStore()
    @State private var isInfoVisible = false

    var body: some Scene {
        WindowGroup {
            NavigationStack {
                rootView
            }
            .environmentObject(store)
            .preferredColorScheme(.light)
            .sheet(isPresented: $isInfoVisible) {
                InfoView(isPresented: $isInfoVisible)
            }
        }
    }
}

