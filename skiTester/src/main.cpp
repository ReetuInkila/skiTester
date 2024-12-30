#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char *ssid = "Ski-Tester-1";
const char *password = "123456789";

int buzzerPin = 12; // set the buzzer control digital IO pin
int sensor = 14;    // sensor pin
int val;            // numeric variable

unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;

float t1 = 0, t2 = 0;

String error = ""; // Variable to store error message
unsigned long messageId = 0; // Unique message ID

AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Function declarations
void buzz();
void readSensors();
void notifyClients(String message);

void setup() {
  pinMode(sensor, INPUT); // set sensor pin as input
  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, 800, 2000);
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)...");
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  // Attach WebSocket to the server
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
      Serial.println("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
      // Kun dataa saapuu, tulosta se
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        // Varmista, että viesti on kokonainen ja tekstimuotoinen
        data[len] = '\0'; // Lisää null-merkki datan loppuun
        Serial.print("Received from client: ");
        Serial.println((char *)data);
      }
  }
  });

  server.addHandler(&ws);

  // Start the server
  server.begin();
}

void loop() {
  readSensors();

  if(error != ""){
    notifyClients(error);
    error = "";
  }

  // Notify clients if t1 and t2 are updated
  if (t1 != 0 && t2 != 0) {
    notifyClients("");
  }
}

// Function to read sensors and update t1 and t2
void readSensors() {
  val = digitalRead(sensor); // Read the sensor
  if (val == 0) {
    time1 = millis();
    Serial.println("Time1");
    error = "Not all sensors read";
    buzz();
    while (millis() - time1 < 20000) { // Allow up to 20 seconds for each run
      val = digitalRead(sensor); // Read the sensor
      if (val == 0) {
        if (time2 == 0) {
          time2 = millis();
          Serial.println("Time2");
          buzz();
        } else {
          time3 = millis();
          Serial.println("Time3");
          t1 = (time2 - time1) / 1000.0;
          t2 = (time3 - time2) / 1000.0;
          error = "";
          buzz();
          break;
        }
      }
    }
    time1 = 0;
    time2 = 0;
    time3 = 0;
    Serial.println("Run ended!");
  }
}

// Function to send t1 and t2 to all connected WebSocket clients
void notifyClients(String message) {

  DynamicJsonDocument jsonDoc(1024);

  if (message != "") {
    // Lähetä virheilmoitus WebSocketin kautta
    jsonDoc["error"] = message;
  } else {
    // Lähetä normaalit tulokset
    jsonDoc["t1"] = t1;
    jsonDoc["t2"] = t2;

    // Reset t1 and t2 after notifying clients
    t1 = 0;
    t2 = 0;
  }

  jsonDoc["id"] = messageId++; // Add unique ID to message

  // Serialize the JSON and send it to all WebSocket clients
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);
  ws.textAll(jsonResponse);
}

void buzz() {
  tone(buzzerPin, 800, 500);
  noTone(buzzerPin);
  delay(500);
}