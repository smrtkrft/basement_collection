#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_task_wdt.h>  // Watchdog timer for safety

// Pin Definitions
#define OLED_SDA 22        // D4
#define OLED_SCL 23        // D5
#define BUTTON_PIN 2       // D2

// Display Settings - 0.91" OLED is usually 128x32
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

// Button Settings
#define DEBOUNCE_DELAY 50  // ms

class DisplayManager {
private:
  Adafruit_SSD1306 display;
  uint8_t currentPage;
  unsigned long lastButtonPress;
  bool lastButtonState;
  uint8_t oledAddress;
  bool displayFound;
  
  // Auto-recovery system
  unsigned long lastRecoveryAttempt;
  uint8_t recoveryAttemptCount;
  static constexpr unsigned long RECOVERY_INTERVAL = 15000; // Try recovery every 15 seconds
  static constexpr uint8_t MAX_RECOVERY_ATTEMPTS = 3;       // Max 3 attempts before giving up for longer
  unsigned long permanentFailureTime;
  static constexpr unsigned long PERMANENT_FAILURE_RETRY = 300000; // Retry after 5 minutes if permanently failed
  
  // Non-blocking initialization state machine
  enum InitState {
    INIT_NOT_STARTED,
    INIT_DETECT_0x3C,
    INIT_DETECT_0x3D,
    INIT_BEGIN_DISPLAY,
    INIT_COMPLETE,
    INIT_FAILED
  };
  InitState initState;
  unsigned long initStateStartTime;
  uint8_t initAttemptCount;
  uint8_t detectedAddress;
  
  // Task counts
  int todayCount;
  int tomorrowCount;
  int weekCount;
  
  // App title
  String appTitle;
  
  // Network info
  String networkSSID;
  String networkIP;
  String networkLocal;
  
  // System ready flag
  bool systemReady;
  
  // Language (EN, DE, TR)
  String currentLanguage;
  
public:
  DisplayManager() 
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      currentPage(0),
      lastButtonPress(0),
      lastButtonState(HIGH),
      oledAddress(0x3C),
      displayFound(false),
      lastRecoveryAttempt(0),
      recoveryAttemptCount(0),
      permanentFailureTime(0),
      initState(INIT_NOT_STARTED),
      initStateStartTime(0),
      initAttemptCount(0),
      detectedAddress(0),
      todayCount(0),
      tomorrowCount(0),
      weekCount(0),
      appTitle("To2Do-SmartKraft"),
      networkSSID(""),
      networkIP(""),
      networkLocal(""),
      systemReady(false),
      currentLanguage("EN") {
  }
  
  bool begin() {
    // Initialize I2C bus only - non-blocking
    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(100000);
    Wire.setTimeOut(200); // 200ms timeout - enough for OLED to stabilize at startup
    
    Serial.println("[Display] I2C initialized - starting OLED detection");
    
    // Give OLED extra time to stabilize after power-on
    delay(100); // 100ms delay only at startup, prevents initial detection failure
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Immediately try to detect OLED (synchronous detection at startup)
    bool detected = false;
    
    // Try 0x3C
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0) {
      Serial.println("[Display] ✓ OLED found at 0x3C");
      detectedAddress = 0x3C;
      detected = true;
    } else {
      // Try 0x3D
      Wire.beginTransmission(0x3D);
      if (Wire.endTransmission() == 0) {
        Serial.println("[Display] ✓ OLED found at 0x3D");
        detectedAddress = 0x3D;
        detected = true;
      }
    }
    
    if (detected) {
      // Try to initialize display
      Serial.printf("[Display] Initializing at 0x%02X...\n", detectedAddress);
      if (display.begin(SSD1306_SWITCHCAPVCC, detectedAddress, false)) {
        Serial.println("[Display] ✓ Init successful!");
        oledAddress = detectedAddress;
        displayFound = true;
        recoveryAttemptCount = 0;
        initState = INIT_COMPLETE;
        
        // Quick test and splash
        display.clearDisplay();
        display.display();
        display.ssd1306_command(SSD1306_SETCONTRAST);
        display.ssd1306_command(255);
        showSplashScreen();
        return true;
      } else {
        Serial.println("[Display] ✗ Init failed");
      }
    }
    
    // If we get here, detection failed - setup non-blocking retry
    Serial.println("[Display] ✗ No OLED found - will retry in background");
    initState = INIT_FAILED;
    initStateStartTime = millis();
    lastRecoveryAttempt = millis();
    displayFound = false;
    
    return false;
  }
  
  // Non-blocking OLED initialization - call this in loop() for recovery attempts
  void processInit() {
    if (initState == INIT_COMPLETE || initState == INIT_FAILED) {
      return; // Already done
    }
    
    unsigned long now = millis();
    
    switch (initState) {
      case INIT_DETECT_0x3C: {
        if (now - initStateStartTime > 150) { // 150ms timeout per state (increased from 50ms)
          Serial.println("[Display] ✗ 0x3C detect timeout");
          initState = INIT_DETECT_0x3D;
          initStateStartTime = now;
          break;
        }
        
        Wire.beginTransmission(0x3C);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
          Serial.println("[Display] ✓ OLED found at 0x3C");
          detectedAddress = 0x3C;
          initState = INIT_BEGIN_DISPLAY;
          initStateStartTime = now;
        } else {
          initState = INIT_DETECT_0x3D;
          initStateStartTime = now;
        }
        break;
      }
      
      case INIT_DETECT_0x3D: {
        if (now - initStateStartTime > 150) { // 150ms timeout (increased from 50ms)
          Serial.println("[Display] ✗ 0x3D detect timeout");
          initState = INIT_FAILED;
          Serial.println("[Display] ✗ No OLED found - system continues");
          lastRecoveryAttempt = now;
          break;
        }
        
        Wire.beginTransmission(0x3D);
        uint8_t error = Wire.endTransmission();
        
        if (error == 0) {
          Serial.println("[Display] ✓ OLED found at 0x3D");
          detectedAddress = 0x3D;
          initState = INIT_BEGIN_DISPLAY;
          initStateStartTime = now;
        } else {
          initState = INIT_FAILED;
          Serial.println("[Display] ✗ No OLED found - system continues");
          lastRecoveryAttempt = now;
        }
        break;
      }
      
      case INIT_BEGIN_DISPLAY: {
        if (now - initStateStartTime > 300) { // 300ms total for display.begin()
          Serial.println("[Display] ✗ display.begin() timeout");
          initState = INIT_FAILED;
          displayFound = false;
          lastRecoveryAttempt = now;
          break;
        }
        
        // Try once per call - non-blocking approach
        if (initAttemptCount == 0) {
          Serial.printf("[Display] Initializing at 0x%02X...\n", detectedAddress);
        }
        
        if (display.begin(SSD1306_SWITCHCAPVCC, detectedAddress, false)) {
          Serial.println("[Display] ✓ Init successful!");
          oledAddress = detectedAddress;
          displayFound = true;
          recoveryAttemptCount = 0;
          initState = INIT_COMPLETE;
          
          // Quick test and splash
          display.clearDisplay();
          display.display();
          display.ssd1306_command(SSD1306_SETCONTRAST);
          display.ssd1306_command(255);
          showSplashScreen();
        } else {
          initAttemptCount++;
          if (initAttemptCount > 5) {
            Serial.println("[Display] ✗ Init failed after 5 attempts");
            initState = INIT_FAILED;
            displayFound = false;
            lastRecoveryAttempt = now;
          }
        }
        break;
      }
      
      default:
        break;
    }
  }
  
  void showSplashScreen() {
    if (!displayFound) return;
    
    systemReady = false;
    unsigned long opStart;
    
    // Protected clearDisplay with timeout and error recovery
    opStart = millis();
    display.clearDisplay();
    if (millis() - opStart > 50) {
      Serial.println("[Display] ✗ Splash clearDisplay timeout - triggering reset");
      handleDisplayError();
      return;
    }
    
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(2);
    int16_t x1 = (SCREEN_WIDTH - (10 * 12)) / 2;
    display.setCursor(x1, 4);
    display.println("SmartKraft");
    
    display.setTextSize(1);
    int16_t x2 = (SCREEN_WIDTH - (5 * 6)) / 2;
    display.setCursor(x2, 22);
    display.println("To2Do");
    
    // Protected display with timeout and error recovery
    opStart = millis();
    display.display();
    if (millis() - opStart > 50) {
      Serial.println("[Display] ✗ Splash display() timeout - triggering reset");
      handleDisplayError();
      return;
    }
  }
  
  // Handle any display error by marking it failed and scheduling recovery
  void handleDisplayError() {
    Serial.println("[Display] ⚠ Error detected - marking OLED as failed");
    displayFound = false;
    lastRecoveryAttempt = millis();
    recoveryAttemptCount = 0;
    permanentFailureTime = 0;
  }
  
  void setAppTitle(const char* title) {
    appTitle = String(title); // Just save, don't display yet
  }
  
  void setSystemReady() {
    systemReady = true;
    currentPage = 0;
    Serial.println("[Display] System marked as ready - button input enabled");
    updateDisplay();
  }
  
  void setTaskCounts(int today, int tomorrow, int week) {
    todayCount = today;
    tomorrowCount = tomorrow;
    weekCount = week;
  }
  
  void setNetworkInfo(String ssid, String ip, String local) {
    networkSSID = ssid;
    networkIP = ip;
    networkLocal = local;
  }
  
  void setLanguage(String lang) {
    currentLanguage = lang;
    currentLanguage.toUpperCase();
  }
  
  // Get translated text for OLED (no Turkish special characters)
  const char* getTranslated(const char* key) {
    // EN translations
    if (currentLanguage == "EN") {
      if (strcmp(key, "today") == 0) return "Today";
      if (strcmp(key, "tomorrow") == 0) return "Tomorrow";
      if (strcmp(key, "week") == 0) return "This Week";
    }
    // DE translations
    else if (currentLanguage == "DE") {
      if (strcmp(key, "today") == 0) return "Heute";
      if (strcmp(key, "tomorrow") == 0) return "Morgen";
      if (strcmp(key, "week") == 0) return "Diese Woche";
    }
    // TR translations (no special chars: ü->u, ö->o, ğ->g, ş->s, ç->c, ı->i)
    else if (currentLanguage == "TR") {
      if (strcmp(key, "today") == 0) return "Bugun";
      if (strcmp(key, "tomorrow") == 0) return "Yarin";
      if (strcmp(key, "week") == 0) return "Bu Hafta";
    }
    // Fallback
    return key;
  }
  
  void loop() {
    // Only process state machine if we're in recovery mode
    if (!displayFound && initState != INIT_FAILED) {
      processInit();
    }
    
    // Check for permanent failure retry
    if (!displayFound && recoveryAttemptCount >= MAX_RECOVERY_ATTEMPTS) {
      unsigned long now = millis();
      if (permanentFailureTime == 0) {
        permanentFailureTime = now;
        Serial.println("[Display] ✗ Max recovery attempts reached - waiting 5 minutes");
      } else if (now - permanentFailureTime >= PERMANENT_FAILURE_RETRY) {
        Serial.println("[Display] Retrying after permanent failure timeout");
        recoveryAttemptCount = 0;
        permanentFailureTime = 0;
        lastRecoveryAttempt = now;
      }
      return; // Skip recovery until timeout
    }
    
    // Auto-recovery for temporary failures
    if (!displayFound && recoveryAttemptCount < MAX_RECOVERY_ATTEMPTS) {
      unsigned long now = millis();
      if (now - lastRecoveryAttempt >= RECOVERY_INTERVAL) {
        lastRecoveryAttempt = now;
        recoveryAttemptCount++;
        
        Serial.printf("[Display] Recovery attempt %d/%d\n", recoveryAttemptCount, MAX_RECOVERY_ATTEMPTS);
        attemptRecovery(); // Start recovery state machine
      }
      return;
    }
    
    // Check if recovery completed successfully
    if (initState == INIT_COMPLETE && displayFound && recoveryAttemptCount > 0) {
      Serial.println("[Display] ✓ OLED recovered successfully!");
      recoveryAttemptCount = 0;
      permanentFailureTime = 0;
      
      // System was ready before, restore it
      // Main loop will call setSystemReady() when it detects display is back
      showSplashScreen();
      
      // Small delay to show splash, then mark ready
      delay(1000);
      systemReady = true;
      currentPage = 0;
      Serial.println("[Display] System ready after recovery - button input enabled");
    }
    
    if (!displayFound) return;
    checkButton();
  }
  
  bool attemptRecovery() {
    Serial.println("[Display] Starting OLED recovery...");
    
    // Reset I2C bus
    Wire.end();
    delay(50); // Small delay to let I2C bus settle before restart
    Wire.begin(OLED_SDA, OLED_SCL);
    Wire.setClock(100000);
    Wire.setTimeOut(200); // Same timeout as initial setup
    
    // Start state machine for non-blocking detection
    initState = INIT_DETECT_0x3C;
    initStateStartTime = millis();
    initAttemptCount = 0;
    detectedAddress = 0;
    
    Serial.println("[Display] Recovery state machine started");
    
    return false; // Will complete via processInit() in loop()
  }
  
  void checkButton() {
    if (!systemReady) {
      // Debug: Button check skipped because system not ready
      return;
    }
    
    bool buttonState = digitalRead(BUTTON_PIN);
    unsigned long currentTime = millis();
    
    if (buttonState == LOW && lastButtonState == HIGH) {
      if (currentTime - lastButtonPress > 500) { // 500ms debounce
        lastButtonPress = currentTime;
        Serial.printf("[Display] Button pressed - Page %d -> %d\n", currentPage, (currentPage + 1) % 5);
        nextPage();
      }
    }
    
    lastButtonState = buttonState;
  }
  
  void nextPage() {
    currentPage++;
    if (currentPage > 4) {  // 5 pages: 0=title, 1=today, 2=tomorrow, 3=week, 4=network
      currentPage = 0;
    }
    updateDisplay();
  }
  
  void updateDisplay() {
    if (!displayFound) return;
    
    unsigned long opStart;
    
    // Protected clearDisplay with strict timeout and error recovery
    opStart = millis();
    display.clearDisplay();
    if (millis() - opStart > 50) {
      Serial.println("[Display] ✗ updateDisplay clearDisplay timeout - triggering reset");
      handleDisplayError();
      return;
    }
    
    switch (currentPage) {
      case 0:
        drawAppTitle();
        break;
      case 1:
        drawTodayPage();
        break;
      case 2:
        drawTomorrowPage();
        break;
      case 3:
        drawWeekPage();
        break;
      case 4:
        drawNetworkPage();
        break;
    }
    
    // Protected display() with strict timeout and error recovery
    opStart = millis();
    display.display();
    if (millis() - opStart > 50) {
      Serial.println("[Display] ✗ updateDisplay display() timeout - triggering reset");
      handleDisplayError();
      return;
    }
  }
  
  void drawAppTitle() {
    // App title page - use saved title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Center text
    int16_t x = (SCREEN_WIDTH - (appTitle.length() * 6)) / 2;
    display.setCursor(x, 12);
    display.println(appTitle);
  }
  
  void drawTodayPage() {
    // Today's tasks - label on left, number on right (lower position)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Translated "Today" text
    display.setCursor(10, 14);
    display.print(getTranslated("today"));
    
    // Count - sağda (x=85), çok büyük rakam (size 3)
    display.setTextSize(3);
    display.setCursor(85, 6);
    display.print(todayCount);
  }
  
  void drawTomorrowPage() {
    // Tomorrow's tasks - label on left, number on right (lower position)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Translated "Tomorrow" text
    display.setCursor(10, 14);
    display.print(getTranslated("tomorrow"));
    
    // Count - sağda (x=85), çok büyük rakam (size 3)
    display.setTextSize(3);
    display.setCursor(85, 6);
    display.print(tomorrowCount);
  }
  
  void drawWeekPage() {
    // This week's tasks - label on left, number on right (lower position)
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Translated "This Week" text
    display.setCursor(10, 14);
    display.print(getTranslated("week"));
    
    // Count - sağda (x=85), çok büyük rakam (size 3)
    display.setTextSize(3);
    display.setCursor(85, 6);
    display.print(weekCount);
  }
  
  void drawNetworkPage() {
    // Network info - 3 lines centered horizontally
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Line 1: SSID (Y=2)
    int16_t x1 = (SCREEN_WIDTH - (networkSSID.length() * 6)) / 2;
    if (x1 < 0) x1 = 0; // Prevent negative
    display.setCursor(x1, 2);
    display.println(networkSSID);
    
    // Line 2: IP Address (Y=12)
    int16_t x2 = (SCREEN_WIDTH - (networkIP.length() * 6)) / 2;
    if (x2 < 0) x2 = 0;
    display.setCursor(x2, 12);
    display.println(networkIP);
    
    // Line 3: .local address (Y=22)
    int16_t x3 = (SCREEN_WIDTH - (networkLocal.length() * 6)) / 2;
    if (x3 < 0) x3 = 0;
    display.setCursor(x3, 22);
    display.println(networkLocal);
  }
  
  bool isDisplayFound() {
    return displayFound;
  }
  
  uint8_t getCurrentPage() {
    return currentPage;
  }
  
  void refreshCurrentPage() {
    // Refresh current page without changing page number
    updateDisplay();
  }
};

#endif
