#include <Arduino.h>
#include <ArduinoJson.h>
#include "display_manager.h"
#include "ft_wifi_manager.h"
#include "flight_data_manager.h"
#include "weather_manager.h"

// Timing constants (in milliseconds)
const unsigned long NIGHT_FLIGHT_UPDATE_INTERVAL = 3600000; // 1 hour during night
const unsigned long DAY_FLIGHT_UPDATE_INTERVAL = 20000;     // 20 seconds during day
const unsigned long WEATHER_UPDATE_INTERVAL = 600000;       // 10 minutes
const unsigned long DISPLAY_REFRESH_INTERVAL = 100;         // 100ms for time/WiFi updates
const unsigned long NIGHT_START_HOUR = 22;                  // 10 PM
const unsigned long NIGHT_END_HOUR = 7;                     // 7 AM

// Application state
struct AppState
{
    bool isInitialized = false;
    unsigned long lastFlightUpdate = 0;
    unsigned long lastWeatherUpdate = 0;
    unsigned long lastDisplayRefresh = 0;
};

AppState appState;

bool isNightHours()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *timeinfo = localtime(&tv.tv_sec);
    int currentHour = timeinfo->tm_hour;

    return (currentHour >= NIGHT_START_HOUR || currentHour < NIGHT_END_HOUR);
}

unsigned long getFlightUpdateInterval()
{
    return isNightHours() ? NIGHT_FLIGHT_UPDATE_INTERVAL : DAY_FLIGHT_UPDATE_INTERVAL;
}

void initializeSystem()
{
    Serial.begin(9600);
    DisplayManager::initDisplay();
    Serial.println("Display initialized");

    if (!FtWiFiManager::connect())
    {
        Serial.println("Failed to connect to WiFi. Halting.");
        DisplayManager::drawError("WiFi Connection Failed!");
        while (true)
        {
            delay(1000); // Prevent watchdog reset
        }
    }

    Serial.println("System initialization complete");
}

void setup()
{
    initializeSystem();
}

bool shouldUpdateFlight()
{
    return !appState.isInitialized ||
           (millis() - appState.lastFlightUpdate > getFlightUpdateInterval());
}

bool shouldUpdateWeather()
{
    return !appState.isInitialized ||
           (millis() - appState.lastWeatherUpdate > WEATHER_UPDATE_INTERVAL);
}

bool shouldRefreshDisplay()
{
    return !appState.isInitialized ||
           (millis() - appState.lastDisplayRefresh > DISPLAY_REFRESH_INTERVAL);
}

void updateFlightData()
{
    if (!FtWiFiManager::isConnected())
    {
        Serial.println("WiFi not connected, cannot fetch flight data.");
        DisplayManager::drawError("No WiFi Connection!");
        return;
    }

    Serial.println("Fetching latest flight data...");
    FlightDataManager::fetchData();
    appState.lastFlightUpdate = millis();
    Serial.println("Flight data updated.");
}

void updateWeatherData()
{
    if (!FtWiFiManager::isConnected())
    {
        Serial.println("WiFi not connected, cannot fetch weather data.");
        return;
    }

    Serial.println("Fetching weather data...");
    WeatherManager::fetchData();
    appState.lastWeatherUpdate = millis();
}

void refreshDisplay()
{
    DisplayManager::displayTime();
    DisplayManager::displayWiFiStrength();
    appState.lastDisplayRefresh = millis();
}

void loop()
{
    // Update weather data periodically
    if (shouldUpdateWeather())
    {
        updateWeatherData();
    }

    // Update display (time and WiFi signal) frequently
    if (shouldRefreshDisplay())
    {
        refreshDisplay();
    }

    // Update flight data based on time of day
    if (shouldUpdateFlight())
    {
        updateFlightData();
        // Refresh display after flight data update
        refreshDisplay();
    }

    // Mark system as initialized after first cycle
    if (!appState.isInitialized)
    {
        appState.isInitialized = true;
        Serial.println("Application fully initialized");
    }

    // Small delay to prevent excessive CPU usage
    delay(10);
}
