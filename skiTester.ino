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
  unsigned long loopStartTime = millis(); // Record the start time of the loop
  
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    float t1=time2-time1;
    float t2=time3-time2;
    Serial.println(time1);
    Serial.println(time2);
    Serial.println(time3);

    client.println("test");
    client.print("<h1>");      
    client.print(t2/t1);
    client.println("</h1>"); 
    client.println();

    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  
  // Calculate the time elapsed in the loop
  unsigned long loopTime = millis() - loopStartTime;

  // Delay to achieve desired loop rate
  if (loopTime < 100) { // If loop time is less than 100 milliseconds
    delay(100 - loopTime); // Delay to maintain 10 loops per second
  }

  // Call your other function here
  lue();
}
