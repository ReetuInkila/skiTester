# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository scope

This directory (`swiftApp/SkiTesterApp`) is the iOS SwiftUI client, one part of a larger `skiTester` monorepo (`/Users/reetu/Dev/skiTester`) that also contains:
- `skiTester/` — PlatformIO/Arduino firmware for the ESP32 measurement device (the BLE peripheral this app talks to)
- `circuit/` — KiCad PCB design files
- `3d_models/` — enclosure/mount models
- `reactNativeApp/` — an inactive alternate client (per the repo README, only the SwiftUI app is under active development)

The purpose of the whole project: a DIY ski glide tester. Two magnet gates measure elapsed time and average acceleration for a ski run; this app configures test runs, receives results from the ESP32 device over BLE, and exports them.

## Build, run, test

This is a standard Xcode project (`SkiTesterApp.xcodeproj`, not SPM) — the `.xcodeproj` itself is gitignored, so it exists only locally.

```bash
# Build for simulator
xcodebuild -project SkiTesterApp.xcodeproj -scheme SkiTesterApp \
  -destination 'platform=iOS Simulator,name=Any iOS Simulator Device' build

# Run all tests
xcodebuild -project SkiTesterApp.xcodeproj -scheme SkiTesterApp \
  -destination 'platform=iOS Simulator,name=Any iOS Simulator Device' test

# Run a single test
xcodebuild -project SkiTesterApp.xcodeproj -scheme SkiTesterApp \
  -destination 'platform=iOS Simulator,name=Any iOS Simulator Device' \
  -only-testing:SkiTesterAppTests/SkiTesterAppTests/testExample test
```

There is one scheme (`SkiTesterApp`) and two test targets: `SkiTesterAppTests` (unit) and `SkiTesterAppUITests` (UI). Both currently contain only scaffold/example tests — there is no real test coverage yet.

BLE features (device discovery, connect) require a physical device or a Mac target with Bluetooth; the iOS Simulator cannot exercise CoreBluetooth peripheral scanning.

## Architecture

**Single-store, enum-driven navigation.** There is no `NavigationLink`/multi-screen push stack. `AppStore` (`AppStore.swift`) is one `@MainActor ObservableObject` holding a single `AppState` (`AppState.swift`), injected app-wide via `.environmentObject`. `SkiTesterAppApp.swift` switches on `store.state.navigation: Route` (`.start`, `.settings`, `.bleSetup`, `.measure`, `.results`) to pick the root view. Screens change flow by mutating `store.state.navigation` directly — there's no router/coordinator object.

**Persistence is UserDefaults, not SwiftData** — despite `SwiftData` being imported in `SkiTesterAppApp.swift`, it's unused. `Storage.swift` JSON-encodes the whole `AppState` into `UserDefaults` under key `"app_state"`; `AppStore.init()` loads it synchronously at launch. Call `Storage.save(store.state)` after any state mutation that should survive relaunch (see `MeasurementView.handleMessage`).

**BLE transport layer (`BLEManager.swift`)** wraps CoreBluetooth as a UART-style link to the ESP32 firmware (NimBLE), matching `skiTester/src/ble_server.cpp`:
- Service `6E400001-...`, RX characteristic `6E400002-...` (phone→device writes), TX `6E400003-...` (device→phone notify).
- Device discovery filters peripherals by name containing `"SkiTester"`.
- Selected device UUID/name persist in `UserDefaults` (`selectedTesterDeviceID`/`selectedTesterDeviceName`) — separate from the `AppState` blob — and are read directly by `BLESetupView`, `StartView`, and `MeasurementView` rather than through `AppStore`.
- `BLEManager` is instantiated twice: once app-wide in `SkiTesterAppApp` (used by `MeasurementView` for real traffic) and once locally as a `@StateObject` inside `BLESetupView` (`discoveryOnly = true`, used purely for scanning/selecting — it never connects for data).
- Uses `CBCentralManagerOptionRestoreIdentifierKey` for state restoration; `willRestoreState` re-attaches the delegate so a backgrounded/relaunched app can resume an existing connection.
- Every inbound JSON message that carries an `id` gets an ACK written back on RX (`sendAck`) — the firmware expects this per-packet handshake.

**Message protocol** (JSON over the TX/RX characteristics, matched by `StatusCode` in `AppState.swift` and handled in `MeasurementView.handleMessage`): `status` field drives a switch — `idle`(0)/`start`(1)/`result`(2, carries `mag_avg` + `time`)/`error`(3, carries `message`)/`imuStatus`(4, carries `imu_cal`). A `result` message appends a `ResultModel`, advances to the next `OrderItem` in `store.state.order`, and auto-navigates to `.results` once the order is exhausted.

**Test run flow**: `SettingsView` builds `store.state.order: [OrderItem]` — a boustrophedon (snake) sequence alternating ski-pair direction each round so consecutive measurements share a pair — then navigates to `.measure`. Ski pairs can be entered manually or via `QRScannerView`, which decodes either a JSON payload (`{"count":Int,"names":[String]}`) or falls back to loose numeric parsing from arbitrary QR text.

**UI language is Finnish** (all user-facing strings, e.g. "Yhdistetään...", "Mittaus valmis") — keep new UI text consistent with this unless told otherwise.

## Conventions to know

- Views read/write `store.state` directly (no reducers/actions) — mutations happen inline in view code and button closures.
- `BLEManager` state changes happen off the delegate callbacks directly on `@Published` properties without explicit main-thread dispatch in most callbacks (CoreBluetooth delegate methods run on the queue passed at init, which is `.main`), except `MeasurementView.handleMessage`, which wraps result handling in `DispatchQueue.main.async` explicitly.
- `Info.plist` declares `UIBackgroundModes` for Bluetooth to support background BLE restoration — don't remove without understanding the restore-identifier flow above.
