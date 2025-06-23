#include <Arduino.h>
#include "display_manager.h"
#include "ft_wifi_manager.h"
#include "flight_data_manager.h"
#include <ArduinoJson.h>

bool firstRun = true;

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

    // DisplayManager::drawTime();
    // drawFlight("LPA", "AT76", "NT307");
}

void getLatestData()
{
    if (FtWiFiManager::isConnected())
    {
        Serial.println("Fetching latest flight data...");

        FlightDataManager::fetchData();
        Serial.println("Flight data updated.");
    }
    else
    {
        Serial.println("WiFi not connected, cannot fetch flight data.");
        DisplayManager::drawError("No WiFi Connection!");
    }
}

void loop()
{

    // Simple heartbeat with backlight toggle
    static unsigned long lastDataFetch = 0;
    static unsigned long lastWiFiSignalUpdate = 0;
    static bool backlightState = true;

    if (firstRun || millis() - lastWiFiSignalUpdate > 100)
    {
        // Serial.println("Updating WiFi signal strength");
        DisplayManager::displayTime(); // Update time display
        DisplayManager::displayWiFiStrength();
        lastWiFiSignalUpdate = millis();
    }

    if (firstRun || millis() - lastDataFetch > 20000) // every 20s
    {
        Serial.println("Getting flight data");
        getLatestData();

        lastDataFetch = millis();
        DisplayManager::displayWiFiStrength();
    }

    firstRun = false;
}
