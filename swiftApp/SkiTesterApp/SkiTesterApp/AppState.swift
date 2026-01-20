import Foundation

//
//  AppState.swift
//  SkiTesterApp
//
//  Created by Reetu Inkilä on 12.1.2026.
//

enum Route: Codable {
    case start
    case settings
    case measure
    case results
}

enum StatusCode: Int {
    case idle       = 0
    case start      = 1
    case result     = 2
    case error      = 3
    case imuStatus  = 4
}

struct AppState: Codable {
    var navigation: Route = .start
    var loadOldResults: Bool = false
    var order: [OrderItem] = []
    var results: [ResultModel] = []
    var settings: SettingsData? = nil
}

struct OrderItem: Codable {
    let name: String
    let round: Int
}

struct ResultModel: Codable {
    let name: String
    let round: Int
    let mag_avg: Double
    let time: Double
}

struct SettingsData: Codable {
    let pairs: Int
    let rounds: Int
    let names: [String]
    let temperature: Int
    let snowQuality: String
    let baseHardness: String
}
