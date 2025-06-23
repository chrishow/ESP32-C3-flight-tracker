#include <sys/time.h>
#include "flight_data_manager.h"
#include "display_manager.h"
#include <Arduino.h>

const char *API_URL = "https://flighttrack.primesolid.com/testX";

HTTPClient FlightDataManager::httpClient;

bool FlightDataManager::fetchData()
{
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
            doc["weather"]["temperature"].as<String>(),
            doc["weather"]["humidity"].as<String>());

        // Check if flight data is actually available
        if (doc["flightDataAvailable"].is<bool>() && doc["flightDataAvailable"].as<bool>() == false)
        {
            Serial.println("No flight data available according to API.");
        }
        DisplayManager::displayFlightData(doc);

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
