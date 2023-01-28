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
int CHARS_PER_LINE = 18;

DisplayHelper::DisplayHelper(void) {
  SpotifyObject currentSpotifyObject = {"","",""};
}

void DisplayHelper::init(void) {
  isClosed = true;
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  display.setFullWindow();
  display.clearScreen();
  xCurs = MARGIN;
  yCurs = MARGIN;
}
void DisplayHelper::setSpotifyInfo(SpotifyObject spotifyObject) {
  Serial.println("in display helper");
  Serial.println(currentSpotifyObject.song);
  if(currentSpotifyObject.song != spotifyObject.song || currentSpotifyObject.artists != spotifyObject.artists || currentSpotifyObject.album != currentSpotifyObject.album) {
    Serial.println("Should render");
    currentSpotifyObject = spotifyObject;
    render();
  }
}
void DisplayHelper::setGarageStatus(bool garageStatus) {
  if(isClosed != garageStatus) {
    isClosed = garageStatus;
    render();
  }
}

void DisplayHelper::printLine(String str) {
  display.setRotation(2);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(str, xCurs, yCurs, &tbx, &tby, &tbw, &tbh);
  
  // center bounding box by transposition of origin:
  uint16_t x = MARGIN;
  uint16_t y = MARGIN;
 //   uint16_t wh = FreeMonoBold9pt7b.yAdvance;
  // uint16_t wy = (display.height() * 3 / 4) - wh / 2;
  Serial.println(String(tbx) + " " +String(tby) + " " +String(tbw) + " " +String(tbh));
  display.setPartialWindow(tbx, tby, tbw, tbh);
  // display.setFullWindow();
  display.firstPage();

  Serial.println(display.width());
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(currentSpotifyObject.song);
  }
  while (display.nextPage());
}

void DisplayHelper::render() {
  array arr = [];
  currentSpotifyObject.split(arr&)
  printLine(currentSpotifyObject.song);
}
