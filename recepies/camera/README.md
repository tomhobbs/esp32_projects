# Camera

NB: This project doesn't yet work.

- Camera does initialise
- Cannot use OLED at the same time
- No pictures taken

## Hardware
- Heltec ESP32 (with WiFi/LoRa/OLED)
- Arducam OV2640

Current pin allocation:

| ESP32 | Camera |
| --- | --- |
| GND | GND |
| 5V | VCC |
| 5 | CS |
| 18 | SCK |
| 19 | MISO |
| 21 | SDA |
| 22 | SCL |
| 23 | MOSI |

# References

- https://github.com/yoursunny/esp32cam
- https://github.com/espressif/esp32-camera