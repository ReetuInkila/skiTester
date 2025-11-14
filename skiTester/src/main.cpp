#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <vector>

// Wi-Fi-verkon tiedot
const char *ssid = "Ski-Tester-1";
const char *password = "123456789";

// Laitteiston määrittely
const int buzzerPin = 13; // Summerin ohjaukseen käytettävä pinni
const int sensorPin = 14; // Anturin lukemiseen käytettävä pinni

// Anturien lukuarvot ja aikaleimat
unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;

float t1 = 0, t2 = 0;

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

void setup() {
  pinMode(sensorPin, INPUT); // Aseta anturipinni syötteeksi
  pinMode(buzzerPin, OUTPUT);

  tone(buzzerPin, 800, 2000); // Soita merkkiääni alustuksen merkiksi
  Serial.begin(115200);

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
    readSensors();

    if (!errorMessage.isEmpty()) {
        notifyClients(errorMessage);
        errorMessage = ""; // Tyhjennä virheilmoitus lähetyksen jälkeen
    }

    // Lähetä tulokset WebSocket-asiakkaille
    if (t1 != 0 && t2 != 0) {
        notifyClients("");
    }
}

void readSensors() {
    int sensorValue = digitalRead(sensorPin);
    if (sensorValue == 0) {
        time1 = millis();
        Serial.println("Time1 recorded");
        errorMessage = "Not all sensors read";
        buzz();

        while (millis() - time1 < 20000) { // Odota 20 sekuntia
            sensorValue = digitalRead(sensorPin);
            if (sensorValue == 0) {
                if (time2 == 0) {
                    time2 = millis();
                    Serial.println("Time2 recorded");
                    buzz();
                } else {
                    time3 = millis();
                    Serial.println("Time3 recorded");

                    // Laske ajat
                    t1 = (time2 - time1) / 1000.0;
                    t2 = (time3 - time2) / 1000.0;
                    errorMessage = "";
                    buzz();
                    break;
                }
            }
        }

        // Nollaa aikamuuttujat
        time1 = 0;
        time2 = 0;
        time3 = 0;
        Serial.println("Run ended");
    }
}

// Lähettää viestin asiakkaalle ja tallentaa sen lähetettyjen listaan
void notifyClients(String message) {
    DynamicJsonDocument jsonDoc(1024);

    if (!message.isEmpty()) {
        jsonDoc["error"] = message;
    } else {
        jsonDoc["t1"] = t1;
        jsonDoc["t2"] = t2;

        t1 = 0; // Nollaa arvot lähetyksen jälkeen
        t2 = 0;
    }

    jsonDoc["id"] = messageId;
    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);

    messages.push_back({messageId, jsonResponse});
    messageId++;

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