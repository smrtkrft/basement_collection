#ifndef FLEXKEY_HTTP_H
#define FLEXKEY_HTTP_H

#include "FlexKey_Config.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

class FlexKeyHTTP {
public:
    FlexKeyHTTP();
    
    // Send GET request to URL (non-blocking, fire and forget)
    // Returns immediately, errors are logged to Serial
    void sendGET(const String& url);
    
    // Send multiple GET requests
    void sendMultipleGET(const String* urls, uint8_t count);
    
    // Check if busy (for future expansion)
    bool isBusy();
    
private:
    bool busy;
    
    // Helper to determine if URL is HTTPS
    bool isHTTPS(const String& url);
    
    // Send single request (internal)
    void sendRequest(const String& url);
};

#endif // FLEXKEY_HTTP_H
