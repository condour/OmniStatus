#include <Arduino.h>
#define ESP32_RTOS
#include "credentials.h"
#include "OTA.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Fetch.h"
#include <Arduino_JSON.h>
#include <Wire.h>
#include <hd44780.h>
#include "GarageHelper.h"
// main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <TimeLib.h>
#include "SpotifyHelper.h"
#include "DisplayHelper.h"
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif
String clearString = "                ";
hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip
struct tm timeinfo;
SpotifyHelper spotifyHelper;
GarageHelper garageHelper;
DisplayHelper displayHelper;
// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

int garageStatus = -1;

/* FUNCTION DEFINITIONS */
void TaskBlink(void *pvParameters);
void keepWiFiAlive(void *pvParameters);
void checkGarageStatus(void *pvParameters);
void setClock();

int songOffset = 0;

void render(void *pvParameters);
void updateClock(void *pvParameters);
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print("WiFi Failed!\n");
    ESP.restart();
  }
  setClock();
}


void checkToggle(void *pvParameters);


struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

Button button1 = {25, 0, false};
void IRAM_ATTR isr() {
  button1.numberKeyPresses += 1;
}
// Not sure if WiFiClientSecure checks the validity date of the certificate.
// Setting clock just to be sure...
void setClock() {
  configTime(0, 0, "pool.ntp.org");

  Serial.print(F("Waiting for NTP time sync: "));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();

  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

void getNowPlaying(void *pvParameters) {
  spotifyHelper.getNowPlaying();
}


// the setup function runs once when you press reset or power the board
void setup() {

  // initialize TelnetStream communication at 115200 bits per second:
  int lcd_status;
  lcd_status = lcd.begin(LCD_COLS, LCD_ROWS);
  setupOTA("OmniStatus", ssid, wifi_password);
  Serial.begin(115200);
  displayHelper.init();
  delay(500);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);

  //  connectWiFi();

  /* LAUNCH TASKS */
  xTaskCreatePinnedToCore(
    getNowPlaying,
    "getNowPlaying",
    10000,
    NULL,
    1,
    NULL,
    ARDUINO_RUNNING_CORE
  );
  xTaskCreatePinnedToCore(
    render,
    "render",
    2048,
    NULL,
    1,
    NULL,
    ARDUINO_RUNNING_CORE
  );

  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    TaskBlink
    ,  "TaskBlink"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(
    updateClock
    ,  "updateClock"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(
    keepWiFiAlive
    ,  "TaskKeepWifiAlive"
    ,  3072  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL
    ,  ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    checkGarageStatus,
    "Check Garage Status",
    5000,
    NULL,
    1,
    NULL,
    ARDUINO_RUNNING_CORE
  );
  xTaskCreatePinnedToCore(
    checkToggle,
    "Check Toggle Status",
    5000,
    NULL,
    1,
    NULL,
    ARDUINO_RUNNING_CORE
  );
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void checkToggle(void *pvParameters) {
  for (;;) {
    if (button1.numberKeyPresses > 0) {
      Serial.print("TOGGLE at: ");
      Serial.println(button1.numberKeyPresses);
      button1.numberKeyPresses = 0;
      garageHelper.toggle();

    }
    vTaskDelay(2000);
  }
}

void updateClock(void *pvParameters) {
  for (;;) {

    vTaskDelay(1000);
  }
}

 void render(void *pvParameters) {
  for (;;) {
    String lastsong = spotifyHelper.lastsong;
    displayHelper.showSpotifyInfo(spotifyHelper.lastsong);
    vTaskDelay(5000);
  }
}

void checkGarageStatus(void *pvParameters) {
  for (;;) {
    garageStatus = garageHelper.getStatus();
    vTaskDelay(5000);
  }
}

void TaskBlink(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  /*
    Blink
    Turns on an LED on for one second, then off for one second, repeatedly.

    If you want to know what pin the on-board LED is connected to on your ESP32 model, check
    the Technical Specs of your board.
  */

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay(250);  // one tick delay (15ms) in between reads for stability
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay(1750);  // one tick delay (15ms) in between reads for stability
  }
}

/**
   Task: monitor the WiFi connection and keep it alive!

   When a WiFi connection is established, this task will check it every 10 seconds
   to make sure it's still alive.

   If not, a reconnect is attempted. If this fails to finish within the timeout,
   the ESP32 will wait for it to recover and try again.
*/
void keepWiFiAlive(void * pvParameters) {
  for (;;) {
    ArduinoOTA.handle();
    vTaskDelay(500);
  }
}
