#include "ft_wifi_manager.h"
#include "display_manager.h"
// #include "config.h"

WiFiManager FtWiFiManager::wm;

bool FtWiFiManager::connect()
{
    WiFi.mode(WIFI_STA);

    // Testing - uncomment to clear saved WiFi credentials
    // wm.resetSettings();

    // Set custom parameters for the config portal
    wm.setConfigPortalTimeout(300); // 5 minutes timeout
    wm.setAPCallback([](WiFiManager *myWiFiManager)
                     { FtWiFiManager::displayAPInfo(myWiFiManager->getConfigPortalSSID(),
                                                    "password", WiFi.softAPIP().toString()); });

    // Try to connect with saved credentials first
    if (wm.autoConnect("FT-Setup", "password"))
    {
        Serial.println("Connected to WiFi successfully!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("Failed to connect to WiFi");
        return false;
    }
}

bool FtWiFiManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

void FtWiFiManager::disconnect()
{
    WiFi.disconnect();
}

String FtWiFiManager::getLocalIP()
{
    return WiFi.localIP().toString();
}

long FtWiFiManager::getRSSI()
{
    return WiFi.RSSI();
}

void FtWiFiManager::displayAPInfo(const String &apName, const String &password, const String &ip)
{
    DisplayManager::displayAPInfo(apName, password, ip);
}
