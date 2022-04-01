#include "heltec.h"
#include "WiFi.h"
#include "esp_app_trace.h"

#define XSTR(x) #x
#define STR(x) XSTR(x)

const bool DISPLAY_ON = true;
const bool LORA_ON = false;
const bool SERIAL_ON = true;
const bool PABOOST_ON = true;
const long RAND = 470E6;

// CHANGE VALUES
const char *NAME = STR(HOSTNAME);
const char *SSID = STR(WIFI_SSID);
const char *PASSWORD = STR(WIFI_PASS);

static const char *TAG = "UploadFile";

void WIFISetUp(void)
{
    Serial.print("WIFI: Connecting to " + String(SSID));

    Heltec.display->clear();

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.disconnect(true);
    delay(1000);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    WiFi.setHostname(NAME);
    WiFi.begin(SSID, PASSWORD);
    delay(100);

    byte count = 0;
    while (WiFi.status() != WL_CONNECTED && count < 10)
    {
        count++;
        delay(500);
        Heltec.display->drawString(0, 0, String(NAME) + ": connecting to " + String(SSID));
        Heltec.display->display();
    }

    delay(500);
https: // docs.platformio.org/en/latest/projectconf/section_env_build.html#dynamic-build-flags
}

void setup()
{
    // put your setup code here, to run once:
    Heltec.begin(DISPLAY_ON, LORA_ON, SERIAL_ON, PABOOST_ON, RAND);

    WIFISetUp();
}

void loop()
{
    // put your main code here, to run repeatedly:
    delay(100);

    if (WiFi.status() == WL_CONNECTED)
    {
        Heltec.display->clear();
        Heltec.display->drawString(0, 0, "Connected: " + String(SSID));
        Heltec.display->drawString(0, 20, "Good morning, Dave.");
        Heltec.display->display();
    }
    else
    {
        Heltec.display->clear();
        Heltec.display->drawString(0, 0, "I'm sorry Dave.\nI can't do that.");
        Heltec.display->display();
    }
}
