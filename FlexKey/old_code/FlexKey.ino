/*
 * FlexKey - RFID Access Control System
 * 
 * Hardware: XIAO ESP32-C6, PN532 NFC/RFID, Relay, Button
 * 
 * Features:
 * - Multi/Single group RFID management
 * - WiFi & AP mode configuration
 * - Web-based configuration interface
 * - HTTP GET trigger URLs
 * - Relay control (toggle/pulse)
 * - Non-volatile storage (NVS)
 * 
 * Author: SmartKraft
 * Website: smartkraft.ch/FlexKey
 * Version: 1.0.0
 */

#include "FlexKey_Config.h"
#include "FlexKey_Storage.h"
#include "FlexKey_Button.h"
#include "FlexKey_WiFi.h"
#include "FlexKey_RFID.h"
#include "FlexKey_Relay.h"
#include "FlexKey_HTTP.h"
#include "FlexKey_Web.h"
#include <WiFi.h>
#include <esp_wifi.h>

// ============================================
// GLOBAL OBJECTS
// ============================================
SystemConfig_t systemConfig;
LastUID_t lastReadUID;

FlexKeyStorage storage;
FlexKeyButton button;
FlexKeyWiFi wifiManager;
FlexKeyRFID rfidReader;
FlexKeyRelay relayControl;
FlexKeyHTTP httpClient;
FlexKeyWeb webServer(&systemConfig, &storage, &rfidReader, &relayControl, &httpClient, &lastReadUID);

// ============================================
// SETUP
// ============================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  " + String(SYSTEM_NAME));
    Serial.println("  Version: " + String(FLEXKEY_VERSION));
    Serial.println("========================================");
    Serial.println();
    
    // Initialize storage
    if (!storage.begin()) {
        Serial.println("[SYSTEM] Storage initialization failed!");
        while(1) { delay(1000); }
    }
    
    // Check for first boot
    if (storage.isFirstBoot()) {
        Serial.println("[SYSTEM] First boot detected - initializing defaults...");
        initializeDefaults();
        storage.setInitialized();
    }
    
    // Load configuration
    storage.loadSystemConfig(systemConfig);
    
    // Generate unique device ID from WiFi MAC address
    if (systemConfig.deviceID.length() == 0) {
        // Initialize WiFi to get MAC address
        WiFi.mode(WIFI_STA);
        delay(100);
        
        // Get WiFi MAC address (unique for each ESP32-C6)
        uint8_t mac[6];
        esp_wifi_get_mac(WIFI_IF_STA, mac);
        
        // Create device ID from last 3 bytes of MAC (6 hex chars)
        char macStr[7];
        sprintf(macStr, "%02X%02X%02X", mac[3], mac[4], mac[5]);
        
        systemConfig.deviceID = String(AP_MODE_SSID_PREFIX) + String(macStr);
        systemConfig.deviceID.toUpperCase();
        storage.saveDeviceID(systemConfig.deviceID);
        
        Serial.println("[SYSTEM] Generated unique Device ID from WiFi MAC");
        Serial.printf("[SYSTEM] Full MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
                     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    
    Serial.println("[SYSTEM] Device ID: " + systemConfig.deviceID);
    
    // Initialize button
    button.begin();
    
    // Initialize WiFi
    wifiManager.begin(systemConfig.deviceID);
    
    // Start AP mode if enabled OR if no WiFi configured (safety)
    bool hasWiFiSSID = (systemConfig.wifi.primarySSID.length() > 0 || 
                        systemConfig.wifi.backupSSID.length() > 0);
    
    if (systemConfig.wifi.apModeEnabled || !hasWiFiSSID) {
        wifiManager.startAP(systemConfig.deviceID);
        if (!hasWiFiSSID) {
            Serial.println("[SYSTEM] AP mode forced - No WiFi configured");
        }
    } else {
        Serial.println("[SYSTEM] AP mode disabled - WiFi only mode");
    }
    
    // Try to connect to WiFi if configured
    if (hasWiFiSSID) {
        wifiManager.connectToWiFi(systemConfig.wifi);
    }
    
    // Initialize RFID reader
    if (!rfidReader.begin()) {
        Serial.println("[SYSTEM] WARNING: RFID reader initialization failed!");
        Serial.println("[SYSTEM] System will continue but RFID functionality disabled");
    }
    
    // Initialize relay
    relayControl.begin();
    
    // Start web server
    webServer.begin();
    
    Serial.println();
    Serial.println("========================================");
    Serial.println("  SYSTEM READY");
    Serial.println("========================================");
    Serial.println("  IP Address: " + wifiManager.getIPAddress());
    Serial.println("  MAC Address: " + wifiManager.getMACAddress());
    Serial.println("  Web Interface: http://" + wifiManager.getIPAddress());
    Serial.println("========================================");
    Serial.println();
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
    // Check for factory reset button
    if (button.update()) {
        Serial.println("[SYSTEM] Factory reset triggered!");
        storage.factoryReset();
        // System will restart automatically
    }
    
    // Update WiFi
    wifiManager.update();
    
    // Handle web server
    webServer.handleClient();
    
    // Update relay (for pulse timing)
    relayControl.update();
    
    // Check for RFID cards
    UID_t readUID;
    if (rfidReader.readCard(readUID)) {
        handleRFIDCard(readUID);
    }
    
    // Small delay to prevent watchdog issues
    delay(10);
}

// ============================================
// RFID CARD HANDLER
// ============================================
void handleRFIDCard(const UID_t& uid) {
    Serial.println();
    Serial.println("[CARD] UID detected: " + uid.toString());
    
    // Update last read UID for web display
    lastReadUID.update(uid);
    
    // Find matching group(s)
    bool matchFound = false;
    
    if (systemConfig.multiGroupMode) {
        // Multi-group mode: check all groups
        Serial.println("[CARD] Multi-group mode active");
        
        for (uint8_t g = 0; g < MAX_GROUPS; g++) {
            if (checkUIDInGroup(uid, g)) {
                matchFound = true;
                Serial.println("[CARD] Match found in Group " + String(g + 1) + " (" + systemConfig.groups[g].name + ")");
                
                // Trigger URLs for this group
                triggerGroupURLs(g);
                
                // Trigger global relay if enabled
                if (systemConfig.globalRelay.enabled) {
                    triggerRelay(systemConfig.globalRelay.toggle, systemConfig.globalRelay.pulseDuration);
                }
            }
        }
    } else {
        // Single-group mode: check only active group
        uint8_t activeGroup = systemConfig.activeGroupIndex;
        Serial.println("[CARD] Single-group mode - checking Group " + String(activeGroup + 1));
        
        if (checkUIDInGroup(uid, activeGroup)) {
            matchFound = true;
            Serial.println("[CARD] Match found in active group (" + systemConfig.groups[activeGroup].name + ")");
            
            // Trigger URLs
            triggerGroupURLs(activeGroup);
            
            // Trigger relay if enabled for this group
            if (systemConfig.groups[activeGroup].relayEnabled) {
                triggerRelay(systemConfig.groups[activeGroup].relayToggle, 
                           systemConfig.groups[activeGroup].relayPulseDuration);
            }
        }
    }
    
    if (!matchFound) {
        Serial.println("[CARD] No match found - access denied");
    }
    
    Serial.println();
}

// ============================================
// HELPER FUNCTIONS
// ============================================
bool checkUIDInGroup(const UID_t& uid, uint8_t groupIndex) {
    if (groupIndex >= MAX_GROUPS) return false;
    
    for (uint16_t i = 0; i < systemConfig.groups[groupIndex].uidCount; i++) {
        if (systemConfig.groups[groupIndex].uids[i].equals(uid)) {
            return true;
        }
    }
    return false;
}

void triggerGroupURLs(uint8_t groupIndex) {
    if (groupIndex >= MAX_GROUPS) return;
    
    uint8_t urlCount = systemConfig.groups[groupIndex].urlCount;
    if (urlCount == 0) {
        Serial.println("[TRIGGER] No URLs configured for this group");
        return;
    }
    
    Serial.println("[TRIGGER] Sending " + String(urlCount) + " HTTP GET requests...");
    httpClient.sendMultipleGET(systemConfig.groups[groupIndex].urls, urlCount);
}

void triggerRelay(bool toggle, uint16_t pulseDuration) {
    if (toggle) {
        Serial.println("[TRIGGER] Relay toggle");
        relayControl.toggle();
    } else {
        Serial.println("[TRIGGER] Relay pulse (" + String(pulseDuration) + "ms)");
        relayControl.pulse(pulseDuration);
    }
}

void initializeDefaults() {
    Serial.println("[INIT] Setting up default configuration...");
    
    // Initialize groups with default names
    for (uint8_t i = 0; i < MAX_GROUPS; i++) {
        systemConfig.groups[i].name = "Grup " + String(i + 1);
        systemConfig.groups[i].active = (i == 0);  // First group active
        systemConfig.groups[i].uidCount = 0;
        systemConfig.groups[i].urlCount = 0;
        systemConfig.groups[i].relayEnabled = false;
        systemConfig.groups[i].relayToggle = true;
        systemConfig.groups[i].relayPulseDuration = 500;
    }
    
    // Default WiFi config (empty, will start in AP mode)
    systemConfig.wifi.primarySSID = "";
    systemConfig.wifi.primaryPassword = "";
    systemConfig.wifi.backupSSID = "";
    systemConfig.wifi.backupPassword = "";
    systemConfig.wifi.primaryUseStaticIP = false;
    systemConfig.wifi.backupUseStaticIP = false;
    systemConfig.wifi.apModeEnabled = true;
    
    // Default system settings
    systemConfig.multiGroupMode = false;
    systemConfig.activeGroupIndex = 0;
    systemConfig.globalRelay.enabled = false;
    systemConfig.globalRelay.toggle = true;
    systemConfig.globalRelay.pulseDuration = 500;
    
    // Save defaults
    storage.saveSystemConfig(systemConfig);
    
    Serial.println("[INIT] Default configuration saved");
}
