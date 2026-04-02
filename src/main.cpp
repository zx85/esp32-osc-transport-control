#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOSCWiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// Globals
Adafruit_SSD1306 display(128, 32, &Wire, -1);

// Manual Debouncing States
int lastBtnPlayState = HIGH;
int lastBtnStopState = HIGH;
int lastBtnRecordState = HIGH;
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
  Serial.printf("OSC Send: %s %.1f\n", address, value);
  OscWiFi.send(reaper_ip, reaper_port, address, value);
  resetInactivity();
}

void onPlayPressed() { Serial.println("Button: Play Pressed"); sendOsc("/play", 1.0f); }
void onStopPressed() { Serial.println("Button: Stop Pressed"); sendOsc("/stop", 1.0f); }
void onRecordPressed() { Serial.println("Button: recor Pressed"); sendOsc("/record", 1.0f); }

void onTransportStateReceived(const OscMessage& m) {
  if (m.size() == 0) return;
  float val = m.arg<float>(0);
  
  if (m.address() == "/play") {
    transportStatus = (val > 0.5f) ? "Playing" : "Stopped";
  }
  updateDisplay();
}

void checkButtons() {
  int play = digitalRead(PIN_BUTTON_PLAY);
  int stop = digitalRead(PIN_BUTTON_STOP);
  int record = digitalRead(PIN_BUTTON_RECORD);
  unsigned long now = millis();

  if (now - lastDebounceTime > DEBOUNCE_DELAY) {
    if (play == LOW && lastBtnPlayState == HIGH) onPlayPressed();
    if (stop == LOW && lastBtnStopState == HIGH) onStopPressed();
    if (record == LOW && lastBtnRecordState == HIGH) onRecordPressed();
    
    lastBtnPlayState = play;
    lastBtnStopState = stop;
    lastBtnRecordState = record;
    lastDebounceTime = now;
  }
}

void onTimecodeReceived(const OscMessage& m) {
  if (m.size() == 0 || m.typeTags().length() == 0) return;

  Serial.printf("OSC Recv: %s (Type: %s)\n", m.address().c_str(), m.typeTags().c_str());

  String newTC = "";
  char firstTag = m.typeTags()[0];
  if (firstTag == 's') newTC = m.arg<String>(0);
  else if (firstTag == 'f') newTC = String(m.arg<float>(0), 2);
  
  if (newTC != "" && newTC != currentTimecode) {
    currentTimecode = newTC;
    updateDisplay();
  }
}

void goToSleep() {
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  
  // Wake up on any button press (Low level)
  gpio_wakeup_enable((gpio_num_t)PIN_BUTTON_PLAY, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable((gpio_num_t)PIN_BUTTON_STOP, GPIO_INTR_LOW_LEVEL);
  gpio_wakeup_enable((gpio_num_t)PIN_BUTTON_RECORD, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

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
  pinMode(PIN_BUTTON_PLAY, INPUT_PULLUP);
  pinMode(PIN_BUTTON_STOP, INPUT_PULLUP);
  pinMode(PIN_BUTTON_RECORD, INPUT_PULLUP);

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
  OscWiFi.subscribe(local_port, "/play", onTransportStateReceived);

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