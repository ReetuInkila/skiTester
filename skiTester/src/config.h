#ifndef CONFIG_H
#define CONFIG_H

// Wi-Fi verkon asetukset
extern const char *ssid = "Ski-Tester-1";
extern const char *password = "123456789";

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
