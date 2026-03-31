#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOSC.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// Globals
Adafruit_SSD1306 display(128, 32, &Wire, -1);

// Manual Debouncing States
int lastBtnStartState = HIGH;
int lastBtnStopState = HIGH;
int lastBtnRewindState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

unsigned long lastActivity = 0;
String currentTimecode = "00:00:00:00";
String transportStatus = "Stopped";
bool isDisplayOn = true;

void updateDisplay() {
  if (!isDisplayOn) return;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(transportStatus);
  display.setTextSize(2);
  display.println(currentTimecode);
  display.display();
}

void resetInactivity() {
  lastActivity = millis();
  if (!isDisplayOn) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    isDisplayOn = true;
    updateDisplay();
  }
}

void sendOsc(const char* address, float value) {
  OscWiFi.send(reaper_ip, reaper_port, address, value);
  resetInactivity();
}

void onStartPressed() { sendOsc("/transport/play", 1.0f); transportStatus = "Playing"; }
void onStopPressed() { sendOsc("/transport/stop", 1.0f); transportStatus = "Stopped"; }
void onRewindPressed() { sendOsc("/transport/rewind", 1.0f); }

void checkButtons() {
  int start = digitalRead(PIN_BUTTON_START);
  int stop = digitalRead(PIN_BUTTON_STOP);
  int rewind = digitalRead(PIN_BUTTON_REWIND);
  unsigned long now = millis();

  if (now - lastDebounceTime > DEBOUNCE_DELAY) {
    if (start == LOW && lastBtnStartState == HIGH) onStartPressed();
    if (stop == LOW && lastBtnStopState == HIGH) onStopPressed();
    if (rewind == LOW && lastBtnRewindState == HIGH) onRewindPressed();
    
    lastBtnStartState = start;
    lastBtnStopState = stop;
    lastBtnRewindState = rewind;
    lastDebounceTime = now;
  }
}

void onTimecodeReceived(const OscMessage& m) {
  String newTC = "";
  if (m.isString(0)) newTC = m.arg<String>(0);
  else if (m.isFloat(0)) newTC = String(m.arg<float>(0), 2);
  
  if (newTC != "" && newTC != currentTimecode) {
    currentTimecode = newTC;
    updateDisplay();
  }
}

void goToSleep() {
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  
  // Wake up on any button press
  esp_sleep_enable_ext1_wakeup(
    (1ULL << PIN_BUTTON_START) | (1ULL << PIN_BUTTON_STOP) | (1ULL << PIN_BUTTON_REWIND),
    ESP_EXT1_WAKEUP_ANY_LOW
  );
  esp_light_sleep_start();
  
  // After wakeup
  resetInactivity();
}

void setup() {
  Serial.begin(115200);
  
  // Initialize Display
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting...");
  display.display();

  // Initialize Buttons
  pinMode(PIN_BUTTON_START, INPUT_PULLUP);
  pinMode(PIN_BUTTON_STOP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_REWIND, INPUT_PULLUP);

  // WiFi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connected!");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);
  
  // OSC Subscriptions
  OscWiFi.subscribe(local_port, "/beattime", onTimecodeReceived);
  OscWiFi.subscribe(local_port, "/time", onTimecodeReceived);

  resetInactivity();
}

void loop() {
  checkButtons();
  OscWiFi.update();

  if (WiFi.status() != WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi Lost!");
    display.display();
    return;
  }

  // Power Management
  unsigned long elapsed = millis() - lastActivity;
  if (elapsed > SLEEP_TIMEOUT) {
    goToSleep();
  } else if (elapsed > OLED_TIMEOUT && isDisplayOn) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    isDisplayOn = false;
  }
}