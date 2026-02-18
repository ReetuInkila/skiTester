#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "config.h"

enum StatusCode : uint8_t {
  STATUS_START  = 1,
  STATUS_RESULT = 2,
  STATUS_ERROR  = 3
};
// Pinnit summerille ja anturille
#define BUZZER_PIN 13
#define SENSOR_PIN 22

// BNO08X pinnimääritykset
#define BNO08X_CS 25
#define BNO08X_INT 26
#define BNO08X_RESET 14

// Mittaustiheys ja max pituus
#define max_measurement_time 30 // Maksimi mittausaika sekunteina
#define measurement_interval 100 // Mittaustaajuus hertzeinä

// Viesti puskurin maksimi pituus
#define MSG_LIMIT 200

#endif
