#include <Arduino.h>
#include "config.h"
#include "imu_sensor.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <Adafruit_BNO08x.h>


// Magneetin havaitsemisen aikaleimat
volatile unsigned long start_time = 0;
volatile unsigned long end_time = 0;

// Mittaus taajuus ja maksimi pituus ja tallennus taulukko kiihtyvyysarvoille
static const size_t MAG_MAX = measurement_interval * max_measurement_time; // Maksimi tallennettavien arvojen määrä
static float mag_buf[MAG_MAX];                                             // Kiihtyvyysarvojen tallennustaulukko
static size_t mag_idx = 0;                                                 // Indeksi seuraavalle tallennettavalle arvolle

// Funktiot kiihtyvyys arvojen tallennukseen ja hallintaan
inline void mag_store(float v)
{
  if (mag_idx < MAG_MAX) mag_buf[mag_idx++] = v;
}
inline void mag_clear()
{
  mag_idx = 0;
}

// Mittaus tilan hallinta
volatile bool measuring = false;

// Virheilmoitusten ja viestien hallinta
String errorMessage = "";    // Virheilmoituksen säilytys
unsigned long messageId = 0; // Uniikki viestin ID


// Rakenne viestien hallintaan
struct Message
{
  unsigned long id;
  String content;
};
std::vector<Message> messages;

// Web-palvelimen ja WebSocketin alustaminen
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Funktioiden esikuvat
void buzz();
void notifyClients(float mag_avg, float total, String message = "");
void removeMessageById(unsigned long id);
void setReports();
void mittaa();

void setup()
{
  // Sarjaväylän alustus
  Serial.begin(115200);

  // Pinnien asetukset
  pinMode(SENSOR_PIN, INPUT);                                          // Aseta anturipinni syötteeksi
  pinMode(BUZZER_PIN, OUTPUT);                                         // Aseta summeripinni ulostuloksi
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), mittaa, FALLING); // Liitä keskeytys anturipinniin

  buzz(); // Soita merkkiääni alustuksen merkiksi

  // BNO08x anturin alustus SPI:llä
  if (!imuBegin())
  {
    Serial.println("Failed to find BNO08x chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");
  setReports();

  // Wi-Fi-yhteyden luominen
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // WebSocket-tapahtumakäsittelijät
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
    // Käsittele yhteyden muodostus, katkaisu ja saapuvat viestit
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

        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, receivedMessage);
        if (!error && doc.containsKey("id")) {
          unsigned long id = doc["id"].as<unsigned long>();
          removeMessageById(id);
        }
      }
    } });

  // Lisää WebSocket palvelimeen ja käynnistä palvelin
  server.addHandler(&ws);
  server.begin();
}

void loop()
{
  // Käsittele ja lähetä mahdolliset virheilmoitukset
  if (!errorMessage.isEmpty() && !measuring)
  {
    notifyClients(0, 0, errorMessage);
    errorMessage = ""; // Tyhjennä virheilmoitus lähetyksen jälkeen
  }

  // Lähetä tulokset WebSocket-asiakkaille
  if (end_time != 0)
  {
    size_t n = mag_idx;
    float sum = 0.0f;
    for (size_t i = 0; i < n; i++)
      sum += mag_buf[i];
    float mag_avg = n ? sum / n : 0.0f;
    mag_clear();
    notifyClients(mag_avg, (end_time - start_time) / 1000.0);
    start_time = 0, end_time = 0;
  }
  // Tallennetaan kiihtyvyysarvot mittauksen aikana
  if (measuring && end_time == 0)
  {
    float x, y, z;
    if (getLinearAcceleration(x, y, z)) {
      float m = sqrtf(x * x + y * y + z * z);
      mag_store(m);
    }
  }
}

// Keskeytyskäsittelijä mittauksen aloittamiseen ja lopettamiseen
void IRAM_ATTR mittaa()
{
  measuring = !measuring;
  unsigned long t = millis();
  if (measuring)
    start_time = t;
  else
    end_time = t;
}



// Lähettää viestin asiakkaalle ja tallentaa sen lähetettyjen listaan
void notifyClients(float mag_avg, float total, String message)
{
  StaticJsonDocument<256> jsonDoc;

  if (!message.isEmpty())
  {
    jsonDoc["error"] = message;
  }
  else
  {
    jsonDoc["time"] = total; // Aika sekunteina
    jsonDoc["mag_avg"] = mag_avg;
  }

  jsonDoc["id"] = messageId;
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);
  Serial.print("Sending to clients: ");
  Serial.println(jsonResponse);
  
  // Tallenna viesti ja rajoita tallennettujen viestien määrää
  noInterrupts();
  if (messages.size() >= MSG_LIMIT) messages.erase(messages.begin());
  messages.push_back({messageId++, jsonResponse});
  interrupts();

  ws.textAll(jsonResponse);
}

// Poistaa tietorakenteesta viestin halutulla id:llä
void removeMessageById(unsigned long id)
{
  // Etsi viesti id:llä ja poista se
  auto it = std::find_if(messages.begin(), messages.end(), [id](const Message &msg){ return msg.id == id; });

  // Poista viesti, jos se löytyy. Muuten tulosta virheilmoitus.
  if (it != messages.end()){
    messages.erase(it);
    Serial.print("Removed message with ID: ");
    Serial.println(id);
  }
  else{
    Serial.print("Message with ID not found: ");
    Serial.println(id);
  }
}

// Soittaa summeria
void buzz()
{
  tone(BUZZER_PIN, 800, 500);
}