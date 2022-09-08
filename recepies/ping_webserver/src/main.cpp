#include "heltec.h"
#include "WiFi.h"
#include "esp_app_trace.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
// #include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#define XSTR(x) #x
#define STR(x) XSTR(x)

const bool DISPLAY_ON = true;
const bool LORA_ON = false;
const bool SERIAL_ON = true;
const bool PABOOST_ON = true;
const long RAND = 470E6;

const char *NAME = STR(HOSTNAME);
const char *SSID = STR(WIFI_SSID);
const char *PASSWORD = STR(WIFI_PASS);

const char *SERVER_IP = STR(WEB_SERVER_IP);
const char *PORT = STR(WEB_PORT);
// const long PORT = atol(STR(WEB_PORT));
const char *PATH = STR(WEB_PATH);

// static const char *REQUEST = "GET " PATH " HTTP/1.0\r\n"
//                              "Host: " SERVER ":" PORT "\r\n"
//                              "User-Agent: esp-idf/1.0 esp32\r\n"
//                              "\r\n";

static const String REQUEST = "GET " + String(PATH); // + "HTTP/1.0\r\n" +
                              // "Host: " + String(SERVER_IP) + ":" + String(PORT) + "\r\n"
                                                                                  // "User-Agent: esp-idf/1.0 esp32\r\n"
                                                                                  // "\r\n";

bool successful_ping = false;

static const char *TAG = "UploadFile";

void WIFISetUp(void)
{
  Serial.print("\nWIFI: Connecting to " + String(SSID));

  Heltec.display->clear();

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  // WiFi.setAutoConnect(true);
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
}

// char cs[REQUEST.length()+ 1];
// strcpy(cs, REQUEST.c_str());

// if (write(s, cs, strlen(cs)) < 0)

void http_get_task(void *pvParameters)
{
  struct addrinfo hints;// = {
      // .ai_socktype = SOCK_STREAM,
      // .ai_family = AF_INET,
  // };
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;
  struct addrinfo *res;
  struct in_addr *addr;
  int s, r;
  char recv_buf[64];

  while (!successful_ping)
  {
    int err = getaddrinfo(SERVER_IP, PORT, &hints, &res);

    if (err != 0 || res == NULL)
    {
      Serial.print("\nDNS lookup failed err=" + String(err));
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }

    /* Code to print the resolved IP.
       Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
    addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    Serial.print("\nDNS lookup succeeded. IP=" + String(inet_ntoa(*addr)));

    s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0)
    {
      Serial.print("\nFailed to allocate socket.");
      freeaddrinfo(res);
      vTaskDelay(1000 / portTICK_PERIOD_MS);
      continue;
    }
    Serial.print("\nAllocated socket");

    if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
    {
      Serial.print("\nSocket connect failed errno=" + String(errno));
      close(s);
      freeaddrinfo(res);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      continue;
    }

    Serial.print("\nConnected");
    freeaddrinfo(res);

    char cs[REQUEST.length() + 1];
    strcpy(cs, REQUEST.c_str());

    if (write(s, cs, strlen(cs)) < 0)
    {
      // if (write(s, REQUEST, strlen(REQUEST)) < 0) {
      Serial.print("\nSocket send failed");
      close(s);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      continue;
    }
    successful_ping = true;
    Serial.print("\nSocket send success");

    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                   sizeof(receiving_timeout)) < 0)
    {
      Serial.print("\nFailed to set socket receiving timeout");
      close(s);
      vTaskDelay(4000 / portTICK_PERIOD_MS);
      continue;
    }
    Serial.print("\nSet socket receiving timeout success");

    /* Read HTTP response */
    do
    {
      bzero(recv_buf, sizeof(recv_buf));
      r = read(s, recv_buf, sizeof(recv_buf) - 1);
      for (int i = 0; i < r; i++)
      {
        putchar(recv_buf[i]);
      }
    } while (r > 0);

    Serial.print("\nDone reading from socket.");
    close(s);
    for (int countdown = 10; countdown >= 0; countdown--)
    {
      Serial.print("\nCountdown: " + String(countdown));
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    Serial.print("\nStarting again!");
  }
}

void setup()
{
  // put your setup code here, to run once:
  Heltec.begin(DISPLAY_ON, LORA_ON, SERIAL_ON, PABOOST_ON, RAND);

  WIFISetUp();

  xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
}

void loop()
{
  // put your main code here, to run repeatedly:
  delay(200);

  if (WiFi.status() == WL_CONNECTED)
  {
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "Connected: " + String(SSID));
    Heltec.display->drawString(0, 20, "Good morning, Dave.");
  }
  else
  {
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, "I'm sorry Dave.\nI can't do that.");
  }

  if (successful_ping)
  {
    Heltec.display->clear();
    Heltec.display->drawString(0, 30, "Ping ... Pong");
  }
  else
  {
    // http_get_task(4096, 0);
    Heltec.display->drawString(0, 40, "Ping " + String(SERVER_IP) + ":" + String(PORT) + "/" + String(PATH));
  }

  Heltec.display->display();
}
