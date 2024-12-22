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

AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Function declarations
void buzz();
void readSensors();
void notifyClients();

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
    }
  });

  server.addHandler(&ws);

  // Start the server
  server.begin();
}

void loop() {
  readSensors();

  // Notify clients if t1 and t2 are updated
  if (t1 != 0 && t2 != 0) {
    notifyClients();
  }
}

void readSensors() {
  static unsigned long startTime = 0;
  static int step = 0; // Vaiheiden hallinta

  val = digitalRead(sensor);
  if (val == 0) {
    if (step == 0) {
      time1 = millis();
      startTime = millis();
      Serial.println("Time1");
      error = "Not all sensors read";
      buzz();
      step = 1;
    } else if (step == 1 && millis() - startTime < 20000) {
      if (digitalRead(sensor) == 0 && time2 == 0) {
        time2 = millis();
        Serial.println("Time2");
        buzz();
        step = 2;
      }
    } else if (step == 2 && millis() - startTime < 20000) {
      if (digitalRead(sensor) == 0 && time3 == 0) {
        time3 = millis();
        Serial.println("Time3");
        t1 = (time2 - time1) / 1000.0;
        t2 = (time3 - time2) / 1000.0;

        // Tulosta tulokset
        Serial.print("T1: "); Serial.println(t1, 3);
        Serial.print("T2: "); Serial.println(t2, 3);

        // Nollaa muuttujat seuraavaa kierrosta varten
        time1 = 0;
        time2 = 0;
        time3 = 0;
        error = "";
        buzz();
        step = 0;
      }
    }
  } else if (millis() - startTime >= 20000) {
    Serial.println("Run ended with timeout!");
    step = 0; // Reset for the next run
  }
}

// Function to send t1 and t2 to all connected WebSocket clients
void notifyClients() {
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["t1"] = t1;
  jsonDoc["t2"] = t2;

  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  // Broadcast the JSON data to all connected WebSocket clients
  ws.textAll(jsonResponse);

  // Reset t1 and t2 after notifying clients
  t1 = 0;
  t2 = 0;
}

void buzz() {
  tone(buzzerPin, 800, 500);
  noTone(buzzerPin);
  delay(500);
}
