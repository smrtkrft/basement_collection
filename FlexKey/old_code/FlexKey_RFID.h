#ifndef FLEXKEY_RFID_H
#define FLEXKEY_RFID_H

#include "FlexKey_Config.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

class FlexKeyRFID {
public:
    FlexKeyRFID();
    
    // Initialize PN532
    bool begin();
    
    // Check for card and read UID
    bool readCard(UID_t& uid);
    
    // Check if RFID is initialized
    bool isInitialized();
    
private:
    Adafruit_PN532* nfc;
    bool initialized;
    unsigned long lastReadTime;
    UID_t lastReadUID;
    unsigned long lastErrorPrintTime;
    bool errorReported;
    
    static const unsigned long READ_DELAY = 500;  // Min delay between reads (ms)
    static const unsigned long ERROR_PRINT_INTERVAL = 5000;  // Print error every 5 seconds
};

#endif // FLEXKEY_RFID_H
