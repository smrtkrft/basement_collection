#ifndef BACKUP_MANAGER_H
#define BACKUP_MANAGER_H

#include <ArduinoJson.h>
#include "Data_Manager.h"

class BackupManager {
private:
    DataManager* dataManager;
    const size_t BACKUP_JSON_CAPACITY = 24576;
    
public:
    BackupManager(DataManager* dm) : dataManager(dm) {}
    
    String exportBackup() {
        if (!dataManager) {
            return "{\"error\":\"DataManager not initialized\"}";
        }
        
        DynamicJsonDocument backupDoc(BACKUP_JSON_CAPACITY);
        
        backupDoc["version"] = "1.0";
        backupDoc["app"] = "SmartKraft-ToDo";
        backupDoc["timestamp"] = millis();
        
        String todosData = dataManager->getTodosData();
        String settingsData = dataManager->getSettings();
        
        DynamicJsonDocument todosDoc(BACKUP_JSON_CAPACITY);
        DeserializationError error = deserializeJson(todosDoc, todosData);
        if (error) {
            return "{\"error\":\"Failed to parse todos\"}";
        }
        
        DynamicJsonDocument settingsDoc(4096);
        error = deserializeJson(settingsDoc, settingsData);
        if (error) {
            return "{\"error\":\"Failed to parse settings\"}";
        }
        
        backupDoc["projects"] = todosDoc["projects"];
        backupDoc["tasks"] = todosDoc["tasks"];
        backupDoc["settings"] = settingsDoc.as<JsonObject>();
        
        String output;
        serializeJson(backupDoc, output);
        return output;
    }
    
    bool importBackup(const String& backupJson) {
        if (!dataManager) {
            return false;
        }
        
        DynamicJsonDocument backupDoc(BACKUP_JSON_CAPACITY);
        DeserializationError error = deserializeJson(backupDoc, backupJson);
        
        if (error) {
            return false;
        }
        
        if (!backupDoc.containsKey("projects") || !backupDoc.containsKey("tasks") || !backupDoc.containsKey("settings")) {
            return false;
        }
        
        String currentNetworkSettings = dataManager->getNetworkSettings();
        
        DynamicJsonDocument todosDoc(BACKUP_JSON_CAPACITY);
        todosDoc["projects"] = backupDoc["projects"];
        todosDoc["tasks"] = backupDoc["tasks"];
        
        String todosJson;
        serializeJson(todosDoc, todosJson);
        
        if (!dataManager->setTodosData(todosJson)) {
            return false;
        }
        
        String settingsJson;
        serializeJson(backupDoc["settings"], settingsJson);
        
        if (!dataManager->setSettings(settingsJson)) {
            return false;
        }
        
        dataManager->setNetworkSettings(currentNetworkSettings);
        
        return true;
    }
};

#endif
