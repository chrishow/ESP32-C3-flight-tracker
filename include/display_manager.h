#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ArduinoJson.h>

#include "DSEG14Modern_Bold18pt7b.h"
// #include "DSEGWeather18pt7b.h"
#include <Fonts/FreeMonoBold12pt7b.h>

// Pin definitions for ESP32-C3 with custom SPI pins
#define TFT_CS 2  // Chip select pin
#define TFT_RST 5 // Reset pin
#define TFT_DC 0  // Data/Command pin
// Custom SPI pins for this board:
#define TFT_MOSI 4 // MOSI pin (based on your board schematic)
#define TFT_SCLK 3 // SCLK pin (based on your board schematic)
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
    static void displayFlightData(const JsonDocument &flightData);
    static void displayTime();
    static void setWeatherInfo(const String &temperature, const String &humidity);

private:
    static String getValueOrQuestion(const JsonDocument &data, const char *key);
    static String getCurrentTimeString();
};