#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <Adafruit_BNO08x.h>

// Wi-Fi-verkon tiedot
const char *ssid = "Ski-Tester-1";
const char *password = "123456789";

// Laitteiston määrittely
const int buzzerPin = 13; // Summerin ohjaukseen käytettävä pinni
const int sensorPin = 22; // Anturin lukemiseen käytettävä pinni

// For IMU SPI mode
#define BNO08X_CS 25
#define BNO08X_INT 26
#define BNO08X_RESET 14

Adafruit_BNO08x  bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

// Anturien lukuarvot ja aikaleimat
unsigned long start_time = 0;
unsigned long end_time = 0;
unsigned long total = 0;

std::vector<float> mag_buf;
float mag_avg;


bool measuring = false;

// Virheilmoitusten ja viestien hallinta
String errorMessage = ""; // Virheilmoituksen säilytys
unsigned long messageId = 0; // Uniikki viestin ID

// Rakenne viestien hallintaan
struct Message {
  unsigned long id;
  String content;
};
std::vector<Message> messages;

// Web-palvelimen ja WebSocketin alustaminen
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Funktio määrittelyt
void buzz();
void readSensors();
void notifyClients(String message);
void removeMessageById(unsigned long id);
void setReports();
void mittaa();

void setup() {
  Serial.begin(115200);

  pinMode(sensorPin, INPUT); // Aseta anturipinni syötteeksi
  pinMode(buzzerPin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(sensorPin), mittaa, FALLING);

  buzz(); // Soita merkkiääni alustuksen merkiksi
  
  if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT)) {
    Serial.println("Failed to find BNO08x chip");
    while (1) { delay(10); }
  }
  Serial.println("BNO08x Found!");
  for (int n = 0; n < bno08x.prodIds.numEntries; n++) {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }
  setReports();

  // Wi-Fi-yhteyden luominen
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // WebSocket-tapahtumakäsittelijät
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("WebSocket client connected");

      // Lähetä kaikki tallennetut viestit uudelle asiakkaalle
      for (const auto &message : messages) {
        client->text(message.content);
      }
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.println("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = '\0'; // Null-terminointi
        String receivedMessage = String((char *)data);

        Serial.print("Received from client: ");
        Serial.println(receivedMessage);

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, receivedMessage);
        if (!error && doc.containsKey("id")) {
          unsigned long id = doc["id"].as<unsigned long>();
          removeMessageById(id);
        }
      }
    }
  });

  server.addHandler(&ws);
  server.begin(); // Käynnistä palvelin
}

void loop() {
    if (!errorMessage.isEmpty() && !measuring) {
        notifyClients(errorMessage);
        errorMessage = ""; // Tyhjennä virheilmoitus lähetyksen jälkeen
    }

    // Lähetä tulokset WebSocket-asiakkaille
    if (total != 0) {
      if (mag_buf.empty()) mag_avg = 0;
      else {
          float s = 0;
          for (float v : mag_buf) s += v;
          mag_avg = s / mag_buf.size();
      }
      notifyClients("");
      mag_buf.clear();
      total = 0;
    }

    if (measuring && end_time == 0) {
      if (bno08x.getSensorEvent(&sensorValue)) {
          if (sensorValue.sensorId == SH2_LINEAR_ACCELERATION) {
              float x = sensorValue.un.linearAcceleration.x;
              float y = sensorValue.un.linearAcceleration.y;
              float z = sensorValue.un.linearAcceleration.z;
              float m = sqrtf(x*x + y*y + z*z);
              mag_buf.push_back(m);
          }
      }
    
  }
}

void mittaa() {
  measuring = !measuring;
  if (measuring) {
    start_time = millis();
  }else {
    end_time = millis();
    total = end_time - start_time;
    start_time = 0;
    end_time = 0;
  }
}

// Here is where you define the sensor outputs you want to receive
void setReports(void) {
  Serial.println("Setting desired reports");
  if (! bno08x.enableReport(SH2_LINEAR_ACCELERATION)) {
    Serial.println("Could not enable game vector");
  }
}

// Lähettää viestin asiakkaalle ja tallentaa sen lähetettyjen listaan
void notifyClients(String message) {
    StaticJsonDocument<256> jsonDoc;

    if (!message.isEmpty()) {
        jsonDoc["error"] = message;
    } else {
        jsonDoc["time"] = total/1000.0; // Aika sekunteina
        jsonDoc["mag_avg"] = mag_avg;
    }

    jsonDoc["id"] = messageId;
    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);

    messages.push_back({messageId, jsonResponse});
    messageId++;
    Serial.print("Sending to clients: ");
    Serial.println(jsonResponse);

    ws.textAll(jsonResponse);
}

// Poistaa tietorakenteesta viestin halutulla id:llä
void removeMessageById(unsigned long id) {
    auto it = std::find_if(messages.begin(), messages.end(), [id](const Message &msg) {
        return msg.id == id;
    });

    if (it != messages.end()) {
        messages.erase(it);
        Serial.print("Removed message with ID: ");
        Serial.println(id);
    } else {
        Serial.print("Message with ID not found: ");
        Serial.println(id);
    }
}

// Soittaa summeria ja viivästyttää seuraavaa mittausta sekunnilla
void buzz() {
    tone(buzzerPin, 800, 500);
    delay(500);
    noTone(buzzerPin);
}