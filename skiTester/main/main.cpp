#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iostream>
extern "C" {
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_http_server.h"
}

static const char* TAG = "skiTester";

// Wi-Fi AP credentials
#define WIFI_SSID      "Ski-Tester-1"
#define WIFI_PASS      "123456789"
#define MAX_STA_CONN   4

static httpd_handle_t server = nullptr; // global SSE server handle

// Pins
#define BUZZER_PIN     GPIO_NUM_13
#define SENSOR_PIN     GPIO_NUM_14

// Sensor times
static uint64_t time1 = 0;
static uint64_t time2 = 0;
static uint64_t time3 = 0;
static float t1 = 0, t2 = 0;

// Error message
static std::string errorMessage = "";
static uint64_t messageId = 0;

// Connected SSE clients
struct sse_client_t {
    httpd_req_t* req;
};
static std::vector<sse_client_t> sse_clients;

// Forward declarations
void buzz();
void readSensors();
void notifyClients(const std::string& message);
void wifi_init_softap();
httpd_handle_t start_sse_server();

// ---------------------------
// SSE Handler
// ---------------------------
esp_err_t sse_handler(httpd_req_t* req) {
    // Set headers for SSE
    httpd_resp_set_type(req, "text/event-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");

    sse_clients.push_back({req});
    ESP_LOGI(TAG, "New SSE client connected");

    // Keep connection open
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        // Check if request was closed
        if (httpd_req_to_sockfd(req) < 0) {
            break;
        }
    }

    // Remove client when disconnected
    sse_clients.erase(std::remove_if(sse_clients.begin(), sse_clients.end(),
        [req](const sse_client_t& c){ return c.req == req; }), sse_clients.end());

    ESP_LOGI(TAG, "SSE client disconnected");
    return ESP_OK;
}

// ---------------------------
// Wi-Fi AP initialization
// ---------------------------
void wifi_init_softap() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strcpy((char*)wifi_config.ap.ssid, WIFI_SSID);
    wifi_config.ap.ssid_len = strlen(WIFI_SSID);
    strcpy((char*)wifi_config.ap.password, WIFI_PASS);
    wifi_config.ap.max_connection = MAX_STA_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP started. SSID: %s", WIFI_SSID);
}

// ---------------------------
// SSE Server
// ---------------------------
httpd_handle_t start_sse_server() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = nullptr;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t sse_uri = {};
        sse_uri.uri = "/events";
        sse_uri.method = HTTP_GET;
        sse_uri.handler = sse_handler;
        sse_uri.user_ctx = nullptr;
        httpd_register_uri_handler(server, &sse_uri);
    }
    return server;
}

// ---------------------------
// Notify clients
// ---------------------------
void notifyClients(const std::string& message) {
    std::ostringstream ss;
    ss << "id: " << messageId++ << "\n";
    if (!message.empty()) {
        ss << "data: {\"error\":\"" << message << "\"}\n\n";
    } else {
        ss << "data: {\"t1\":" << t1 << ",\"t2\":" << t2 << "}\n\n";
        t1 = t2 = 0;
    }

    std::string payload = ss.str();
    for (auto& client : sse_clients) {
        httpd_resp_send(client.req, payload.c_str(), payload.size());
    }
}

// ---------------------------
// Read sensors
// ---------------------------
void readSensors() {
    int sensorValue = gpio_get_level(SENSOR_PIN);
    ESP_LOGI(TAG, "Sensor value: %d", sensorValue);

    if (sensorValue == 0) {
        uint64_t now = esp_timer_get_time() / 1000; // ms
        if (time1 == 0) {
            time1 = now;
            ESP_LOGI(TAG, "Time1 recorded");
            errorMessage = "Not all sensors read";
            buzz();
        } else if (time2 == 0) {
            time2 = now;
            ESP_LOGI(TAG, "Time2 recorded");
            buzz();
        } else {
            time3 = now;
            ESP_LOGI(TAG, "Time3 recorded");
            t1 = (time2 - time1) / 1000.0f;
            t2 = (time3 - time2) / 1000.0f;
            errorMessage.clear();
            buzz();

            time1 = time2 = time3 = 0;
        }
    }
}

// ---------------------------
// Buzz function
// ---------------------------
void buzz() {
    gpio_set_level(BUZZER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(BUZZER_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
}

// ---------------------------
// Application main loop
// ---------------------------
extern "C" void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());

    // Initialize Wi-Fi AP
    wifi_init_softap();

    // Setup GPIOs
    gpio_reset_pin(SENSOR_PIN);
    gpio_set_direction(SENSOR_PIN, GPIO_MODE_INPUT);

    gpio_reset_pin(BUZZER_PIN);
    gpio_set_direction(BUZZER_PIN, GPIO_MODE_OUTPUT);

    // Startup buzzer
    buzz();

    // Start SSE server
    server = start_sse_server();

    // Main loop
    while (true) {
        readSensors();

        if (!errorMessage.empty()) {
            notifyClients(errorMessage);
            errorMessage.clear();
        }

        if (t1 != 0 && t2 != 0) {
            notifyClients("");
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 100 ms
    }
}
