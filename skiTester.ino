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
int * order = new int[1];
TestRun * results = new TestRun[1];
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
        <h3 class=\"error\">"+error+"</h3>";

        if(runIndex<pairs*rounds){
          site= site+"<h3>Next ski:"+String(results[runIndex].getSki())+"run:"+String(results[runIndex].getRun())+"</h3>";
        }
        if(runIndex>0){
          site= site+"<table>\
            <tr><th>Time 1</th><th>Time 2</th><th>Total Time</th><th>Ratio (t2/t1)</th><th>Ski Number</th><th>Run</th></tr>\
            <tr><td>"+String(results[runIndex-1].getT1()/1000, 3)+"</td><td>"+String(results[runIndex-1].getT2()/1000, 3)+"</td><td>"+String(results[runIndex-1].getT1()/1000+results[runIndex-1].getT2()/1000, 3)+"</td><td>"+String((results[runIndex-1].getT2()/results[runIndex-1].getT1()), 3)+"</td><td>"+String(results[runIndex-1].getSki())+"</td><td>"+String(results[runIndex-1].getRun())+"</td></tr>\
            </table>";
        }

        site= site+"<form action=\"/settings\" method=\"get\"><input type=\"submit\" value=\"Settings\" /></form>\
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
          <label>Number of pairs<input type="number" min="1" max="20" name="pairs"></label><br>
          <label>Rounds<input type="number" min="1" max="10" name="rounds"></label><br>
          <input type="submit" value="Save">
          </form>
        </body></html>)rawliteral";

        request->send(200, "text/html", settings_html);
    });

    server.on("/save", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("POST /save");

    // Read the form data
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      for (int i = 0; i < pairs * rounds; i++) {
        Serial.print(order[i]);
      }
      pairs = request->getParam(PARAM_INPUT_1)->value().toInt();
      rounds = request->getParam(PARAM_INPUT_2)->value().toInt();
      if(pairs==0 || rounds==0){
        request->redirect("/settings");
      }
      Serial.println("");
      runIndex = 0;
      TestRun* newResults = new TestRun[pairs * rounds];
      int * newOrder = new int[pairs * rounds];
      int t=0;
      for (int i = 1; i <= rounds; ++i) {
        if (!isEven(i)) {
          for (int j = 1; j <= pairs; ++j) {
            Serial.print(j);
            newOrder[t]=j;
            newResults[t].addRun(j, i);
            t++;
          }
        } else {
          for (int j = pairs; j >= 1; --j) {
            Serial.print(j);
            newOrder[t]=j;
            newResults[t].addRun(j, i);
            t++;
          }
        }
      }
      Serial.println("");
      delete[] results; 
      results = newResults;
      delete[] order;
      order = newOrder;
      for (int i = 0; i < pairs * rounds; i++) {
        Serial.print(results[i].getSki());
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
                Serial.print(runIndex);
                Serial.print(" ");
                Serial.println(pairs*rounds);
                if(runIndex<pairs*rounds){
                  results[runIndex].addTimes(t1, t2);
                  runIndex++;
                }
                
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

// Returns true if n is 
// even, else odd 
bool isEven(int n) { return (n % 2 == 0); } 
