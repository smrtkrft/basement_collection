/*
 * SmartKraft - Hobby Project
 * github.com/smrtkrft/basement_collection/dw_getSound
 * SEU - Emek Ulas
 * ESP32-C6 HTTP Audio Player with DFPlayer Mini
 * 
 * Features:
 * - Toggle system: Button press switches ON/OFF mode
 * - Different URL list triggered for each mode
 * - Audio playback via DFPlayer Mini
 * - WiFi and URL configuration via Serial port
 * - LED status indicators
 * 
 * Connections:
 * - Button: D9 (GPIO 20, pull-up)
 * - LED D2: GPIO 2 (WiFi status)
 * - LED D3: GPIO 21 (HTTP activity)
 * - LED D6: GPIO 16 (Audio - optional)
 * - DFPlayer RX: D4 (GPIO 22 - ESP32 TX)
 * - DFPlayer TX: D5 (GPIO 23 - ESP32 RX)
 * - Speaker: DFPlayer SPK+ / SPK-
 * 
 * Toggle Logic:
 * - x=0 (OFF) → Button press → x=1 (ON): 001.mp3 + URL List 1
 * - x=1 (ON) → Button press → x=0 (OFF): 005.mp3 + URL List 2
 * - x=0 on every boot (not stored in memory)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <HardwareSerial.h>

// Pin definitions (Xiao ESP32-C6)
#define BUTTON_PIN 20
#define LED_D2 2      // WiFi status
#define LED_D3 21     // HTTP activity
#define LED_D6 16     // Audio (optional)
#define DFPLAYER_RX 23
#define DFPLAYER_TX 22

HardwareSerial dfSerial(1);
Preferences preferences;

String ssid = "";
String password = "";

String urlList1[10]; // ON state (x: 0→1)
int urlCount1 = 0;
String urlList2[10]; // OFF state (x: 1→0)
int urlCount2 = 0;

int systemState = 0; // 0: OFF, 1: ON

unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 300;

bool wifiConnected = false;
bool wifiConnecting = false;
unsigned long wifiConnectStart = 0;
const unsigned long wifiTimeout = 10000;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("SmartKraft - Hobby Project");
  Serial.println("github.com/smrtkrft/basement_collection/dw_getSound");
  Serial.println("Nov25 - Zurich");
  Serial.println("=================================\n");
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_D2, OUTPUT);
  pinMode(LED_D3, OUTPUT);
  pinMode(LED_D6, OUTPUT);
  
  testLEDs();
  
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  delay(500);
  
  Serial.println("\n⏳ Initializing DFPlayer (3-5 seconds)...");
  
  sendDFCommand(0x0C, 0x00, 0x00);
  delay(500);
  
  unsigned long startTime = millis();
  bool cardDetected = false;
  Serial.println("📀 Waiting for SD card...");
  
  while(millis() - startTime < 3000) {
    if(dfSerial.available()) {
      byte response[10];
      int idx = 0;
      while(dfSerial.available() && idx < 10) {
        response[idx++] = dfSerial.read();
        delay(2);
      }
      
      if(idx >= 6 && response[3] == 0x3F) {
        cardDetected = true;
        Serial.println("✅ SD card detected!");
        break;
      }
    }
    delay(100);
  }
  
  if(!cardDetected) {
    Serial.println("⚠️  No SD card response (continuing...)");
  }
  
  delay(200);
  sendDFCommand(0x06, 0x00, 0x1E);
  delay(200);
  
  Serial.println("📊 Querying SD card file count...");
  sendDFCommand(0x48, 0x00, 0x00);
  delay(500);
  
  if(dfSerial.available()) {
    Serial.print("📥 DFPlayer response: ");
    while(dfSerial.available()) {
      byte b = dfSerial.read();
      if(b < 0x10) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
      delay(2);
    }
    Serial.println();
  }
  
  Serial.println("✅ DFPlayer ready (Raw UART)");
  Serial.println("Use 'test uart' command for detailed testing!\n");
  
  preferences.begin("wifi-config", false);
  loadSettings();
  
  if (ssid.length() > 0) {
    connectWiFi();
  } else {
    Serial.println("\nWiFi settings not found!");
    printHelp();
  }
  
  Serial.println("\nSystem ready!");
  Serial.println("Type 'help' for command list\n");
  
  digitalWrite(LED_D2, LOW);
  
  Serial.println("⚡ POWER WARNING:");
  Serial.println("DFPlayer Mini can draw 200-500mA when reading SD card!");
  Serial.println("Use external 5V adapter if USB port is insufficient.\n");
}

void loop() {
  checkWiFiStatus();
  
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    processCommand(command);
  }
  
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();
      handleButtonPress();
    }
  }
  
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck > 10000) {
    lastWiFiCheck = millis();
    if (!wifiConnecting && WiFi.status() != WL_CONNECTED && ssid.length() > 0 && wifiConnected) {
      Serial.println("⚠️  WiFi connection lost, reconnecting...");
      connectWiFi();
    }
  }
}

void handleButtonPress() {
  Serial.println("\n>>> Button pressed!");
  
  if (systemState == 0) {
    Serial.println("🟢 System turning ON...");
    
    // Step 1: Quick flicker
    flickerLED(LED_D2, 3, 60);
    
    // Step 2: Setup PWM for LED fade
    ledcAttach(LED_D2, 5000, 8); // 5kHz, 8-bit resolution
    
    // Step 3: Play 001.mp3 with LED_D3 blinking AND LED_D2 fade-in simultaneously
    playAudioWithLED(1, true);  // Play 001.mp3 with fade
    
    // Step 4: Now 001.mp3 is finished, continue with URLs
    if (urlCount1 > 0) {
      makeHTTPRequest(1);  // This will handle success/fail and play 002.mp3
    } else {
      Serial.println("⚠️ URL List 1 is empty, no action taken");
      delay(2000);
      playAudio(2);
    }
    
    // After everything is done, change state to 1
    systemState = 1;
    Serial.printf("📊 New state: x = %d (ON)\n", systemState);
    
  } else {
    Serial.println("🔴 System turning OFF...");
    
    systemState = 0;
    
    playAudioNonBlocking(5);
    
    if (urlCount2 > 0) {
      makeHTTPRequest(2);
    } else {
      Serial.println("⚠️ URL List 2 is empty, no action taken");
    }
    
    Serial.println("💡 LED fading out...");
    
    flickerLED(LED_D2, 3, 40);
    fadeLED(LED_D2, 255, 0, 2000);
    
    Serial.printf("📊 New state: x = %d (OFF)\n", systemState);
  }
}

// Send HTTP request (with list parameter)
void makeHTTPRequest(int listNumber) {
  String* urlList = (listNumber == 1) ? urlList1 : urlList2;
  int urlCount = (listNumber == 1) ? urlCount1 : urlCount2;
  
  Serial.printf("\n🌐 URL List %d triggering (%d URL)...\n", listNumber, urlCount);
  
  // Internet check
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ No internet connection!");
    blinkLED(LED_D2, 3);
    
    // 003.mp3 + 004.mp3 sequentially (1 second interval)
    playAudio(3);
    delay(500);
    playAudio(4);
    delay(500);
    
    // Wait 5 seconds and reboot
    Serial.println("\n⚠️ Internet problem detected!");
    Serial.println("🔄 System will restart in 5 seconds...");
    for(int i = 5; i > 0; i--) {
      Serial.printf("   %d...\n", i);
      delay(1000);
    }
    Serial.println("🔄 Restarting...\n");
    ESP.restart();
    return;
  }
  
  // Trigger all URLs with retry logic
  bool allSuccess = true;
  for (int i = 0; i < urlCount; i++) {
    String url = urlList[i];
    bool urlSuccess = false;
    
    // Try each URL twice
    for (int attempt = 1; attempt <= 2; attempt++) {
      Serial.printf("  [%d/%d] %s (attempt %d/2) ... ", i + 1, urlCount, url.c_str(), attempt);
      
      digitalWrite(LED_D3, HIGH);
      
      HTTPClient http;
      http.begin(url);
      http.setTimeout(5000); // 5 second timeout
      int httpCode = http.GET();
      
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
          Serial.println("✅ Successful");
          urlSuccess = true;
          http.end();
          digitalWrite(LED_D3, LOW);
          break; // Success, no need to retry
        } else {
          Serial.printf("⚠️ HTTP %d\n", httpCode);
        }
      } else {
        Serial.printf("❌ Error: %s\n", http.errorToString(httpCode).c_str());
      }
      
      http.end();
      digitalWrite(LED_D3, LOW);
      
      // If first attempt failed and we'll retry, wait a bit
      if (attempt == 1) {
        Serial.println("      ↻ Retrying...");
        delay(1000);
      }
    }
    
    if (!urlSuccess) {
      allSuccess = false;
      Serial.println("      ✗ Failed after 2 attempts");
    }
    
    delay(500); // Delay between URLs
  }
  
  // Result sound (only in open state)
  if (listNumber == 1) {
    if (allSuccess) {
      Serial.println("✅ All URLs successful!");
      delay(1000); // 1 second pause before 002.mp3
      playAudio(2); // 002.mp3
      delay(3000);
    } else {
      Serial.println("❌ Error occurred in some URLs!");
      playAudio(4); // 004.mp3
      delay(3000);
    }
  }
}

void playAudio(int trackNumber) {
  Serial.printf("🎵 Playing audio file: %03d.mp3\n", trackNumber);
  // digitalWrite(LED_D6, HIGH); // Temporarily disabled
  
  // Send command with minimal debug (always root directory - 0x03)
  byte packet[10];
  packet[0] = 0x7E;
  packet[1] = 0xFF;
  packet[2] = 0x06;
  packet[3] = 0x03; // Root dizin komutu
  packet[4] = 0x00;
  packet[5] = 0x00;
  packet[6] = (byte)trackNumber;
  packet[9] = 0xEF;
  
  int16_t checksum = -(packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6]);
  packet[7] = (byte)(checksum >> 8);
  packet[8] = (byte)(checksum & 0xFF);
  
  Serial.println("   📂 Location: SD:/ (root)");
  
  // Send directly
  dfSerial.write(packet, 10);
  
  // LED_D3 blinking duration (based on track number)
  int ledDuration = 5500; // Default 5.5 seconds
  if(trackNumber == 1) {
    ledDuration = 4500; // 4.5 seconds for 001.mp3 (1 second shorter)
  } else if(trackNumber == 2) {
    ledDuration = 5000; // 5.0 seconds for 002.mp3 (0.5 seconds shorter)
  }
  
  // LED_D3 (GPIO21) blinks randomly during audio playback
  unsigned long audioStartTime = millis();
  while(millis() - audioStartTime < ledDuration) {
    // Random speed blinking (50-200ms interval)
    int randomDelay = random(50, 200);
    digitalWrite(LED_D3, HIGH);
    delay(randomDelay);
    digitalWrite(LED_D3, LOW);
    delay(randomDelay);
  }
  
  // digitalWrite(LED_D6, LOW); // Temporarily disabled
  digitalWrite(LED_D3, LOW); // Turn off LED_D3 after audio ends
}

// Non-blocking audio playback (simultaneous with URLs)
void playAudioNonBlocking(int trackNumber) {
  Serial.printf("🎵 Starting audio file: %03d.mp3 (in background)\n", trackNumber);
  // digitalWrite(LED_D6, HIGH); // Temporarily disabled
  
  // Send command with minimal debug (always root directory - 0x03)
  byte packet[10];
  packet[0] = 0x7E;
  packet[1] = 0xFF;
  packet[2] = 0x06;
  packet[3] = 0x03; // Root directory command
  packet[4] = 0x00;
  packet[5] = 0x00;
  packet[6] = (byte)trackNumber;
  packet[9] = 0xEF;
  
  int16_t checksum = -(packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6]);
  packet[7] = (byte)(checksum >> 8);
  packet[8] = (byte)(checksum & 0xFF);
  
  Serial.println("   📂 Location: SD:/ (root) - URLs will be triggered simultaneously");
  
  // Send directly
  dfSerial.write(packet, 10);
  
  // LED_D3 blinks randomly while audio plays in background (5.5 seconds)
  unsigned long audioStartTime = millis();
  while(millis() - audioStartTime < 5500) {
    // Random speed blinking (50-200ms interval)
    int randomDelay = random(50, 200);
    digitalWrite(LED_D3, HIGH);
    delay(randomDelay);
    digitalWrite(LED_D3, LOW);
    delay(randomDelay);
  }
  
  // digitalWrite(LED_D6, LOW); // Temporarily disabled
  digitalWrite(LED_D3, LOW); // Turn off LED_D3 after audio ends
}

// Start audio command only (no waiting, truly non-blocking)
void startAudioCommand(int trackNumber) {
  byte packet[10];
  packet[0] = 0x7E;
  packet[1] = 0xFF;
  packet[2] = 0x06;
  packet[3] = 0x03; // Root directory command
  packet[4] = 0x00;
  packet[5] = 0x00;
  packet[6] = (byte)trackNumber;
  packet[9] = 0xEF;
  
  int16_t checksum = -(packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6]);
  packet[7] = (byte)(checksum >> 8);
  packet[8] = (byte)(checksum & 0xFF);
  
  dfSerial.write(packet, 10);
}

// Play audio with LED_D3 blinking and optional fade-in for LED_D2
void playAudioWithLED(int trackNumber, bool fadeLEDWifi) {
  Serial.printf("🎵 Playing audio file: %03d.mp3\n", trackNumber);
  
  byte packet[10];
  packet[0] = 0x7E;
  packet[1] = 0xFF;
  packet[2] = 0x06;
  packet[3] = 0x03; // Root directory command
  packet[4] = 0x00;
  packet[5] = 0x00;
  packet[6] = (byte)trackNumber;
  packet[9] = 0xEF;
  
  int16_t checksum = -(packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6]);
  packet[7] = (byte)(checksum >> 8);
  packet[8] = (byte)(checksum & 0xFF);
  
  dfSerial.write(packet, 10);
  
  // Audio duration based on track number
  int audioDuration = 4500; // Default 4.5 seconds for 001.mp3
  if(trackNumber == 2) {
    audioDuration = 5000; // 5.0 seconds for 002.mp3
  } else if(trackNumber >= 3) {
    audioDuration = 5500; // 5.5 seconds for others
  }
  
  // If fade is requested, start it in parallel
  unsigned long fadeStartTime = millis();
  int fadeSteps = 50;
  int fadeStepDelay = 2000 / fadeSteps; // 2 seconds total fade
  int currentFadeStep = 0;
  
  unsigned long audioStartTime = millis();
  
  // Play audio with LED_D3 blinking
  while(millis() - audioStartTime < audioDuration) {
    // LED_D3 random blinking
    int randomDelay = random(50, 200);
    digitalWrite(LED_D3, HIGH);
    delay(randomDelay);
    digitalWrite(LED_D3, LOW);
    delay(randomDelay);
    
    // If fade is enabled, update LED_D2 brightness during blinks
    if(fadeLEDWifi && currentFadeStep < fadeSteps) {
      unsigned long elapsed = millis() - fadeStartTime;
      if(elapsed >= currentFadeStep * fadeStepDelay) {
        int brightness = map(currentFadeStep, 0, fadeSteps, 0, 255);
        ledcWrite(LED_D2, brightness);
        currentFadeStep++;
      }
    }
  }
  
  digitalWrite(LED_D3, LOW);
  
  // Finalize fade if it was enabled
  if(fadeLEDWifi) {
    ledcDetach(LED_D2);
    pinMode(LED_D2, OUTPUT);
    digitalWrite(LED_D2, HIGH);
  }
}

void processCommand(String command) {
  // First save the original command
  String originalCommand = command;
  command.toLowerCase();
  
  Serial.println("\n>>> Command: " + command);
  // Debug: Also show original command (for case sensitivity check)
  if(originalCommand != command) {
    Serial.println("    Original: " + originalCommand);
  }
  
  if (command == "help") {
    printHelp();
  }
  else if (command.startsWith("wifi ")) {
    // Format: wifi SSID PASSWORD
    // Use ORIGINAL command (preserve case sensitivity)
    int spaceIndex = originalCommand.indexOf(' ', 5);
    if (spaceIndex > 0) {
      ssid = originalCommand.substring(5, spaceIndex);
      password = originalCommand.substring(spaceIndex + 1);
      ssid.trim();
      password.trim();
      
      preferences.putString("ssid", ssid);
      preferences.putString("password", password);
      
      Serial.println("✅ WiFi settings saved!");
      Serial.println("   SSID: " + ssid);
      Serial.print("   Password: ");
      for(int i = 0; i < password.length(); i++) Serial.print("*");
      Serial.println();
      
      connectWiFi();
    } else {
      Serial.println("ERROR: Format: wifi SSID PASSWORD");
    }
  }
  else if (command.startsWith("add url1 ")) {
    // Format: add url1 https://example.com/api
    if (urlCount1 >= 10) {
      Serial.println("ERROR: List 1 is full (maximum 10 URLs)!");
      return;
    }
    
    String url = originalCommand.substring(9);
    url.trim();
    
    if (url.startsWith("http://") || url.startsWith("https://")) {
      urlList1[urlCount1] = url;
      urlCount1++;
      saveUrls();
      Serial.printf("✅ Added to URL List 1 [%d/10]: %s\n", urlCount1, url.c_str());
    } else {
      Serial.println("ERROR: URL must start with http:// or https://!");
    }
  }
  else if (command.startsWith("add url2 ")) {
    // Format: add url2 https://example.com/api
    if (urlCount2 >= 10) {
      Serial.println("ERROR: List 2 is full (maximum 10 URLs)!");
      return;
    }
    
    String url = originalCommand.substring(9);
    url.trim();
    
    if (url.startsWith("http://") || url.startsWith("https://")) {
      urlList2[urlCount2] = url;
      urlCount2++;
      saveUrls();
      Serial.printf("✅ Added to URL List 2 [%d/10]: %s\n", urlCount2, url.c_str());
    } else {
      Serial.println("ERROR: URL must start with http:// or https://!");
    }
  }
  else if (command.startsWith("del url1 ")) {
    // Format: del url1 1
    int index = command.substring(9).toInt() - 1;
    
    if (index >= 0 && index < urlCount1) {
      Serial.printf("🗑️ Deleting: %s\n", urlList1[index].c_str());
      
      // Shift the list
      for (int i = index; i < urlCount1 - 1; i++) {
        urlList1[i] = urlList1[i + 1];
      }
      urlCount1--;
      saveUrls();
      Serial.println("✅ Deleted from URL List 1!");
    } else {
      Serial.printf("ERROR: Invalid index (1-%d)!\n", urlCount1);
    }
  }
  else if (command.startsWith("del url2 ")) {
    // Format: del url2 1
    int index = command.substring(9).toInt() - 1;
    
    if (index >= 0 && index < urlCount2) {
      Serial.printf("🗑️ Deleting: %s\n", urlList2[index].c_str());
      
      // Shift the list
      for (int i = index; i < urlCount2 - 1; i++) {
        urlList2[i] = urlList2[i + 1];
      }
      urlCount2--;
      saveUrls();
      Serial.println("✅ Deleted from URL List 2!");
    } else {
      Serial.printf("ERROR: Invalid index (1-%d)!\n", urlCount2);
    }
  }
  else if (command == "list url") {
    Serial.println("\n═══════════════════════════════════");
    Serial.println("📋 URL LISTS");
    Serial.println("═══════════════════════════════════\n");
    
    Serial.printf("📂 List 1 (OPEN - x: 0→1) [%d/10]:\n", urlCount1);
    if (urlCount1 == 0) {
      Serial.println("   (empty)\n");
    } else {
      for (int i = 0; i < urlCount1; i++) {
        Serial.printf("   %d. %s\n", i + 1, urlList1[i].c_str());
      }
      Serial.println();
    }
    
    Serial.printf("📂 List 2 (CLOSE - x: 1→0) [%d/10]:\n", urlCount2);
    if (urlCount2 == 0) {
      Serial.println("   (empty)\n");
    } else {
      for (int i = 0; i < urlCount2; i++) {
        Serial.printf("   %d. %s\n", i + 1, urlList2[i].c_str());
      }
      Serial.println();
    }
  }
  else if (command == "status") {
    printStatus();
  }
  else if (command == "reset") {
    Serial.println("⚠️ Resetting all settings...");
    preferences.clear();
    Serial.println("🔄 Restarting system...");
    delay(1000);
    ESP.restart();
  }
  else if (command.startsWith("volume ")) {
    int vol = command.substring(7).toInt();
    if (vol >= 0 && vol <= 30) {
      sendDFCommand(0x06, 0x00, vol);
      preferences.putInt("volume", vol);
      Serial.printf("🔊 Volume level: %d/30\n", vol);
    } else {
      Serial.println("ERROR: Volume level must be between 0-30!");
    }
  }
  else if (command.startsWith("play ")) {
    int track = command.substring(5).toInt();
    if (track > 0 && track <= 255) {
      Serial.printf("🎵 Manual playback: %03d.mp3\n", track);
      playAudio(track);
      delay(3000);
      Serial.println("✅ Completed!");
    } else {
      Serial.println("ERROR: Invalid track number (1-255)!");
    }
  }
  else if (command == "test quiet") {
    Serial.println("\n🔇 QUIET TEST (No debug messages)");
    Serial.println("═══════════════════════════════════════\n");
    Serial.println("Trying to play audio...");
    
    digitalWrite(LED_D6, HIGH);
    
    // Clear buffer
    while(dfSerial.available()) dfSerial.read();
    
    // Volume 30
    byte vol[10] = {0x7E, 0xFF, 0x06, 0x06, 0x00, 0x00, 0x1E, 0xFE, 0xD7, 0xEF};
    dfSerial.write(vol, 10);
    delay(300);
    
    // Play command
    byte play[10] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x01, 0xFE, 0xF7, 0xEF};
    dfSerial.write(play, 10);
    
    Serial.println("✅ Commands sent!");
    Serial.println("⏳ Waiting 5 seconds...");
    Serial.println("🔊 IS SOUND COMING?\n");
    
    delay(5000);
    
    digitalWrite(LED_D6, LOW);
    
    Serial.println("Test finished.\n");
  }
  else if (command == "test analog") {
    Serial.println("\n🔊 Speaker Melody Test (without SD card):");
    Serial.println("═══════════════════════════════════════");
    Serial.println("By sending fast commands to DFPlayer,");
    Serial.println("we'll create rhythmic clicks/crackles from the speaker.\n");
    
    digitalWrite(LED_D6, HIGH);
    
    Serial.println("🎵 Rhythm starting (10 seconds)...\n");
    
    // Maximum volume
    sendDFCommand(0x06, 0x00, 0x1E);
    delay(100);
    
    // Rhythmic commands for 10 seconds
    for(int i = 0; i < 20; i++) {
      Serial.print("♪ ");
      
      // Reset command (creates click sound)
      sendDFCommand(0x0C, 0x00, 0x00);
      delay(100);
      
      // Volume change (activates amplifier)
      sendDFCommand(0x06, 0x00, 0x1E);
      delay(100);
      
      // Play command (will error but activates DAC)
      sendDFCommand(0x03, 0x00, 0x01);
      delay(200);
      
      // Different command for different frequency
      sendDFCommand(0x03, 0x00, 0x02);
      delay(100);
    }
    
    digitalWrite(LED_D6, LOW);
    
    Serial.println("\n\n═══════════════════════════════════════");
    Serial.println("🎧 Did RHYTHMIC sound come from the speaker?");
    Serial.println("(Click, crackle, buzz, any sound)\n");
    Serial.println("→ Yes: Speaker and DFPlayer are working!");
    Serial.println("→ No: Speaker connection needs checking");
    Serial.println("═══════════════════════════════════════\n");
  }
  else if (command == "test beep") {
    Serial.println("\n🔔 ESP32 PWM Beep Test:");
    Serial.println("═══════════════════════════════════════");
    Serial.println("⚠️  For this test, you need to disconnect the speaker from DFPlayer");
    Serial.println("    and connect it to ESP32's D4 (GPIO22) pin!\n");
    Serial.println("Speaker + ───> GPIO22 (LED_D6)");
    Serial.println("Speaker - ───> GND\n");
    
    Serial.println("When ready, type 'y' and press Enter (cancel: n)");
  }
  else if (command == "y" || command == "beep") {
    Serial.println("\n🎵 Playing melody with ESP32 PWM...\n");
    
    // PWM settings (ESP32-C6 new API)
    int buzzerPin = LED_D6;  // GPIO22
    
    // Notes (frequencies in Hz)
    int melody[] = {262, 294, 330, 349, 392, 440, 494, 523}; // C, D, E, F, G, A, B, C
    int noteDuration = 200;
    
    for(int i = 0; i < 8; i++) {
      Serial.printf("🎵 Note %d: %d Hz\n", i+1, melody[i]);
      
      // ESP32-C6 new PWM API
      ledcAttach(buzzerPin, melody[i], 8);  // pin, frequency, resolution
      ledcWrite(buzzerPin, 128);  // 50% duty cycle
      
      delay(noteDuration);
      
      // Silence
      ledcWrite(buzzerPin, 0);
      delay(50);
    }
    
    ledcDetach(buzzerPin);
    
    Serial.println("\n✅ Melody finished!");
    Serial.println("Did sound come from the speaker?\n");
  }
  else if (command == "test uart") {
    Serial.println("\n🔌 UART Connection Test:");
    Serial.println("═══════════════════════════════════════");
    Serial.printf("ESP32 TX Pin: GPIO%d (D4) -> DFPlayer RX\n", DFPLAYER_TX);
    Serial.printf("ESP32 RX Pin: GPIO%d (D5) <- DFPlayer TX\n", DFPLAYER_RX);
    Serial.println("Baud Rate: 9600, 8N1");
    Serial.println("═══════════════════════════════════════\n");
    
    // Clear buffer
    while(dfSerial.available()) {
      dfSerial.read();
    }
    
    Serial.println("📡 Test 1: Reset command (0x0C)");
    sendDFCommand(0x0C, 0x00, 0x00);
    delay(200);
    checkUARTResponseDetailed();
    
    delay(2000); // Long wait for reset
    
    Serial.println("\n📡 Test 2: SD card check (0x3F - Device status)");
    sendDFCommand(0x3F, 0x00, 0x00);
    delay(200);
    checkUARTResponseDetailed();
    
    delay(500);
    
    Serial.println("\n📡 Test 3: SD card file count (0x48)");
    sendDFCommand(0x48, 0x00, 0x00);
    delay(200);
    checkUARTResponseDetailed();
    
    delay(500);
    
    Serial.println("\n📡 Test 4: Set volume to 30 (0x06)");
    sendDFCommand(0x06, 0x00, 0x1E);
    delay(200);
    checkUARTResponseDetailed();
    
    delay(500);
    
    Serial.println("\n📡 Test 5: Play first file (0x03 -> 001.mp3)");
    digitalWrite(LED_D6, HIGH);
    
    // First set volume to max
    Serial.println("   → Setting volume to 30...");
    sendDFCommand(0x06, 0x00, 0x1E);
    delay(300);
    
    // Clear buffer
    while(dfSerial.available()) dfSerial.read();
    
    // Play file (from root directory)
    Serial.println("   → Playing 001.mp3 (root directory)...");
    sendDFCommand(0x03, 0x00, 0x01);
    delay(200);
    checkUARTResponseDetailed();
    
    Serial.println("\n⏳ Waiting 3 seconds (for audio)...");
    Serial.println("🔊 Is sound coming from the speaker?\n");
    
    // Listen to all messages for 5 seconds and interpret
    unsigned long startWait = millis();
    while(millis() - startWait < 5000) {
      if(dfSerial.available()) {
        delay(50); // Wait for complete packet
        
        byte buffer[50];
        int idx = 0;
        Serial.print("   📥 [RX]: ");
        while(dfSerial.available() && idx < 50) {
          buffer[idx] = dfSerial.read();
          if(buffer[idx] < 0x10) Serial.print("0");
          Serial.print(buffer[idx], HEX);
          Serial.print(" ");
          idx++;
          delay(2);
        }
        Serial.println();
        
        // Interpret packets
        for(int i = 0; i < idx - 9; i++) {
          if(buffer[i] == 0x7E && buffer[i+1] == 0xFF && buffer[i+2] == 0x06 && buffer[i+9] == 0xEF) {
            byte response[10];
            for(int j = 0; j < 10; j++) {
              response[j] = buffer[i+j];
            }
            Serial.print("      └─> ");
            interpretDFPlayerPacket(response);
            i += 9;
          }
        }
      }
      delay(100);
    }
    
    digitalWrite(LED_D6, LOW);
    
    Serial.println("\n═══════════════════════════════════════");
    Serial.println("🔁 TEST 6: Play from MP3 folder (0x12)");
    Serial.println("═══════════════════════════════════════");
    Serial.println("Command: 0x12 → SD:/MP3/0001.mp3\n");
    
    // Clear buffer
    while(dfSerial.available()) dfSerial.read();
    
    digitalWrite(LED_D6, HIGH);
    sendDFCommand(0x12, 0x00, 0x01); // Play MP3/001.mp3
    delay(200);
    checkUARTResponseDetailed();
    
    Serial.println("\n⏳ Waiting 3 seconds...");
    Serial.println("🔊 Is sound coming from the speaker?\n");
    
    unsigned long startWait2 = millis();
    while(millis() - startWait2 < 3000) {
      if(dfSerial.available()) {
        delay(50);
        byte buffer2[50];
        int idx2 = 0;
        Serial.print("   📥 [RX]: ");
        while(dfSerial.available() && idx2 < 50) {
          buffer2[idx2] = dfSerial.read();
          if(buffer2[idx2] < 0x10) Serial.print("0");
          Serial.print(buffer2[idx2], HEX);
          Serial.print(" ");
          idx2++;
          delay(2);
        }
        Serial.println();
        
        for(int i = 0; i < idx2 - 9; i++) {
          if(buffer2[i] == 0x7E && buffer2[i+1] == 0xFF && buffer2[i+2] == 0x06 && buffer2[i+9] == 0xEF) {
            byte response[10];
            for(int j = 0; j < 10; j++) {
              response[j] = buffer2[i+j];
            }
            Serial.print("      └─> ");
            interpretDFPlayerPacket(response);
            i += 9;
          }
        }
      }
      delay(100);
    }
    digitalWrite(LED_D6, LOW);
    
    Serial.println("\n═══════════════════════════════════════");
    Serial.println("📊 DFPLAYER RESPONSE CODES:");
    Serial.println("   0x3F = Device online (0x01=USB, 0x02=SD)");
    Serial.println("   0x40 = ERROR message");
    Serial.println("   0x41 = ACK confirmation message");
    Serial.println("   0x48 = SD card file count response");
    Serial.println("\n🚫 ERROR CODES (param in 0x40 message):");
    Serial.println("   0x01 = Module busy");
    Serial.println("   0x02 = SD card not inserted/cannot be read");
    Serial.println("   0x03 = File not found");
    Serial.println("   0x04 = Checksum error");
    Serial.println("   0x05 = File index out of bounds");
    Serial.println("   0x06 = File mismatch/cannot be read");
    Serial.println("═══════════════════════════════════════\n");
    
    Serial.println("💡 SOLUTION SUGGESTIONS:");
    Serial.println("✅ If you get 0x02 error:");
    Serial.println("   1. Remove and reinsert the SD card");
    Serial.println("   2. Try a different SD card (2-32GB, FAT32)");
    Serial.println("   3. Clean the SD card contacts");
    Serial.println("   4. Disconnect DFPlayer from USB and power with external 5V");
    Serial.println("\n✅ If you get 0x03/0x06 error:");
    Serial.println("   1. Name MP3 files as 001.mp3, 002.mp3 (3 digits!)");
    Serial.println("   2. Place files in SD card root directory (no folders)");
    Serial.println("   3. MP3 format: 128kbps, 44.1kHz, mono/stereo");
    Serial.println("═══════════════════════════════════════\n");
  }
  else if (command == "test led") {
    Serial.println("\n💡 LED Test starting...");
    testLEDs();
    Serial.println("✅ LED test completed!\n");
  }
  else if (command == "test button") {
    Serial.println("\n🔘 Button Test:");
    Serial.println("Press the button within 5 seconds...\n");
    unsigned long buttonTestStart = millis();
    bool buttonPressed = false;
    int pressCount = 0;
    
    while(millis() - buttonTestStart < 5000) {
      if(digitalRead(BUTTON_PIN) == LOW) {
        if(!buttonPressed) {
          pressCount++;
          Serial.printf("✅ Button pressed! (%d. time)\n", pressCount);
          buttonPressed = true;
        }
      } else {
        buttonPressed = false;
      }
      delay(50);
    }
    
    if(pressCount == 0) {
      Serial.println("❌ Button not pressed or not working!");
      Serial.println("Check: D9 (GPIO20) → Button → GND");
    } else {
      Serial.printf("\n✅ Button working! Total %d presses detected.\n", pressCount);
    }
    Serial.println();
  }
  else if (command == "test dfplayer") {
    Serial.println("\n🎵 DFPlayer Test starting...");
    Serial.println("Connection check:");
    Serial.println("  ESP D6 (GPIO16) TX → DFPlayer RX");
    Serial.println("  ESP D7 (GPIO17) RX ← DFPlayer TX");
    Serial.println("  5V → DFPlayer VCC");
    Serial.println("  GND → DFPlayer GND\n");
    testDFPlayerRaw();
  }
  else if (command == "test speaker") {
    Serial.println("\n🔊 Speaker Test:");
    Serial.println("3 different audio files will be played...\n");
    
    for(int i = 1; i <= 3; i++) {
      Serial.printf("📢 Playing Track %d (00%d.mp3)...\n", i, i);
      digitalWrite(LED_D6, HIGH);
      
      sendDFCommand(0x03, 0x00, i);
      delay(3000);
      
      digitalWrite(LED_D6, LOW);
      Serial.printf("✅ Track %d completed.\n\n", i);
      delay(500);
    }
    
    Serial.println("🎵 Speaker test completed!");
    Serial.println("If sound came → Speaker is working");
    Serial.println("If no sound came → Check SD card/MP3 files\n");
  }
  else if (command == "led button") {
    Serial.println("\n💡 LED + Button Interactive Test:");
    Serial.println("Each time you press the button, LEDs will light up in sequence.");
    Serial.println("Type 'q' in Serial to exit.\n");
    
    int ledIndex = 0;
    int ledPins[] = {LED_D2, LED_D3, LED_D6};
    String ledNames[] = {"WiFi", "HTTP", "Audio"};
    bool lastButtonState = HIGH;
    
    while(true) {
      // Check for Serial exit
      if(Serial.available() > 0) {
        char c = Serial.read();
        if(c == 'q' || c == 'Q') {
          Serial.println("\n✅ Test terminated.\n");
          // Turn off all LEDs
          digitalWrite(LED_D2, LOW);
          digitalWrite(LED_D3, LOW);
          digitalWrite(LED_D6, LOW);
          break;
        }
      }
      
      // Read button state
      bool currentButtonState = digitalRead(BUTTON_PIN);
      
      // When button is pressed (HIGH→LOW transition)
      if(lastButtonState == HIGH && currentButtonState == LOW) {
        // Turn off previous LED
        digitalWrite(ledPins[ledIndex], LOW);
        
        // Move to next LED
        ledIndex = (ledIndex + 1) % 3;
        
        // Turn on new LED
        digitalWrite(ledPins[ledIndex], HIGH);
        Serial.printf("🔘 Button pressed → LED_%s lit\n", ledNames[ledIndex].c_str());
        
        delay(200); // Debounce
      }
      
      lastButtonState = currentButtonState;
      delay(10);
    }
  }
  else if (command == "test all") {
    Serial.println("\n========================================");
    Serial.println("      ALL HARDWARE TEST STARTING");
    Serial.println("========================================\n");
    
    // 1. LED Test
    Serial.println("1️⃣  LED Test...");
    testLEDs();
    Serial.println("   ✅ LED test completed!\n");
    delay(1000);
    
    // 2. Button Test
    Serial.println("2️⃣  Button Test:");
    Serial.println("   Press the button within 3 seconds...");
    unsigned long buttonTestStart = millis();
    bool buttonPressed = false;
    while(millis() - buttonTestStart < 3000) {
      if(digitalRead(BUTTON_PIN) == LOW) {
        Serial.println("   ✅ Button working!");
        buttonPressed = true;
        delay(500);
        break;
      }
      delay(50);
    }
    if(!buttonPressed) {
      Serial.println("   ⚠️  Button not pressed or not working!");
    }
    delay(1000);
    
    // 3. DFPlayer UART Test
    Serial.println("\n3️⃣  DFPlayer UART Test:");
    testDFPlayerRaw();
    
    Serial.println("\n========================================");
    Serial.println("        TEST COMPLETED");
    Serial.println("========================================\n");
  }
  else {
    Serial.println("ERROR: Unknown command! Type 'help'.");
  }
}

void connectWiFi() {
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("📡 WiFi CONNECTION (In background)");
  Serial.println("═══════════════════════════════════════");
  Serial.println("SSID: " + ssid);
  Serial.print("Password length: ");
  Serial.print(password.length());
  Serial.println(" characters");
  Serial.print("Password: ");
  for(int i = 0; i < password.length(); i++) Serial.print("*");
  Serial.println("\n");
  
  // First disconnect current connection
  WiFi.disconnect(true);
  delay(100);
  
  WiFi.mode(WIFI_STA);
  Serial.println("📶 Connecting (system continues operating)...");
  WiFi.begin(ssid.c_str(), password.c_str());
  
  wifiConnecting = true;
  wifiConnectStart = millis();
  wifiConnected = false;
  
  Serial.println("💡 WiFi connecting in background, you can continue using commands.");
  Serial.println("═══════════════════════════════════════\n");
}

void checkWiFiStatus() {
  if (!wifiConnecting) return;
  
  // Timeout check
  if (millis() - wifiConnectStart > wifiTimeout) {
    wifiConnecting = false;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("\n❌ WiFi connection timeout!");
      Serial.print("   Last status: ");
      switch(WiFi.status()) {
        case WL_NO_SSID_AVAIL: 
          Serial.println("SSID not found");
          Serial.println("\n💡 SOLUTION SUGGESTIONS:");
          Serial.println("   1. Check SSID (case sensitive)");
          Serial.println("   2. Check if modem/router is on");
          Serial.println("   3. Make sure 2.4GHz WiFi is used (5GHz not supported)");
          break;
        case WL_CONNECT_FAILED: 
          Serial.println("Connection failed (probably password error)");
          Serial.println("\n💡 SOLUTION SUGGESTIONS:");
          Serial.println("   1. Check password (case sensitive)");
          Serial.println("   2. If special characters exist, make sure they are entered correctly");
          Serial.println("   3. Check if MAC filter is active on router");
          break;
        default: 
          Serial.printf("Unknown error (%d)\n", WiFi.status());
          break;
      }
      Serial.println("\n📝 Try again with 'wifi SSID PASSWORD' command.\n");
      digitalWrite(LED_D2, LOW);
      wifiConnected = false;
    }
    return;
  }
  
  // Connection check
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnecting = false;
    wifiConnected = true;
    
    Serial.println("\n═══════════════════════════════════════");
    Serial.println("✅ WiFi connection successful!");
    Serial.print("   IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("   Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    unsigned long connectTime = (millis() - wifiConnectStart) / 1000;
    Serial.printf("   Connection time: %lu seconds\n", connectTime);
    Serial.println("═══════════════════════════════════════\n");
    
    // Turn off LED_D2 (system still in x=0 state)
    digitalWrite(LED_D2, LOW);
  }
}

void loadSettings() {
  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  
  // URL List 1 (Open)
  urlCount1 = preferences.getInt("urlCount1", 0);
  for (int i = 0; i < urlCount1; i++) {
    String key = "url1_" + String(i);
    urlList1[i] = preferences.getString(key.c_str(), "");
  }
  
  // URL List 2 (Close)
  urlCount2 = preferences.getInt("urlCount2", 0);
  for (int i = 0; i < urlCount2; i++) {
    String key = "url2_" + String(i);
    urlList2[i] = preferences.getString(key.c_str(), "");
  }
  
  // Volume level
  int savedVolume = preferences.getInt("volume", 20);
  sendDFCommand(0x06, 0x00, savedVolume);
  
  Serial.println("\n✅ Saved settings loaded:");
  Serial.printf("   WiFi: %s\n", ssid.length() > 0 ? ssid.c_str() : "(none)");
  Serial.printf("   URL List 1: %d\n", urlCount1);
  Serial.printf("   URL List 2: %d\n", urlCount2);
  Serial.printf("   Volume: %d/30\n", savedVolume);
  Serial.printf("   System state: x = %d (always 0 at startup)\n", systemState);
}

void saveUrls() {
  // List 1
  preferences.putInt("urlCount1", urlCount1);
  for (int i = 0; i < urlCount1; i++) {
    String key = "url1_" + String(i);
    preferences.putString(key.c_str(), urlList1[i]);
  }
  for (int i = urlCount1; i < 10; i++) {
    String key = "url1_" + String(i);
    preferences.remove(key.c_str());
  }
  
  // List 2
  preferences.putInt("urlCount2", urlCount2);
  for (int i = 0; i < urlCount2; i++) {
    String key = "url2_" + String(i);
    preferences.putString(key.c_str(), urlList2[i]);
  }
  for (int i = urlCount2; i < 10; i++) {
    String key = "url2_" + String(i);
    preferences.remove(key.c_str());
  }
}

void printHelp() {
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("📋 COMMAND LIST");
  Serial.println("═══════════════════════════════════════\n");
  
  Serial.println("🌐 WiFi Settings:");
  Serial.println("   wifi SSID PASSWORD     - Setup WiFi connection\n");
  
  Serial.println("🔗 URL Management:");
  Serial.println("   add url1 http://...    - Add URL to List 1 (OPEN)");
  Serial.println("   add url2 http://...    - Add URL to List 2 (CLOSE)");
  Serial.println("   del url1 N             - Delete Nth URL from List 1");
  Serial.println("   del url2 N             - Delete Nth URL from List 2");
  Serial.println("   list url               - Show all URL lists\n");
  
  Serial.println("🔊 Audio Control:");
  Serial.println("   volume 0-30            - Set volume level");
  Serial.println("   play N                 - Play Nth MP3 (test)\n");
  
  Serial.println("📊 System:");
  Serial.println("   status                 - Show system status");
  Serial.println("   reset                  - Reset all settings\n");
  
  Serial.println("🧪 Test:");
  Serial.println("   test quiet             - Audio test (minimal debug)\n");
  
  Serial.println("═══════════════════════════════════════");
  Serial.println("💡 HOW IT WORKS?");
  Serial.println("═══════════════════════════════════════");
  Serial.println("System works in toggle mode (x variable):");
  Serial.println("• x=0 → Button press → TURNS ON (x=1)");
  Serial.println("  └─> 001.mp3 → URL List 1 → Result sound");
  Serial.println("• x=1 → Button press → TURNS OFF (x=0)");
  Serial.println("  └─> 005.mp3 → URL List 2");
  Serial.println("\nResult sounds (for ON state):");
  Serial.println("• Successful: 002.mp3");
  Serial.println("• No internet: 003.mp3 + 004.mp3");
  Serial.println("• Error: 004.mp3\n");
  
  Serial.println("⚠️  NOTE: Device resets to x=0 on every boot\n");
}

void printStatus() {
  Serial.println("\n═══════════════════════════════════════");
  Serial.println("📊 SYSTEM STATUS");
  Serial.println("═══════════════════════════════════════\n");
  
  Serial.println("🔘 Toggle State:");
  Serial.printf("   x = %d (%s)\n", systemState, systemState == 1 ? "🟢 ON" : "🔴 OFF");
  Serial.println("   (Resets to 0 on every boot)\n");
  
  Serial.println("🌐 WiFi:");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("   ✅ Connected: %s\n", ssid.c_str());
    Serial.printf("   IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   Signal: %d dBm\n", WiFi.RSSI());
  } else {
    Serial.printf("   ❌ Not connected\n");
    if (ssid.length() > 0) {
      Serial.printf("   Saved SSID: %s\n", ssid.c_str());
    }
  }
  Serial.println();
  
  Serial.printf("🔗 URL List 1 (OPEN - x: 0→1): %d/10\n", urlCount1);
  for (int i = 0; i < urlCount1; i++) {
    Serial.printf("   %d. %s\n", i + 1, urlList1[i].c_str());
  }
  if (urlCount1 == 0) Serial.println("   (empty)");
  Serial.println();
  
  Serial.printf("🔗 URL List 2 (CLOSE - x: 1→0): %d/10\n", urlCount2);
  for (int i = 0; i < urlCount2; i++) {
    Serial.printf("   %d. %s\n", i + 1, urlList2[i].c_str());
  }
  if (urlCount2 == 0) Serial.println("   (empty)");
  Serial.println();
  
  Serial.println("🔊 Audio:");
  Serial.printf("   Level: %d/30\n\n", preferences.getInt("volume", 20));
  
  Serial.println("💾 Memory:");
  Serial.printf("   Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("   Uptime: %lu seconds\n", millis() / 1000);
  Serial.println("\n═══════════════════════════════════════\n");
}

void testLEDs() {
  Serial.println("Starting LED test...");
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_D2, HIGH);
    delay(200);
    digitalWrite(LED_D2, LOW);
    
    digitalWrite(LED_D3, HIGH);
    delay(200);
    digitalWrite(LED_D3, LOW);
    
    digitalWrite(LED_D6, HIGH);
    delay(200);
    digitalWrite(LED_D6, LOW);
  }
}

void blinkLED(int pin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(150);
    digitalWrite(pin, LOW);
    delay(150);
  }
}

// LED fade function (slow on/off with PWM)
void fadeLED(int pin, int startBrightness, int endBrightness, int duration) {
  // PWM setup (ESP32-C6)
  ledcAttach(pin, 5000, 8); // 5kHz, 8-bit resolution
  
  int steps = 50; // Fade in 50 steps
  int stepDelay = duration / steps;
  
  for(int i = 0; i <= steps; i++) {
    int brightness = map(i, 0, steps, startBrightness, endBrightness);
    ledcWrite(pin, brightness);
    delay(stepDelay);
  }
  
  // Turn off PWM after fade and switch to normal digital mode
  ledcDetach(pin);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, endBrightness > 0 ? HIGH : LOW);
}

// LED flicker effect (before fade)
void flickerLED(int pin, int times, int delayMs) {
  for(int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(delayMs);
    digitalWrite(pin, LOW);
    delay(delayMs);
  }
}

void blinkError() {
  while (true) {
    digitalWrite(LED_D6, HIGH);
    delay(100);
    digitalWrite(LED_D6, LOW);
    delay(100);
  }
}

// Raw UART functions (used if library doesn't work)
void sendDFCommand(byte cmd, byte param1, byte param2) {
  byte packet[10];
  
  // Build packet
  packet[0] = 0x7E;  // Start byte
  packet[1] = 0xFF;  // Version
  packet[2] = 0x06;  // Length
  packet[3] = cmd;   // Command
  packet[4] = 0x00;  // Feedback (0=no, 1=yes)
  packet[5] = param1; // Parameter high byte
  packet[6] = param2; // Parameter low byte
  packet[9] = 0xEF;  // End byte
  
  // Calculate checksum (DFRobot official formula)
  int16_t checksum = -(packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6]);
  packet[7] = (byte)(checksum >> 8);   // Checksum high byte
  packet[8] = (byte)(checksum & 0xFF); // Checksum low byte
  
  // Debug: Show sent packet
  Serial.print("📤 [TX]: ");
  for(int i = 0; i < 10; i++) {
    if(packet[i] < 0x10) Serial.print("0");
    Serial.print(packet[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  Serial.flush(); // Empty Serial buffer
  
  // Send
  dfSerial.write(packet, 10);
  dfSerial.flush(); // Empty UART buffer
  
  delay(50); // Command processing time
}

void testDFPlayerRaw() {
  // Clear buffer
  while(dfSerial.available()) {
    dfSerial.read();
  }
  
  Serial.println("   📡 Sending reset command...");
  sendDFCommand(0x0C, 0x00, 0x00);  // Reset
  delay(500);
  
  Serial.println("   📡 Setting volume (25/30)...");
  sendDFCommand(0x06, 0x00, 0x19);  // Volume 25
  delay(300);
  
  Serial.println("   📡 Playing track 1 (001.mp3)...");
  digitalWrite(LED_D6, HIGH);
  sendDFCommand(0x03, 0x00, 0x01);  // Play track 1
  
  Serial.println("\n   ⏳ Waiting 3 seconds...");
  Serial.println("   🔊 Is sound coming from the speaker?");
  Serial.println("   → Yes: DFPlayer is working! Check SD card.");
  Serial.println("   → No: Check connections or DFPlayer.\n");
  
  // Response check
  bool gotResponse = false;
  unsigned long start = millis();
  while(millis() - start < 1000) {
    if(dfSerial.available() > 0) {
      gotResponse = true;
      Serial.print("   ✅ DFPlayer responded: ");
      while(dfSerial.available()) {
        byte b = dfSerial.read();
        if(b < 0x10) Serial.print("0");
        Serial.print(b, HEX);
        Serial.print(" ");
        delay(2);
      }
      Serial.println();
      break;
    }
  }
  
  if(!gotResponse) {
    Serial.println("   ⚠️  DFPlayer didn't respond (normal for some clones)");
    Serial.println("   💡 If sound is playing, no problem!");
  }
  
  delay(3000);  // Audio playback duration
  digitalWrite(LED_D6, LOW);
}

void checkUARTResponse() {
  Serial.print("📥 [RX]: ");
  unsigned long start = millis();
  bool gotData = false;
  
  while(millis() - start < 500) {
    if(dfSerial.available() > 0) {
      gotData = true;
      byte b = dfSerial.read();
      if(b < 0x10) Serial.print("0");
      Serial.print(b, HEX);
      Serial.print(" ");
      delay(2);
    }
  }
  
  if(!gotData) {
    Serial.println("❌ No response (may be normal)");
  } else {
    Serial.println("✅ Response received!");
  }
}

void checkUARTResponseDetailed() {
  unsigned long start = millis();
  byte buffer[50];
  int bufferIdx = 0;
  bool gotData = false;
  
  // Wait 500ms, but stop collecting once packet is complete
  while(millis() - start < 500 && bufferIdx < 50) {
    if(dfSerial.available() > 0) {
      gotData = true;
      buffer[bufferIdx] = dfSerial.read();
      bufferIdx++;
      start = millis(); // Reset timeout on each byte received
      delay(1);
    }
  }
  
  if(!gotData) {
    Serial.println("   📥 [RX]: ❌ No response");
    return;
  }
  
  // Collection complete, now print
  Serial.print("   📥 [RX]: ");
  for(int i = 0; i < bufferIdx; i++) {
    if(buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // HEX DUMP - Raw packet analysis
  Serial.print("   🔍 Raw: ");
  for(int i = 0; i < bufferIdx; i++) {
    if(buffer[i] < 0x10) Serial.print("0");
    Serial.print(buffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Find 0x7E starts in buffer and interpret packets
  for(int i = 0; i < bufferIdx - 9; i++) {
    if(buffer[i] == 0x7E && buffer[i+1] == 0xFF && buffer[i+2] == 0x06) {
      // Check packet end (should be 0xEF)
      if((i+9) < bufferIdx && buffer[i+9] == 0xEF) {
        // Valid packet found
        byte response[10];
        for(int j = 0; j < 10; j++) {
          response[j] = buffer[i+j];
        }
        
        // Interpret packet
        Serial.print("   ╰─> ");
        interpretDFPlayerPacket(response);
        
        i += 9; // Skip to next packet
      }
    }
  }
}

void interpretDFPlayerPacket(byte* response) {
  // Packet format: 7E FF 06 CMD 00 PARAM1 PARAM2 CHKH CHKL EF
  if(response[0] != 0x7E || response[1] != 0xFF) {
    Serial.println("⚠️  Invalid packet header");
    return;
  }
  
  byte cmd = response[3];
  byte param1 = response[5];
  byte param2 = response[6];
  uint16_t param = (param1 << 8) | param2;
  
  // Show hex packet
  Serial.print("[");
  for(int i = 0; i < 10; i++) {
    if(response[i] < 0x10) Serial.print("0");
    Serial.print(response[i], HEX);
    if(i < 9) Serial.print(" ");
  }
  Serial.print("] → ");
  
  switch(cmd) {
    case 0x3F:
      Serial.print("📀 Device status: ");
      if(param == 0x01) Serial.println("USB online");
      else if(param == 0x02) Serial.println("✅ SD CARD ONLINE!");
      else if(param == 0x03) Serial.println("USB + SD card online");
      else Serial.printf("Unknown (0x%04X)\n", param);
      break;
      
    case 0x40:
      Serial.print("🚫 ERROR: ");
      switch(param) {
        case 0x01: Serial.println("Module busy"); break;
        case 0x02: Serial.println("⚠️  SD card not inserted / cannot read"); break;
        case 0x03: Serial.println("❌ File not found"); break;
        case 0x04: Serial.println("Checksum error"); break;
        case 0x05: Serial.println("File index out of bounds"); break;
        case 0x06: Serial.println("❌ File mismatch / cannot read"); break;
        default: Serial.printf("Unknown error (0x%02X)\n", param); break;
      }
      break;
      
    case 0x41:
      Serial.println("✅ ACK - Command acknowledged");
      break;
      
    case 0x48:
      Serial.printf("📂 SD card has %d files ✅\n", param);
      break;
      
    case 0x3A:
      if(param & 0x02) Serial.println("📀 SD card inserted");
      if(param & 0x01) Serial.println("💾 USB inserted");
      break;
      
    case 0x3B:
      if(param & 0x02) Serial.println("📀 SD card removed");
      if(param & 0x01) Serial.println("💾 USB removed");
      break;
      
    case 0x3D:
      Serial.printf("🎵 File %d playback finished\n", param);
      break;
      
    default:
      Serial.printf("Unknown command: 0x%02X, Param: 0x%04X\n", cmd, param);
      break;
  }
}
