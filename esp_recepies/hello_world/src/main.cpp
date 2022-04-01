#include "heltec.h"

const bool DISPLAY_ON = true;
const bool LORA_ON = true;
const bool SERIAL_ON = true;
const bool PABOOST_ON = true;
const long RAND = 470E6;

// the setup routine runs once when starts up
void setup(){

  // Initialize the Heltec ESP32 object
  Heltec.begin(DISPLAY_ON, LORA_ON, SERIAL_ON, PABOOST_ON, RAND);

}

// the loop routine runs over and over again forever
void loop() {
  delay(100);
  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Good morning, Dave.");

  Heltec.display->display();
}
