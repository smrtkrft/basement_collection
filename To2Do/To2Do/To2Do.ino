/*
 * ESP32-C6 To2Do App - SmartKraft Edition
 * All data persists across firmware updates (stored in SPIFFS)
 * Smart WiFi Manager with AP Mode fallback
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "Persistence_Manager.h"
#include "WiFi_Manager.h"
#include "Web_Interface.h"
#include "Web_CSS.h"
#include "Web_JavaScript.h"
#include "Web_JavaScript_Lang.h"
#include "Web_JavaScript_Lang_Handler.h"
#include "Backup_Manager.h"
#include "Time_Manager.h"
#include "Notification_Manager.h"
#include "Display_Manager.h"
#include "Language_Manager.h"

WebServer server(80);
PersistenceManager persistence;
WiFiManager* wifiManager;
BackupManager* backupManager;
NotificationManager* notificationManager;
TimeManager* timeManager;
DisplayManager* displayManager;
LanguageManager* languageManager;

const size_t JSON_BUFFER_SIZE = 16384;

// No more time sync tracking - browser provides all time data
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_UPDATE_INTERVAL = 5000; // Update every 5 seconds
bool firstDisplayUpdate = true; // Flag to force immediate first update



void setup() {
  Serial.begin(115200);
  delay(500);
  
  Serial.println("\n=== SmartKraft To2Do ===");
  unsigned long setupStart = millis();
  
  displayManager = new DisplayManager();
  if (displayManager->begin()) {
    Serial.println("[Setup] ✓ OLED ready");
  } else {
    Serial.println("[Setup] ⚠ OLED not found - will retry in background");
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(200);
  
  if (!persistence.begin()) {
    Serial.println("[ERROR] ✗ Persistence failed!");
    return;
  }
  
  backupManager = new BackupManager(persistence.getDataManager());
  notificationManager = new NotificationManager(persistence.getDataManager());
  timeManager = new TimeManager();
  languageManager = new LanguageManager();
  
  // Load saved language from settings
  String settingsData = persistence.getDataManager()->getSettings();
  JsonDocument settingsDoc;
  deserializeJson(settingsDoc, settingsData);
  String savedLang = settingsDoc["language"] | "EN";
  languageManager->begin(savedLang);
  
  notificationManager->setTimeManager(timeManager);
  
  if (timeManager->loadDateFromSPIFFS() && !timeManager->isDateValid()) {
    timeManager->setManualDate(2025, 10, 21, 12, 0);
  }
  
  if (displayManager && displayManager->isDisplayFound()) {
    String settingsData = persistence.getDataManager()->getSettings();
    JsonDocument settingsDoc;
    deserializeJson(settingsDoc, settingsData);
    String appTitle = settingsDoc["appTitle"] | "To2Do-SmartKraft";
    
    displayManager->setAppTitle(appTitle.c_str());
    
    // Set language for OLED
    if (languageManager) {
      displayManager->setLanguage(languageManager->getCurrentLanguage());
    }
    
    // Get task counts immediately
    String todayJson = notificationManager->getNotifications("today");
    String tomorrowJson = notificationManager->getNotifications("tomorrow");
    String weekJson = notificationManager->getNotifications("week");
    
    JsonDocument todayDoc, tomorrowDoc, weekDoc;
    deserializeJson(todayDoc, todayJson);
    deserializeJson(tomorrowDoc, tomorrowJson);
    deserializeJson(weekDoc, weekJson);
    
    int todayCount = todayDoc["count"] | 0;
    int tomorrowCount = tomorrowDoc["count"] | 0;
    int weekCount = weekDoc["count"] | 0;
    
    displayManager->setTaskCounts(todayCount, tomorrowCount, weekCount);
  }
  
  wifiManager = new WiFiManager(&persistence);
  wifiManager->begin();
  
  setupServerRoutes();
  server.begin();
  
  updateDisplayNetworkInfo();
  
  if (displayManager && displayManager->isDisplayFound()) {
    displayManager->setSystemReady();
  }
  
  Serial.printf("[Setup] ✓ Ready (%lums)\n\n", millis() - setupStart);
}

void loop() {
  wifiManager->loop();
  server.handleClient();
  
  // Display Manager loop - handles button and screen updates
  // Safe to call even if display is not found or fails
  if (displayManager) {
    displayManager->loop(); // This is safe - internally checks displayFound
    
    // Only update display data if display is actually working
    if (displayManager->isDisplayFound()) {
      // Update all display data periodically (every 5 seconds) or immediately on first run
      unsigned long currentTime = millis();
      if (firstDisplayUpdate || (currentTime - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)) {
        lastDisplayUpdate = currentTime;
        firstDisplayUpdate = false; // Clear the flag after first update
        
        // Update app title from settings
        updateDisplayAppTitle();
        
        // Update task counts
        updateDisplayTaskCounts();
        
        // Refresh current page on screen
        displayManager->refreshCurrentPage();
      }
    }
  }
  
  // No more internet time sync - browser provides time via /api/time endpoint
  
  delay(2); // Small delay for stability
}

void updateDisplayAppTitle() {
  if (!displayManager) return;
  
  String settingsData = persistence.getDataManager()->getSettings();
  JsonDocument settingsDoc;
  deserializeJson(settingsDoc, settingsData);
  String appTitle = settingsDoc["appTitle"] | "To2Do-SmartKraft";
  
  displayManager->setAppTitle(appTitle.c_str());
  
  // Update language for OLED
  if (languageManager) {
    displayManager->setLanguage(languageManager->getCurrentLanguage());
  }
}

void updateDisplayTaskCounts() {
  if (!displayManager || !notificationManager || !timeManager) return;
  
  // Get task counts from notification manager using getNotifications
  String todayJson = notificationManager->getNotifications("today");
  String tomorrowJson = notificationManager->getNotifications("tomorrow");
  String weekJson = notificationManager->getNotifications("week");
  
  // Parse JSON to get counts
  JsonDocument todayDoc;
  JsonDocument tomorrowDoc;
  JsonDocument weekDoc;
  
  deserializeJson(todayDoc, todayJson);
  deserializeJson(tomorrowDoc, tomorrowJson);
  deserializeJson(weekDoc, weekJson);
  
  int todayCount = todayDoc["count"] | 0;
  int tomorrowCount = tomorrowDoc["count"] | 0;
  int weekCount = weekDoc["count"] | 0;
  
  // Update display
  displayManager->setTaskCounts(todayCount, tomorrowCount, weekCount);
  
  // Update network info
  updateDisplayNetworkInfo();
}

void updateDisplayNetworkInfo() {
  if (!displayManager || !wifiManager) return;
  
  String ssid = "";
  String ip = "";
  String local = "";
  
  String networkData = persistence.getDataManager()->getNetworkSettings();
  JsonDocument networkDoc;
  deserializeJson(networkDoc, networkData);
  
  if (WiFi.getMode() == WIFI_AP) {
    String apMDNS = networkDoc["apMDNS"] | "to2do";
    ssid = "AP: " + String(WiFi.softAPSSID());
    ip = WiFi.softAPIP().toString();
    local = String(apMDNS) + ".local";
  } else if (WiFi.status() == WL_CONNECTED) {
    String priMDNS = networkDoc["primaryMDNS"] | "smartkraft-to2do";
    ssid = WiFi.SSID();
    ip = WiFi.localIP().toString();
    local = String(priMDNS) + ".local";
  } else {
    // Not connected
    ssid = "Not Connected";
    ip = "---";
    local = "---";
  }
  
  displayManager->setNetworkInfo(ssid, ip, local);
}

void setupServerRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/app.js", HTTP_GET, []() {
    server.send(200, "application/javascript", getJavaScriptCombined());
  });
  server.on("/app.css", HTTP_GET, []() {
    server.send(200, "text/css", getCSS());
  });
  server.on("/api/todos", HTTP_GET, handleGetTodos);
  server.on("/api/todos", HTTP_POST, handleCreateTodo);
  server.on("/api/todos", HTTP_PUT, handleUpdateTodo);
  server.on("/api/todos", HTTP_DELETE, handleDeleteTodo);
  
  // Settings API endpoints
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, handleSaveSettings);
  
  // Language API endpoints
  server.on("/api/language", HTTP_GET, handleGetLanguage);
  server.on("/api/language", HTTP_POST, handleSetLanguage);
  server.on("/lang.js", HTTP_GET, []() {
    server.send(200, "application/javascript", getLanguageJS());
  });
  server.on("/lang-handler.js", HTTP_GET, []() {
    server.send(200, "application/javascript", getLanguageHandlerJS());
  });
  
  // Network API endpoints
  server.on("/api/network/status", HTTP_GET, handleNetworkStatus);
  server.on("/api/network/settings", HTTP_GET, handleGetNetworkSettings);
  server.on("/api/network/config", HTTP_POST, handleNetworkConfig);
  server.on("/api/network/test", HTTP_POST, handleNetworkTest);
  
  // System API endpoints
  server.on("/api/factory-reset", HTTP_POST, handleFactoryReset);
  server.on("/api/system/info", HTTP_GET, handleSystemInfo);
  
  // Backup API endpoints
  server.on("/api/backup/export", HTTP_GET, handleBackupExport);
  server.on("/api/backup/import", HTTP_POST, handleBackupImport);
  
  // Notification API endpoints
  server.on("/api/notifications/today", HTTP_GET, []() {
    handleNotifications("today");
  });
  server.on("/api/notifications/tomorrow", HTTP_GET, []() {
    handleNotifications("tomorrow");
  });
  server.on("/api/notifications/week", HTTP_GET, []() {
    handleNotifications("week");
  });
  server.on("/api/notifications/overdue", HTTP_GET, []() {
    handleNotifications("overdue");
  });
  server.on("/api/notifications/timezone", HTTP_POST, handleSetTimezone);
  
  // Time API endpoints
  server.on("/api/time", HTTP_GET, handleGetTime);
  server.on("/api/time", HTTP_POST, handleSetManualTime);
  server.on("/api/time/sync", HTTP_POST, handleTimeSyncNow);
  
  server.on("/api/health", HTTP_GET, []() {
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  });
  server.onNotFound([]() {
    server.send(404, "text/plain", "404");
  });
}

void handleRoot() {
  Serial.println("[Root] Root page requested");
  Serial.println("[Root] Serving main page");
  server.send(200, "text/html", getHTML());
}

void handleGetTodos() {
  Serial.println("[API] GET /api/todos - Loading todos...");
  String content = persistence.loadTodos();
  Serial.printf("[API] Sending %d bytes\n", content.length());
  server.send(200, "application/json", content);
}

void handleCreateTodo() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  String body = server.arg("plain");
  
  Serial.println("[Todos] Received save request");
  Serial.printf("[Todos] Data size: %d bytes\n", body.length());
  
  if (persistence.saveTodos(body)) {
    Serial.println("[Todos] ✓ Saved successfully to SPIFFS");
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    Serial.println("[Todos] ✗ Save failed!");
    server.send(500, "application/json", "{\"error\":\"Write failed\"}");
  }
}

void handleUpdateTodo() {
  server.send(200, "application/json", "{\"success\":true}");
}

void handleDeleteTodo() {
  server.send(200, "application/json", "{\"success\":true}");
}

// ==================== NETWORK API ====================

void handleNetworkStatus() {
  String json = wifiManager->getStatusJSON();
  server.send(200, "application/json", json);
}

void handleGetNetworkSettings() {
  String networkSettings = persistence.loadNetworkSettings();
  server.send(200, "application/json", networkSettings);
}

void handleNetworkConfig() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  String body = server.arg("plain");
  Serial.println("[Network] Received network config:");
  Serial.println(body);
  
  // Parse JSON to extract WiFi settings
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // Extract settings
  String apSSID = doc["apSSID"] | "SmartKraft-To2Do";
  String apMDNS = doc["apMDNS"] | "to2do";
  
  String priSSID = doc["primarySSID"] | "";
  String priPass = doc["primaryPassword"] | "";
  String priIP = doc["primaryIP"] | "";
  String priMDNS = doc["primaryMDNS"] | "";
  
  String bkpSSID = doc["backupSSID"] | "";
  String bkpPass = doc["backupPassword"] | "";
  String bkpIP = doc["backupIP"] | "";
  String bkpMDNS = doc["backupMDNS"] | "";
  
  Serial.printf("[Network] AP SSID: %s (mDNS: %s.local)\n", apSSID.c_str(), apMDNS.c_str());
  Serial.printf("[Network] Primary SSID: %s\n", priSSID.c_str());
  Serial.printf("[Network] Primary IP: %s\n", priIP.c_str());
  Serial.printf("[Network] Primary mDNS: %s\n", priMDNS.c_str());
  
  // Save network settings to persistence
  bool saved = persistence.saveNetworkSettings(body);
  Serial.printf("[Network] Data saved to SPIFFS: %s\n", saved ? "YES" : "NO");
  
  // Apply new settings to WiFi Manager
  wifiManager->applyNewSettings(apSSID, apMDNS,
                                 priSSID, priPass, priIP, priMDNS,
                                 bkpSSID, bkpPass, bkpIP, bkpMDNS);
  
  server.send(200, "application/json", "{\"success\":true,\"message\":\"Settings saved and connecting...\"}");
}

void handleNetworkTest() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  String body = server.arg("plain");
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String type = doc["type"] | "";
  String ssid = doc["ssid"] | "";
  
  if (ssid.isEmpty()) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"SSID is required\"}");
    return;
  }
  
  // Scan for the specified network
  Serial.printf("[WiFi Test] Scanning for network: %s\n", ssid.c_str());
  
  int n = WiFi.scanNetworks();
  bool found = false;
  int rssi = 0;
  
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == ssid) {
      found = true;
      rssi = WiFi.RSSI(i);
      break;
    }
  }
  
  WiFi.scanDelete();
  
  if (found) {
    String response = "{\"success\":true,\"message\":\"Network found\",\"rssi\":";
    response += String(rssi);
    response += "}";
    server.send(200, "application/json", response);
  } else {
    server.send(200, "application/json", "{\"success\":false,\"message\":\"Network not found\"}");
  }
}

// ==================== SETTINGS API ====================

void handleGetSettings() {
  String settings = persistence.loadSettings();
  server.send(200, "application/json", settings);
}

void handleSaveSettings() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  String body = server.arg("plain");
  
  if (persistence.saveSettings(body)) {
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Settings saved\"}");
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed to save settings\"}");
  }
}

// ==================== SYSTEM API ====================

void handleFactoryReset() {
  Serial.println("\n!!! FACTORY RESET REQUESTED !!!");
  
  // Reset all user data (projects, tasks, settings)
  if (persistence.factoryReset()) {
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Factory reset completed. Restarting...\"}");
    delay(1000);
    ESP.restart();
  } else {
    server.send(500, "application/json", "{\"error\":\"Factory reset failed\"}");
  }
}

void handleSystemInfo() {
  DynamicJsonDocument doc(1024);
  
  doc["version"] = "SmartKraft-To2Do V1.1";
  doc["chipModel"] = ESP.getChipModel();
  doc["chipCores"] = ESP.getChipCores();
  doc["cpuFreq"] = ESP.getCpuFreqMHz();
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["heapSize"] = ESP.getHeapSize();
  doc["flashSize"] = ESP.getFlashChipSize();
  doc["spiffsTotal"] = SPIFFS.totalBytes();
  doc["spiffsUsed"] = SPIFFS.usedBytes();
  doc["spiffsFree"] = SPIFFS.totalBytes() - SPIFFS.usedBytes();
  doc["uptime"] = millis() / 1000;
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleBackupExport() {
  if (!backupManager) {
    server.send(500, "application/json", "{\"error\":\"Backup manager not ready\"}");
    return;
  }
  
  String backupData = backupManager->exportBackup();
  server.sendHeader("Content-Disposition", "attachment; filename=backup.json");
  server.send(200, "application/json", backupData);
}

void handleBackupImport() {
  if (!backupManager) {
    server.send(500, "application/json", "{\"error\":\"Backup manager not ready\"}");
    return;
  }
  
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  String backupData = server.arg("plain");
  
  if (backupManager->importBackup(backupData)) {
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(500, "application/json", "{\"error\":\"Import failed\"}");
  }
}

void handleNotifications(String filterType) {
  if (!notificationManager) {
    server.send(500, "application/json", "{\"error\":\"Notification manager not ready\"}");
    return;
  }
  
  Serial.printf("[API] GET /api/notifications/%s\n", filterType.c_str());
  
  String notifications = notificationManager->getNotifications(filterType);
  
  Serial.printf("[API] Notification response length: %d bytes\n", notifications.length());
  
  server.send(200, "application/json", notifications);
}

void handleSetTimezone() {
  if (!notificationManager) {
    server.send(500, "application/json", "{\"error\":\"Notification manager not ready\"}");
    return;
  }
  
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  DynamicJsonDocument doc(256);
  deserializeJson(doc, server.arg("plain"));
  
  int offset = doc["offset"] | 0;
  notificationManager->setTimezoneOffset(offset);
  
  server.send(200, "application/json", "{\"success\":true}");
}

// Time API handlers
void handleGetTime() {
  if (!timeManager) {
    server.send(500, "application/json", "{\"error\":\"Time manager not ready\"}");
    return;
  }
  
  DynamicJsonDocument doc(512);
  doc["date"] = timeManager->getFormattedDate();
  doc["time"] = timeManager->getFormattedTime();
  doc["lastSync"] = timeManager->getLastSyncTime();
  doc["isValid"] = timeManager->isDateValid();
  doc["wifiConnected"] = (WiFi.status() == WL_CONNECTED);
  
  String response;
  serializeJson(doc, response);
  
  Serial.printf("[API] GET /api/time - Date: %s, Time: %s, Valid: %s\n", 
                doc["date"].as<String>().c_str(), 
                doc["time"].as<String>().c_str(),
                doc["isValid"].as<bool>() ? "true" : "false");
  
  server.send(200, "application/json", response);
}

void handleSetManualTime() {
  if (!timeManager) {
    server.send(500, "application/json", "{\"error\":\"Time manager not ready\"}");
    return;
  }
  
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"No data\"}");
    return;
  }
  
  String requestBody = server.arg("plain");
  Serial.printf("[API] POST /api/time - Request body: %s\n", requestBody.c_str());
  
  DynamicJsonDocument doc(256);
  deserializeJson(doc, requestBody);
  
  int year = doc["year"] | 0;
  int month = doc["month"] | 0;
  int day = doc["day"] | 0;
  int hour = doc["hour"] | 0;
  int minute = doc["minute"] | 0;
  
  Serial.printf("[API] Browser date received: %04d-%02d-%02d %02d:%02d\n", 
                year, month, day, hour, minute);
  
  if (year < 2024 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
    Serial.printf("[API] INVALID date/time values: %04d-%02d-%02d %02d:%02d\n",
                 year, month, day, hour, minute);
    server.send(400, "application/json", "{\"error\":\"Invalid date\"}");
    return;
  }
  
  Serial.println("[API] Calling timeManager->setManualDate()...");
  timeManager->setManualDate(year, month, day, hour, minute);
  Serial.println("[API] Date set and saved to SPIFFS successfully");
  
  DynamicJsonDocument response(256);
  response["success"] = true;
  response["date"] = timeManager->getFormattedDate();
  response["time"] = timeManager->getFormattedTime();
  response["saved"] = true;
  
  String responseStr;
  serializeJson(response, responseStr);
  Serial.printf("[API] Response: %s\n", responseStr.c_str());
  server.send(200, "application/json", responseStr);
}

void handleTimeSyncNow() {
  if (!timeManager) {
    server.send(500, "application/json", "{\"error\":\"Time manager not ready\"}");
    return;
  }
  
  // No more internet sync - just return current time from browser-synced data
  DynamicJsonDocument doc(256);
  doc["success"] = true;
  doc["date"] = timeManager->getFormattedDate();
  doc["time"] = timeManager->getFormattedTime();
  doc["source"] = "browser";
  doc["message"] = "Time is synced from browser, not from internet";
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleGetLanguage() {
  if (!languageManager) {
    server.send(500, "application/json", "{\"error\":\"Language manager not ready\"}");
    return;
  }
  
  String response = languageManager->getLanguageInfo();
  server.send(200, "application/json", response);
}

void handleSetLanguage() {
  if (!languageManager) {
    server.send(500, "application/json", "{\"error\":\"Language manager not ready\"}");
    return;
  }
  
  String body = server.arg("plain");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  String newLang = doc["language"] | "";
  if (newLang.length() == 0) {
    server.send(400, "application/json", "{\"error\":\"Language code required\"}");
    return;
  }
  
  if (!languageManager->setLanguage(newLang)) {
    server.send(400, "application/json", "{\"error\":\"Invalid language code\"}");
    return;
  }
  
  // Save to settings.json
  String settingsData = persistence.getDataManager()->getSettings();
  JsonDocument settingsDoc;
  deserializeJson(settingsDoc, settingsData);
  settingsDoc["language"] = languageManager->getCurrentLanguage();
  
  String updatedSettings;
  serializeJson(settingsDoc, updatedSettings);
  
  if (persistence.getDataManager()->setSettings(updatedSettings)) {
    JsonDocument response;
    response["success"] = true;
    response["language"] = languageManager->getCurrentLanguage();
    
    String responseStr;
    serializeJson(response, responseStr);
    server.send(200, "application/json", responseStr);
    
    Serial.printf("[Language] Saved: %s\n", languageManager->getCurrentLanguage().c_str());
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed to save language\"}");
  }
}




