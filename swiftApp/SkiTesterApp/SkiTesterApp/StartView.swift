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

    var oldResults: Bool {
            !store.state.results.isEmpty
    }

    var body: some View {
        VStack(spacing: 20) {

            Image("logo")
                .resizable()
                .scaledToFit()
                .frame(maxWidth: .infinity)
                .aspectRatio(1, contentMode: .fit)

            Button {
                store.state.navigation = .settings
            } label: {
                Text("Uusi mittaus")
            }

            if oldResults {
                Button {
                    store.state.loadOldResults = true
                    store.state.navigation = .measure
                } label: {
                    Text("Jatka edellistä")
                }
            }
            Spacer()
        }
        .toolbar {
                    ToolbarItem(placement: .navigationBarTrailing) {
                        Button {
                            isInfoVisible = true
                        } label: {
                            Image(systemName: "info.circle")
                                .font(.system(size: 22))
                        }
                    }
                }
        .padding()
        .background(Color.white)
    }
}

