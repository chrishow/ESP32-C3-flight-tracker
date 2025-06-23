#ifndef FLIGHT_DATA_H
#define FLIGHT_DATA_H

#include <ArduinoJson.h>
#include <HTTPClient.h>

class FlightDataManager
{
public:
    static bool fetchData();

private:
    static HTTPClient httpClient;
};

#endif // FLIGHT_DATA_H
