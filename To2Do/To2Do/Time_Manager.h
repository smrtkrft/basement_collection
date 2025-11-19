#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <time.h>

class TimeManager {
private:
    struct DateTimeInfo {
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        String timezone;
        int offset; // UTC offset in seconds
        unsigned long lastSync; // millis() when last synced
    };
    
    DateTimeInfo currentDateTime;
    bool isInitialized;
    unsigned long lastMillis;  // For real-time updates
    
    // Update time based on millis() elapsed
    void updateCurrentTime() {
        if (!isInitialized || currentDateTime.lastSync == 0) {
            return;
        }
        
        unsigned long currentMillis = millis();
        unsigned long elapsed = currentMillis - currentDateTime.lastSync;
        
        // Add elapsed seconds to the time
        unsigned long totalSeconds = elapsed / 1000;
        
        if (totalSeconds > 0) {
            currentDateTime.second += totalSeconds;
            
            // Handle overflow
            if (currentDateTime.second >= 60) {
                currentDateTime.minute += currentDateTime.second / 60;
                currentDateTime.second %= 60;
                
                if (currentDateTime.minute >= 60) {
                    currentDateTime.hour += currentDateTime.minute / 60;
                    currentDateTime.minute %= 60;
                    
                    if (currentDateTime.hour >= 24) {
                        currentDateTime.day += currentDateTime.hour / 24;
                        currentDateTime.hour %= 24;
                        
                        // Simple month overflow (not perfect but good enough)
                        if (currentDateTime.day > 31) {
                            currentDateTime.month++;
                            currentDateTime.day = 1;
                            
                            if (currentDateTime.month > 12) {
                                currentDateTime.year++;
                                currentDateTime.month = 1;
                            }
                        }
                    }
                }
            }
            
            // Update the sync time to current millis
            currentDateTime.lastSync = currentMillis;
        }
    }
    
public:
    TimeManager() : isInitialized(false), lastMillis(0) {
        currentDateTime.year = 0;
        currentDateTime.month = 0;
        currentDateTime.day = 0;
        currentDateTime.hour = 0;
        currentDateTime.minute = 0;
        currentDateTime.second = 0;
        currentDateTime.timezone = "";
        currentDateTime.offset = 0;
        currentDateTime.lastSync = 0;
    }
    
    // Load date from SPIFFS
    bool loadDateFromSPIFFS() {
        File file = SPIFFS.open("/data/last_date.json", "r");
        if (!file) {
            return false;
        }
        
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        
        if (error) {
            return false;
        }
        
        currentDateTime.year = doc["year"] | 0;
        currentDateTime.month = doc["month"] | 0;
        currentDateTime.day = doc["day"] | 0;
        currentDateTime.hour = doc["hour"] | 0;
        currentDateTime.minute = doc["minute"] | 0;
        currentDateTime.second = doc["seconds"] | 0;
        currentDateTime.timezone = doc["timezone"].as<String>();
        currentDateTime.offset = doc["offset"] | 0;
        currentDateTime.lastSync = doc["lastSync"] | 0;
        
        isInitialized = true;
        return true;
    }
    
    // Save date to SPIFFS
    void saveDateToSPIFFS() {
        DynamicJsonDocument doc(512);
        
        doc["year"] = currentDateTime.year;
        doc["month"] = currentDateTime.month;
        doc["day"] = currentDateTime.day;
        doc["hour"] = currentDateTime.hour;
        doc["minute"] = currentDateTime.minute;
        doc["seconds"] = currentDateTime.second;
        doc["timezone"] = currentDateTime.timezone;
        doc["offset"] = currentDateTime.offset;
        doc["lastSync"] = currentDateTime.lastSync;
        
        File file = SPIFFS.open("/data/last_date.json", "w");
        if (file) {
            serializeJson(doc, file);
            file.close();
        }
    }
    
    // Check if date is valid (after 2024)
    bool isDateValid() {
        return isInitialized && (currentDateTime.year > 2024);
    }
    
    // Get formatted date: "14.10.2025"
    String getFormattedDate() {
        updateCurrentTime();  // Update time before formatting
        
        if (!isInitialized || currentDateTime.year == 0) {
            return "--.--.----";
        }
        
        char buffer[16];
        sprintf(buffer, "%02d.%02d.%04d", 
                currentDateTime.day, 
                currentDateTime.month, 
                currentDateTime.year);
        return String(buffer);
    }
    
    // Get formatted time: "15:30"
    String getFormattedTime() {
        updateCurrentTime();  // Update time before formatting
        
        if (!isInitialized || currentDateTime.year == 0) {
            return "--:--";
        }
        
        char buffer[8];
        sprintf(buffer, "%02d:%02d", 
                currentDateTime.hour, 
                currentDateTime.minute);
        return String(buffer);
    }
    
    // Get last sync time in human readable format
    String getLastSyncTime() {
        if (currentDateTime.lastSync == 0) {
            return "Hic";
        }
        
        unsigned long elapsed = millis() - currentDateTime.lastSync;
        unsigned long hours = elapsed / 3600000;
        unsigned long minutes = (elapsed % 3600000) / 60000;
        
        char buffer[32];
        if (hours > 0) {
            sprintf(buffer, "%lu saat %lu dk once", hours, minutes);
        } else {
            sprintf(buffer, "%lu dk once", minutes);
        }
        return String(buffer);
    }
    
    // Set manual date
    void setManualDate(int year, int month, int day, int hour, int minute) {
        Serial.printf("[Time] setManualDate called: %04d-%02d-%02d %02d:%02d\n",
                     year, month, day, hour, minute);
        
        currentDateTime.year = year;
        currentDateTime.month = month;
        currentDateTime.day = day;
        currentDateTime.hour = hour;
        currentDateTime.minute = minute;
        currentDateTime.second = 0;
        currentDateTime.lastSync = millis();
        
        isInitialized = true;
        
        Serial.println("[Time] Saving date to SPIFFS...");
        saveDateToSPIFFS();
        Serial.printf("[Time] Date saved to SPIFFS: %02d.%02d.%04d %02d:%02d\n",
                     day, month, year, hour, minute);
        Serial.printf("[Time] Current date string: %s\n", getCurrentDateString().c_str());
    }
    
    // Get date info for notifications (YYYY-MM-DD format)
    String getCurrentDateString() {
        if (!isInitialized || currentDateTime.year == 0) {
            return "";
        }
        
        char buffer[16];
        sprintf(buffer, "%04d-%02d-%02d", 
                currentDateTime.year, 
                currentDateTime.month, 
                currentDateTime.day);
        return String(buffer);
    }
    
    // Get date components
    void getDateComponents(int &year, int &month, int &day) {
        updateCurrentTime();  // Update time before getting components
        year = currentDateTime.year;
        month = currentDateTime.month;
        day = currentDateTime.day;
    }
};

#endif
