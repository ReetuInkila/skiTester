#include "config.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <algorithm>
#include <vector>
#include <ArduinoJson.h>

static String errorMessage = "";

static const char* BLE_DEVICE_NAME = "SkiTester-1";
static const NimBLEUUID SERVICE_UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // Nordic UART style
static const NimBLEUUID TX_UUID     ("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"); // Notify (device -> phone)
static const NimBLEUUID RX_UUID     ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // Write  (phone -> device)

static NimBLEServer* pServer = nullptr;
static NimBLECharacteristic* pTx = nullptr;
static NimBLECharacteristic* pRx = nullptr;

static volatile bool bleClientConnected = false;

// RX write callback: reserved for future use.
class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* chr, NimBLEConnInfo& connInfo) override {
    std::string v = chr->getValue();
    if (v.empty()) return;

    String received((const char*)v.data(), v.size());
    Serial.print("Received from client: ");
    Serial.println(received);
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* s, NimBLEConnInfo& connInfo) override {
    bleClientConnected = true;
    Serial.println("BLE client connected");
  }

  void onDisconnect(NimBLEServer* s, NimBLEConnInfo& connInfo, int reason) override {
    bleClientConnected = false;
    Serial.print("BLE client disconnected, reason: ");
    Serial.println(reason);

    // Keep advertising so the phone can reconnect
    NimBLEDevice::startAdvertising();
  }
};

// Public API: same signature as before.
void notifyClients(StatusCode status,
                   float mag_avg,
                   float total,
                   const String& message) {
  Serial.print("NotifyClients called with status: ");
  Serial.println(status);
  
  StaticJsonDocument<256> doc;

  doc["status"] = status;

  if (status == STATUS_RESULT) {
    doc["time"] = total;
    doc["mag_avg"] = mag_avg;
  }

  if (status == STATUS_ERROR || !message.isEmpty()) {
    doc["message"] = message;
  }

  String json;
  serializeJson(doc, json);

  if (bleClientConnected && pTx != nullptr) {
    Serial.print("Sending notification: ");
    Serial.println(json);
    pTx->setValue(json.c_str());
    pTx->notify();
  } else {
    Serial.println("Client not connected, message not sent");
  }
}

// Call this once from setup()
void initBLE() {
  Serial.println("Starting BLE...");
  NimBLEDevice::init("SkiTester");
  NimBLEDevice::setPower(ESP_PWR_LVL_P9); // max TX power
  NimBLEDevice::setSecurityAuth(false, false, false);

  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* svc = pServer->createService(SERVICE_UUID);

  pTx = svc->createCharacteristic(
      TX_UUID,
      NIMBLE_PROPERTY::NOTIFY
  );

  pRx = svc->createCharacteristic(
      RX_UUID,
      NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
  );
  pRx->setCallbacks(new RxCallbacks());

  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();

  NimBLEAdvertisementData ad;
  ad.setName("SkiTester");
  ad.addServiceUUID(SERVICE_UUID);
  adv->setAdvertisementData(ad);

  adv->setScanResponseData(NimBLEAdvertisementData());
  adv->start();



  Serial.println("BLE started, advertising");
}
