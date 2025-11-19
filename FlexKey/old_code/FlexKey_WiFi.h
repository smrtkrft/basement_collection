#ifndef FLEXKEY_WIFI_H
#define FLEXKEY_WIFI_H

#include "FlexKey_Config.h"
#include <WiFi.h>

class FlexKeyWiFi {
public:
    FlexKeyWiFi();
    
    // Initialize WiFi with device ID
    void begin(const String& deviceID);
    
    // Start AP mode
    bool startAP(const String& deviceID);
    
    // Stop AP mode
    void stopAP();
    
    // Connect to WiFi network
    bool connectToWiFi(const WiFiConfig_t& config);
    
    // Check if connected
    bool isConnected();
    
    // Get current IP address
    String getIPAddress();
    
    // Get MAC address
    String getMACAddress();
    
    // Get AP SSID
    String getAPSSID();
    
    // Update loop (call in main loop)
    void update();
    
private:
    String apSSID;
    bool apMode;
    unsigned long lastReconnectAttempt;
    WiFiConfig_t currentConfig;
    
    bool tryConnect(const String& ssid, const String& password, bool useStatic, 
                   const IPAddress& ip, const IPAddress& gw, 
                   const IPAddress& sn, const IPAddress& dns);
};

#endif // FLEXKEY_WIFI_H
