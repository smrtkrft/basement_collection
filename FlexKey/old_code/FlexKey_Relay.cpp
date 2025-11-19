#include "FlexKey_Relay.h"

FlexKeyRelay::FlexKeyRelay() 
    : currentState(false), pulseActive(false), 
      pulseStartTime(0), pulseDuration(0) {
}

void FlexKeyRelay::begin() {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
    currentState = false;
    Serial.println("[RELAY] Initialized on pin " + String(PIN_RELAY));
}

void FlexKeyRelay::toggle() {
    setState(!currentState);
    Serial.println("[RELAY] Toggled to " + String(currentState ? "ON" : "OFF"));
}

void FlexKeyRelay::pulse(uint16_t durationMs) {
    if (pulseActive) {
        Serial.println("[RELAY] Pulse already active, ignoring");
        return;
    }
    
    Serial.println("[RELAY] Pulse started (" + String(durationMs) + "ms)");
    pulseActive = true;
    pulseDuration = durationMs;
    pulseStartTime = millis();
    turnOn();
}

void FlexKeyRelay::setState(bool state) {
    currentState = state;
    digitalWrite(PIN_RELAY, state ? HIGH : LOW);
}

bool FlexKeyRelay::getState() {
    return currentState;
}

void FlexKeyRelay::turnOn() {
    setState(true);
    Serial.println("[RELAY] Turned ON");
}

void FlexKeyRelay::turnOff() {
    setState(false);
    Serial.println("[RELAY] Turned OFF");
}

void FlexKeyRelay::update() {
    // Handle pulse timing
    if (pulseActive) {
        if (millis() - pulseStartTime >= pulseDuration) {
            turnOff();
            pulseActive = false;
            Serial.println("[RELAY] Pulse completed");
        }
    }
}
