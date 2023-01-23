// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// Display Library based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Display ePaper for Arduino: https://forum.arduino.cc/index.php?topic=436411.0


#define GxEPD2_154_D67_IS_BW true

#if defined(GxEPD2_DISPLAY_CLASS) && defined(GxEPD2_DRIVER_CLASS)
#define IS_GxEPD2_DRIVER(c, x) (c##x)
#define IS_GxEPD2_DRIVER_BW(x) IS_GxEPD2_DRIVER(x, _IS_BW)
#define IS_GxEPD2_DRIVER_3C(x) IS_GxEPD2_DRIVER(x, _IS_3C)
#define IS_GxEPD2_DRIVER_7C(x) IS_GxEPD2_DRIVER(x, _IS_7C)
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS) && IS_GxEPD2_DRIVER_3C(GxEPD2_DRIVER_CLASS)
#error "GxEPD2_BW used with 3-color driver class"
#endif
#if IS_GxEPD2_3C(GxEPD2_DISPLAY_CLASS) && IS_GxEPD2_DRIVER_BW(GxEPD2_DRIVER_CLASS)
#error "GxEPD2_3C used with b/w driver class"
#endif
#if !IS_GxEPD2_DRIVER_BW(GxEPD2_DRIVER_CLASS) && !IS_GxEPD2_DRIVER_3C(GxEPD2_DRIVER_CLASS) && !IS_GxEPD2_DRIVER_7C(GxEPD2_DRIVER_CLASS)
#error "neither BW nor 3C nor 7C kind defined for driver class (error in GxEPD2_selection_check.h)"
#endif

#endif
