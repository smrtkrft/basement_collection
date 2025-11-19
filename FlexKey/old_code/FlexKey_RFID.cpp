#include "FlexKey_RFID.h"

FlexKeyRFID::FlexKeyRFID() 
    : nfc(nullptr), initialized(false), lastReadTime(0), lastErrorPrintTime(0), errorReported(false) {
}

bool FlexKeyRFID::begin() {
    Serial.println("========================================");
    Serial.println("[RFID] Initializing RFID Reader...");
    Serial.println("========================================");
    
    // Configure I2C pins with pull-up resistors
    pinMode(PIN_I2C_SDA, INPUT_PULLUP);
    pinMode(PIN_I2C_SCL, INPUT_PULLUP);
    delay(100);
    
    // Initialize I2C with verified working pins for XIAO ESP32-C6
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(100000);  // 100kHz for PN532
    Wire.setTimeOut(1000);
    
    Serial.println("[RFID] I2C initialized");
    Serial.println("[RFID]   SDA: GPIO" + String(PIN_I2C_SDA) + " (D4 on board)");
    Serial.println("[RFID]   SCL: GPIO" + String(PIN_I2C_SCL) + " (D5 on board)");
    Serial.println("[RFID]   Clock: 100kHz");
    
    delay(500);
    
    // Create PN532 instance for I2C mode
    Serial.println("[RFID] Creating PN532 instance...");
    nfc = new Adafruit_PN532(-1, -1, &Wire);
    
    if (!nfc) {
        Serial.println("[RFID] ERROR: Failed to create PN532 instance");
        Serial.println("[RFID] ERROR: Out of memory");
        return false;
    }
    
    Serial.println("[RFID] PN532 instance created");
    
    // Initialize PN532
    Serial.println("[RFID] Calling nfc.begin()...");
    nfc->begin();
    
    delay(500);
    
    // Get firmware version
    Serial.println("[RFID] Reading firmware version...");
    uint32_t versiondata = nfc->getFirmwareVersion();
    
    delay(100);
    
    if (!versiondata) {
        Serial.println("[RFID] ERROR: PN532 not responding!");
        Serial.println("[RFID] ERROR: No response from PN532 module");
        Serial.println();
        Serial.println("[RFID] Troubleshooting:");
        Serial.println("  1. Check I2C wiring:");
        Serial.println("      SDA: GPIO" + String(PIN_I2C_SDA) + " (D4)");
        Serial.println("      SCL: GPIO" + String(PIN_I2C_SCL) + " (D5)");
        Serial.println("  2. Check power (3.3V to PN532)");
        Serial.println("  3. Check GND connection");
        Serial.println("  4. Verify PN532 DIP switches for I2C mode:");
        Serial.println("      Elechouse V3: SET0=OPEN, SET1=BRIDGED");
        Serial.println("  5. Check for loose connections");
        
        delete nfc;
        nfc = nullptr;
        return false;
    }
    
    // Print firmware version
    Serial.print("[RFID] PN532 Firmware version: ");
    Serial.print((versiondata >> 24) & 0xFF, DEC);
    Serial.print('.');
    Serial.println((versiondata >> 8) & 0xFF, DEC);
    
    // Configure PN532 to read RFID tags
    Serial.println("[RFID] Configuring PN532...");
    nfc->SAMConfig();
    
    Serial.println("========================================");
    Serial.println("[RFID] READY - Waiting for RFID cards...");
    Serial.println("========================================");
    Serial.println();
    
    initialized = true;
    return true;
}

bool FlexKeyRFID::readCard(UID_t& uid) {
    if (!initialized || !nfc) {
        // Report error only once and then every 5 seconds
        if (!errorReported || (millis() - lastErrorPrintTime > 5000)) {
            Serial.println("[RFID] ERROR: RFID not initialized");
            errorReported = true;
            lastErrorPrintTime = millis();
        }
        return false;
    }
    
    // Prevent reading too frequently
    if (millis() - lastReadTime < RFID_READ_DELAY) {
        return false;
    }
    
    uint8_t uidBuffer[7];
    uint8_t uidLength;
    
    // Try to read a card (100ms timeout)
    bool success = nfc->readPassiveTargetID(PN532_MIFARE_ISO14443A, uidBuffer, &uidLength, 100);
    
    if (success) {
        // Valid card detected
        uid.length = uidLength;
        uid.isValid = true;
        memcpy(uid.data, uidBuffer, uidLength);
        
        lastReadTime = millis();
        
        // Print UID to Serial
        Serial.println();
        Serial.println("[RFID] ========================================");
        Serial.println("[RFID] CARD DETECTED!");
        Serial.println("[RFID] ========================================");
        Serial.print("[RFID] UID: ");
        Serial.println(uid.toString());
        Serial.print("[RFID] Length: ");
        Serial.print(uid.length);
        Serial.println(" bytes");
        Serial.println("[RFID] ========================================");
        Serial.println();
        
        return true;
    }
    
    return false;
}

bool FlexKeyRFID::isInitialized() {
    return initialized;
}