#include "config.h"
#include <Arduino.h>
#include <NimBLEDevice.h>
#include <algorithm>
#include <vector>
#include <ArduinoJson.h>

struct Message {
  unsigned long id;
  String content;
};

static std::vector<Message> messages;
static String errorMessage = "";
static unsigned long messageId = 0;

static const char* BLE_DEVICE_NAME = "SkiTester";
static const NimBLEUUID SERVICE_UUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E"); // Nordic UART style
static const NimBLEUUID TX_UUID     ("6E400003-B5A3-F393-E0A9-E50E24DCCA9E"); // Notify (device -> phone)
static const NimBLEUUID RX_UUID     ("6E400002-B5A3-F393-E0A9-E50E24DCCA9E"); // Write  (phone -> device)

static NimBLEServer* pServer = nullptr;
static NimBLECharacteristic* pTx = nullptr;
static NimBLECharacteristic* pRx = nullptr;

static volatile bool bleClientConnected = false;

static void removeMessageById(unsigned long id) {
  auto it = std::find_if(messages.begin(), messages.end(),
                         [id](const Message& msg) { return msg.id == id; });

  if (it != messages.end()) {
    messages.erase(it);
    Serial.print("Removed message with ID: ");
    Serial.println(id);
  } else {
    Serial.print("Message with ID not found: ");
    Serial.println(id);
  }
}

static void clearAllMessages() {
  noInterrupts();
  messages.clear();
  interrupts();
  messageId = 0;
  Serial.println("Cleared all messages");
}

static void pushBufferedMessagesToClient() {
  if (!bleClientConnected || pTx == nullptr) return;

  for (const auto& m : messages) {
    pTx->setValue(m.content.c_str());
    pTx->notify();
    delay(5);
  }
}

// RX write callback: phone sends {"id":123} to ACK, or {"cmd":"clear"} to clear.
class RxCallbacks : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* chr, NimBLEConnInfo& connInfo) override {
    std::string v = chr->getValue();
    if (v.empty()) return;

    String received((const char*)v.data(), v.size());
    Serial.print("Received from client: ");
    Serial.println(received);

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, received);
    if (err) return;

    if (doc.containsKey("cmd")) {
      const char* cmd = doc["cmd"];
      if (cmd && String(cmd) == "clear") {
        clearAllMessages();
      }
      return;
    }

    if (doc.containsKey("id")) {
      unsigned long id = doc["id"].as<unsigned long>();
      removeMessageById(id);
    }
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* s, NimBLEConnInfo& connInfo) override {
    bleClientConnected = true;
    Serial.println("BLE client connected");

    // Push buffered backlog
    pushBufferedMessagesToClient();
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
  StaticJsonDocument<256> doc;

  doc["id"] = messageId;
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

  noInterrupts();
  if (messages.size() >= MSG_LIMIT) {
    messages.erase(messages.begin());
  }
  messages.push_back({messageId++, json});
  interrupts();

  if (bleClientConnected && pTx != nullptr) {
    pTx->setValue(json.c_str());
    pTx->notify();
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
