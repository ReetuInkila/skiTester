#ifndef WEBSOCKET_SERVER_H
#define WEBSOCKET_SERVER_H

#include <Arduino.h>
#include <vector>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

struct Message {
    unsigned long id;
    String content;
};

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern std::vector<Message> messages;
extern String errorMessage;
extern unsigned long messageId;

enum StatusCode : uint8_t {
    STATUS_IDLE       = 0,
    STATUS_START      = 1,
    STATUS_RESULT     = 2,
    STATUS_ERROR      = 3,
    STATUS_IMU_STATUS = 4
};

void notifyClients(StatusCode status,
                   float mag_avg = 0.0f,
                   float total = 0.0f,
                   const String &message = "");

                   void removeMessageById(unsigned long id);
void initWebSocket();

#endif
