//
//  InfoView.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//

import SwiftUI

struct InfoView: View {
    @Binding var isPresented: Bool

    var body: some View {
        VStack {
            Text("Info")
            Button("Sulje") {
                isPresented = false
            }
        }
        .padding()
    }
}
