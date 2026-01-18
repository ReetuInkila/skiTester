//
//  AppStore.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//

import Combine
import SwiftUI

@MainActor
final class AppStore: ObservableObject {

    @Published var state = AppState()
}
