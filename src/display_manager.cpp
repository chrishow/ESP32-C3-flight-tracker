#include <Arduino.h>
#include <SPI.h>
#include "display_manager.h"
#include "ft_wifi_manager.h"
#include <sys/time.h>

// Initialize display using hardware SPI (CS, DC, RST pins only)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

bool isDisplayInitialized = false;
String currentFlightNumber = "";
String currentTimeString = "";
String currentTemperature = "";
String currentHumidity = "";
String newTemperature = "";
String newHumidity = "";
bool isInErrorState = false;
String currentErrorMessage = "";

void DisplayManager::initDisplay()
{
    if (!isDisplayInitialized)
    {
        // Initialize hardware SPI with custom pins
        SPI.begin(TFT_SCLK, -1, TFT_MOSI, -1); // (SCK, MISO, MOSI, SS)

        // Initialize the display with hardware SPI
        tft.initR(INITR_GREENTAB);    // Initialize with green tab settings
        tft.fillScreen(ST77XX_BLACK); // Clear the screen to black

        isDisplayInitialized = true;
    }
}

void DisplayManager::clearScreen()
{
    tft.fillScreen(ST77XX_BLACK);
    // Reset error state when screen is cleared
    isInErrorState = false;
    currentErrorMessage = "";
    // Reset display state variables so first update after clear doesn't try to erase non-existent text
    currentTimeString = "";
    currentTemperature = "";
    currentHumidity = "";
    Serial.println("Screen cleared - reset display state variables");
}

void DisplayManager::displayWiFiStrength()
{
    // Don't draw WiFi strength if we're in error state
    if (isInErrorState)
    {
        return;
    }

    int iconX = SCREEN_WIDTH - WIFI_ICON_WIDTH;
    int iconY = 5;

    // Choose color based on display mode: yellow for flight data, green for time display
    uint16_t wifiColor = (currentFlightNumber != "") ? ST77XX_YELLOW : ST77XX_GREEN;

    if (!FtWiFiManager::isConnected())
    {
        tft.setTextSize(1);
        tft.setFont();
        tft.setTextColor(wifiColor);
        tft.setCursor(iconX + (WIFI_ICON_WIDTH - 5) / 2, iconY + (WIFI_ICON_HEIGHT - 8) / 2);
        tft.print("X");
        return;
    }

    long rssi = FtWiFiManager::getRSSI();
    int bars = calculateWiFiBars(rssi);

    int totalBarsWidth = (4 * WIFI_BAR_WIDTH) + (3 * WIFI_BAR_SPACING);
    int startX = iconX + (WIFI_ICON_WIDTH - totalBarsWidth) / 2;

    drawWiFiBars(bars, startX, iconY, wifiColor);
}

void DisplayManager::drawError(const char *message)
{
    String messageStr = String(message);

    // Only redraw error if it's a new error message or we're not already in error state
    if (!isInErrorState || currentErrorMessage != messageStr)
    {
        tft.fillScreen(ST77XX_RED);
        tft.setTextColor(ST77XX_WHITE);
        tft.setTextSize(1);
        tft.setFont();

        // Display error message
        tft.setCursor(BORDER_OFFSET, 50);
        tft.print(message);

        isInErrorState = true;
        currentErrorMessage = messageStr;
        Serial.println("=== Error Displayed ===");
    }
}

void DisplayManager::clearError()
{
    if (isInErrorState)
    {
        isInErrorState = false;
        currentErrorMessage = "";
        clearScreen();
        Serial.println("=== Error State Cleared ===");
    }
}

void DisplayManager::displayTime()
{
    // Only clear error state if WiFi is connected (meaning we can successfully display time)
    if (FtWiFiManager::isConnected())
    {
        clearError();
    }

    if (currentFlightNumber == "")
    {
        drawTime();
    }
}

void DisplayManager::setWeatherInfo(const String &temperature, const String &humidity)
{
    newTemperature = temperature;
    newHumidity = humidity;
}

void DisplayManager::drawTime()
{
    // Don't draw time if we're in error state
    if (isInErrorState)
    {
        return;
    }

    drawBorderedRect(ST77XX_GREEN);
    tft.setTextSize(1);

    // Update time display
    String currentTime = getCurrentTimeString();
    if (currentTime != currentTimeString && currentTime != "00:00")
    {
        Serial.printf("Time update: old='%s' new='%s'\n", currentTimeString.c_str(), currentTime.c_str());
        drawTextWithErase(BORDER_OFFSET, TIME_Y_POS, currentTimeString, currentTime,
                          ST77XX_GREEN, &DSEG14ModernMini_Bold18pt7b);
        currentTimeString = currentTime;
        Serial.printf("Current time: %s\n", currentTime.c_str());
    }

    // Update temperature display
    if (currentTemperature != newTemperature && !newTemperature.isEmpty())
    {
        String oldTempText = currentTemperature.isEmpty() ? "" : currentTemperature + "C";
        String newTempText = newTemperature + "C";

        drawTextWithErase(BORDER_OFFSET, TEMP_Y_POS, oldTempText, newTempText,
                          ST77XX_GREEN, &FreeMonoBold12pt7b);
        currentTemperature = newTemperature;
    }

    // Update humidity display
    if (currentHumidity != newHumidity && !newHumidity.isEmpty())
    {
        String oldHumidityText = currentHumidity.isEmpty() ? "" : currentHumidity + "%";
        String newHumidityText = newHumidity + "%";

        drawTextWithErase(BORDER_OFFSET, HUMIDITY_Y_POS, oldHumidityText, newHumidityText,
                          ST77XX_GREEN, &FreeMonoBold12pt7b);
        currentHumidity = newHumidity;
    }
}

String DisplayManager::getValueOrQuestion(const JsonDocument &data, const char *key)
{
    if (data[key].is<String>())
    {
        String value = data[key].as<String>();
        if (value == "null" || value.isEmpty())
        {
            return "?";
        }
        return value;
    }
    return "?";
}

void DisplayManager::displayFlightData(const JsonDocument &flightData)
{
    // Only clear error state if WiFi is connected
    if (FtWiFiManager::isConnected())
    {
        clearError();
    }

    if (flightData.isNull() || !flightData["callsign"].is<String>() ||
        flightData["callsign"].as<String>() == "null" ||
        flightData["callsign"].as<String>().isEmpty())
    {
        Serial.println("No flight data to display or callsign is null/empty.");
        if (currentFlightNumber != "")
        {
            clearScreen();
            // Note: clearScreen() already resets time/weather variables
        }
        currentFlightNumber = "";
        return;
    }

    Serial.println("Updating display with flight data...");

    String airline = getValueOrQuestion(flightData, "airlineIcao");
    String flightNumber = getValueOrQuestion(flightData, "callsign");
    String origin = getValueOrQuestion(flightData, "originAirportIata");
    String destination = getValueOrQuestion(flightData, "destinationAirportIata");
    String aircraftCode = getValueOrQuestion(flightData, "aircraftCode");

    if (destination == "SPC")
    {
        destination = origin;
    }
    drawFlight(destination.c_str(), aircraftCode.c_str(), flightNumber.c_str());
}

void DisplayManager::drawFlight(const char *airport, const char *aircraft, const char *flightNumber)
{
    // Only clear screen when switching between different flights or from time to flight display
    if (currentFlightNumber != flightNumber)
    {
        Serial.printf("Flight change: '%s' -> '%s', clearing screen\n", currentFlightNumber.c_str(), flightNumber);
        clearScreen();
    }
    currentFlightNumber = flightNumber;

    drawBorderedRect(ST77XX_YELLOW);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);

    // Display airport destination
    tft.setFont(&DSEG14ModernMini_Bold18pt7b);
    tft.setCursor(BORDER_OFFSET, AIRPORT_Y_POS);
    tft.print(airport);

    // Display aircraft and flight number
    tft.setFont(&FreeMonoBold12pt7b);
    tft.setCursor(5, AIRCRAFT_Y_POS);
    tft.print(aircraft);
    tft.setCursor(5, FLIGHT_NUM_Y_POS);
    tft.print(flightNumber);
}

void DisplayManager::displayAPInfo(const String &apName, const String &password, const String &ip)
{
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_GREEN);

    tft.setCursor(0, 10);
    tft.println("WiFi Setup Mode");
    tft.println("");

    tft.print("AP: ");
    tft.println(apName);

    tft.print("Pass: ");
    tft.println(password);

    tft.print("IP: ");
    tft.println(ip);

    tft.println("");
    tft.println("Connect & browse to");
    tft.println("192.168.4.1");

    Serial.println("=== WiFi Setup Mode ===");
    Serial.printf("AP Name: %s\n", apName.c_str());
    Serial.printf("Password: %s\n", password.c_str());
    Serial.printf("IP: %s\n", ip.c_str());
    Serial.println("Connect and browse to 192.168.4.1");
}

String DisplayManager::getCurrentTimeString()
{
    // Set timezone to London (handles GMT/BST automatically)
    setenv("TZ", "GMT0BST,M3.5.0/1,M10.5.0", 1);
    tzset();

    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Convert to London local time (automatically handles GMT/BST)
    struct tm *timeinfo = localtime(&tv.tv_sec);

    // Format as HH:MM
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);

    return String(timeStr);
}

// Helper function to draw text with automatic erasing of old text
void DisplayManager::drawTextWithErase(int x, int y, const String &oldText, const String &newText,
                                       uint16_t color, const GFXfont *font)
{
    // Make local copies to prevent any potential string reference issues
    String oldTextCopy = oldText;
    String newTextCopy = newText;

    if (font)
    {
        tft.setFont(font);
    }

    // Erase old text by drawing it in black
    if (!oldTextCopy.isEmpty())
    {
        tft.setCursor(x, y);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(oldTextCopy);
        Serial.printf("Erasing old text: '%s' at (%d,%d)\n", oldTextCopy.c_str(), x, y);
    }

    // Draw new text in specified color
    if (!newTextCopy.isEmpty())
    {
        tft.setCursor(x, y);
        tft.setTextColor(color);
        tft.print(newTextCopy);
        Serial.printf("Drawing new text: '%s' at (%d,%d)\n", newTextCopy.c_str(), x, y);
    }
}

// Helper function to draw bordered rectangle
void DisplayManager::drawBorderedRect(uint16_t color)
{
    tft.drawRect(0, BORDER_OFFSET, SCREEN_WIDTH, SCREEN_HEIGHT - BORDER_OFFSET, color);
}

// Helper function to calculate WiFi signal bars
int DisplayManager::calculateWiFiBars(long rssi)
{
    if (rssi >= -55)
        return 4;
    else if (rssi >= -65)
        return 3;
    else if (rssi >= -75)
        return 2;
    else
        return 1;
}

// Helper function to draw WiFi signal bars
void DisplayManager::drawWiFiBars(int bars, int startX, int startY, uint16_t activeColor)
{
    for (int i = 1; i <= 4; i++)
    {
        int barHeight = i * 2;
        int barY = startY + (WIFI_ICON_HEIGHT - barHeight);
        uint16_t barColor = (bars >= i) ? activeColor : ST77XX_BLACK;

        tft.fillRect(startX, barY, WIFI_BAR_WIDTH, barHeight, barColor);
        startX += (WIFI_BAR_WIDTH + WIFI_BAR_SPACING);
    }
}