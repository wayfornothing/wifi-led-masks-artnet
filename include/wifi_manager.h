#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <functional>

class WiFiManager {

private:
    using wifi_callback_t = std::function<void()>;

    const char* _ssid;
    const char* _pass;
    unsigned long _timeout_ms;
    unsigned long _retry_interval_ms;
    unsigned long _last_attempt_ts;
    bool _connected;
    wifi_callback_t _on_disconnect;
    wifi_callback_t _on_connect;

public:

    WiFiManager(const char* ssid, const char* pass, 
                unsigned long timeout_ms, unsigned long retry_interval_ms) :
        _ssid(ssid),
        _pass(pass),
        _timeout_ms(timeout_ms),
        _retry_interval_ms(retry_interval_ms),
        _last_attempt_ts(0),
        _connected(false) {

    }

    void on_disconnect(wifi_callback_t cb) {
        _on_disconnect = cb;
    }

    void on_connect(wifi_callback_t cb) {
        _on_connect = cb;
    }

    void set_hostname(const char *name) {
        WiFi.hostname(name);
    }

    void connect() {
        Serial.printf("Connecting to %s", _ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid, _pass);

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < _timeout_ms) {
            delay(500);
            Serial.print(".");
        }
        
        wl_status_t status = WiFi.status();
        if (status == WL_CONNECTED) {
            Serial.printf("\nConnected with IP: %s\n", WiFi.localIP().toString().c_str());
            _connected = true;
            _on_connect();
        } else {
            Serial.printf("\nConnection failed: %d\n", status);
            _connected = false;
        }
    }

    void tick() {
        if (WiFi.status() == WL_CONNECTED) {
            if (_connected == false) {
                // network restored, trigger callback just once
                Serial.println("Connection restored !!\n");
                _on_connect();
            }
            _connected = true;
        }
        else {
            if (_connected) {
                // was connected but lost connection, trigger callback just once
                Serial.println("Connection lost !!\n");
                _connected = false;
                if (_on_disconnect) {
                    _on_disconnect();
                }
            }

            unsigned long now = millis();
            if (now - _last_attempt_ts >= _retry_interval_ms) {
                _last_attempt_ts = now;
                Serial.println("Retrying to connect.\n");
                connect();
            }
        }
    }

    bool is_connected() const {
        return WiFi.status() == WL_CONNECTED;
    }
};
