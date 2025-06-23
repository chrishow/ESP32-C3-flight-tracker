#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "DSEG14Modern_Bold18pt7b.h"
// #include "DSEGWeather18pt7b.h"
#include <Fonts/FreeMonoBold12pt7b.h>

// Pin definitions for TTGO T-OI PLUS
#define TFT_CS 2
#define TFT_RST 5
#define TFT_DC 0
#define TFT_MOSI 4
#define TFT_SCLK 3
// #define TFT_BL 9 /* No backlight control on this device  */

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

class DisplayManager
{
public:
    static void initDisplay();
    static void clearScreen();
    static void drawTime();
    static void drawFlight(const char *airport, const char *aircraft, const char *flightNumber);
    static void displayAPInfo(const String &apName, const String &password, const String &ip);
    static void drawError(const char *message);
    static void displayWiFiStrength();

private:
};