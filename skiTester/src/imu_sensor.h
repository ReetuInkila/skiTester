#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

#include <Adafruit_BNO08x.h>
#include "config.h"

bool imuBegin();
void setReports();
bool getLinearAcceleration(float &x, float &y, float &z);

#endif
