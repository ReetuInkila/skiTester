#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "Ski-Tester-1";
const char* password = "123456789";

unsigned long time1;
unsigned long time2;
unsigned long time3;

int sensor = 12; //sensor pin
int val; //numeric variable

// Set web server port number to 80
WiFiServer server(80);

int baseValue;
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
  
  server.begin();
}

void lue() {
  val = digitalRead(sensor); //Read the sensor
  if (val==0) {
    Serial.println("Magnet detected!");
    time1 = time2;
    time2 = time3;
    time3 = millis();
    delay(1000);
  }
}

void loop() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {
    float t1 = (time2 - time1);
    float t2 = (time3 - time2);
    float ratio = t2 / t1;

    Serial.println("New Client.");
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();

    // CSS styles for the table
    client.println("<style>");
    client.println("table { width: 100%; border-collapse: collapse; }");
    client.println("th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }");
    client.println("tr:hover {background-color: #f5f5f5;}");
    client.println("</style>");

    // Start HTML table
    client.println("<table border=\"1\">");
    client.println("<tr><th>Time 1 (s)</th><th>Time 2 (s)</th><th>Ratio (t2/t1)</th></tr>");

    // Add data rows
    client.print("<tr><td>");
    client.print(t1/1000);
    client.print("</td><td>");
    client.print(t2/1000);
    client.print("</td><td>");
    client.print(ratio);
    client.println("</td></tr>");

    // End HTML table
    client.println("</table>");

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  // Read hall sensor
  lue();
}
