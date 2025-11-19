#ifndef FLEXKEY_RELAY_H
#define FLEXKEY_RELAY_H

#include "FlexKey_Config.h"

class FlexKeyRelay {
public:
    FlexKeyRelay();
    
    // Initialize relay
    void begin();
    
    // Toggle relay state
    void toggle();
    
    // Pulse relay (turn on for duration, then off)
    void pulse(uint16_t durationMs);
    
    // Set relay state directly
    void setState(bool state);
    
    // Get current state
    bool getState();
    
    // Turn on
    void turnOn();
    
    // Turn off
    void turnOff();
    
    // Update loop (handles pulse timing)
    void update();
    
private:
    bool currentState;
    bool pulseActive;
    unsigned long pulseStartTime;
    uint16_t pulseDuration;
};

#endif // FLEXKEY_RELAY_H
