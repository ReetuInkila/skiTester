#include <Arduino.h>
#include "config.h"
#include "imu_sensor.h"
#include "websocket_server.h"
#include <WiFi.h>


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

// Funktioiden esikuvat
void buzz();
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

  initWebSocket();
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

// Soittaa summeria
void buzz()
{
  tone(BUZZER_PIN, 800, 500);
}