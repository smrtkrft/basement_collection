#ifndef FLEXKEY_WEB_H
#define FLEXKEY_WEB_H

#include "FlexKey_Config.h"
#include <WebServer.h>

// Forward declarations
class FlexKeyStorage;
class FlexKeyRFID;
class FlexKeyRelay;
class FlexKeyHTTP;

class FlexKeyWeb {
public:
    FlexKeyWeb(SystemConfig_t* config, FlexKeyStorage* storage, 
               FlexKeyRFID* rfid, FlexKeyRelay* relay, FlexKeyHTTP* http,
               LastUID_t* lastUID);
    
    // Initialize web server
    void begin();
    
    // Handle client requests (call in loop)
    void handleClient();
    
    // Get server status
    bool isRunning();
    
private:
    WebServer* server;
    SystemConfig_t* sysConfig;
    FlexKeyStorage* storage;
    FlexKeyRFID* rfid;
    FlexKeyRelay* relay;
    FlexKeyHTTP* http;
    LastUID_t* lastUID;
    
    // HTML Generators
    String generateHTML();
    String generateCSS();
    String generateJS();
    
    // Page handlers
    void handleRoot();
    void handleCSS();
    void handleJS();
    void handleNotFound();
    
    // API endpoints
    void handleAPIGetConfig();
    void handleAPISetGroupMode();
    void handleAPISetActiveGroup();
    void handleAPIGetGroups();
    void handleAPISaveGroup();
    void handleAPIDeleteUID();
    void handleAPIAddUID();
    void handleAPIGetWiFiConfig();
    void handleAPISaveWiFiConfig();
    void handleAPIGetSystemInfo();
    void handleAPITestRelay();
    void handleAPIGetLastUID();
    void handleAPISaveGlobalRelay();
    void handleAPICheckUIDConflicts();
    
    // Helper functions
    String jsonEscape(const String& str);
    bool checkUIDConflicts(String& conflictMessage);
};

#endif // FLEXKEY_WEB_H
