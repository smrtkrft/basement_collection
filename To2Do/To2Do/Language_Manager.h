#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

/*
 * Language Manager - Minimal Version
 * 
 * This manager ONLY handles language selection state.
 * All translations are stored in JavaScript (Web_JavaScript_Lang.h)
 * 
 * Supported Languages:
 * - EN (English) - Default
 * - DE (Deutsch/German)
 * - TR (Türkçe/Turkish)
 * 
 * Storage: language setting is saved in settings.json via PersistenceManager
 */

class LanguageManager {
private:
  String currentLanguage;
  
  // Valid language codes
  const char* VALID_LANGUAGES[3] = {"EN", "DE", "TR"};
  const char* DEFAULT_LANGUAGE = "EN";
  
public:
  LanguageManager() : currentLanguage("EN") {
    // Default to English
  }
  
  // Initialize with saved language from settings
  void begin(const String& savedLang = "") {
    if (savedLang.length() > 0 && isValidLanguage(savedLang)) {
      currentLanguage = savedLang;
      currentLanguage.toUpperCase();
    } else {
      currentLanguage = String(DEFAULT_LANGUAGE);
    }
    
    Serial.printf("[Language] Initialized: %s\n", currentLanguage.c_str());
  }
  
  // Get current language code
  String getCurrentLanguage() const {
    return currentLanguage;
  }
  
  // Set language (validates input)
  bool setLanguage(const String& lang) {
    String upperLang = lang;
    upperLang.toUpperCase();
    
    if (!isValidLanguage(upperLang)) {
      Serial.printf("[Language] ✗ Invalid language: %s\n", lang.c_str());
      return false;
    }
    
    currentLanguage = upperLang;
    Serial.printf("[Language] ✓ Changed to: %s\n", currentLanguage.c_str());
    return true;
  }
  
  // Check if language code is valid
  bool isValidLanguage(const String& lang) const {
    String upperLang = lang;
    upperLang.toUpperCase();
    
    for (int i = 0; i < 3; i++) {
      if (upperLang == VALID_LANGUAGES[i]) {
        return true;
      }
    }
    return false;
  }
  
  // Get all supported languages as JSON array
  String getSupportedLanguages() const {
    return "[\"EN\",\"DE\",\"TR\"]";
  }
  
  // Get language info as JSON
  String getLanguageInfo() const {
    JsonDocument doc;
    doc["current"] = currentLanguage;
    doc["default"] = DEFAULT_LANGUAGE;
    JsonArray supported = doc["supported"].to<JsonArray>();
    supported.add("EN");
    supported.add("DE");
    supported.add("TR");
    
    String result;
    serializeJson(doc, result);
    return result;
  }
};

#endif
