#include <Arduino.h>
#include <FS.h>
#ifdef ESP32
#include <SPIFFS.h>
#include <AsyncTCP.h>
#else
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);

void __setup() {
    Serial.begin(115200);
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return;
    }

    // Serve static files (HTML, JS, CSS)
    server.serveStatic("/", SPIFFS, "/").setDefaultFile("clients.html");

    // Serve existing clients.json
    server.on("/clients.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (SPIFFS.exists("/clients.json")) {
            request->send(SPIFFS, "/clients.json", "application/json");
        } else {
            request->send(404, "application/json", "{\"clients\":[]}");
        } 
    });

    // Save clients.json
    server.on("/save_clients", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t) {
        File file = SPIFFS.open("/clients.json", FILE_WRITE);
        if (!file) {
            request->send(500, "text/plain", "Failed to open file for writing");
            return;
        }
        file.write(data, len);
        file.close();
        request->send(200, "text/plain", "Saved");
        Serial.println("Clients config saved!"); 
    });

    server.begin();
    Serial.println("Server started");
}

void __loop()
{
    // Nothing required here
}
