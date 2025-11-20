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

void notifyClients(float mag_avg, float total, String message = "");
void removeMessageById(unsigned long id);
void initWebSocket();

#endif
