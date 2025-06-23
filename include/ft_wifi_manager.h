#ifndef FT_WIFI_MANAGER_H
#define FT_WIFI_MANAGER_H

#include <WiFi.h>
#include <WiFiManager.h>

class FtWiFiManager
{
public:
    static bool connect();
    static bool isConnected();
    static void disconnect();
    static String getLocalIP();
    static long getRSSI();

private:
    static WiFiManager wm;
    static void displayAPInfo(const String &apName, const String &password, const String &ip);
};

#endif // FT_WIFI_MANAGER_H
