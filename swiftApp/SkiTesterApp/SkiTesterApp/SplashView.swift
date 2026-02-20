//
//  SplashView.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 20.2.2026.
//

import SwiftUI

struct SplashView: View {
    var body: some View {
        VStack {
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
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color.white)
        .ignoresSafeArea()
    }
}
