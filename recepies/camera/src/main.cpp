/**
 * This example takes a picture every 5s and print its size on serial monitor.
 */

// =============================== SETUP ======================================

// 1. Board setup (Uncomment):
// #define BOARD_WROVER_KIT
// #define BOARD_ESP32CAM_AITHINKER

/**
 * 2. Kconfig setup
 *
 * If you have a Kconfig file, copy the content from
 *  https://github.com/espressif/esp32-camera/blob/master/Kconfig into it.
 * In case you haven't, copy and paste this Kconfig file inside the src directory.
 * This Kconfig file has definitions that allows more control over the camera and
 * how it will be initialized.
 */

/**
 * 3. Enable PSRAM on sdkconfig:
 *
 * CONFIG_ESP32_SPIRAM_SUPPORT=y
 *
 * More info on
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/kconfig.html#config-esp32-spiram-support
 */

// ================================ CODE ======================================
#include "heltec.h"

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <SPI.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduCAM.h>
#include "memorysaver.h"

#include "ov2640_regs.h"

#define XSTR(x) #x
#define STR(x) XSTR(x)

const bool DISPLAY_ON = false;
const bool LORA_ON = false;
const bool SERIAL_ON = true;
const bool PABOOST_ON = true;
const long RAND = 470E6;

const char *NAME = STR(HOSTNAME);

#define FORMAT_SPIFFS_IF_FAILED true

const int CAM_POWER_ON = A10;
// set GPIO16 as the slave se even tried with a new bought ArduCam, but no luck, I even did not get the sample code worlect :
const int CS = 17;
//Version 2,set GPIO0 as the slave select :
char pname[20];
//static int  index=0;
byte buf[256];
static int i = 0;
static int k = 0;
uint8_t temp = 0, temp_last = 0;
uint32_t length = 0;
bool is_header = false;

ArduCAM myCAM(OV2640, CS);

void capture2SD(fs::FS &fs, const char * path) {
  File file ;
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("Start Capture"));
  while (!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK)) {
    //Serial.println("...capturing");
  }
  Serial.println(F("Capture Done."));
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
  }
  if (length == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }
  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  i = 0;
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  while ( length-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buf[i++] = temp;  //save the last  0XD9
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      file.write(buf, i);
      //Close the file
      file.close();
      Serial.println(F("Image save OK."));
      is_header = false;
      i = 0;
    }
    if (is_header == true)
    {
      //Write image data to buffer if not full
      if (i < 256)
        buf[i++] = temp;
      else
      {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        file.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;
    }
  }
}

void setup() {
  Heltec.begin(DISPLAY_ON, LORA_ON, SERIAL_ON, PABOOST_ON, RAND);

  uint8_t vid, pid;
  uint8_t temp;
  static int i = 0;
  //set the CS as an output:
  pinMode(CS, OUTPUT);
  pinMode(CAM_POWER_ON , OUTPUT);
  digitalWrite(CAM_POWER_ON, HIGH);
  Wire.begin();
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  //initialize SPI:
  SPI.begin();
  while (1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    if (temp != 0x55) {
      Serial.println(F("SPI interface Error!"));
      delay(2);
      continue;
    }
    else
      break;
  }

  //Add support for SPIFFS
  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS Mount Successful");

  //Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 )))
    Serial.println(F("Can't find OV2640 module!"));
  else
    Serial.println(F("OV2640 detected."));
  
  delay(1000);
}

void loop() {
  sprintf((char*)pname, "/%05d.jpg", k);
  capture2SD(SPIFFS, pname);

//  listDir(SPIFFS, "/", 0);
//  readImage(SPIFFS, pname);
// deleteFile(SPIFFS, pname);
   k++;
  delay(5000);

}

// void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
//   Serial.printf("Listing directory: %s\r\n", dirname);

//   File root = fs.open(dirname);
//   if (!root) {
//     Serial.println("- failed to open directory");
//     return;
//   }
//   if (!root.isDirectory()) {
//     Serial.println(" - not a directory");
//     return;
//   }

//   File file = root.openNextFile();
//   while (file) {
//     if (file.isDirectory()) {
//       Serial.print("  DIR : ");
//       Serial.println(file.name());
//       if (levels) {
//         listDir(fs, file.name(), levels - 1);
//       }
//     } else {
//       Serial.print("  FILE: ");
//       Serial.print(file.name());
//       Serial.print("\tSIZE: ");
//       Serial.println(file.size());
//     }
//     file = root.openNextFile();
//   }
// }

// void readImage(fs::FS &fs, const char * path) {
//   Serial.printf("Reading Image: %s\r\n", path);
//   File file = fs.open(path);
//   if (!file || file.isDirectory()) {
//     Serial.println("- failed to open file for reading");
//     return;
//   }

//   Serial.println("- read from file:");
//   while (file.available()) {
//     Serial.println(file.read(), HEX);
//   }
// }



// void deleteFile(fs::FS &fs, const char * path){
//     Serial.printf("Deleting file: %s\r\n", path);
//     if(fs.remove(path)){
//         Serial.println("- file deleted");
//     } else {
//         Serial.println("- delete failed");
//     }
// }

// void setup()
// {
// }