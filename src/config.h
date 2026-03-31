#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi Credentials - Update these!
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Reaper OSC Settings
const char* reaper_ip = "192.168.75.71";
const uint16_t reaper_port = 8000;
const uint16_t local_port = 9000;

// Pin Mapping (ESP32-S2 Mini)
#define PIN_BUTTON_START 1
#define PIN_BUTTON_STOP 3
#define PIN_BUTTON_REWIND 5
#define PIN_I2C_SDA 33
#define PIN_I2C_SCL 35

// Timeouts (ms)
#define OLED_TIMEOUT (5 * 60 * 1000)
#define SLEEP_TIMEOUT (30 * 60 * 1000)

#endif