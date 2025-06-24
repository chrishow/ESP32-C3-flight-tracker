#include <sys/time.h>
#include "weather_manager.h"
#include "display_manager.h"
#include <Arduino.h>

HTTPClient WeatherManager::httpClient;

bool WeatherManager::fetchData()
{
    String API_URL = "https://api.open-meteo.com/v1/forecast?latitude=28.652107&longitude=-17.7754653&current=temperature_2m,relative_humidity_2m";

    Serial.println("Attempting to fetch data from URL: " + String(API_URL));
    httpClient.begin(API_URL);
    int httpCode = httpClient.GET();
    Serial.print("HTTP GET request sent. Response code: ");
    Serial.println(httpCode);

    if (httpCode > 0)
    {
        String payload = httpClient.getString();
        httpClient.end();
        JsonDocument doc;

        DeserializationError error = deserializeJson(doc, payload);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return false;
        }
        Serial.println("JSON parsing successful.");
        DisplayManager::setWeatherInfo(
            doc["current"]["temperature_2m"].as<String>(),
            doc["current"]["relative_humidity_2m"].as<String>());
        return true;
    }
    else
    {
        Serial.print("HTTP GET request failed, error: ");
        Serial.println(httpClient.errorToString(httpCode).c_str());
        httpClient.end();
        return false;
    }
}
