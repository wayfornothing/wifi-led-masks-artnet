#pragma once

#include <functional>

#include "hal/hal.h"
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
        _timeout_ms(timeout_ms),
        _retry_interval_ms(retry_interval_ms),
        _attempts(0),
        _max_attempts(max_attempts),
        _last_attempt_ts(0),
        _pin_ui(pin_ui),
        _connected(false),
        _active(true) {
            pin_set_output(_pin_ui);
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
        wifi_set_hostname(name);
    }

    void connect() {
        
        LEDDevice led(_pin_ui, "D4");
        
        DeviceConfig& cfg = DeviceConfig::instance();
        Logger::info("Connecting to %s...", cfg.get_SSID().c_str());
        
        wifi_connect_to_ap(cfg.get_SSID().c_str(), cfg.get_password().c_str());

        unsigned long start = millis();
        bool enable = true;
        while (wifi_get_status() != WL_CONNECTED && millis() - start < _timeout_ms) {
            pin_digital_write(_pin_ui, !pin_digital_read(_pin_ui));
            led.enable(enable);
            Logger::info(".");
            delay_ms(200);
            enable = !enable;
        }

        led.enable(false);
        int status = wifi_get_status();
        if (status == WL_CONNECTED) {
            Logger::info("\nConnected with IP: %s\n", wifi_get_local_ip());
            _connected = true;
            _on_connect();
        } else {
            Logger::error("\nConnection failed: %d\n", status);
            _connected = false;
        }
    }


    void tick() {
        if (_active) {
            if (wifi_get_status() == WL_CONNECTED) {
                if (_connected == false) {
                    // network restored, trigger callback just once
                    Logger::info("Connection restored !!\n");
                    _attempts = 0;
                    _on_connect();
                }
                _connected = true;
            }
            else {
                if (_connected) {
                    // was connected but lost connection, trigger callback just once
                    _connected = false;
                    Logger::warn("Connection lost !!\n");
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
                        Logger::info("Retrying to connect.\n");
                        connect();
                    }
                }
                else {
                    // max attempts reached, don't try to reconnect until reset
                    Logger::error("Max attempts reached, disabled WiFi manager.\n");
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
        return wifi_get_status() == WL_CONNECTED;
    }
};
