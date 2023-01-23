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

DisplayHelper::DisplayHelper(void) {

}
void DisplayHelper::init(void) {
  Serial.println("got right before reset");
  display.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
  Serial.println("got beyond it");
}
void DisplayHelper::showSpotifyInfo(String str) {
    Serial.println("should be in showSpotifyInfo");
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
 
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
  Serial.println(tbx);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  Serial.println(display.width());
  do
  {
    Serial.print(x);
    Serial.print(" ");
    Serial.print(y + "\n");
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y);
    display.print(str);
  }
  while (display.nextPage());
}
