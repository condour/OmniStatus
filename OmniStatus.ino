#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "credentials.h"
#include <Arduino_JSON.h>
#include "Fetch.h"
#include <Wire.h>
#include <hd44780.h>                       // main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // i2c expander i/o class header
#include <TimeLib.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

hd44780_I2Cexp lcd; // declare lcd object: auto locate & auto config expander chip
struct tm timeinfo;
String access_token;
// LCD geometry
const int LCD_COLS = 16;
const int LCD_ROWS = 2;
String lastsong = "";
String garageStatus = "Initializing...";
String thissong = "";
String clearString = "                ";
int songOffset = 0;

void render(void *pvParameters);
void updateClock(void *pvParameters);
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wifi_password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    ESP.restart();
  }
  setClock();
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



// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  int lcd_status;
  lcd_status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (lcd_status) // non zero status means it was unsuccesful
  {
    // hd44780 has a fatalError() routine that blinks an led if possible
    // begin() failed so blink error code using the onboard LED if possible
    hd44780::fatalError(lcd_status); // does not return
  }
  delay(500);
  connectWiFi();
  getNowPlaying();
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
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void updateClock(void *pvParameters) {
  for (;;) {
    Serial.println("should update clock here");
    vTaskDelay(1000);
  }
}

String hitAPIWithToken(String token) {
  RequestOptions options;
  options.method = "GET";
  options.headers["Authorization"] = "Bearer " + token;
  Response response = fetch("https://api.spotify.com/v1/me/player/currently-playing", options);
  JSONVar resultJSON = JSON.parse(response.text());
  JSONVar item = resultJSON["item"];
  JSONVar song = item["name"];
  JSONVar album = item["album"]["name"];
  // Serial.println(album);
  Serial.println(item["artists"]);
  return JSON.stringify(album) + " " + JSON.stringify(song);
}
void refreshToken() {
  RequestOptions options;
  options.method = "GET";
  options.headers["Content-Type"] = "application/x-www-form-urlencoded";
  options.headers["Authorization"] = BASIC_AUTH;
  options.method = "POST";

  options.body = refresh_token;
  options.headers["Content-Length"] = options.body.text().length();
  Serial.println("going to fetch");
  Response response = fetch("https://accounts.spotify.com/api/token", options);
  JSONVar resultJSON = JSON.parse(response.text());
  access_token = resultJSON["access_token"];
}

void getNowPlaying(void *pvParameters) {
  for (;;) {
    // use token to hit api
    try {
      if (access_token.length() > 0) {
        thissong = hitAPIWithToken(access_token);
        if (thissong != lastsong) {
          songOffset = 0;
          lastsong = thissong;
        }
      } else {
        refreshToken();
      }

    } catch (int whichError) {
      Serial.println("i'm in the catch");
      Serial.println(whichError);
      // if token fails, refresh token then reuse
      refreshToken();
    }
    // display song

    vTaskDelay(15000);
  }
}

void render(void *pvParameters) {
  for (;;) {
    lcd.setCursor(0, 0);

    if (garageStatus == "Closed") {
      lcd.write("            Shut");
    } else {
      lcd.write("            Open");
    }
    // add spaces to beginning and end
    String displaySongString = clearString + lastsong + clearString;
    String stringToPrint = displaySongString.substring(songOffset, songOffset + 16);
    Serial.println(stringToPrint);
    lcd.setCursor(0, 1);
    lcd.print(stringToPrint);
    if (songOffset < displaySongString.length() - 1) {
      songOffset++;
    } else {
      Serial.println("RESET OFFSET");

      songOffset = 0;
    }

    vTaskDelay(200);
  }
}

void checkGarageStatus(void *pvParameters) {
  for (;;) {
    HTTPClient http;

    // Your IP address with path or Domain name with URL path
    http.begin("http://garage.door/");

    // Send HTTP POST request
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0) {
      payload = http.getString();
      JSONVar resultJSON = JSON.parse(payload);
      if (JSON.typeof(resultJSON) == "undefined") {
        Serial.println("PARSE FAILURE");
      }
      garageStatus = resultJSON["status"];

    }

    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
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
    Serial.println("checking wifi");
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    } else {
      vTaskDelay(60000);
    }
  }
}
