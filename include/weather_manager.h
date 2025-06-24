#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <ArduinoJson.h>
#include <HTTPClient.h>

class WeatherManager
{
public:
    static bool fetchData();

private:
    static HTTPClient httpClient;
};

#endif // WEATHER_MANAGER_H
