#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "TestRun.h"
#include <ArduinoJson.h>

const char *ssid = "Ski-Tester-1";
const char *password = "123456789";

int buzzerPin = 12; // set the buzzer control digital IO pin
int sensor = 14;    // sensor pin
int val;            // numeric variable

unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;

float t1, t2;



String error = ""; // Variable to store error message

AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// put function declarations here:
void buzz();
bool isEven(int);
int sort(const void*, const void*);

void setup(){

  pinMode(sensor, INPUT); // set sensor pin as input
  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, 800, 2000);
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);


  server.on("/ski", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("GET /ski");

    // Create a JSON document with a sufficient capacity
    DynamicJsonDocument jsonDoc(1024);

    // Add pairs and rounds to the JSON document
    jsonDoc["t1"] = t1;
    jsonDoc["t2"] = t2;

     // Serialize the JSON document to a string
    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);

    // Set CORS headers
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    request->send(response);
  });

  // Start the server
  server.begin();
}

void loop()
{
  val = digitalRead(sensor); // Read the sensor
  if (val == 0)
  {
    time1 = millis();
    Serial.println("Time1");
    error = "Not all sensors read";
    buzz();
    while (millis() - time1 < 60000)
    {                            // Allow up to 60 seconds for each run
      val = digitalRead(sensor); // Read the sensor
      if (val == 0)
      {
        if (time2 == 0)
        {
          time2 = millis();
          Serial.println("Time2");
          buzz();
        }
        else
        {
          time3 = millis();
          Serial.println("Time3");
          t1 = (time2 - time1)/1000;
          t2 = (time3 - time2)/1000;

          time1 = 0;
          time2 = 0;
          time3 = 0;
          error = "";
          buzz();
          break;
        }
      }
    }
    Serial.println("Run ended!");
  }
}

void buzz()
{
  tone(buzzerPin, 800, 500);
  noTone(buzzerPin);
  delay(500);
}