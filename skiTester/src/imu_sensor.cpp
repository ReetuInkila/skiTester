#include "imu_sensor.h"
#include <Arduino.h>


Adafruit_BNO08x bno08x(BNO08X_RESET);
sh2_SensorValue_t sensorValue;

bool imuBegin() {
    return bno08x.begin_SPI(BNO08X_CS, BNO08X_INT);
}

void setReports() {
    Serial.println("Setting desired reports");
    if (!bno08x.enableReport(SH2_LINEAR_ACCELERATION)) {
        Serial.println("Failed to enable linear acceleration report");
    }
}

bool getLinearAcceleration(float &x, float &y, float &z) {
    if (bno08x.getSensorEvent(&sensorValue)) {
        if (sensorValue.sensorId == SH2_LINEAR_ACCELERATION) {
            x = sensorValue.un.linearAcceleration.x;
            y = sensorValue.un.linearAcceleration.y;
            z = sensorValue.un.linearAcceleration.z;
            return true;
        }
    }
    return false;
}
