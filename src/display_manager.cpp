#include <Arduino.h>
#include "display_manager.h"
#include "ft_wifi_manager.h"

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
bool isDisplayInitialized = false;

void DisplayManager::initDisplay()
{
    if (!isDisplayInitialized)
    {
        // Initialize the display
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

void DisplayManager::drawTime()
{

    tft.drawRect(0, 3, 128, 127, ST77XX_GREEN);
    tft.setTextColor(ST77XX_GREEN);
    tft.setTextSize(1);
    tft.setFont(&DSEG14Modern_Bold18pt7b);

    // Time
    tft.setCursor(3, 55);
    tft.print("23:45");

    // Temp
    tft.setFont(&FreeMonoBold12pt7b);
    tft.setCursor(3, 85);
    tft.print("25.0C");
    tft.setCursor(3, 105);
    tft.print("55%");
}

void DisplayManager::drawFlight(const char *airport, const char *aircraft, const char *flightNumber)
{

    // tft.drawCircle(64, 64, 30, ST77XX_WHITE);
    tft.drawRect(0, 3, 128, 127, ST77XX_YELLOW);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setTextSize(1);
    tft.setFont(&DSEG14Modern_Bold18pt7b);

    // Destination
    tft.setCursor(3, 55);
    tft.print(airport);

    // Aircraft
    tft.setFont(&FreeMonoBold12pt7b);
    tft.setCursor(3, 85);
    tft.print(aircraft);
    tft.setCursor(3, 105);
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