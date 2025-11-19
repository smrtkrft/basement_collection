#include "FlexKey_Button.h"

FlexKeyButton::FlexKeyButton() 
    : pressStartTime(0), isPressed(false), lastState(HIGH), lastDebounceTime(0) {
}

void FlexKeyButton::begin() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    Serial.println("[BUTTON] Button initialized on GPIO" + String(PIN_BUTTON));
}

bool FlexKeyButton::update() {
    bool currentState = digitalRead(PIN_BUTTON);
    unsigned long currentTime = millis();
    
    // Debounce
    if (currentState != lastState) {
        lastDebounceTime = currentTime;
    }
    
    if ((currentTime - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // Button state is stable
        if (currentState == LOW && !isPressed) {
            // Button just pressed
            isPressed = true;
            pressStartTime = currentTime;
            Serial.println("[BUTTON] Button pressed");
        }
        else if (currentState == HIGH && isPressed) {
            // Button released
            isPressed = false;
            unsigned long pressDuration = currentTime - pressStartTime;
            Serial.println("[BUTTON] Button released (held for " + String(pressDuration) + "ms)");
            
            if (pressDuration >= FACTORY_RESET_HOLD_MS) {
                Serial.println("[BUTTON] Factory reset triggered!");
                return true;
            }
        }
        else if (currentState == LOW && isPressed) {
            // Button still held - check if factory reset threshold reached
            unsigned long holdDuration = currentTime - pressStartTime;
            if (holdDuration >= FACTORY_RESET_HOLD_MS) {
                // Don't trigger multiple times
                static bool resetReported = false;
                if (!resetReported) {
                    Serial.println("[BUTTON] Factory reset threshold reached!");
                    resetReported = true;
                }
            }
        }
    }
    
    lastState = currentState;
    return false;
}
