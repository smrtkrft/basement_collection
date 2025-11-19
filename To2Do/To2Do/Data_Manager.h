#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <SPIFFS.h>
#include <ArduinoJson.h>

/*
 * UNIFIED DATA MANAGER
 * Tüm kullanıcı verileri tek bir JSON dosyasında (/userdata.json)
 * - Projects & Tasks (Projeler ve Todo'lar)
 * - GUI Settings (Tema, kategori isimleri, app başlığı)
 * - Network Settings (WiFi credentials - Primary & Backup)
 * 
 * Factory Reset: Tüm kullanıcı verilerini siler, demo içeriği yükler
 */

class DataManager {
private:
    const char* DATA_FILE = "/userdata.json";
    const size_t JSON_CAPACITY = 24576; // 24KB buffer (reduced from 32KB)
    
    DynamicJsonDocument userData;
    
public:
    DataManager() : userData(JSON_CAPACITY) {
        // Initialize empty structure
        resetToDefaults();
    }
    
    // ==================== INITIALIZATION ====================
    
    bool begin() {
        if (!SPIFFS.begin(true)) {
            return false;
        }
        
        if (SPIFFS.exists(DATA_FILE)) {
            if (loadFromFile()) {
                return true;
            }
        }
        
        resetToDefaults();
        saveToFile();
        
        return true;
    }
    
    // ==================== CORE PERSISTENCE ====================
    
    bool loadFromFile() {
        File file = SPIFFS.open(DATA_FILE, "r");
        if (!file) {
            return false;
        }
        
        DeserializationError error = deserializeJson(userData, file);
        file.close();
        
        if (error) {
            return false;
        }
        
        return true;
    }
    
    bool saveToFile() {
        File file = SPIFFS.open(DATA_FILE, "w");
        if (!file) {
            return false;
        }
        
        size_t bytesWritten = serializeJson(userData, file);
        file.close();
        
        return (bytesWritten > 0);
    }
    
    // ==================== DATA ACCESS ====================
    
    // Get complete user data as JSON string
    String getAllData() {
        String output;
        serializeJson(userData, output);
        return output;
    }
    
    // Update complete user data from JSON string
    bool setAllData(const String& jsonString) {
        DynamicJsonDocument newDoc(JSON_CAPACITY);
        DeserializationError error = deserializeJson(newDoc, jsonString);
        
        if (error) {
            return false;
        }
        
        userData = newDoc;
        return saveToFile();
    }
    
    // Get projects array as JSON string
    String getProjects() {
        String output;
        JsonArray projects = userData["projects"];
        serializeJson(projects, output);
        return output;
    }
    
    // Get tasks array as JSON string
    String getTasks() {
        String output;
        JsonArray tasks = userData["tasks"];
        serializeJson(tasks, output);
        return output;
    }
    
    // Get projects and tasks together (frontend format)
    String getTodosData() {
        String output;
        JsonObject root = userData.as<JsonObject>();
        
        DynamicJsonDocument doc(JSON_CAPACITY);
        doc["projects"] = root["projects"];
        doc["tasks"] = root["tasks"];
        
        serializeJson(doc, output);
        return output;
    }
    
    // Update projects and tasks
    bool setTodosData(const String& jsonString) {
        DynamicJsonDocument doc(JSON_CAPACITY);
        DeserializationError error = deserializeJson(doc, jsonString);
        
        if (error) {
            return false;
        }
        
        if (!doc.containsKey("projects") || !doc.containsKey("tasks")) {
            return false;
        }
        
        userData["projects"] = doc["projects"];
        userData["tasks"] = doc["tasks"];
        
        return saveToFile();
    }
    
    // Get GUI settings as JSON string
    String getSettings() {
        String output;
        JsonObject settings = userData["settings"];
        serializeJson(settings, output);
        return output;
    }
    
    // Update GUI settings
    bool setSettings(const String& jsonString) {
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, jsonString);
        
        if (error) {
            return false;
        }
        
        JsonObject settings = userData["settings"].as<JsonObject>();
        for (JsonPair kv : doc.as<JsonObject>()) {
            settings[kv.key()] = kv.value();
        }
        
        return saveToFile();
    }
    
    // Get network settings as JSON string
    String getNetworkSettings() {
        String output;
        JsonObject network = userData["network"];
        serializeJson(network, output);
        return output;
    }
    
    // Update network settings
    bool setNetworkSettings(const String& jsonString) {
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, jsonString);
        
        if (error) {
            return false;
        }
        
        JsonObject network = userData["network"].as<JsonObject>();
        for (JsonPair kv : doc.as<JsonObject>()) {
            network[kv.key()] = kv.value();
        }
        
        return saveToFile();
    }
    

    
    // ==================== FACTORY RESET ====================
    
    bool factoryReset() {
        if (SPIFFS.exists(DATA_FILE)) {
            if (!SPIFFS.remove(DATA_FILE)) {
                return false;
            }
        }
        

        
        resetToDefaults();
        
        if (!saveToFile()) {
            return false;
        }
        
        return true;
    }
    
    // ==================== DEFAULT DATA ====================
    
    void resetToDefaults() {
        userData.clear();
        
        // GUI Settings
        JsonObject settings = userData.createNestedObject("settings");
        settings["appTitle"] = "ToDo - SmartKraft";
        settings["category1"] = "WORK";
        settings["category2"] = "PERSONAL";
        settings["category3"] = "PROJECTS";
        settings["theme"] = "dark";
        
        // Network Settings
        JsonObject network = userData.createNestedObject("network");
        network["apSSID"] = "SmartKraft-To2Do";
        network["apMDNS"] = "to2do";
        network["primarySSID"] = "";
        network["primaryPassword"] = "";
        network["primaryIP"] = "";
        network["primaryMDNS"] = "smartkraft-todo";
        network["backupSSID"] = "";
        network["backupPassword"] = "";
        network["backupIP"] = "";
        network["backupMDNS"] = "smartkraft-todo-backup";
        

        
        // Demo Projects
        JsonArray projects = userData.createNestedArray("projects");
        
        JsonObject p1 = projects.createNestedObject();
        p1["id"] = 1;
        p1["name"] = "Website Redesign";
        p1["category"] = "Work";
        p1["description"] = "Company website modernization project";
        p1["priority"] = "high";
        p1["archived"] = false;
        
        JsonObject p2 = projects.createNestedObject();
        p2["id"] = 2;
        p2["name"] = "Team Meeting Prep";
        p2["category"] = "Work";
        p2["description"] = "Prepare presentation and reports";
        p2["priority"] = "medium";
        p2["archived"] = false;
        
        JsonObject p3 = projects.createNestedObject();
        p3["id"] = 3;
        p3["name"] = "Home Improvement";
        p3["category"] = "Personal";
        p3["description"] = "Living room renovation tasks";
        p3["priority"] = "medium";
        p3["archived"] = false;
        
        JsonObject p4 = projects.createNestedObject();
        p4["id"] = 4;
        p4["name"] = "Fitness Goals";
        p4["category"] = "Personal";
        p4["description"] = "Weekly workout and diet plan";
        p4["priority"] = "high";
        p4["archived"] = false;
        
        JsonObject p5 = projects.createNestedObject();
        p5["id"] = 5;
        p5["name"] = "Smart Home System";
        p5["category"] = "Projects";
        p5["description"] = "IoT devices integration project";
        p5["priority"] = "high";
        p5["archived"] = false;
        
        // Demo Tasks (Sample - will add more)
        JsonArray tasks = userData.createNestedArray("tasks");
        
        JsonObject t1 = tasks.createNestedObject();
        t1["id"] = 1;
        t1["projectId"] = 1;
        t1["title"] = "Design homepage mockup";
        t1["description"] = "Create new homepage design with modern UI/UX principles";
        t1["type"] = "task";
        t1["priority"] = "high";
        t1["completed"] = false;
        t1["date"] = "2025-10-15";
        JsonArray t1_checklist = t1.createNestedArray("checklist");
        JsonObject t1_c1 = t1_checklist.createNestedObject();
        t1_c1["id"] = 1;
        t1_c1["text"] = "Research competitor websites";
        t1_c1["completed"] = true;
        JsonObject t1_c2 = t1_checklist.createNestedObject();
        t1_c2["id"] = 2;
        t1_c2["text"] = "Create wireframe layout";
        t1_c2["completed"] = false;
        t1.createNestedArray("dependencies");
        
        JsonObject t2 = tasks.createNestedObject();
        t2["id"] = 2;
        t2["projectId"] = 1;
        t2["title"] = "Setup development environment";
        t2["description"] = "Install required tools and frameworks";
        t2["type"] = "task";
        t2["priority"] = "high";
        t2["completed"] = true;
        t2["date"] = "2025-10-13";
        t2.createNestedArray("checklist");
        t2.createNestedArray("dependencies");
        
        JsonObject t3 = tasks.createNestedObject();
        t3["id"] = 3;
        t3["projectId"] = 3;
        t3["title"] = "Paint living room walls";
        t3["description"] = "Choose color and paint all walls";
        t3["type"] = "task";
        t3["priority"] = "medium";
        t3["completed"] = false;
        t3["date"] = "2025-10-16";
        JsonArray t3_checklist = t3.createNestedArray("checklist");
        JsonObject t3_c1 = t3_checklist.createNestedObject();
        t3_c1["id"] = 1;
        t3_c1["text"] = "Buy paint and supplies";
        t3_c1["completed"] = true;
        JsonObject t3_c2 = t3_checklist.createNestedObject();
        t3_c2["id"] = 2;
        t3_c2["text"] = "Prepare walls";
        t3_c2["completed"] = false;
        t3.createNestedArray("dependencies");
        
        JsonObject t4 = tasks.createNestedObject();
        t4["id"] = 4;
        t4["projectId"] = 4;
        t4["title"] = "Morning workout routine";
        t4["description"] = "30 minutes cardio + strength training";
        t4["type"] = "task";
        t4["priority"] = "high";
        t4["completed"] = true;
        t4["date"] = "2025-10-13";
        t4.createNestedArray("checklist");
        t4.createNestedArray("dependencies");
        
        JsonObject t5 = tasks.createNestedObject();
        t5["id"] = 5;
        t5["projectId"] = 5;
        t5["title"] = "Setup WiFi sensors";
        t5["description"] = "Install temperature and humidity sensors";
        t5["type"] = "task";
        t5["priority"] = "high";
        t5["completed"] = false;
        t5["date"] = "2025-10-17";
        JsonArray t5_checklist = t5.createNestedArray("checklist");
        JsonObject t5_c1 = t5_checklist.createNestedObject();
        t5_c1["id"] = 1;
        t5_c1["text"] = "Order ESP32 modules";
        t5_c1["completed"] = true;
        JsonObject t5_c2 = t5_checklist.createNestedObject();
        t5_c2["id"] = 2;
        t5_c2["text"] = "Mount sensors in rooms";
        t5_c2["completed"] = false;
        t5.createNestedArray("dependencies");
    }
};

#endif
