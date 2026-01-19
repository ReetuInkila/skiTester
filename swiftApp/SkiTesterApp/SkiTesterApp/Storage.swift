//
//  Storage.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 19.1.2026.
//

import Foundation

enum Storage {
    private static let key = "app_state"

    static func save(_ state: AppState) {
        if let data = try? JSONEncoder().encode(state) {
            UserDefaults.standard.set(data, forKey: key)
        }
    }

    static func load() -> AppState? {
        guard
            let data = UserDefaults.standard.data(forKey: key),
            let state = try? JSONDecoder().decode(AppState.self, from: data)
        else { return nil }

        return state
    }

    static func clear() {
        UserDefaults.standard.removeObject(forKey: key)
    }
}

