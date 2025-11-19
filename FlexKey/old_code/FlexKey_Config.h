#ifndef FLEXKEY_CONFIG_H
#define FLEXKEY_CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

// ========================================
// SYSTEM VERSION
// ========================================
#define FLEXKEY_VERSION "1.0.3"
#define SYSTEM_NAME "SmartKraft - FlexKey"

// ========================================
// HARDWARE PIN DEFINITIONS (XIAO ESP32-C6)
// ========================================
// XIAO ESP32-C6 I2C Pins (VERIFIED WORKING):
// Board labels D4/D5 actually map to GPIO22/GPIO23
// Confirmed working with PN532 Elechouse V3 module
#define PIN_I2C_SDA         22  // GPIO22 (labeled D4 on XIAO ESP32-C6 board)
#define PIN_I2C_SCL         23  // GPIO23 (labeled D5 on XIAO ESP32-C6 board)
#define PIN_BUTTON          2   // D1 (GPIO2) - Internal pullup
#define PIN_RELAY           3   // D0 (GPIO3)
#define PIN_STATUS_LED      10  // D10 (Optional status LED)

// ========================================
// SYSTEM LIMITS
// ========================================
#define MAX_GROUPS          5
#define MAX_UIDS            200  // Total UID limit
#define MAX_URLS_PER_GROUP  15
#define MAX_URL_LENGTH      256
#define MAX_SSID_LENGTH     32
#define MAX_PASSWORD_LENGTH 64
#define UID_DISPLAY_TIME    60000  // 1 minute in milliseconds

// ========================================
// BUTTON TIMING
// ========================================
#define BUTTON_DEBOUNCE_MS      50
#define FACTORY_RESET_HOLD_MS   10000  // 10 seconds

// ========================================
// WIFI SETTINGS
// ========================================
#define AP_MODE_SSID_PREFIX "FlexKey-"
#define AP_MODE_PASSWORD    ""  // No password (open)
#define WIFI_CONNECT_TIMEOUT 10000  // 10 seconds

// ========================================
// WEB SERVER
// ========================================
#define WEB_SERVER_PORT 80

// ========================================
// RFID SETTINGS
// ========================================
#define RFID_READ_DELAY     500   // Minimum delay between card reads (ms)

// ========================================
// DATA STRUCTURES
// ========================================

// UID Structure (supports 4 or 7 byte UIDs)
struct UID_t {
    uint8_t data[7];
    uint8_t length;  // 4 or 7
    bool isValid;
    
    UID_t() : length(0), isValid(false) {
        memset(data, 0, 7);
    }
    
    bool equals(const UID_t& other) const {
        if (length != other.length) return false;
        return memcmp(data, other.data, length) == 0;
    }
    
    String toString() const {
        String result = "";
        for (uint8_t i = 0; i < length; i++) {
            if (i > 0) result += ":";
            if (data[i] < 0x10) result += "0";
            result += String(data[i], HEX);
        }
        result.toUpperCase();
        return result;
    }
    
    bool fromString(const String& str) {
        String cleanStr = str;
        cleanStr.replace(":", "");
        cleanStr.replace(" ", "");
        cleanStr.toUpperCase();
        
        if (cleanStr.length() != 8 && cleanStr.length() != 14) {
            return false;  // Invalid length
        }
        
        length = cleanStr.length() / 2;
        for (uint8_t i = 0; i < length; i++) {
            String byteStr = cleanStr.substring(i * 2, i * 2 + 2);
            data[i] = strtoul(byteStr.c_str(), NULL, 16);
        }
        isValid = true;
        return true;
    }
};

// Group Structure
struct Group_t {
    String name;
    bool active;
    UID_t uids[MAX_UIDS];
    uint16_t uidCount;
    String urls[MAX_URLS_PER_GROUP];
    uint8_t urlCount;
    
    // Relay settings (per group, only used in single group mode)
    bool relayEnabled;
    bool relayToggle;  // true = toggle, false = pulse
    uint16_t relayPulseDuration;  // milliseconds
    
    Group_t() : name(""), active(false), uidCount(0), urlCount(0), 
                relayEnabled(false), relayToggle(true), relayPulseDuration(500) {}
};

// WiFi Configuration
struct WiFiConfig_t {
    // Primary Network
    String primarySSID;
    String primaryPassword;
    bool primaryUseStaticIP;
    IPAddress primaryStaticIP;
    IPAddress primaryGateway;
    IPAddress primarySubnet;
    IPAddress primaryDNS;
    
    // Backup Network
    String backupSSID;
    String backupPassword;
    bool backupUseStaticIP;
    IPAddress backupStaticIP;
    IPAddress backupGateway;
    IPAddress backupSubnet;
    IPAddress backupDNS;
    
    // AP Mode Control
    bool apModeEnabled;
    
    WiFiConfig_t() : primarySSID(""), primaryPassword(""), 
                     primaryUseStaticIP(false),
                     primaryStaticIP(192, 168, 1, 100),
                     primaryGateway(192, 168, 1, 1),
                     primarySubnet(255, 255, 255, 0),
                     primaryDNS(8, 8, 8, 8),
                     backupSSID(""), backupPassword(""),
                     backupUseStaticIP(false),
                     backupStaticIP(192, 168, 2, 100),
                     backupGateway(192, 168, 2, 1),
                     backupSubnet(255, 255, 255, 0),
                     backupDNS(8, 8, 8, 8),
                     apModeEnabled(true) {}
};

// Global Relay Configuration (for multi-group mode)
struct GlobalRelayConfig_t {
    bool enabled;
    bool toggle;  // true = toggle, false = pulse
    uint16_t pulseDuration;  // milliseconds
    
    GlobalRelayConfig_t() : enabled(false), toggle(true), pulseDuration(500) {}
};

// System Configuration
struct SystemConfig_t {
    bool multiGroupMode;  // false = single group, true = multi group
    uint8_t activeGroupIndex;  // Only used in single group mode (0-4)
    GlobalRelayConfig_t globalRelay;  // Only used in multi-group mode
    Group_t groups[MAX_GROUPS];
    WiFiConfig_t wifi;
    String deviceID;  // FlexKey-XXXXXX (MAC-based)
    
    SystemConfig_t() : multiGroupMode(false), activeGroupIndex(0), deviceID("") {}
};

// Last Read UID (for display in web interface)
struct LastUID_t {
    UID_t uid;
    unsigned long timestamp;
    
    LastUID_t() : timestamp(0) {}
    
    bool isExpired() const {
        return (millis() - timestamp) > UID_DISPLAY_TIME;
    }
    
    void update(const UID_t& newUID) {
        uid = newUID;
        timestamp = millis();
    }
    
    void clear() {
        uid.isValid = false;
        timestamp = 0;
    }
};

#endif
