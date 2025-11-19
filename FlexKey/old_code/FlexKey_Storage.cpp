#include "FlexKey_Storage.h"

FlexKeyStorage::FlexKeyStorage() {
}

bool FlexKeyStorage::begin() {
    return prefs.begin("flexkey", false);  // Read-write mode
}

void FlexKeyStorage::factoryReset() {
    Serial.println("[STORAGE] Factory reset initiated...");
    prefs.clear();
    Serial.println("[STORAGE] All data cleared. Restarting...");
    delay(1000);
    ESP.restart();
}

bool FlexKeyStorage::saveSystemConfig(const SystemConfig_t& config) {
    if (!prefs.begin("flexkey", false)) return false;
    
    // Save basic settings
    prefs.putBool("multiGroup", config.multiGroupMode);
    prefs.putUChar("activeGrp", config.activeGroupIndex);
    prefs.putString("deviceID", config.deviceID);
    
    // Save all groups
    for (uint8_t i = 0; i < MAX_GROUPS; i++) {
        saveGroup(i, config.groups[i]);
    }
    
    // Save WiFi config
    saveWiFiConfig(config.wifi);
    
    // Save global relay
    saveGlobalRelay(config.globalRelay);
    
    prefs.end();
    Serial.println("[STORAGE] System config saved");
    return true;
}

bool FlexKeyStorage::loadSystemConfig(SystemConfig_t& config) {
    if (!prefs.begin("flexkey", true)) return false;  // Read-only
    
    // Load basic settings
    config.multiGroupMode = prefs.getBool("multiGroup", false);
    config.activeGroupIndex = prefs.getUChar("activeGrp", 0);
    config.deviceID = prefs.getString("deviceID", "");
    
    // Load all groups
    for (uint8_t i = 0; i < MAX_GROUPS; i++) {
        loadGroup(i, config.groups[i]);
    }
    
    // Load WiFi config
    loadWiFiConfig(config.wifi);
    
    // Load global relay
    loadGlobalRelay(config.globalRelay);
    
    prefs.end();
    Serial.println("[STORAGE] System config loaded");
    return true;
}

bool FlexKeyStorage::saveGroup(uint8_t groupIndex, const Group_t& group) {
    if (groupIndex >= MAX_GROUPS) return false;
    if (!prefs.begin("flexkey", false)) return false;
    
    String prefix = "g" + String(groupIndex) + "_";
    
    // Save group basic info
    prefs.putString((prefix + "name").c_str(), group.name);
    prefs.putBool((prefix + "active").c_str(), group.active);
    prefs.putUShort((prefix + "uidCnt").c_str(), group.uidCount);
    prefs.putUChar((prefix + "urlCnt").c_str(), group.urlCount);
    
    // Save relay settings
    prefs.putBool((prefix + "relEn").c_str(), group.relayEnabled);
    prefs.putBool((prefix + "relTog").c_str(), group.relayToggle);
    prefs.putUShort((prefix + "relPls").c_str(), group.relayPulseDuration);
    
    // Save UIDs
    for (uint16_t i = 0; i < group.uidCount && i < MAX_UIDS; i++) {
        String uidKey = prefix + "uid" + String(i);
        String uidStr = group.uids[i].toString();
        prefs.putString(uidKey.c_str(), uidStr);
    }
    
    // Save URLs
    for (uint8_t i = 0; i < group.urlCount && i < MAX_URLS_PER_GROUP; i++) {
        String urlKey = prefix + "url" + String(i);
        prefs.putString(urlKey.c_str(), group.urls[i]);
    }
    
    prefs.end();
    return true;
}

bool FlexKeyStorage::loadGroup(uint8_t groupIndex, Group_t& group) {
    if (groupIndex >= MAX_GROUPS) return false;
    if (!prefs.begin("flexkey", true)) return false;
    
    String prefix = "g" + String(groupIndex) + "_";
    
    // Load group basic info
    group.name = prefs.getString((prefix + "name").c_str(), "Group " + String(groupIndex + 1));
    group.active = prefs.getBool((prefix + "active").c_str(), groupIndex == 0);  // First group active by default
    group.uidCount = prefs.getUShort((prefix + "uidCnt").c_str(), 0);
    group.urlCount = prefs.getUChar((prefix + "urlCnt").c_str(), 0);
    
    // Load relay settings
    group.relayEnabled = prefs.getBool((prefix + "relEn").c_str(), false);
    group.relayToggle = prefs.getBool((prefix + "relTog").c_str(), true);
    group.relayPulseDuration = prefs.getUShort((prefix + "relPls").c_str(), 500);
    
    // Load UIDs
    for (uint16_t i = 0; i < group.uidCount && i < MAX_UIDS; i++) {
        String uidKey = prefix + "uid" + String(i);
        String uidStr = prefs.getString(uidKey.c_str(), "");
        if (uidStr.length() > 0) {
            group.uids[i].fromString(uidStr);
        }
    }
    
    // Load URLs
    for (uint8_t i = 0; i < group.urlCount && i < MAX_URLS_PER_GROUP; i++) {
        String urlKey = prefix + "url" + String(i);
        group.urls[i] = prefs.getString(urlKey.c_str(), "");
    }
    
    prefs.end();
    return true;
}

bool FlexKeyStorage::saveWiFiConfig(const WiFiConfig_t& config) {
    if (!prefs.begin("flexkey", false)) return false;
    
    // Primary Network
    prefs.putString("wifiSSID", config.primarySSID);
    prefs.putString("wifiPass", config.primaryPassword);
    prefs.putBool("priStaticIP", config.primaryUseStaticIP);
    prefs.putString("priIP", config.primaryStaticIP.toString());
    prefs.putString("priGW", config.primaryGateway.toString());
    prefs.putString("priSN", config.primarySubnet.toString());
    prefs.putString("priDNS", config.primaryDNS.toString());
    
    // Backup Network
    prefs.putString("wifiBkSSID", config.backupSSID);
    prefs.putString("wifiBkPass", config.backupPassword);
    prefs.putBool("bkStaticIP", config.backupUseStaticIP);
    prefs.putString("bkIP", config.backupStaticIP.toString());
    prefs.putString("bkGW", config.backupGateway.toString());
    prefs.putString("bkSN", config.backupSubnet.toString());
    prefs.putString("bkDNS", config.backupDNS.toString());
    
    // AP Mode
    prefs.putBool("apModeEn", config.apModeEnabled);
    
    prefs.end();
    return true;
}

bool FlexKeyStorage::loadWiFiConfig(WiFiConfig_t& config) {
    if (!prefs.begin("flexkey", true)) return false;
    
    // Primary Network
    config.primarySSID = prefs.getString("wifiSSID", "");
    config.primaryPassword = prefs.getString("wifiPass", "");
    config.primaryUseStaticIP = prefs.getBool("priStaticIP", false);
    config.primaryStaticIP.fromString(prefs.getString("priIP", "192.168.1.100"));
    config.primaryGateway.fromString(prefs.getString("priGW", "192.168.1.1"));
    config.primarySubnet.fromString(prefs.getString("priSN", "255.255.255.0"));
    config.primaryDNS.fromString(prefs.getString("priDNS", "8.8.8.8"));
    
    // Backup Network
    config.backupSSID = prefs.getString("wifiBkSSID", "");
    config.backupPassword = prefs.getString("wifiBkPass", "");
    config.backupUseStaticIP = prefs.getBool("bkStaticIP", false);
    config.backupStaticIP.fromString(prefs.getString("bkIP", "192.168.2.100"));
    config.backupGateway.fromString(prefs.getString("bkGW", "192.168.2.1"));
    config.backupSubnet.fromString(prefs.getString("bkSN", "255.255.255.0"));
    config.backupDNS.fromString(prefs.getString("bkDNS", "8.8.8.8"));
    
    // AP Mode
    config.apModeEnabled = prefs.getBool("apModeEn", true);
    
    prefs.end();
    return true;
}

bool FlexKeyStorage::saveDeviceID(const String& deviceID) {
    if (!prefs.begin("flexkey", false)) return false;
    prefs.putString("deviceID", deviceID);
    prefs.end();
    return true;
}

String FlexKeyStorage::loadDeviceID() {
    if (!prefs.begin("flexkey", true)) return "";
    String id = prefs.getString("deviceID", "");
    prefs.end();
    return id;
}

bool FlexKeyStorage::saveMultiGroupMode(bool enabled) {
    if (!prefs.begin("flexkey", false)) return false;
    prefs.putBool("multiGroup", enabled);
    prefs.end();
    return true;
}

bool FlexKeyStorage::loadMultiGroupMode() {
    if (!prefs.begin("flexkey", true)) return false;
    bool mode = prefs.getBool("multiGroup", false);
    prefs.end();
    return mode;
}

bool FlexKeyStorage::saveActiveGroup(uint8_t groupIndex) {
    if (groupIndex >= MAX_GROUPS) return false;
    if (!prefs.begin("flexkey", false)) return false;
    prefs.putUChar("activeGrp", groupIndex);
    prefs.end();
    return true;
}

uint8_t FlexKeyStorage::loadActiveGroup() {
    if (!prefs.begin("flexkey", true)) return 0;
    uint8_t grp = prefs.getUChar("activeGrp", 0);
    prefs.end();
    return grp;
}

bool FlexKeyStorage::saveGlobalRelay(const GlobalRelayConfig_t& relay) {
    if (!prefs.begin("flexkey", false)) return false;
    prefs.putBool("gRelEn", relay.enabled);
    prefs.putBool("gRelTog", relay.toggle);
    prefs.putUShort("gRelPls", relay.pulseDuration);
    prefs.end();
    return true;
}

bool FlexKeyStorage::loadGlobalRelay(GlobalRelayConfig_t& relay) {
    if (!prefs.begin("flexkey", true)) return false;
    relay.enabled = prefs.getBool("gRelEn", false);
    relay.toggle = prefs.getBool("gRelTog", true);
    relay.pulseDuration = prefs.getUShort("gRelPls", 500);
    prefs.end();
    return true;
}

bool FlexKeyStorage::isFirstBoot() {
    if (!prefs.begin("flexkey", true)) return true;
    bool initialized = prefs.getBool("initialized", false);
    prefs.end();
    return !initialized;
}

void FlexKeyStorage::setInitialized() {
    if (!prefs.begin("flexkey", false)) return;
    prefs.putBool("initialized", true);
    prefs.end();
    Serial.println("[STORAGE] System marked as initialized");
}

String FlexKeyStorage::getGroupKey(uint8_t groupIndex, const char* suffix) {
    return "g" + String(groupIndex) + "_" + String(suffix);
}

bool FlexKeyStorage::saveStringArray(const char* key, const String* arr, uint8_t count, uint8_t maxCount) {
    if (count > maxCount) count = maxCount;
    for (uint8_t i = 0; i < count; i++) {
        String itemKey = String(key) + String(i);
        prefs.putString(itemKey.c_str(), arr[i]);
    }
    return true;
}

uint8_t FlexKeyStorage::loadStringArray(const char* key, String* arr, uint8_t maxCount) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < maxCount; i++) {
        String itemKey = String(key) + String(i);
        String value = prefs.getString(itemKey.c_str(), "");
        if (value.length() > 0) {
            arr[count++] = value;
        } else {
            break;
        }
    }
    return count;
}
