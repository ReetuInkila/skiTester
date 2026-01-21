#include <Arduino.h>
#include "config.h"
#include "imu_sensor.h"
#include "ble_server.h"

// Magneetin havaitsemisen aikaleimat
volatile unsigned long start_time = 0;
volatile unsigned long end_time = 0;
volatile unsigned long last_measurement_time = 0;
static const unsigned long MEASUREMENT_TIMEOUT_MS = 30000;

String errorMessage = "";

// Mittaus taajuus ja maksimi pituus ja tallennus taulukko kiihtyvyysarvoille
static const size_t MAG_MAX = measurement_interval * max_measurement_time; // Maksimi tallennettavien arvojen määrä
static float mag_buf[MAG_MAX];                                             // Kiihtyvyysarvojen tallennustaulukko
static size_t mag_idx = 0;  // Indeksi seuraavalle tallennettavalle arvolle

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
volatile bool start_event = false;
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

  initBLE();
}

void loop()
{
    // Aloitusportin ilmoitus
    if (start_event)
    {
        notifyClients(STATUS_START, 0, 0, "Measurement started");
        Serial.println("Measurement started");
        start_event = false;
    }

    // Timeout-valvonta
    if (measuring && start_time != 0)
    {
        if (millis() - start_time > MEASUREMENT_TIMEOUT_MS)
        {
            measuring = false;
            mag_clear();
            errorMessage = "Measurement timed out";
            start_time = 0;
            end_time = 0;
        }
    }

    // Virheilmoitusten käsittely
    if (!errorMessage.isEmpty() && !measuring)
    {
        notifyClients(STATUS_ERROR, 0, 0, errorMessage);
        errorMessage = "";
    }

    // Tulosten lähetys vain, jos end_time asetettu normaalisti
    if (end_time != 0 && !errorMessage.length())
    {
        size_t n = mag_idx;
        float sum = 0.0f;
        for (size_t i = 0; i < n; i++)
            sum += mag_buf[i];
        float mag_avg = n ? sum / n : 0.0f;

        notifyClients(STATUS_RESULT, mag_avg, (end_time - start_time) / 1000.0f);

        mag_clear();
        start_time = 0;
        end_time = 0;
    }

    // Kiihtyvyysarvojen tallennus mittauksen aikana
    if (measuring && end_time == 0)
    {
        float x, y, z;
        if (getLinearAcceleration(x, y, z))
        {
            float m = sqrtf(x * x + y * y + z * z);
            mag_store(m);
        }
    }
}


// Keskeytyskäsittelijä mittauksen aloittamiseen ja lopettamiseen
void IRAM_ATTR mittaa()
{
  unsigned long t = millis();
  if (t-last_measurement_time > 1000){
    measuring = !measuring;
    last_measurement_time = t;
    if (measuring){
      start_event = true;
      start_time = t;
    } 
    else end_time = t;
  }
}

// Soittaa summeria
void buzz()
{
  tone(BUZZER_PIN, 800, 500);
}