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

// For keeping track next ski and results
int* order = new int[pairs*rounds]; // Array of ski numbers sorted
int runIndex = 0; // index of next ski to test
TestRun* results = new TestRun[pairs]; // Array of each ski results


String error = ""; // Variable to store error message

AsyncWebServer server(80);

void setup() {
  order[0]=0;
    results[0].addRun(1, rounds);
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
          site= site+"<h3>Next ski: "+String(results[order[runIndex]].getSki())+" run: "+String(results[order[runIndex]].getNextRun())+"</h3>";
        }
        if(runIndex>0 && runIndex<pairs*rounds){
          site= site+"<table>\
            <tr><th>Time 1</th><th>Time 2</th><th>Total Time</th><th>Ratio (t2/t1)</th><th>Ski Number</th><th>Run</th></tr>\
            <tr><td>"+String(results[order[runIndex-1]].getLastT1()/1000, 3)+"</td><td>"+String(results[order[runIndex-1]].getLastT2()/1000, 3)+"</td><td>"+String(results[order[runIndex-1]].getLastT1()/1000+results[order[runIndex-1]].getLastT2()/1000, 3)+"</td><td>"+String((results[order[runIndex-1]].getLastT2()/results[order[runIndex-1]].getLastT1()), 3)+"</td><td>"+String(results[order[runIndex-1]].getSki())+"</td><td>"+String(results[order[runIndex-1]].getLastRun())+"</td></tr>\
            </table>";
        }else if(runIndex>0){
          site= site+"<table><tr><th>Ski Number</th><th>Time 1</th><th>Time 2</th><th>Total Time</th><th>Ratio (t2/t1)</th></tr>";
          for(int i=0;i<pairs;i++){
            site= site+"<tr><td>"+String(results[i].getSki())+"</td><td>"+String(results[i].avgT1()/1000, 3)+"</td><td>"+String(results[i].avgT2()/1000, 3)+"</td><td>"+String(results[i].avgT1()/1000+results[i].avgT2()/1000, 3)+"</td><td>"+String((results[i].avgT2()/results[i].avgT1()), 3)+"</td></tr>";
          }
          site= site+"</table>";
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
      pairs = request->getParam(PARAM_INPUT_1)->value().toInt();
      rounds = request->getParam(PARAM_INPUT_2)->value().toInt();
      if(pairs==0 || rounds==0){
        request->redirect("/settings");
      }

      runIndex = 0;
      TestRun* newResults = new TestRun[pairs];
      for (int i = 0; i < pairs; ++i) {
        newResults[i].addRun(i+1, rounds);
      }

      int * newOrder = new int[pairs * rounds];
      int t=0;
      for (int i = 1; i <= rounds; ++i) {
        if (!isEven(i)) {
          for (int j = 0; j < pairs; ++j) {
            newOrder[t]=j;
            t++;
          }
        } else {
          for (int j = pairs-1; j >= 0; --j) {
            newOrder[t]=j;
            t++;
          }
        }
      }
      delete[] results; 
      results = newResults;
      delete[] order;
      order = newOrder;

      Serial.println(runIndex);
      for(int i=0;i<pairs*rounds;i++){
        Serial.print(order[i]);
      }
      Serial.println("");
      
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
                
                if(runIndex<pairs*rounds){
                  results[order[runIndex]].addTimes(t1, t2);
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
