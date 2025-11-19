#include "FlexKey_WiFi.h"
#include <esp_wifi.h>

FlexKeyWiFi::FlexKeyWiFi() 
    : apMode(false), lastReconnectAttempt(0) {
}

void FlexKeyWiFi::begin(const String& deviceID) {
    WiFi.mode(WIFI_MODE_NULL);
    apSSID = AP_MODE_SSID_PREFIX + deviceID.substring(deviceID.length() - 6);
    Serial.println("[WIFI] Initialized - AP SSID will be: " + apSSID);
}

bool FlexKeyWiFi::startAP(const String& deviceID) {
    apSSID = AP_MODE_SSID_PREFIX + deviceID.substring(deviceID.length() - 6);
    
    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP(apSSID.c_str(), AP_MODE_PASSWORD);
    
    if (success) {
        apMode = true;
        IPAddress IP = WiFi.softAPIP();
        Serial.println("[WIFI] AP Mode started");
        Serial.println("[WIFI] SSID: " + apSSID);
        Serial.println("[WIFI] Password: " + String(AP_MODE_PASSWORD[0] == '\0' ? "None (Open)" : AP_MODE_PASSWORD));
        Serial.println("[WIFI] IP: " + IP.toString());
    } else {
        Serial.println("[WIFI] Failed to start AP mode");
    }
    
    return success;
}

void FlexKeyWiFi::stopAP() {
    if (apMode) {
        WiFi.softAPdisconnect(true);
        apMode = false;
        Serial.println("[WIFI] AP Mode stopped");
    }
}

bool FlexKeyWiFi::connectToWiFi(const WiFiConfig_t& config) {
    currentConfig = config;
    
    // Determine WiFi mode based on AP setting and SSID availability
    bool hasWiFiSSID = (config.primarySSID.length() > 0 || config.backupSSID.length() > 0);
    
    if (config.apModeEnabled) {
        // AP mode enabled
        if (hasWiFiSSID) {
            WiFi.mode(WIFI_AP_STA);
            Serial.println("[WIFI] Mode: AP+STA (AP enabled, WiFi configured)");
        } else {
            WiFi.mode(WIFI_AP);
            Serial.println("[WIFI] Mode: AP only (No WiFi configured)");
        }
    } else {
        // AP mode disabled (only possible if WiFi is configured)
        if (hasWiFiSSID) {
            WiFi.mode(WIFI_STA);
            Serial.println("[WIFI] Mode: STA only (AP disabled)");
        } else {
            // Force AP mode if no WiFi configured
            WiFi.mode(WIFI_AP);
            Serial.println("[WIFI] Mode: AP only (Forced - No WiFi configured)");
        }
    }
    
    // Try primary SSID
    if (config.primarySSID.length() > 0) {
        Serial.println("[WIFI] Attempting to connect to primary SSID: " + config.primarySSID);
        
        if (tryConnect(config.primarySSID, config.primaryPassword, 
                      config.primaryUseStaticIP, config.primaryStaticIP, 
                      config.primaryGateway, config.primarySubnet, config.primaryDNS)) {
            return true;
        }
    }
    
    // Try backup SSID if primary failed
    if (config.backupSSID.length() > 0) {
        Serial.println("[WIFI] Primary failed, trying backup SSID: " + config.backupSSID);
        
        if (tryConnect(config.backupSSID, config.backupPassword, 
                      config.backupUseStaticIP, config.backupStaticIP, 
                      config.backupGateway, config.backupSubnet, config.backupDNS)) {
            return true;
        }
    }
    
    Serial.println("[WIFI] No WiFi connection established");
    return false;
}

bool FlexKeyWiFi::tryConnect(const String& ssid, const String& password, 
                            bool useStatic, const IPAddress& ip, 
                            const IPAddress& gw, const IPAddress& sn, 
                            const IPAddress& dns) {
    if (ssid.length() == 0) return false;
    
    // Configure static IP if needed
    if (useStatic) {
        if (!WiFi.config(ip, gw, sn, dns)) {
            Serial.println("[WIFI] Static IP configuration failed");
            return false;
        }
        Serial.println("[WIFI] Static IP configured: " + ip.toString());
    }
    
    // Start connection
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // Wait for connection
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT) {
            Serial.println("[WIFI] Connection timeout");
            WiFi.disconnect();
            return false;
        }
        delay(100);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.println("[WIFI] Connected!");
    Serial.println("[WIFI] SSID: " + ssid);
    Serial.println("[WIFI] IP: " + WiFi.localIP().toString());
    Serial.println("[WIFI] Gateway: " + WiFi.gatewayIP().toString());
    Serial.println("[WIFI] Subnet: " + WiFi.subnetMask().toString());
    Serial.println("[WIFI] DNS: " + WiFi.dnsIP().toString());
    
    return true;
}

bool FlexKeyWiFi::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String FlexKeyWiFi::getIPAddress() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    } else if (apMode) {
        return WiFi.softAPIP().toString();
    }
    return "0.0.0.0";
}

String FlexKeyWiFi::getMACAddress() {
    uint8_t mac[6];
    char macStr[18];
    
    // Get WiFi STA MAC address
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return String(macStr);
}

String FlexKeyWiFi::getAPSSID() {
    return apSSID;
}

void FlexKeyWiFi::update() {
    // Auto-reconnect logic (check every 30 seconds)
    if (!isConnected() && !apMode) {
        if (millis() - lastReconnectAttempt > 30000) {
            lastReconnectAttempt = millis();
            Serial.println("[WIFI] Connection lost, attempting to reconnect...");
            connectToWiFi(currentConfig);
        }
    }
}
