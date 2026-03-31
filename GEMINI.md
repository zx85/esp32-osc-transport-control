# Agent Specification: ESP32-S2 Reaper Transport Controller
## Project Overview

* Hardware: Wemos ESP32-S2 Mini.
* Display: 0.91" I2C OLED (SSD1306), 128x32 resolution.
* Inputs: 3x Momentary Push Buttons (Start, Stop, Rewind).
* Framework: Arduino / PlatformIO.
* Communication: OSC (Open Sound Control) over TCP/IP via WiFi.
* Target: Reaper DAW (Digital Audio Workstation).

## Hardware Mapping
| Component	     | Pin (ESP32-S2 Mini) | Logic                      |
| -------------- | ------------------- | -------------------------- |
| I2C SDA	     | GPIO 33	           | Data line for OLED         |
| I2C SCL	     | GPIO 35             | Clock line for OLED        |
| Button: Start  | GPIO 1              | Input Pull-up (Active Low) |
| Button: Stop	 | GPIO 3	           | Input Pull-up (Active Low) |
| Button: Rewind | GPIO 5	           | Input Pull-up (Active Low) |

## Software Architecture & File Structure

The project must adhere to the following structure:

* platformio.ini: Configure for lolin_s2_mini and include libraries: CNMAT/OSC, Adafruit SSD1306, Adafruit GFX.
* src/config.h: Store sensitive credentials (SSID, Password) and Reaper IP/Port settings.
* src/main.cpp: Main logic, debouncing, WiFi management, and OSC handling.

## Functional Requirements
### WiFi & Connection

* Connect to WiFi using credentials from config.h.
* On startup, the OLED should display "Connecting..." and the local IP address once connected.
* The system must handle reconnection logic if the WiFi signal is lost.

### OSC Messaging (Reaper Protocol)

* Start Button: Send /transport/play (float 1.0).
* Stop Button: Send /transport/stop (float 1.0).
* Rewind Button: Send /transport/rewind (float 1.0).
* Incoming Timecode: Listen for OSC message /beattime or /time (string or float depending on Reaper settings) and update the OLED display in real-time.

### Display Logic

* Default View: Show "Stopped" or "Playing" status and the current Timecode.
* Refresh Rate: The display should refresh only when the timecode value changes to avoid flickering.

## Other requirements

### Library Specifics
* **OSC Library:** Use `hideakitai/ArduinoOSC`. Do NOT use `cnmat/OSC`.
* **Display:** `adafruit/Adafruit SSD1306` and `adafruit/Adafruit GFX Library`.
* **Button Handling:** Use `ftrias/TeensyVariablePlayback` or simply custom non-blocking debouncing logic to keep dependencies lean.

### Default OSC parameters

The following parameters should be read from config.h

* Local Port: default 9000.
* Reaper Port: default 8000.
* IP Address: default 192.168.75.71


### Power Saving

Ensure the display turns off after a period of inactivity (5 minutes).
Ensure the device goes into light sleep after a longer period of inactivity (30 minutes).
Pressing any button should turn the display back on or wake up from sleep


### Error handling

Provide visual feedback on the OLED if the Reaper host is unreachable or if the WiFi connection fails. 
