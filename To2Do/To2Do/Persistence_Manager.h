#ifndef PERSISTENCE_MANAGER_H
#define PERSISTENCE_MANAGER_H

#include "Data_Manager.h"

class PersistenceManager {
private:
    DataManager* dataManager;
    
public:
    PersistenceManager() {
        dataManager = nullptr;
    }
    
    ~PersistenceManager() {
        if (dataManager) {
            delete dataManager;
        }
    }
    
    bool begin() {
        dataManager = new DataManager();
        return dataManager->begin();
    }
    
    String loadTodos() {
        if (!dataManager) return "{}";
        return dataManager->getTodosData();
    }
    
    bool saveTodos(const String& todosJson) {
        if (!dataManager) return false;
        return dataManager->setTodosData(todosJson);
    }
    
    String loadSettings() {
        if (!dataManager) return "{}";
        return dataManager->getSettings();
    }
    
    bool saveSettings(const String& settingsJson) {
        if (!dataManager) return false;
        return dataManager->setSettings(settingsJson);
    }
    
    String loadNetworkSettings() {
        if (!dataManager) return "{}";
        return dataManager->getNetworkSettings();
    }
    
    bool saveNetworkSettings(const String& networkJson) {
        if (!dataManager) return false;
        return dataManager->setNetworkSettings(networkJson);
    }
    
    bool factoryReset() {
        if (!dataManager) return false;
        return dataManager->factoryReset();
    }
    
    DataManager* getDataManager() {
        return dataManager;
    }
};

#endif