import Foundation

//
//  AppState.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//

enum Route {
    case start
    case settings
    case measure
    case results
}

struct AppState {
    var navigation: Route = .start
    var loadOldResults: Bool = false
    var order: [OrderItem] = []
    var results: [ResultModel] = []
    var settings: SettingsData? = nil
}

struct OrderItem {
    let name: String
    let round: Int
}

struct ResultModel {
    let name: String
    let round: Int
    let mag_avg: Double
    let time: Double
}

struct SettingsData {
    let pairs: Int
    let rounds: Int
    let names: [String]
    let temperature: Int
    let snowQuality: String
    let baseHardness: String
}
