#ifndef BLE_SERVER_H
#define BLE_SERVER_H

// Public API: same signature as before.
void notifyClients(StatusCode status,
                   float mag_avg = 0.0f,
                   float total = 0.0f,
                   const String& message = "");

// Call this once from setup()
void initBLE();

#endif