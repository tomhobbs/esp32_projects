// https://github.com/ArduCAM/ArduCAM_ESP32S_UNO

// #define ESP32 1;
#include "heltec.h"
// #include <Arduino.h>
#include "FS.h"
#include "SPIFFS.h"
#include <Wire.h>
#include <SPI.h>
#include <ArduCAM.h>
#include "memorysaver.h"

#define OV2640_CHIPID_HIGH 	0x0A
#define OV2640_CHIPID_LOW 	0x0B

// Camera is not found if display is on
const bool DISPLAY_ON = false;
const bool LORA_ON = false;
const bool SERIAL_ON = true;
const bool PABOOST_ON = true;
const long RAND = 470E6;

// From https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
/*
SPI	  MOSI	  MISO	  CLK	CS
VSPI	GPIO 23	GPIO 19	GPIO 18	GPIO 5
HSPI	GPIO 13	GPIO 12	GPIO 14	GPIO 15
*/
// const int CS = 16;  // VSPI=5 / HSPI=15
const int CS = 5;  // VSPI=5 / HSPI=15
// Version 2,set GPIO0 as the slave select :
const int SD_CS = 0;
ArduCAM myCAM(OV2640, CS);
uint32_t length = 0;

const int CAM_SDA = 21;
const int CAM_SCL = 22;

void capture()
{

  // Flush the FIFO
  myCAM.flush_fifo();
  // Clear the capture done flag

  myCAM.clear_fifo_flag();
  // Start capture
  myCAM.start_capture();
  Serial.println(F("Start Capture")); // Not enough power to complete?
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
    ;
  Serial.println(F("Capture Done"));
}

void setup()
{
  Heltec.begin(DISPLAY_ON, LORA_ON, SERIAL_ON, PABOOST_ON, RAND);

  uint8_t vid, pid; 
  uint8_t temp;
  Wire.begin(CAM_SDA, CAM_SCL);
  // Wire.begin(4, 15);
  
  Serial.begin(115200);
  Serial.println("ArduCAM Start !");
 
  // set the CS as an output:
  pinMode(CS, OUTPUT);
  // initialize SPI:
  SPI.begin();
  SPI.setFrequency(8000000); // 4MHZ
  SPI.setDataMode(0);

  // Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);

  delay(100);

  while (1)
  {
    // Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55)
    {
      Serial.println(F("SPI interface Error !"));
      delay(1000);
      continue;
    }
    else
    {
      Serial.println(F("SPI interface OK !"));
      break;
    }
  }

  while (1)
  {
    // Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.wrSensorReg8_8(0x12, 0x80);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26) && ((pid != 0x41) || (pid != 0x42)))
    // if ( ( vid == 0x21 ) && ( pid == 0x4E ) )
    {
      Serial.println(F("Can’t find OV2640 module !"));
      delay(1000);
      continue;
    }
    else
    {
      Serial.println(F("OV2640 detected."));
      break;
    }
  }

  myCAM.set_format(JPEG);
  myCAM.InitCAM();

  myCAM.OV2640_set_JPEG_size(OV2640_320x240);

  delay(1000);
  myCAM.clear_fifo_flag();
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(5000);

  capture();
}