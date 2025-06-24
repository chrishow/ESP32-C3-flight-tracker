#include <Arduino.h>
#include <ArduinoJson.h>
#include "display_manager.h"
#include "ft_wifi_manager.h"
#include "flight_data_manager.h"
#include "weather_manager.h"

bool firstRun = true;

bool isNightHours()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);
    int currentHour = timeinfo->tm_hour;

    // Night hours: 22:00 to 07:00 (22, 23, 0, 1, 2, 3, 4, 5, 6)
    return (currentHour >= 22 || currentHour < 7);
}

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
}

void getLatestFlightData()
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

void getWeatherData()
{
    if (FtWiFiManager::isConnected())
    {
        Serial.println("Fetching weather data...");
        WeatherManager::fetchData();
    }
}

void loop()
{
    // Simple heartbeat with backlight toggle
    static unsigned long lastDataFetch = 0;
    static unsigned long lastWiFiSignalUpdate = 0;
    static unsigned long lastWeatherUpdate = 0;
    static bool backlightState = true;

    // Dynamic flight data update interval based on time of day
    int flightDataUpdateInterval;
    if (isNightHours())
    {
        flightDataUpdateInterval = 3600000; // 1 hour (3600 seconds * 1000 ms)
    }
    else
    {
        flightDataUpdateInterval = 20000; // 20 seconds
    }

    if (firstRun || millis() - lastWeatherUpdate > 600000) // Every 10 minutes
    {
        getWeatherData();
    }

    if (firstRun || millis() - lastWiFiSignalUpdate > 100)
    {
        // Serial.println("Updating WiFi signal strength");
        DisplayManager::displayTime(); // Update time display
        DisplayManager::displayWiFiStrength();
        lastWiFiSignalUpdate = millis();
    }

    if (firstRun || millis() - lastDataFetch > flightDataUpdateInterval)
    {
        getLatestFlightData();

        lastDataFetch = millis();
        DisplayManager::displayWiFiStrength();
    }

    firstRun = false;
}
