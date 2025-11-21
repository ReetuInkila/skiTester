#include "websocket_server.h"
#include "config.h"

#include <algorithm>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

std::vector<Message> messages;
String errorMessage = "";
unsigned long messageId = 0;

void notifyClients(float mag_avg, float total, String message) {
    StaticJsonDocument<256> jsonDoc;

    if (!message.isEmpty()) {
        jsonDoc["error"] = message;
    } else {
        jsonDoc["time"] = total;
        jsonDoc["mag_avg"] = mag_avg;
    }

    jsonDoc["id"] = messageId;
    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);

    Serial.print("Sending to clients: ");
    Serial.println(jsonResponse);

    noInterrupts();
    if (messages.size() >= MSG_LIMIT) messages.erase(messages.begin());
    messages.push_back({messageId++, jsonResponse});
    interrupts();

    ws.textAll(jsonResponse);
}

void removeMessageById(unsigned long id) {
    auto it = std::find_if(messages.begin(), messages.end(), [id](const Message &msg) { return msg.id == id; });

    if (it != messages.end()) {
        messages.erase(it);
        Serial.print("Removed message with ID: ");
        Serial.println(id);
    } else {
        Serial.print("Message with ID not found: ");
        Serial.println(id);
    }
}

void initWebSocket() {
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.println("WebSocket client connected");
            for (const auto &message : messages) {
                client->text(message.content);
            }
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("WebSocket client disconnected");
        } else if (type == WS_EVT_DATA) {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                data[len] = '\0';
                String receivedMessage = String((char *)data);
                Serial.print("Received from client: ");
                Serial.println(receivedMessage);

                StaticJsonDocument<256> doc;
                DeserializationError error = deserializeJson(doc, receivedMessage);
                if (!error && doc.containsKey("id")) {
                    unsigned long id = doc["id"].as<unsigned long>();
                    removeMessageById(id);
                }
            }
        }
    });

    server.addHandler(&ws);
    server.begin();
}
