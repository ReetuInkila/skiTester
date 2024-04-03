#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "TestRun.h"

const char* ssid     = "Ski-Tester-1";
const char* password = "123456789";

int sensor = 12; //sensor pin
int val; //numeric variable

unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;

const char* PARAM_INPUT_1 = "pairs";
const char* PARAM_INPUT_2 = "rounds";

int pairs = 1;
int rounds = 1;
int runIndex = 0;
TestRun results[] = {TestRun(1,1)};
String error = "";

AsyncWebServer server(80);

void setup() {
    pinMode(sensor, INPUT); //set sensor pin as input
    Serial.begin(115200);

    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  

  // Define a route to serve the HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("GET /");        // for debugging
        String site ="\
        <html>\
        <head>\
        <style>\
        .error {color:red;}table { width: 100%;}th, td { padding: 8px; text-align: left; border: 2px solid #ddd; }th, td { padding: 8px; text-align: left; border: 2px solid #ddd; }\
        </style>\
        </head>\
        <body>\
        <form action=\"/\" method=\"get\"><input type=\"submit\" value=\"Refresh\" /></form>\
        <h3 class=\"error\">"+error+"</h3>\
        <table>\
        <tr><th>Time 1</th><th>Time 2</th><th>Ratio (t2/t1)</th><th>Ski Number</th></tr>\
        <tr><td>"+String(results[runIndex].getT1()/1000, 3)+"</td><td>"+String(results[runIndex].getT2()/1000, 3)+"</td><td>"+String((results[runIndex].getT2()/results[runIndex].getT1()), 3)+"</td><form action=\"/\" method=\"post\"><td><input type=\"text\" name=\"ski_number\"><button type=\"submit\">Send</button></td></form></tr>\
        </table>\
        </body>\
        </html>";

        request->send(200, "text/html", site);
    });


    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
        Serial.println("GET /settings");        // for debugging
        const char settings_html[] PROGMEM = R"rawliteral(
        <!DOCTYPE HTML><html><head>
          <title>Ski tester</title>
          <meta name="viewport" content="width=device-width, initial-scale=1">
          <style>
            table { width: 100%;}th, td { padding: 8px; text-align: left; border: 2px solid #ddd; }th, td { padding: 8px; text-align: left; border: 2px solid #ddd; }\
          </style>
          </head><body>
          <form action="/save">
          <label>Number of pairs<input type="text" name="pairs"></label><br>
          <label>Rounds<input type="text" name="rounds"></label><br>
          <input type="submit" value="Save">
          </form>
        </body></html>)rawliteral";

        request->send(200, "text/html", settings_html);
    });

    server.on("/save", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("POST /save");

    // Read the form data
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      int pairs = request->getParam(PARAM_INPUT_1)->value().toInt();
      int rounds = request->getParam(PARAM_INPUT_2)->value().toInt();
      if(pairs==0 || rounds==0){
        request->redirect("/settings");
      }
      request->redirect("/");
    } else {
      request->redirect("/settings");
    }
  });

  // Start the server
  server.begin();
}

void loop() {
  val = digitalRead(sensor); //Read the sensor
  if (val==0) {
    time1 = millis();
    Serial.println("Time1");
    error="Not all sensors read";
    delay(1000);
    while (millis() - time1 < 60000) { // Allow up to 60 seconds for each run
        val = digitalRead(sensor); //Read the sensor
        if (val==0) {
            if(time2==0){
                time2=millis();
                Serial.println("Time2");
                delay(1000);
            }else{
                time3=millis();
                Serial.println("Time3");
                float t1 = (time2 - time1);
                float t2 = (time3 - time2);
                results[runIndex].addTimes(t1, t2);
                time1 = 0; 
                time2 = 0;
                time3 = 0;
                error="";
                delay(1000);
                break;
            }
        }
    }
    Serial.println("Run ended!");
  }
}
