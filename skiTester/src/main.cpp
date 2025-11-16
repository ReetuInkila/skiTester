#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <vector>
#include <Adafruit_BNO08x.h>

// Wi-Fi verkon asetukset
const char *ssid = "Ski-Tester-1";
const char *password = "123456789";

// Pinnit summerille ja anturille
const int buzzerPin = 13; // Summerin ohjaukseen käytettävä pinni
const int sensorPin = 22; // Anturin lukemiseen käytettävä pinni

// BNO085 IMU liitäntöjen määrittelyt ja sensori olio
#define BNO08X_CS 25
#define BNO08X_INT 26
#define BNO08X_RESET 14
Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

// Magneetin havaitsemisen aikaleimat
unsigned long start_time = 0;
unsigned long end_time = 0;

// Mittaus taajuus ja maksimi pituus ja tallennus taulukko kiihtyvyysarvoille
const unsigned long max_measurement_time = 30;                             // Maksimi mittausaika s
const unsigned long measurement_interval = 100;                            // Mittaustaajuus hz
static const size_t MAG_MAX = measurement_interval * max_measurement_time; // Maksimi tallennettavien arvojen määrä
static float mag_buf[MAG_MAX];                                             // Kiihtyvyysarvojen tallennustaulukko
static size_t mag_idx = 0;                                                 // Indeksi seuraavalle tallennettavalle arvolle

// Funktiot kiihtyvyys arvojen tallennukseen ja hallintaan
inline void mag_store(float v)
{
  if (mag_idx < MAG_MAX)
    mag_buf[mag_idx++] = v;
}
inline void mag_clear()
{
  mag_idx = 0;
}

// Mittaus tilan hallinta
bool measuring = false;

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
  pinMode(sensorPin, INPUT);                                          // Aseta anturipinni syötteeksi
  pinMode(buzzerPin, OUTPUT);                                         // Aseta summeripinni ulostuloksi
  attachInterrupt(digitalPinToInterrupt(sensorPin), mittaa, FALLING); // Liitä keskeytys anturipinniin

  buzz(); // Soita merkkiääni alustuksen merkiksi

  // BNO08x anturin alustus SPI:llä
  if (!bno08x.begin_SPI(BNO08X_CS, BNO08X_INT))
  {
    Serial.println("Failed to find BNO08x chip");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("BNO08x Found!");
  for (int n = 0; n < bno08x.prodIds.numEntries; n++)
  {
    Serial.print("Part ");
    Serial.print(bno08x.prodIds.entry[n].swPartNumber);
    Serial.print(": Version :");
    Serial.print(bno08x.prodIds.entry[n].swVersionMajor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionMinor);
    Serial.print(".");
    Serial.print(bno08x.prodIds.entry[n].swVersionPatch);
    Serial.print(" Build ");
    Serial.println(bno08x.prodIds.entry[n].swBuildNumber);
  }
  setReports();

  // Wi-Fi-yhteyden luominen
  Serial.println("Setting up Access Point...");
  WiFi.softAP(ssid);
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

        DynamicJsonDocument doc(1024);
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
    if (bno08x.getSensorEvent(&sensorValue))
    {
      if (sensorValue.sensorId == SH2_LINEAR_ACCELERATION)
      {
        float x = sensorValue.un.linearAcceleration.x;
        float y = sensorValue.un.linearAcceleration.y;
        float z = sensorValue.un.linearAcceleration.z;
        float m = sqrtf(x * x + y * y + z * z);
        mag_store(m);
      }
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

// Määritä halutut raportit BNO08x anturille
void setReports(void)
{
  Serial.println("Setting desired reports");
  if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION))
  {
    Serial.println("Failed to enable linear acceleration report");
  }
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

  messages.push_back({messageId, jsonResponse});
  messageId++;
  Serial.print("Sending to clients: ");
  Serial.println(jsonResponse);

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
  tone(buzzerPin, 800, 500);
}