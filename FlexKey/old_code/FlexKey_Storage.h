#ifndef FLEXKEY_STORAGE_H
#define FLEXKEY_STORAGE_H

#include "FlexKey_Config.h"
#include <Preferences.h>

class FlexKeyStorage {
public:
    FlexKeyStorage();
    
    // Initialize storage
    bool begin();
    
    // Factory reset - clear all data
    void factoryReset();
    
    // System config
    bool saveSystemConfig(const SystemConfig_t& config);
    bool loadSystemConfig(SystemConfig_t& config);
    
    // Individual group save/load
    bool saveGroup(uint8_t groupIndex, const Group_t& group);
    bool loadGroup(uint8_t groupIndex, Group_t& group);
    
    // WiFi config
    bool saveWiFiConfig(const WiFiConfig_t& config);
    bool loadWiFiConfig(WiFiConfig_t& config);
    
    // Device ID
    bool saveDeviceID(const String& deviceID);
    String loadDeviceID();
    
    // Multi-group mode
    bool saveMultiGroupMode(bool enabled);
    bool loadMultiGroupMode();
    
    // Active group (single mode)
    bool saveActiveGroup(uint8_t groupIndex);
    uint8_t loadActiveGroup();
    
    // Global relay config (multi-group mode)
    bool saveGlobalRelay(const GlobalRelayConfig_t& relay);
    bool loadGlobalRelay(GlobalRelayConfig_t& relay);
    
    // Check if first boot
    bool isFirstBoot();
    void setInitialized();
    
private:
    Preferences prefs;
    
    // Helper functions
    String getGroupKey(uint8_t groupIndex, const char* suffix);
    bool saveStringArray(const char* key, const String* arr, uint8_t count, uint8_t maxCount);
    uint8_t loadStringArray(const char* key, String* arr, uint8_t maxCount);
};

#endif // FLEXKEY_STORAGE_H
