//
//  StartView.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//
import SwiftUI
import Combine

struct StartView: View {
    @Binding var isInfoVisible: Bool
    @EnvironmentObject var store: AppStore

    @State private var hasSavedDevice: Bool = UserDefaults.standard.string(forKey: "selectedTesterDeviceID") != nil
    private var selectedDeviceName: String { UserDefaults.standard.string(forKey: "selectedTesterDeviceName") ?? "" }

    var oldResults: Bool {
            !store.state.order.isEmpty
    }

    var body: some View {
        VStack(spacing: 20) {
            Spacer()
            Image(systemName: "snowflake")
                .resizable()
                .scaledToFit()
                .frame(width: 120, height: 120)
                .foregroundColor(.accentColor)
            Text("SkiTest")
                .font(.largeTitle).bold()
                .foregroundColor(.primary)
            Spacer()
            Button {
                store.state.navigation = .settings
            } label: {
                Text("Uusi mittaus")
                    .foregroundColor(.primary)
            }

            if oldResults {
                Button {
                    store.state.loadOldResults = true
                    store.state.navigation = .measure
                } label: {
                    Text("Jatka edellistä")
                        .foregroundColor(.primary)
                }
            }
            Spacer()
        }
        .toolbar {
            ToolbarItem(placement: .navigationBarTrailing) {
                Button {
                    store.state.navigation = .bleSetup
                } label: {
                    Image(systemName: "antenna.radiowaves.left.and.right")
                        .font(.system(size: 22))
                        .foregroundColor(.primary)
                }
            }
            ToolbarItem(placement: .navigationBarTrailing) {
                Button {
                    isInfoVisible = true
                } label: {
                    Image(systemName: "info.circle")
                        .font(.system(size: 22))
                        .foregroundColor(.primary)
                }
            }
        }
        .padding()
        .background(Color.white)
        .onAppear {
            hasSavedDevice = UserDefaults.standard.string(forKey: "selectedTesterDeviceID") != nil
        }
    }
}
