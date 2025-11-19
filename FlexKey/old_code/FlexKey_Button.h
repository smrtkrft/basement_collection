#ifndef FLEXKEY_BUTTON_H
#define FLEXKEY_BUTTON_H

#include "FlexKey_Config.h"

class FlexKeyButton {
public:
    FlexKeyButton();
    
    // Initialize button
    void begin();
    
    // Update button state (call in loop)
    // Returns true if factory reset was triggered
    bool update();
    
private:
    unsigned long pressStartTime;
    bool isPressed;
    bool lastState;
    unsigned long lastDebounceTime;
};

#endif // FLEXKEY_BUTTON_H
