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
}

void DisplayManager::displayWiFiStrength()
{
    int iconWidth = 22;
    int iconHeight = 8;
    int iconX = SCREEN_WIDTH - iconWidth;
    int iconY = 5;

    // tft.fillRect(iconX, iconY, iconWidth, iconHeight, ST77XX_BLACK);

    if (!FtWiFiManager::isConnected())
    {
        tft.setTextSize(1);
        tft.setTextColor(ST77XX_GREEN);
        tft.setCursor(iconX + (iconWidth - 5) / 2, iconY + (iconHeight - 8) / 2);
        tft.print("X");
        return;
    }

    long rssi = FtWiFiManager::getRSSI();
    int bars = 0;
    if (rssi >= -55)
        bars = 4;
    else if (rssi >= -65)
        bars = 3;
    else if (rssi >= -75)
        bars = 2;
    else
        bars = 1;

    int barWidth = 2;
    int barSpacing = 2;
    int totalBarsWidth = (4 * barWidth) + (3 * barSpacing);
    int currentX = iconX + (iconWidth - totalBarsWidth) / 2;

    // Draw bars from shortest to tallest (ascending height)
    for (int i = 1; i <= 4; i++)
    {
        int barHeight = i * 2;
        int barY = iconY + (8 - barHeight);

        if (bars >= i)
        {
            tft.fillRect(currentX, barY, barWidth, barHeight, ST77XX_GREEN);
        }
        else
        {
            tft.fillRect(currentX, barY, barWidth, barHeight, ST77XX_BLACK);
        }
        currentX += (barWidth + barSpacing);
    }
}

void DisplayManager::drawError(const char *message)
{

    tft.fillScreen(ST77XX_RED);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setFont(&DSEG14Modern_Bold18pt7b);

    // Display error message
    tft.setCursor(3, 50);
    tft.print(message);

    Serial.println("=== Error Displayed ===");
}

void DisplayManager::displayTime()
{
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
    // Erase any existing time string

    tft.drawRect(0, 3, 128, 127, ST77XX_GREEN);
    tft.setTextSize(1);
    tft.setFont(&DSEG14Modern_Bold18pt7b);

    // Time
    String currentTime = getCurrentTimeString();
    if (currentTime != currentTimeString && currentTime != "00:00")
    {
        // Erase current time string
        tft.setCursor(3, 55);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(currentTimeString);
        currentTimeString = currentTime;

        tft.setCursor(3, 55);
        tft.setTextColor(ST77XX_GREEN);
        tft.print(currentTime);
        Serial.printf("Current time: %s\n", currentTime.c_str());
    }

    // Temp
    tft.setFont(&FreeMonoBold12pt7b);
    if (currentTemperature != newTemperature && !newTemperature.isEmpty())
    {
        // Erase current temperature string
        tft.setCursor(3, 85);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(currentTemperature + "C");
        currentTemperature = newTemperature;

        tft.setCursor(3, 85);
        tft.setTextColor(ST77XX_GREEN);
        tft.setCursor(3, 85);
        tft.print(currentTemperature + "C");
    }

    // Humidity
    if (currentHumidity != newHumidity && !newHumidity.isEmpty())
    {
        // Erase current humidity string
        tft.setCursor(3, 105);
        tft.setTextColor(ST77XX_BLACK);
        tft.print(currentHumidity + "%");
        currentHumidity = newHumidity;

        tft.setTextColor(ST77XX_GREEN);
        tft.setCursor(3, 105);
        tft.print(currentHumidity + "%");
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

    if (flightData.isNull() || !flightData["callsign"].is<String>() ||
        flightData["callsign"].as<String>() == "null" ||
        flightData["callsign"].as<String>().isEmpty())
    {
        Serial.println("No flight data to display or callsign is null/empty.");
        if (currentFlightNumber != "")
        {
            clearScreen();
        }
        currentFlightNumber = "";
        // Unset time and weather info so they will be redrawn
        currentHumidity = "";
        currentTemperature = "";
        currentTimeString = "";
        return;
    }

    Serial.println("Updating display with flight data...");

    String airline = getValueOrQuestion(flightData, "airlineIcao");
    String flightNumber = getValueOrQuestion(flightData, "number");
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
    if (currentFlightNumber != flightNumber)
    {
        clearScreen();
    }
    currentFlightNumber = flightNumber;

    tft.drawRect(0, 3, 128, 127, ST77XX_YELLOW);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.setFont(&DSEG14Modern_Bold18pt7b);

    // Destination
    tft.setCursor(3, 55);
    tft.print(airport);

    // Aircraft
    tft.setFont(&FreeMonoBold12pt7b);
    tft.setCursor(5, 85);
    tft.print(aircraft);
    tft.setCursor(5, 105);
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