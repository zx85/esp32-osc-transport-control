# esp32-osc-transport-control
A project to create a simple wireless transport control for Reaper using an ESP32


## Kit I've used
ESP-32 S2 mini
0.91" I2C display
four push buttons


## Reaper settings
Create a new control surface (Options -> Preferences -> Control/OSC/Web)
- Control surface mode: OSC (Open Sound Control)
- Device name: ESP32 (or whatever you like)
- Pattern config: Default
- Mode: Configure device IP+local port
- Device port: 9000 (or the local_port in config.h)
- Device IP: (your ESP32 IP address)
- Local listen port: 8000 (or the reaper_port in config.h)

You can click the 'Listen' button to confirm it works. You might need to set UDP port 8000 in the firewall.