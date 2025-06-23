#include <Arduino.h>
#include "display_manager.h"
#include "ft_wifi_manager.h"

void setup()
{
    Serial.begin(9600);
    DisplayManager::initDisplay();
    Serial.println("Display initialized");

    if (!FtWiFiManager::connect())
    {
        Serial.println("Failed to connect to WiFi. Halting.");
        DisplayManager::drawError("WiFi Connection Failed!");
        while (true)
            ;
    }

    DisplayManager::drawTime();
    // drawFlight("LPA", "AT76", "NT307");
}

void loop()
{
    // Get flight information

    // Simple heartbeat with backlight toggle
    static unsigned long lastHeartbeat = 0;
    static bool backlightState = true;

    if (millis() - lastHeartbeat > 3000)
    {
        Serial.println("Heartbeat - Adafruit ST7735 system running normally");

        lastHeartbeat = millis();
        DisplayManager::displayWiFiStrength();
    }
}
