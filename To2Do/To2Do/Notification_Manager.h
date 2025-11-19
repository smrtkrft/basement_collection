#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <ArduinoJson.h>
#include "Data_Manager.h"
#include "Time_Manager.h"

class NotificationManager {
private:
    DataManager* dataManager;
    TimeManager* timeManager;
    int timezoneOffset; // in hours (fallback)
    
    struct DateInfo {
        int year;
        int month;
        int day;
    };
    
public:
    NotificationManager(DataManager* dm) : dataManager(dm), timeManager(nullptr), timezoneOffset(0) {}
    
    void setTimeManager(TimeManager* tm) {
        timeManager = tm;
    }
    
    void setTimezoneOffset(int offset) {
        timezoneOffset = offset;
    }
    
    DateInfo getCurrentDate() {
        DateInfo date;
        
        // Try to get date from TimeManager first (browser-synced date from SPIFFS)
        if (timeManager) {
            int year, month, day;
            timeManager->getDateComponents(year, month, day);
            
            if (year > 2024) { // Valid date
                date.year = year;
                date.month = month;
                date.day = day;
                return date;
            }
        }
        
        // Fallback to system time (ESP32 built-in)
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Check if time is valid (after 2024)
        if (timeinfo.tm_year + 1900 > 2024) {
            date.year = timeinfo.tm_year + 1900;
            date.month = timeinfo.tm_mon + 1;
            date.day = timeinfo.tm_mday;
            return date;
        }
        
        // Final fallback: use current actual date (October 21, 2025)
        date.year = 2025;
        date.month = 10;
        date.day = 21;
        
        return date;
    }
    
    String dateToString(DateInfo date) {
        char buffer[11];
        sprintf(buffer, "%04d-%02d-%02d", date.year, date.month, date.day);
        return String(buffer);
    }
    
    DateInfo stringToDate(String dateStr) {
        DateInfo date;
        if (dateStr.length() >= 10) {
            date.year = dateStr.substring(0, 4).toInt();
            date.month = dateStr.substring(5, 7).toInt();
            date.day = dateStr.substring(8, 10).toInt();
        } else {
            date.year = 0;
            date.month = 0;
            date.day = 0;
        }
        return date;
    }
    
    DateInfo addDays(DateInfo date, int days) {
        struct tm timeinfo = {0};
        timeinfo.tm_year = date.year - 1900;
        timeinfo.tm_mon = date.month - 1;
        timeinfo.tm_mday = date.day + days;
        
        time_t timestamp = mktime(&timeinfo);
        struct tm* newTime = localtime(&timestamp);
        
        DateInfo newDate;
        newDate.year = newTime->tm_year + 1900;
        newDate.month = newTime->tm_mon + 1;
        newDate.day = newTime->tm_mday;
        return newDate;
    }
    
    int compareDates(DateInfo date1, DateInfo date2) {
        if (date1.year != date2.year) return date1.year - date2.year;
        if (date1.month != date2.month) return date1.month - date2.month;
        return date1.day - date2.day;
    }
    
    String getNotifications(String filterType) {
        if (!dataManager) {
            return "{\"error\":\"DataManager not initialized\"}";
        }
        
        DateInfo today = getCurrentDate();
        
        if (today.year == 0) {
            return "{\"error\":\"No valid date available\",\"count\":0}";
        }
        
        String todosData = dataManager->getTodosData();
        DynamicJsonDocument doc(24576);
        DeserializationError error = deserializeJson(doc, todosData);
        
        if (error) {
            return "{\"error\":\"Failed to parse data\"}";
        }
        
        DateInfo tomorrow = addDays(today, 1);
        DateInfo weekEnd = addDays(today, 7);
        
        JsonArray tasks = doc["tasks"];
        JsonArray projects = doc["projects"];
        
        DynamicJsonDocument result(24576);
        JsonObject categories = result.createNestedObject("categories");
        int totalCount = 0;
        
        for (JsonObject task : tasks) {
            String taskDateStr = task["date"].as<String>();
            
            // Skip tasks without date
            if (taskDateStr.length() == 0 || taskDateStr == "null" || taskDateStr == "") {
                continue;
            }
            
            DateInfo taskDate = stringToDate(taskDateStr);
            if (taskDate.year == 0) {
                continue;
            }
            
            bool include = false;
            
            if (filterType == "today") {
                include = (compareDates(taskDate, today) == 0);
            } else if (filterType == "tomorrow") {
                include = (compareDates(taskDate, tomorrow) == 0);
            } else if (filterType == "week") {
                include = (compareDates(taskDate, today) >= 0 && compareDates(taskDate, weekEnd) <= 0);
            } else if (filterType == "overdue") {
                include = (compareDates(taskDate, today) < 0 && !task["completed"].as<bool>());
            }
            
            if (!include) continue;
            
            totalCount++;
            
            // Find project
            int projectId = task["projectId"];
            String projectName = "Unknown";
            String category = "Uncategorized";
            
            for (JsonObject project : projects) {
                if (project["id"] == projectId) {
                    projectName = project["name"].as<String>();
                    category = project["category"].as<String>();
                    break;
                }
            }
            
            // Build nested structure
            if (!categories.containsKey(category)) {
                categories.createNestedObject(category);
            }
            
            JsonObject categoryObj = categories[category];
            if (!categoryObj.containsKey(projectName)) {
                categoryObj.createNestedArray(projectName);
            }
            
            JsonArray projectTasks = categoryObj[projectName];
            JsonObject taskInfo = projectTasks.createNestedObject();
            taskInfo["id"] = task["id"];
            taskInfo["title"] = task["title"];
            taskInfo["date"] = taskDateStr;
            taskInfo["completed"] = task["completed"];
            taskInfo["priority"] = task["priority"];
        }
        
        result["count"] = totalCount;
        result["filter"] = filterType;
        result["currentDate"] = dateToString(today);
        
        String output;
        serializeJson(result, output);
        return output;
    }
};

#endif
