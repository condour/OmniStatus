#include "DisplayHelper.h"
// see GxEPD2_wiring_examples.h for wiring suggestions and examples

// base class GxEPD2_GFX can be used to pass references or pointers to the display instance as parameter, uses ~1.2k more code
// enable or disable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>

#include <GxEPD2_BW.h>
// #include <GxEPD2_3C.h>
// #include <GxEPD2_7C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Regexp.h>


// select the display constructor line in one of the following files (old style):
//#include "GxEPD2_display_selection.h"
//#include "GxEPD2_display_selection_added.h"
//#include "GxEPD2_display_selection_more.h" // private

// or select the display class and display driver class in the following file (new style):
#include "display_support/GxEPD2_display_selection_new_style.h"

#if defined(ESP8266) || defined(ESP32)
#include <StreamString.h>
#define PrintString StreamString
#else
class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};
#endif

int16_t MARGIN = 10;
int16_t LINE_SPACING = 5;
int CHARS_PER_LINE = 18;

DisplayHelper::DisplayHelper(void) {
  SpotifyObject currentSpotifyObject = {"","",""};
}

void DisplayHelper::init(void) {
  isClosed = true;
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setFullWindow();
  display.clearScreen();
  xCurs = 0;
  yCurs = MARGIN;
}
void DisplayHelper::setSpotifyInfo(SpotifyObject spotifyObject) {
  if(currentSpotifyObject.song != spotifyObject.song) {
    currentSpotifyObject = spotifyObject;
    render(true);
  }
}
void DisplayHelper::setGarageStatus(bool garageStatus) {
  if(isClosed != garageStatus) {
    isClosed = garageStatus;
    render(true);
  }
}

void DisplayHelper::printLine(String str, bool shouldClear) {
  int16_t lastYCurs = yCurs;
  if(shouldClear) {
    yCurs = MARGIN;
  }
  display.setRotation(2);
  Serial.println("coming into printLine with " + String(yCurs));
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(str, xCurs, yCurs, &tbx, &tby, &tbw, &tbh);
  int16_t realX = round((display.width() * 0.5) - (tbw * 0.5));
  Serial.println(String(tbx) + " " +String(tby) + " " +String(tbw) + " " +String(tbh));
  Serial.println(realX);
  display.setPartialWindow(tbx, tby, display.width(), shouldClear ? lastYCurs : tbh);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(realX, yCurs);
    display.print(str);
  }
  while (display.nextPage());
  yCurs += tbh + LINE_SPACING;
}

void DisplayHelper::breakStringAndCenter(String str) {
  int counter = 0;
  int lastWord = 0;
  int lastLineEnd = 0;
  int lineLength = 0;
  while(counter <= str.length()) {
    int foo[] = {counter, lastWord, lastLineEnd, lineLength};
    if(str.charAt(counter) == ' ') {
      lastWord = counter;
    }
    if(String(str.charAt(counter)) == "(" || String(str.charAt(counter)) == "-") {
      // truncate here

    }

    if(lineLength >= CHARS_PER_LINE) {
      printLine(str.substring(lastLineEnd, lastWord), false);
      lastLineEnd = lastWord+1;
      lineLength = 0;
    } else {
      lineLength++; 
    }
    counter++;
  }
  printLine(str.substring(lastLineEnd, counter), false);
}

void DisplayHelper::printDivider() {
  yCurs += LINE_SPACING * 2;
  display.setPartialWindow(0, yCurs, display.width(), 1);
  display.firstPage();
    do
  {
  display.drawFastHLine(0, yCurs, display.width(), GxEPD_BLACK);
  } while (display.nextPage());
  yCurs += LINE_SPACING * 2;
  Serial.println("coming out of printDivider with " + String(yCurs));
}

void DisplayHelper::render(bool doFull) {
  if(doFull) {
    display.setFullWindow();
    display.firstPage();
    do {

    display.clearScreen();
    }  while (display.nextPage());
    yCurs = MARGIN;
  }
  breakStringAndCenter(currentSpotifyObject.song);
  printDivider();
  breakStringAndCenter(currentSpotifyObject.album);
  printDivider();
  breakStringAndCenter(currentSpotifyObject.artists);
}
