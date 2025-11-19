#include "FlexKey_HTTP.h"
#include <WiFi.h>

FlexKeyHTTP::FlexKeyHTTP() : busy(false) {
}

void FlexKeyHTTP::sendGET(const String& url) {
    if (url.length() == 0) {
        Serial.println("[HTTP] Empty URL, skipping");
        return;
    }
    
    sendRequest(url);
}

void FlexKeyHTTP::sendMultipleGET(const String* urls, uint8_t count) {
    Serial.println("[HTTP] Sending " + String(count) + " GET requests...");
    
    for (uint8_t i = 0; i < count; i++) {
        if (urls[i].length() > 0) {
            sendRequest(urls[i]);
            delay(50);  // Small delay between requests to prevent overwhelming
        }
    }
    
    Serial.println("[HTTP] All requests sent");
}

bool FlexKeyHTTP::isBusy() {
    return busy;
}

bool FlexKeyHTTP::isHTTPS(const String& url) {
    return url.startsWith("https://") || url.startsWith("HTTPS://");
}

void FlexKeyHTTP::sendRequest(const String& url) {
    Serial.println("[HTTP] GET: " + url);
    
    HTTPClient http;
    WiFiClient client;
    WiFiClientSecure secureClient;
    
    bool useHTTPS = isHTTPS(url);
    
    // Configure HTTPS client (skip certificate validation for simplicity)
    if (useHTTPS) {
        secureClient.setInsecure();  // Skip certificate validation
        http.begin(secureClient, url);
    } else {
        http.begin(client, url);
    }
    
    // Set timeout to prevent blocking
    http.setTimeout(5000);  // 5 seconds timeout
    
    // Send GET request
    int httpCode = http.GET();
    
    // Check response
    if (httpCode > 0) {
        Serial.println("[HTTP] Response code: " + String(httpCode));
        
        if (httpCode == HTTP_CODE_OK) {
            // Success - we don't need the response body for trigger URLs
            Serial.println("[HTTP] Request successful");
        } else {
            Serial.println("[HTTP] Request returned non-OK status");
        }
    } else {
        // Error occurred
        Serial.print("[HTTP] ERROR: ");
        Serial.println(http.errorToString(httpCode));
        Serial.println("[HTTP] Error code: " + String(httpCode));
        Serial.println("[HTTP] System continues operation (non-blocking)");
    }
    
    // Clean up
    http.end();
}
