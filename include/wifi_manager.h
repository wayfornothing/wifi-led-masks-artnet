#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <functional>

#include "led_device.h"

/**
 * @brief Manage the WiFi connection and error handling state.
 * Call 'tick()' on every loop.
 * If wifi connection succeeds, a callback is triggered, and the connection is checked at every tick.
 * If wifi connection fails (or times out), a callback is triggered, and the connection is retried.
 * If the max number of retries is reached, this wifi manager is disabled until reset.
 */
class WiFiManager {

private:
    using wifi_callback_t = std::function<void()>;

    // const char* _ssid;
    // const char* _pass;
    uint32_t _timeout_ms;
    uint32_t _retry_interval_ms;
    uint16_t  _attempts;
    uint16_t  _max_attempts;
    uint32_t _last_attempt_ts;
    uint8_t _pin_ui;
    bool _connected;
    bool _active;
    wifi_callback_t _on_disconnect;
    wifi_callback_t _on_connect;
    wifi_callback_t _on_connect_failed;

public:

    /**
     * @brief Construct a new Wi Fi Manager object
     * 
     * @param ssid 
     * @param pass 
     * @param timeout_ms 
     * @param retry_interval_ms 
     * @param max_attempts if 0, infinite attempts
     * @param pin_ui LED pin to signal wifi status
     */
    WiFiManager(uint32_t timeout_ms, 
                uint32_t retry_interval_ms,
                uint16_t max_attempts,
                uint8_t pin_ui) :
        // _ssid(ssid),
        // _pass(pass),
        _timeout_ms(timeout_ms),
        _retry_interval_ms(retry_interval_ms),
        _attempts(0),
        _max_attempts(max_attempts),
        _last_attempt_ts(0),
        _pin_ui(pin_ui),
        _connected(false),
        _active(true) {
            pinMode(_pin_ui, OUTPUT);
    }

    void on_disconnect(wifi_callback_t cb) {
        _on_disconnect = cb;
    }

    void on_connect(wifi_callback_t cb) {
        _on_connect = cb;
    }

    void on_connect_failed(wifi_callback_t cb) {
        _on_connect_failed = cb;
    }

    void set_hostname(const char *name) {
        WiFi.hostname(name);
    }

    void connect() {
        
        LEDDevice led(_pin_ui, "D4");
        led.blink_interval_ms = 200;
        
        DeviceConfig& cfg = DeviceConfig::instance();
        Serial.printf("Connecting to %s", cfg.get_SSID().c_str());
        
        led.start_blink();
        WiFi.mode(WIFI_STA);
        WiFi.begin(cfg.get_SSID(), cfg.get_password());
        

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < _timeout_ms) {
            digitalWrite(_pin_ui, !digitalRead(_pin_ui));
            delay(500);
            Serial.print(".");
        }
        
        led.enable(false);
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
        if (_active) {
            if (WiFi.status() == WL_CONNECTED) {
                if (_connected == false) {
                    // network restored, trigger callback just once
                    Serial.println("Connection restored !!\n");
                    _attempts = 0;
                    _on_connect();
                }
                _connected = true;
            }
            else {
                if (_connected) {
                    // was connected but lost connection, trigger callback just once
                    _connected = false;
                    Serial.println("Connection lost !!\n");
                    if (_on_disconnect) {
                        _on_disconnect();
                    }
                }

                // retry to connect
                if (_attempts < _max_attempts || _max_attempts == 0) {
                    unsigned long now = millis();
                    if (now - _last_attempt_ts >= _retry_interval_ms) {
                        _attempts++;
                        _last_attempt_ts = now;
                        Serial.println("Retrying to connect.\n");
                        connect();
                    }
                }
                else {
                    // max attempts reached, don't try to reconnect until reset
                    Serial.println("Max attempts reached, disabled WiFi manager.\n");
                    if (_on_connect_failed ) {
                        _on_connect_failed();
                    }
                    // disable this manager
                    _active = false;
                }
            }
        }
    }

    void reset() {
        _attempts = 0;
        _active = true;
    }

    bool is_connected() const {
        return WiFi.status() == WL_CONNECTED;
    }
};
