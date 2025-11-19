/*
 * ShifterKey v1.1
 * By SmartKraft
 * July 2025
 * 
 * Hardware:
 * - ESP12S (ESP8266 based WiFi-enabled microcontroller)
 * - PN532 RFID Reader (I2C - connected to SDA:GPIO4, SCL:GPIO5)
 * - OLED 128x64 Display (I2C - connected to same SDA/SCL pins)
 * - 3 Buttons (Right: GPIO13, Left: GPIO14, OK: GPIO12)
 * 
 * Note: ESP12S has WiFi capabilities but NO Bluetooth.
 * 
 * Memory usage:
 * ESP12S has limited memory (~80KB usable). This code has been
 * optimized to run within these constraints.
 */

// Uncomment to enable debug output
// #define DEBUG_MODE

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

// Pin definitions
#define PN532_IRQ   -1    // IRQ pin not used with I2C
#define PN532_RESET -1    // Reset pin not used with I2C
#define RIGHT_BTN   13
#define LEFT_BTN    14
#define OK_BTN      12
#define SDA_PIN     4
#define SCL_PIN     5

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Language settings
#define LANG_EN 0
#define LANG_DE 1
#define LANG_TR 2
#define LANGUAGE_ADDR 0

// UID storage settings
#define MAX_SAVED_UIDS 10
#define UID_ADDR_START 10
#define UID_SIZE 16 // Max length for hex string (14 hex chars + null terminator)

// Initialize devices
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// Global variables
uint8_t currentLanguage = LANG_EN;
int menuPosition = 0;
int submenuPosition = 0;
bool inSubmenu = false;
bool inSubSubmenu = false;
char currentUID[UID_SIZE] = "";
char savedUIDs[MAX_SAVED_UIDS][UID_SIZE]; // Hex string for UIDs  
char savedUIDNames[MAX_SAVED_UIDS][16]; // Names for saved UIDs
bool buttonState[3] = {false, false, false};
bool lastButtonState[3] = {false, false, false};
unsigned long lastDebounceTime[3] = {0, 0, 0};
unsigned long debounceDelay = 500; // 500ms delay for better control
unsigned long lastButtonPress[3] = {0, 0, 0}; // Son buton basma zamanları
char hexChars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

// Menu structure
const char* mainMenuEN[] = {"Read", "Copy", "New UID", "RF List", "Settings", NULL};
const char* mainMenuDE[] = {"Lesen", "Kopieren", "Neue UID", "RF List", "Einstellung", NULL};
const char* mainMenuTR[] = {"Oku", "Kopyala", "Yeni UID", "RF Liste", "Ayarlar", NULL};

// Helper function to optimize ESP12S memory usage
// This detects available heap memory and can be called periodically to monitor
void checkMemory() {
  #ifdef DEBUG_MODE
  Serial.print(F("Free heap: "));
  Serial.println(ESP.getFreeHeap());
  #endif
  
  // Critical memory check - restart prevention
  if (ESP.getFreeHeap() < 2048) { // 2KB minimum
    // Emergency memory cleanup
    #ifdef DEBUG_MODE
    Serial.println(F("LOW MEMORY WARNING!"));
    #endif
    
    // Clear any large buffers if needed
    // Force garbage collection
    yield();
    delay(100);
  }
}

const char* settingsMenuEN[] = {"Language", "Info", "Back to Menu", NULL};
const char* settingsMenuDE[] = {"Sprache", "Info", "Zuruck zum Menu", NULL};
const char* settingsMenuTR[] = {"Dil", "Bilgi", "Ana Menuye Don", NULL};

const char* languageMenuEN[] = {"English", "Deutsch", "Turkce", "Back", NULL};
const char* languageMenuDE[] = {"English", "Deutsch", "Turkce", "Zuruck", NULL};
const char* languageMenuTR[] = {"English", "Deutsch", "Turkce", "Geri", NULL};

const char* readSubmenuEN[] = {"Save", "Back", NULL};
const char* readSubmenuDE[] = {"Speichern", "Zuruck", NULL};
const char* readSubmenuTR[] = {"Kaydet", "Geri", NULL};

const char* copySubmenuEN[] = {"Single Copy", "Multi Copy", "Back", NULL};
const char* copySubmenuDE[] = {"Einzelkopie", "Mehrfachkopie", "Zuruck", NULL};
const char* copySubmenuTR[] = {"Tekli Kopya", "Coklu Kopya", "Geri", NULL};

const char* newUidSubmenuEN[] = {"4 Byte", "7 Byte", "Back", NULL};
const char* newUidSubmenuDE[] = {"4 Byte", "7 Byte", "Zuruck", NULL};
const char* newUidSubmenuTR[] = {"4 Byte", "7 Byte", "Geri", NULL};

const char* savedSubmenuEN[] = {"Copy", "Delete", "Back", NULL};
const char* savedSubmenuDE[] = {"Kopieren", "Loschen", "Zuruck", NULL};
const char* savedSubmenuTR[] = {"Kopyala", "Sil", "Geri", NULL};

// Function declarations
void setupButtons();
void setupOLED();
void setupPN532();
void loadSettings();
void saveSettings();
void loadSavedUIDs();
void saveUID(const char* uid, int position);
void deleteUID(int position);
void showStartAnimation();
void showMainMenu();
void showSubmenu(const char** submenu);
void handleMainMenu();
void handleButtons();
bool isButtonPressed(int pin, int buttonIndex);
void readRFID();
void copyRFID(bool multiCopy);
void createNewUID(bool is4Byte);
void handleSavedUID(int position);
void showLanguageMenu();
void showInfoScreen();
void handleSettingsMenu();
void setLanguage(uint8_t lang);
void handleCopyMenu();
void handleNewUIDMenu();
void handleSettingsDirectly();
const char** getCurrentMenuByLanguage(int menuType);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  EEPROM.begin(512);
  
  setupButtons();
  setupOLED();
  setupPN532();
  
  loadSettings();
  loadSavedUIDs();
  
  // Check available memory at startup
  checkMemory();
  
  showStartAnimation();
  delay(1000);
}

void loop() {
  handleButtons();
  
  if (!inSubmenu && !inSubSubmenu) {
    showMainMenu();
    handleMainMenu();
    
    // Check memory periodically (every ~10 seconds)
    static unsigned long lastMemCheck = 0;
    if (millis() - lastMemCheck > 10000) {
      checkMemory();
      lastMemCheck = millis();
    }
  }
}

void setupButtons() {
  pinMode(RIGHT_BTN, INPUT_PULLUP);
  pinMode(LEFT_BTN, INPUT_PULLUP);
  pinMode(OK_BTN, INPUT_PULLUP);
}

void setupOLED() {
  // ESP12S I2C pins are already set in Wire.begin(SDA_PIN, SCL_PIN)
  // Standard OLED address is 0x3C, but some displays use 0x3D
  // If display doesn't work with 0x3C, try 0x3D instead
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    // Try alternative address if first attempt fails
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("SSD1306 allocation failed with alternative address"));
      for (;;); // Don't proceed, loop forever
    }
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.dim(false);
}

void setupPN532() {
  // PN532 shares I2C bus with OLED display
  // Default I2C address for PN532 is 0x24 (cannot be changed)
  nfc.begin();
  
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("PN532 not found!");
    display.println("Check connections");
    display.display();
    delay(2000);
    while (1); // halt
  }
  
  Serial.print("Found PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Configure board to read RFID tags
  nfc.SAMConfig();
}

void loadSettings() {
  currentLanguage = EEPROM.read(LANGUAGE_ADDR);
  if (currentLanguage > LANG_TR) {
    currentLanguage = LANG_EN;
    EEPROM.write(LANGUAGE_ADDR, currentLanguage);
    EEPROM.commit();
  }
}

void saveSettings() {
  EEPROM.write(LANGUAGE_ADDR, currentLanguage);
  EEPROM.commit();
}

void loadSavedUIDs() {
  int addr = UID_ADDR_START;
  for (int i = 0; i < MAX_SAVED_UIDS; i++) {
    for (int j = 0; j < UID_SIZE; j++) {
      savedUIDs[i][j] = EEPROM.read(addr++);
    }
    // Read name (optional feature)
    for (int j = 0; j < 16; j++) {
      savedUIDNames[i][j] = EEPROM.read(addr++);
    }
  }
}

void saveUID(const char* uid, int position) {
  if (position < 0 || position >= MAX_SAVED_UIDS) return;
  
  // Aynı UID zaten kayıtlı mı kontrol et
  if (strcmp(savedUIDs[position], uid) == 0) {
    // Aynı UID zaten bu pozisyonda kayıtlı, işlem yapma
    return;
  }
  
  int addr = UID_ADDR_START + (position * (UID_SIZE + 16));
  
  // Save UID as hex string
  for (int i = 0; i < UID_SIZE; i++) {
    char c = (i < strlen(uid)) ? uid[i] : '\0';
    EEPROM.write(addr++, c);
    savedUIDs[position][i] = c;
  }
  
  // Save a default name (optional)
  for (int i = 0; i < 16; i++) {
    if (i < 8) {
      char c = i < strlen("CARD") ? "CARD"[i] : '0' + position; // Varsayılan isim
      EEPROM.write(addr++, c);
      savedUIDNames[position][i] = c;
    } else {
      EEPROM.write(addr++, '\0');
      savedUIDNames[position][i] = '\0';
    }
  }
  
  EEPROM.commit();
}

void deleteUID(int position) {
  if (position < 0 || position >= MAX_SAVED_UIDS) return;
  
  int addr = UID_ADDR_START + (position * (UID_SIZE + 16));
  
  // Clear UID and name
  for (int i = 0; i < UID_SIZE + 16; i++) {
    EEPROM.write(addr++, 0);
  }
  
  memset(savedUIDs[position], 0, UID_SIZE);
  memset(savedUIDNames[position], 0, 16);
  
  EEPROM.commit();
}

void showStartAnimation() {
  display.clearDisplay();
  
  display.setTextSize(2);
  const char* title = "ShifterKEY";
  int titleLen = strlen(title);
  int centerX = (SCREEN_WIDTH - (titleLen * 12)) / 2;
  int centerY = 20;
  
  // FASE 1: "Shifter" harflerini tek tek yaz
  String shifterPart = "Shifter";
  for (int i = 0; i < shifterPart.length(); i++) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(centerX, centerY);
    
    // Şu ana kadar yazılan harfleri göster
    for (int j = 0; j <= i; j++) {
      display.print(shifterPart[j]);
    }
    display.display();
    delay(150); // Her harf için 150ms bekleme
    yield();
  }
  
  // FASE 2: "Key" kısmının yanıp sönme efekti
  // 0.2s yanar, 0.2s söner, 0.2s yanar, 0.6s söner, sonra yanar
  
  // Shifter kısmını sabit tut
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX, centerY);
  display.print("Shifter");
  display.display();
  delay(100); // Kısa ara
  
  // Key yanıp sönme sekansı - yeni zamanlama
  // 1. 0.1s yanar
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX, centerY);
  display.print("ShifterKOD");
  display.display();
  delay(100);
  
  // 2. 0.1s söner
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX, centerY);
  display.print("Shifter");
  display.display();
  delay(100);
  
  // 3. 0.1s yanar
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX, centerY);
  display.print("ShifterKiT");
  display.display();
  delay(100);
  
  // 6. 0.3s söner
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX, centerY);
  display.print("Shifter");
  display.display();
  delay(600);
  
  // 7. Final yanar ve kalır
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX, centerY);
  display.print("ShifterKEY");
  display.display();
  delay(300);
  
  // FASE 3: SmartKraft matrix efekti (eski haliyle)
  display.setTextSize(1);
  const char* subtitle = "SmartKraft";
  int subTitleLen = strlen(subtitle);
  int subCenterX = (SCREEN_WIDTH - (subTitleLen * 6)) / 2;
  int subCenterY = centerY + 25;
  
  // Harf harf sabitlenme matrix efekti - 2 kat hızlandırıldı
  bool charStable[10] = {false}; // SmartKraft 10 karakter
  
  for (int iteration = 0; iteration < 20; iteration++) { // 40'tan 20'ye düştü
    display.clearDisplay();
    
    // ShifterKey sabit pozisyonda
    display.setTextSize(2);
    display.setCursor(centerX, centerY);
    display.print(title);
    
    // SmartKraft matrix efekti - her harf kendi zamanında sabitlenir
    display.setTextSize(1);
    for (int i = 0; i < subTitleLen; i++) {
      display.setCursor(subCenterX + (i * 6), subCenterY);
      
      // Her harf farklı zamanda sabitlenir - 2 kat hızlı
      int stabilizeTime = 8 + (i * 1); // 15+(i*2)'den 8+(i*1)'e düştü
      
      if (iteration >= stabilizeTime) {
        // Harf sabitlendi
        charStable[i] = true;
        display.print(subtitle[i]);
      } else if (iteration >= stabilizeTime - 3) { // 5'ten 3'e düştü
        // Sabitlenme öncesi flicker
        if (random(0, 3) == 0) {
          display.print(subtitle[i]);
        } else {
          char randomChar = random('A', 'Z');
          display.print(randomChar);
        }
      } else {
        // Tamamen rastgele matrix karakterleri
        char randomChar = random(33, 126);
        display.print(randomChar);
      }
    }
    display.display();
    delay(38); // 75'ten 38'e düştü (yarısı)
    yield(); // ESP8266 watchdog reset
  }
  
  // Final display with both texts stable
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(centerX, centerY);
  display.print(title);
  
  display.setTextSize(1);
  display.setCursor(subCenterX, subCenterY);
  display.print(subtitle);
  
  display.display();
  delay(500); // Yarıya indi (1000'den 500'e)
}

void showMainMenu() {
  display.clearDisplay();
  
  const char** menu = getCurrentMenuByLanguage(0); // 0 = Main Menu
  
  int menuSize = 0;
  while (menu[menuSize] != NULL) menuSize++;
  
  // Ana başlık - otomatik boyut ayarı (uzun başlıklar için küçültme)
  String currentTitle = String(menu[menuPosition]);
  int textSize = 2;
  int titleWidth = currentTitle.length() * 12; // 2x boyut için 12 pixel per char
  
  // Uzun başlıklar için boyutu küçült
  if (titleWidth > 120) { // 128-8 margin
    textSize = 1;
    titleWidth = currentTitle.length() * 6; // 1x boyut için 6 pixel per char
  }
  
  display.setTextSize(textSize);
  int titleX = (128 - titleWidth) / 2; // Ekranın ortasında
  display.setCursor(titleX, 10);
  display.setTextColor(SSD1306_WHITE);
  display.print(currentTitle);
  
  // Ok işaretleri (başlığın altında, sol ve sağda)
  display.setTextSize(1);
  if (menuSize > 1) {
    display.setCursor(10, 30); // Sol tarafta < işareti
    display.print("<");
    display.setCursor(118, 30); // Sağ tarafta > işareti
    display.print(">");
  }
  
  // Önceki ve sonraki menü öğeleri (ok işaretlerinin altında)
  if (menuSize > 1) {
    int prevIndex = (menuPosition - 1 + menuSize) % menuSize;
    int nextIndex = (menuPosition + 1) % menuSize;
    
    // Önceki öğe (sol tarafta, küçük)
    String prevTitle = String(menu[prevIndex]);
    if (prevTitle.length() > 10) prevTitle = prevTitle.substring(0, 10) + "..";
    display.setCursor(5, 45);
    display.print(prevTitle);
    
    // Sonraki öğe (sağ tarafta, küçük)
    String nextTitle = String(menu[nextIndex]);
    if (nextTitle.length() > 10) nextTitle = nextTitle.substring(0, 10) + "..";
    int nextWidth = nextTitle.length() * 6;
    int nextX = 128 - nextWidth - 5;
    display.setCursor(nextX, 45);
    display.print(nextTitle);
  }
  
  display.display();
}

void showSubmenu(const char** submenu) {
  display.clearDisplay();
  
  int menuSize = 0;
  while (submenu[menuSize] != NULL) menuSize++;
  
  // Ana başlık - otomatik boyut ayarı (uzun başlıklar için küçültme)
  String currentTitle = String(submenu[submenuPosition]);
  int textSize = 2;
  int titleWidth = currentTitle.length() * 12; // 2x boyut için 12 pixel per char
  
  // Uzun başlıklar için boyutu küçült
  if (titleWidth > 120) { // 128-8 margin
    textSize = 1;
    titleWidth = currentTitle.length() * 6; // 1x boyut için 6 pixel per char
  }
  
  display.setTextSize(textSize);
  int titleX = (128 - titleWidth) / 2; // Ekranın ortasında
  display.setCursor(titleX, 10);
  display.setTextColor(SSD1306_WHITE);
  display.print(currentTitle);
  
  // Ok işaretleri (başlığın altında, sol ve sağda)
  display.setTextSize(1);
  if (menuSize > 1) {
    display.setCursor(10, 30); // Sol tarafta < işareti
    display.print("<");
    display.setCursor(118, 30); // Sağ tarafta > işareti
    display.print(">");
  }
  
  // Önceki ve sonraki menü öğeleri (ok işaretlerinin altında)
  if (menuSize > 1) {
    int prevIndex = (submenuPosition - 1 + menuSize) % menuSize;
    int nextIndex = (submenuPosition + 1) % menuSize;
    
    // Önceki öğe (sol tarafta, küçük)
    String prevTitle = String(submenu[prevIndex]);
    if (prevTitle.length() > 10) prevTitle = prevTitle.substring(0, 10) + "..";
    display.setCursor(5, 45);
    display.print(prevTitle);
    
    // Sonraki öğe (sağ tarafta, küçük)
    String nextTitle = String(submenu[nextIndex]);
    if (nextTitle.length() > 10) nextTitle = nextTitle.substring(0, 10) + "..";
    int nextWidth = nextTitle.length() * 6;
    int nextX = 128 - nextWidth - 5;
    display.setCursor(nextX, 45);
    display.print(nextTitle);
  }
  
  display.display();
}

void handleMainMenu() {
  const char** menu = getCurrentMenuByLanguage(0);
  int menuSize = 0;
  while (menu[menuSize] != NULL) menuSize++;
  
  if (isButtonPressed(RIGHT_BTN, 0)) {
    menuPosition = (menuPosition + 1) % menuSize;
  }
  
  if (isButtonPressed(LEFT_BTN, 1)) {
    menuPosition = (menuPosition - 1 + menuSize) % menuSize;
  }
  
  if (isButtonPressed(OK_BTN, 2)) {
    switch (menuPosition) {
      case 0: // Read
        inSubmenu = true;
        submenuPosition = 0;
        readRFID();
        break;
      case 1: // Copy
        inSubmenu = true;
        submenuPosition = 0;
        handleCopyMenu();
        break;
      case 2: // New UID
        inSubmenu = true;
        submenuPosition = 0;
        handleNewUIDMenu();
        break;
      case 3: { // Saved - değişken tanımı için blok içine aldık
        // Logic to browse saved UIDs
        int savedPosition = 0;
        bool selectingSaved = true;
        
        while (selectingSaved) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(0, 0);
          
          const char* savedText;
          switch (currentLanguage) {
            case LANG_DE: savedText = "RF List:"; break;
            case LANG_TR: savedText = "RF Liste:"; break;
            default: savedText = "RF List:"; break;
          }
          display.println(savedText);
          display.print(savedPosition + 1);
          display.print(": ");
          
          if (strlen(savedUIDs[savedPosition]) > 0) {
            display.println(savedUIDs[savedPosition]);
          } else {
            display.println("[Empty]");
          }
          
          display.display();
          
          handleButtons();
          
          if (isButtonPressed(RIGHT_BTN, 0)) {
            savedPosition = (savedPosition + 1) % MAX_SAVED_UIDS;
          }
          
          if (isButtonPressed(LEFT_BTN, 1)) {
            savedPosition = (savedPosition - 1 + MAX_SAVED_UIDS) % MAX_SAVED_UIDS;
          }
          
          if (isButtonPressed(OK_BTN, 2)) {
            selectingSaved = false;
            handleSavedUID(savedPosition);
          }
          yield(); // ESP8266 watchdog reset
        }
        break;
      }
      case 4: // Settings
        // Settings için özel handling - inSubmenu false yapmadan direkt menu'ya gir
        handleSettingsDirectly();
        break;
    }
  }
}

void handleButtons() {
  for (int i = 0; i < 3; i++) {
    int pin;
    switch (i) {
      case 0: pin = RIGHT_BTN; break;
      case 1: pin = LEFT_BTN; break;
      case 2: pin = OK_BTN; break;
      default: continue;
    }
    
    bool reading = digitalRead(pin) == LOW;
    
    if (reading != lastButtonState[i]) {
      lastDebounceTime[i] = millis();
    }
    
    // Debounce süresini 50ms'e düşürdük, asıl kontrol isButtonPressed'de
    if ((millis() - lastDebounceTime[i]) > 50) {
      if (reading != buttonState[i]) {
        buttonState[i] = reading;
      }
    }
    
    lastButtonState[i] = reading;
  }
}

bool isButtonPressed(int pin, int buttonIndex) {
  if (buttonState[buttonIndex]) {
    // Yarım saniye geçmişse buton basımına izin ver
    if (millis() - lastButtonPress[buttonIndex] > 500) {
      buttonState[buttonIndex] = false;
      lastButtonPress[buttonIndex] = millis();
      return true;
    }
  }
  return false;
}

void readRFID() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  const char* waitText;
  const char* cancelText;
  
  switch (currentLanguage) {
    case LANG_DE: 
      waitText = "Warte auf Karte..."; 
      cancelText = "OK zum Abbrechen";
      break;
    case LANG_TR: 
      waitText = "Kart bekleniyor..."; 
      cancelText = "Iptal: OK tusuna basin";
      break;
    default: 
      waitText = "Waiting for card..."; 
      cancelText = "Press OK to cancel";
      break;
  }
  
  display.println(waitText);
  display.println();
  display.println(cancelText);
  display.display();
  
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  bool success = false;
  bool cancelled = false;
  unsigned long startTime = millis();
  
  // Allow the user to cancel the reading process
  while (!success && !cancelled) {
    // Check for card
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
    
    // Check if user pressed cancel button
    handleButtons();
    if (isButtonPressed(OK_BTN, 2)) {
      cancelled = true;
      break;
    }
    
    // Show scanning animation
    if (millis() - startTime > 300) {
      static int dots = 0;
      display.fillRect(0, 0, 128, 8, SSD1306_BLACK);
      display.setCursor(0, 0);
      display.print(waitText);
      for (int i = 0; i < dots; i++) {
        display.print(".");
      }
      dots = (dots + 1) % 4;
      display.display();
      startTime = millis();
    }
    
    delay(50);
    yield(); // ESP8266 watchdog timer reset
  }
  
  if (cancelled) {
    inSubmenu = false;
    return;
  }
  
  if (success) {
    memset(currentUID, 0, UID_SIZE);
    
    // Güvenli UID string oluşturma - buffer overflow prevention
    int pos = 0;
    for (uint8_t i = 0; i < uidLength && pos < UID_SIZE - 3; i++) {
      sprintf(currentUID + pos, "%02X", uid[i]);
      pos += 2;
    }
    currentUID[pos] = '\0'; // Null terminator güvencesi
    
    // Attempt to identify card type for better user feedback
    String cardType = "Unknown";
    if (uidLength == 4) {
      cardType = "MIFARE Classic/1K";
    } else if (uidLength == 7) {
      cardType = "MIFARE Ultralight";
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Card detected!");
    display.println();
    display.print("UID: ");
    
    // Format UID with spaces for readability
    for (uint8_t i = 0; i < uidLength; i++) {
      char hex[3];
      sprintf(hex, "%02X", uid[i]);
      display.print(hex);
      if (i < uidLength - 1) display.print(" ");
    }
    
    display.println();
    display.print("Type: ");
    display.println(cardType);
    display.print("Length: ");
    display.print(uidLength);
    display.print(" bytes");
    display.display();
    
    delay(1000);
    
    // Show submenu for Read operation
    showSubmenu(getCurrentMenuByLanguage(2)); // 2 = Read submenu
    
    // Handle read submenu
    while (inSubmenu) {
      handleButtons();
      
      const char** submenu = getCurrentMenuByLanguage(2);
      int submenuSize = 0;
      while (submenu[submenuSize] != NULL) submenuSize++;
      
      if (isButtonPressed(RIGHT_BTN, 0)) {
        submenuPosition = (submenuPosition + 1) % submenuSize;
        showSubmenu(submenu);
      }
      
      if (isButtonPressed(LEFT_BTN, 1)) {
        submenuPosition = (submenuPosition - 1 + submenuSize) % submenuSize;
        showSubmenu(submenu);
      }
      
      if (isButtonPressed(OK_BTN, 2)) {
        if (submenuPosition == 0) { // Save
          // Show position selection
          int savePos = 0;
          bool selecting = true;
          
          while (selecting) {
            display.clearDisplay();
            display.setTextSize(1);
            display.setCursor(0, 0);
            display.print("Select position (1-");
            display.print(MAX_SAVED_UIDS);
            display.println(")");
            display.print("Position: ");
            display.print(savePos + 1);
            display.display();
            
            handleButtons();
            
            if (isButtonPressed(RIGHT_BTN, 0)) {
              savePos = (savePos + 1) % MAX_SAVED_UIDS;
            }
            
            if (isButtonPressed(LEFT_BTN, 1)) {
              savePos = (savePos - 1 + MAX_SAVED_UIDS) % MAX_SAVED_UIDS;
            }
            
            if (isButtonPressed(OK_BTN, 2)) {
              saveUID(currentUID, savePos);
              selecting = false;
              
              display.clearDisplay();
              display.setCursor(0, 0);
              display.print("UID saved at pos ");
              display.println(savePos + 1);
              display.display();
              delay(1000);
            }
            yield(); // ESP8266 watchdog reset
          }
          inSubmenu = false;
        } else if (submenuPosition == 1) { // Back
          inSubmenu = false;
        }
      }
      yield(); // ESP8266 watchdog reset
    }
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    const char* errorText;
    switch (currentLanguage) {
      case LANG_DE: errorText = "Fehler beim Lesen"; break;
      case LANG_TR: errorText = "Okuma hatasi"; break;
      default: errorText = "Error reading card"; break;
    }
    display.print(errorText);
    display.display();
    delay(1000);
    inSubmenu = false;
  }
}

void copyRFID(bool multiCopy) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  
  const char* sourceText;
  switch (currentLanguage) {
    case LANG_DE: sourceText = "Quellkarte lesen..."; break;
    case LANG_TR: sourceText = "Kaynak karti oku..."; break;
    default: sourceText = "Read source card..."; break;
  }
  
  display.print(sourceText);
  display.display();
  
  // Read source card
  uint8_t success;
  uint8_t sourceUid[7] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t sourceUidLength;
  
  // Wait for source card
  while (true) {
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, sourceUid, &sourceUidLength);
    if (success) break;
    
    // Check if OK button is pressed to cancel
    handleButtons();
    if (isButtonPressed(OK_BTN, 2)) {
      inSubmenu = false;
      return;
    }
    delay(100);
  }
  
  // Display source UID
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Source UID: ");
  for (uint8_t i = 0; i < sourceUidLength; i++) {
    char hex[3];
    sprintf(hex, "%02X", sourceUid[i]);
    display.print(hex);
  }
  display.display();
  delay(1000);
  
  // Start copying process
  bool continueCopying = true;
  int copyCount = 0;
  
  while (continueCopying) {
    display.clearDisplay();
    display.setCursor(0, 0);
    
    const char* targetText;
    switch (currentLanguage) {
      case LANG_DE: targetText = "Zielkarte auflegen..."; break;
      case LANG_TR: targetText = "Hedef karti okut..."; break;
      default: targetText = "Place target card..."; break;
    }
    
    display.print(targetText);
    display.display();
    
    // Wait for target card
    uint8_t targetUid[7] = { 0, 0, 0, 0, 0, 0, 0 };
    uint8_t targetUidLength;
    
    // Reset success flag
    success = false;
    
    while (!success) {
      success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, targetUid, &targetUidLength);
      
      // Check if OK button is pressed to cancel
      handleButtons();
      if (isButtonPressed(OK_BTN, 2)) {
        continueCopying = false;
        break;
      }
      delay(100);
      yield(); // ESP8266 watchdog reset
    }
    
    if (continueCopying && success) {
      // Gerçek UID kopyalama işlemi
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Checking target card...");
      display.display();
      delay(500);
      
      bool canWriteToTarget = false;
      
      // Hedef kartın yazılabilir olup olmadığını kontrol et
      if (targetUidLength == 4 && sourceUidLength == 4) {
        uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        uint8_t keyB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        
        // Block 0'a erişim kontrolü
        if (nfc.mifareclassic_AuthenticateBlock(targetUid, targetUidLength, 0, 1, keyA) ||
            nfc.mifareclassic_AuthenticateBlock(targetUid, targetUidLength, 0, 1, keyB)) {
          canWriteToTarget = true;
        }
      }
      
      if (canWriteToTarget) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Copying UID...");
        display.display();
        
        // Block 0 verisini hazırla
        uint8_t newBlock0[16];
        
        // Kaynak UID'yi kopyala
        for (int i = 0; i < 4; i++) {
          newBlock0[i] = sourceUid[i];
        }
        
        // BCC hesapla
        newBlock0[4] = sourceUid[0] ^ sourceUid[1] ^ sourceUid[2] ^ sourceUid[3];
        
        // SAK ve ATQA değerlerini koru
        newBlock0[5] = 0x08; // SAK
        newBlock0[6] = 0x04; // ATQA LSB
        newBlock0[7] = 0x00; // ATQA MSB
        
        // Geri kalan byteları sıfırla
        for (int i = 8; i < 16; i++) {
          newBlock0[i] = 0x00;
        }
        
        // UID'yi yaz
        bool writeSuccess = nfc.mifareclassic_WriteDataBlock(0, newBlock0);
        
        display.clearDisplay();
        display.setCursor(0, 0);
        
        if (writeSuccess) {
          display.println("UID kopyalandi!");
          copyCount++;
          display.print("Kopyalar: ");
          display.println(copyCount);
          display.println();
          display.print("Yeni UID: ");
          for (int i = 0; i < 4; i++) {
            char buf[3];
            sprintf(buf, "%02X", sourceUid[i]);
            display.print(buf);
            if (i < 3) display.print(" ");
          }
        } else {
          display.println("Kopyalama hatasi!");
          display.println();
          display.println("Magic card gerekli");
        }
      } else {
        display.println("Hedef kart");
        display.println("yazilabilir degil!");
        display.println();
        display.println("Magic MIFARE card");
        display.println("gereklidir");
      }
      
      display.display();
      delay(1500); // Sonucu göster - watchdog safe
      
      if (!multiCopy) {
        continueCopying = false;
      } else {
        // Çoklu kopyalamada OK tuşu ile ana menüye dönme bilgilendirmesi
        display.println();
        
        const char* mainMenuText;
        switch (currentLanguage) {
          case LANG_DE: mainMenuText = "OK: Hauptmenu"; break;
          case LANG_TR: mainMenuText = "OK: Ana Menu"; break;
          default: mainMenuText = "OK: Main Menu"; break;
        }
        
        display.print(mainMenuText);
        display.display();
        
        // OK tuşuna basılırsa ana menüye dön
        unsigned long startTime = millis();
        bool exitToMain = false;
        while (millis() - startTime < 3000 && !exitToMain) { // 3 saniye bekle
          handleButtons();
          if (isButtonPressed(OK_BTN, 2)) {
            continueCopying = false;
            exitToMain = true;
          }
          delay(50);
        }
      }
    }
    yield(); // ESP8266 watchdog reset
  }
  
  inSubmenu = false;
}

void createNewUID(bool is4Byte) {
  int uidLength = is4Byte ? 8 : 14; // 4 bytes = 8 hex chars, 7 bytes = 14 hex chars
  char newUID[16] = ""; // 14 chars + null terminator
  int cursorPosition = 0;
  
  while (true) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    
    const char* titleText;
    switch (currentLanguage) {
      case LANG_DE: titleText = is4Byte ? "Neue 4-Byte UID:" : "Neue 7-Byte UID:"; break;
      case LANG_TR: titleText = is4Byte ? "Yeni 4-Byte UID:" : "Yeni 7-Byte UID:"; break;
      default: titleText = is4Byte ? "New 4-Byte UID:" : "New 7-Byte UID:"; break;
    }
    
    display.println(titleText);
    
    // Display current UID being built
    for (int i = 0; i < uidLength; i++) {
      if (i == cursorPosition) {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        display.setTextColor(SSD1306_WHITE);
      }
      
      display.print(i < strlen(newUID) ? newUID[i] : '0');
      
      if (i % 2 == 1) display.print(' ');
    }
    // Text rengini normale döndür
    display.setTextColor(SSD1306_WHITE);
    display.display();
    
    handleButtons();
    
    if (isButtonPressed(RIGHT_BTN, 0)) {
      // Increment current hex digit
      if (cursorPosition < strlen(newUID)) {
        int currentIndex = 0;
        for (int i = 0; i < 16; i++) {
          if (toupper(newUID[cursorPosition]) == hexChars[i]) {
            currentIndex = i;
            break;
          }
        }
        
        currentIndex = (currentIndex + 1) % 16;
        newUID[cursorPosition] = hexChars[currentIndex];
      } else {
        newUID[cursorPosition] = '0';
        newUID[cursorPosition + 1] = '\0';
      }
    }
    
    if (isButtonPressed(LEFT_BTN, 1)) {
      // Decrement current hex digit
      if (cursorPosition < strlen(newUID)) {
        int currentIndex = 0;
        for (int i = 0; i < 16; i++) {
          if (toupper(newUID[cursorPosition]) == hexChars[i]) {
            currentIndex = i;
            break;
          }
        }
        
        currentIndex = (currentIndex - 1 + 16) % 16;
        newUID[cursorPosition] = hexChars[currentIndex];
      } else {
        newUID[cursorPosition] = 'F';
        newUID[cursorPosition + 1] = '\0';
      }
    }
    
    if (isButtonPressed(OK_BTN, 2)) {
      cursorPosition++;
      
      if (cursorPosition >= uidLength) {
        // UID is complete, write to card
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        
        const char* readyText;
        switch (currentLanguage) {
          case LANG_DE: readyText = "Karte auflegen..."; break;
          case LANG_TR: readyText = "Karti okutun..."; break;
          default: readyText = "Place card to write..."; break;
        }
        
        display.println(readyText);
        display.display();
        
        // Wait for card
        uint8_t card_uid[7];
        uint8_t card_uidLength;
        bool cardDetected = false;
        
        while (!cardDetected) {
          cardDetected = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, card_uid, &card_uidLength);
          
          handleButtons();
          if (isButtonPressed(OK_BTN, 2)) {
            inSubmenu = false;
            return;
          }
          delay(100);
          yield(); // ESP8266 watchdog reset
        }
        
        // Convert hex string to byte array - büyük harfe çevir
        uint8_t newUIDBytes[7];
        for (int i = 0; i < (is4Byte ? 4 : 7); i++) {
          char hexPair[3] = {toupper(newUID[i*2]), toupper(newUID[i*2+1]), 0};
          newUIDBytes[i] = strtol(hexPair, NULL, 16);
        }
        
        display.clearDisplay();
        display.setCursor(0, 0);
        
        // Gerçek UID yazma işlemi için magic card kontrolü
        bool canWrite = false;
        bool isWritableCard = false;
        
        display.println("Checking card type...");
        display.display();
        delay(500);
        
        // MIFARE Classic kartları için UID yazma denemesi
        if (card_uidLength == 4 && is4Byte) {
          // Magic card kontrolü - genellikle block 0'a yazabilir olup olmadığını test ederiz
          // Ama ESP12S+PN532 kombinasyonu için sınırlı yazma kabiliyeti var
          
          uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Default key
          uint8_t keyB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Default key
          
          // Block 0'a erişim denemesi (UID bloku)
          if (nfc.mifareclassic_AuthenticateBlock(card_uid, card_uidLength, 0, 1, keyA)) {
            isWritableCard = true;
            canWrite = true;
          } else if (nfc.mifareclassic_AuthenticateBlock(card_uid, card_uidLength, 0, 1, keyB)) {
            isWritableCard = true;
            canWrite = true;
          }
        }
        
        if (canWrite && isWritableCard) {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Writing new UID...");
          display.display();
          
          // Block 0 verisini hazırla (UID + BCC + SAK + ATQA)
          uint8_t block0[16];
          
          // Yeni UID'yi kopyala
          for (int i = 0; i < 4; i++) {
            block0[i] = newUIDBytes[i];
          }
          
          // BCC hesapla (XOR of first 4 bytes)
          block0[4] = block0[0] ^ block0[1] ^ block0[2] ^ block0[3];
          
          // SAK ve ATQA değerlerini varsayılan olarak ayarla
          block0[5] = 0x08; // SAK (MIFARE Classic 1K)
          block0[6] = 0x04; // ATQA LSB
          block0[7] = 0x00; // ATQA MSB
          
          // Geri kalan byteları sıfırla
          for (int i = 8; i < 16; i++) {
            block0[i] = 0x00;
          }
          
          // Block 0'a yaz (sadece magic cardlarda mümkün)
          bool writeSuccess = false;
          
          // Magic card komutları kullanarak yazma denemesi
          // Not: Bu özel magic card komutları gerektirir
          if (nfc.mifareclassic_WriteDataBlock(0, block0)) {
            writeSuccess = true;
          }
          
          display.clearDisplay();
          display.setCursor(0, 0);
          
          if (writeSuccess) {
            display.println("UID yazildi!");
            display.println();
            display.print("Yeni UID: ");
            
            // Yazılan UID'yi daha okunabilir formatta göster
            for (int i = 0; i < 4; i++) {
              char buf[3];
              sprintf(buf, "%02X", newUIDBytes[i]);
              display.print(buf);
              if (i < 3) display.print(" ");
            }
            display.println();
            display.println();
            display.println("Basarili!");
          } else {
            display.println("Yazma hatasi!");
            display.println();
            display.println("Magic card gerekli");
            display.println("veya kart korumali");
          }
        } else {
          display.clearDisplay();
          display.setCursor(0, 0);
          display.println("Hata: Kart tipi");
          display.println("desteklenmiyor");
          display.println();
          display.println("Gereksinimler:");
          display.println("- Magic MIFARE card");
          display.println("- Yazilabilir UID bloku");
          display.println();
          display.println("Normal kartlarda");
          display.println("UID yazma mumkun degil");
        }
        
        display.display();
        delay(1500); // Sonucu göstermek için bekleme - watchdog safe
        
        inSubmenu = false;
        return;
      }
    }
    yield(); // ESP8266 watchdog reset
  }
}

void handleSavedUID(int position) {
  // Boş slot kontrolü - string uzunluğu 0 ise boş
  if (strlen(savedUIDs[position]) == 0) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("No RF at position ");
    display.println(position + 1);
    display.display();
    delay(1000);
    inSubmenu = false;
    return;
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("UID: ");
  display.println(savedUIDs[position]);
  display.display();
  delay(1000);
  
  showSubmenu(getCurrentMenuByLanguage(5)); // 5 = Saved submenu
  
  // Handle saved submenu
  while (inSubmenu) {
    handleButtons();
    
    const char** submenu = getCurrentMenuByLanguage(5);
    int submenuSize = 0;
    while (submenu[submenuSize] != NULL) submenuSize++;
    
    if (isButtonPressed(RIGHT_BTN, 0)) {
      submenuPosition = (submenuPosition + 1) % submenuSize;
      showSubmenu(submenu);
    }
    
    if (isButtonPressed(LEFT_BTN, 1)) {
      submenuPosition = (submenuPosition - 1 + submenuSize) % submenuSize;
      showSubmenu(submenu);
    }
    
    if (isButtonPressed(OK_BTN, 2)) {
      switch (submenuPosition) {
        case 0: { // Copy - değişken tanımı için süslü parantez içine alındı
          strcpy(currentUID, savedUIDs[position]);
          showSubmenu(getCurrentMenuByLanguage(3)); // 3 = Copy submenu
          
          // Handle copy submenu
          inSubSubmenu = true;
          int tempSubmenuPosition = 0; // Geçici alt menü konumu
          while (inSubSubmenu) {
            handleButtons();
            
            const char** copySubmenu = getCurrentMenuByLanguage(3);
            int copySubmenuSize = 0;
            while (copySubmenu[copySubmenuSize] != NULL) copySubmenuSize++;
            
            if (isButtonPressed(RIGHT_BTN, 0)) {
              tempSubmenuPosition = (tempSubmenuPosition + 1) % copySubmenuSize;
              showSubmenu(copySubmenu);
            }
            
            if (isButtonPressed(LEFT_BTN, 1)) {
              tempSubmenuPosition = (tempSubmenuPosition - 1 + copySubmenuSize) % copySubmenuSize;
              showSubmenu(copySubmenu);
            }
            
            if (isButtonPressed(OK_BTN, 2)) {
              if (tempSubmenuPosition == 0) { // Single Copy
                copyRFID(false);
              } else if (tempSubmenuPosition == 1) { // Multi Copy
                copyRFID(true);
              }
              
              inSubSubmenu = false;
            }
          }
          break;
        }
        case 1: // Delete
          deleteUID(position);
          inSubmenu = false;
          break;
        case 2: // Back
          inSubmenu = false;
          break;
      }
    }
  }
}

void showLanguageMenu() {
  bool inLanguageMenu = true;
  int languagePosition = 0;
  
  while (inLanguageMenu) {
    display.clearDisplay();
    
    const char** menu = getCurrentMenuByLanguage(6); // 6 = Language menu
    
    int menuSize = 0;
    while (menu[menuSize] != NULL) menuSize++;
    
    // Ana başlık - otomatik boyut ayarı (uzun başlıklar için küçültme)
    String currentTitle = String(menu[languagePosition]);
    int textSize = 2;
    int titleWidth = currentTitle.length() * 12; // 2x boyut için 12 pixel per char
    
    // Uzun başlıklar için boyutu küçült
    if (titleWidth > 120) { // 128-8 margin
      textSize = 1;
      titleWidth = currentTitle.length() * 6; // 1x boyut için 6 pixel per char
    }
    
    display.setTextSize(textSize);
    int titleX = (128 - titleWidth) / 2; // Ekranın ortasında
    display.setCursor(titleX, 10);
    display.setTextColor(SSD1306_WHITE);
    display.print(currentTitle);
    
    // Ok işaretleri (başlığın altında, sol ve sağda)
    display.setTextSize(1);
    if (menuSize > 1) {
      display.setCursor(10, 30); // Sol tarafta < işareti
      display.print("<");
      display.setCursor(118, 30); // Sağ tarafta > işareti
      display.print(">");
    }
    
    // Önceki ve sonraki menü öğeleri (ok işaretlerinin altında)
    if (menuSize > 1) {
      int prevIndex = (languagePosition - 1 + menuSize) % menuSize;
      int nextIndex = (languagePosition + 1) % menuSize;
      
      // Önceki öğe (sol tarafta, küçük)
      String prevTitle = String(menu[prevIndex]);
      if (prevTitle.length() > 10) prevTitle = prevTitle.substring(0, 10) + "..";
      display.setCursor(5, 45);
      display.print(prevTitle);
      
      // Sonraki öğe (sağ tarafta, küçük)
      String nextTitle = String(menu[nextIndex]);
      if (nextTitle.length() > 10) nextTitle = nextTitle.substring(0, 10) + "..";
      int nextWidth = nextTitle.length() * 6;
      int nextX = 128 - nextWidth - 5;
      display.setCursor(nextX, 45);
      display.print(nextTitle);
    }
    
    display.display();
    
    handleButtons();
    
    if (isButtonPressed(RIGHT_BTN, 0)) {
      languagePosition = (languagePosition + 1) % menuSize;
    }
    
    if (isButtonPressed(LEFT_BTN, 1)) {
      languagePosition = (languagePosition - 1 + menuSize) % menuSize;
    }
    
    if (isButtonPressed(OK_BTN, 2)) {
      if (languagePosition == 0) { // English
        setLanguage(LANG_EN);
      } else if (languagePosition == 1) { // Deutsch
        setLanguage(LANG_DE);
      } else if (languagePosition == 2) { // Turkce
        setLanguage(LANG_TR);
      } else if (languagePosition == 3) { // Back
        // Do nothing, just exit
      }
      inLanguageMenu = false;
    }
    
    delay(50);
    yield(); // ESP8266 watchdog reset
  }
}

void showInfoScreen() {
  display.clearDisplay();
  display.setTextSize(1); // Tüm satırlar için size 1
  
  // SmartKraft - Size 1, ortalanmış
  String smartkraft = "SmartKraft";
  int smartkraftWidth = smartkraft.length() * 6; // 1x boyut için 6 pixel per char
  int smartkraftX = (128 - smartkraftWidth) / 2;
  display.setCursor(smartkraftX, 15);
  display.print(smartkraft);
  
  // ShifterKey V1.1 - Size 1, ortalanmış
  String shifterkey = "ShifterKey V1.1";
  int shifterkeyWidth = shifterkey.length() * 6; // 1x boyut için 6 pixel per char
  int shifterkeyX = (128 - shifterkeyWidth) / 2;
  display.setCursor(shifterkeyX, 30);
  display.print(shifterkey);
  
  // July-2025 - Size 1, ortalanmış
  String date = "July-2025";
  int dateWidth = date.length() * 6; // 1x boyut için 6 pixel per char
  int dateX = (128 - dateWidth) / 2;
  display.setCursor(dateX, 45);
  display.print(date);
  
  display.display();
  
  // Wait for user to press OK to return
  while (!isButtonPressed(OK_BTN, 2)) {
    handleButtons();
    delay(100);
  }
}

void handleSettingsMenu() {
  const char** menu = getCurrentMenuByLanguage(1);
  int menuSize = 0;
  while (menu[menuSize] != NULL) menuSize++;
  
  if (isButtonPressed(RIGHT_BTN, 0)) {
    submenuPosition = (submenuPosition + 1) % menuSize;
  }
  
  if (isButtonPressed(LEFT_BTN, 1)) {
    submenuPosition = (submenuPosition - 1 + menuSize) % menuSize;
  }
  
  if (isButtonPressed(OK_BTN, 2)) {
    switch (submenuPosition) {
      case 0: // Language
        showLanguageMenu();
        break;
      case 1: // Info
        showInfoScreen();
        break;
      case 2: // Back to Menu
        inSubmenu = false;
        break;
    }
  }
}

void setLanguage(uint8_t lang) {
  currentLanguage = lang;
  saveSettings();
}

const char** getCurrentMenuByLanguage(int menuType) {
  switch (menuType) {
    case 0:
      switch (currentLanguage) {
        case LANG_DE: return mainMenuDE;
        case LANG_TR: return mainMenuTR;
        default: return mainMenuEN;
      }
    case 1:
      switch (currentLanguage) {
        case LANG_DE: return settingsMenuDE;
        case LANG_TR: return settingsMenuTR;
        default: return settingsMenuEN;
      }
    case 2:
      switch (currentLanguage) {
        case LANG_DE: return readSubmenuDE;
        case LANG_TR: return readSubmenuTR;
        default: return readSubmenuEN;
      }
    case 3:
      switch (currentLanguage) {
        case LANG_DE: return copySubmenuDE;
        case LANG_TR: return copySubmenuTR;
        default: return copySubmenuEN;
      }
    case 4:
      switch (currentLanguage) {
        case LANG_DE: return newUidSubmenuDE;
        case LANG_TR: return newUidSubmenuTR;
        default: return newUidSubmenuEN;
      }
    case 5:
      switch (currentLanguage) {
        case LANG_DE: return savedSubmenuDE;
        case LANG_TR: return savedSubmenuTR;
        default: return savedSubmenuEN;
      }
    case 6: // Language menu için yeni indeks
      switch (currentLanguage) {
        case LANG_DE: return languageMenuDE;
        case LANG_TR: return languageMenuTR;
        default: return languageMenuEN;
      }
    default:
      return mainMenuEN;
  }
}

// Copy menüsünü işleyen fonksiyon
void handleCopyMenu() {
  showSubmenu(getCurrentMenuByLanguage(3)); // 3 = Copy submenu
  
  while (inSubmenu) {
    handleButtons();
    
    const char** submenu = getCurrentMenuByLanguage(3);
    int submenuSize = 0;
    while (submenu[submenuSize] != NULL) submenuSize++;
    
    if (isButtonPressed(RIGHT_BTN, 0)) {
      submenuPosition = (submenuPosition + 1) % submenuSize;
      showSubmenu(submenu);
    }
    
    if (isButtonPressed(LEFT_BTN, 1)) {
      submenuPosition = (submenuPosition - 1 + submenuSize) % submenuSize;
      showSubmenu(submenu);
    }
    
    if (isButtonPressed(OK_BTN, 2)) {
      if (submenuPosition == 0) { // Single copy
        copyRFID(false);
      } else if (submenuPosition == 1) { // Multi copy
        copyRFID(true);
      } else if (submenuPosition == 2) { // Back
        // Do nothing, just exit
      }
      inSubmenu = false;
    }
  }
}

// New UID menüsünü işleyen fonksiyon
void handleNewUIDMenu() {
  showSubmenu(getCurrentMenuByLanguage(4)); // 4 = New UID submenu
  
  while (inSubmenu) {
    handleButtons();
    
    const char** submenu = getCurrentMenuByLanguage(4);
    int submenuSize = 0;
    while (submenu[submenuSize] != NULL) submenuSize++;
    
    if (isButtonPressed(RIGHT_BTN, 0)) {
      submenuPosition = (submenuPosition + 1) % submenuSize;
      showSubmenu(submenu);
    }
    
    if (isButtonPressed(LEFT_BTN, 1)) {
      submenuPosition = (submenuPosition - 1 + submenuSize) % submenuSize;
      showSubmenu(submenu);
    }
    
    if (isButtonPressed(OK_BTN, 2)) {
      if (submenuPosition == 0) { // 4 Byte
        createNewUID(true);
      } else if (submenuPosition == 1) { // 7 Byte
        createNewUID(false);
      } else if (submenuPosition == 2) { // Back
        // Do nothing, just exit
      }
      inSubmenu = false;
    }
  }
}

// Settings menüsü için standalone handler
void handleSettingsDirectly() {
  bool inSettings = true;
  int settingsPosition = 0;
  
  while (inSettings) {
    // Settings menüsünü göster
    const char** settingsMenu = getCurrentMenuByLanguage(1);
    int menuSize = 0;
    while (settingsMenu[menuSize] != NULL) menuSize++;
    
    display.clearDisplay();
    
    // Ana başlık - otomatik boyut ayarı (uzun başlıklar için küçültme)
    String currentTitle = String(settingsMenu[settingsPosition]);
    int textSize = 2;
    int titleWidth = currentTitle.length() * 12; // 2x boyut için 12 pixel per char
    
    // Uzun başlıklar için boyutu küçült
    if (titleWidth > 120) { // 128-8 margin
      textSize = 1;
      titleWidth = currentTitle.length() * 6; // 1x boyut için 6 pixel per char
    }
    
    display.setTextSize(textSize);
    int titleX = (128 - titleWidth) / 2; // Ekranın ortasında
    display.setCursor(titleX, 10);
    display.setTextColor(SSD1306_WHITE);
    display.print(currentTitle);
    
    // Ok işaretleri (başlığın altında, sol ve sağda)
    display.setTextSize(1);
    if (menuSize > 1) {
      display.setCursor(10, 30); // Sol tarafta < işareti
      display.print("<");
      display.setCursor(118, 30); // Sağ tarafta > işareti
      display.print(">");
    }
    
    // Önceki ve sonraki menü öğeleri (ok işaretlerinin altında)
    if (menuSize > 1) {
      int prevIndex = (settingsPosition - 1 + menuSize) % menuSize;
      int nextIndex = (settingsPosition + 1) % menuSize;
      
      // Önceki öğe (sol tarafta, küçük)
      String prevTitle = String(settingsMenu[prevIndex]);
      if (prevTitle.length() > 10) prevTitle = prevTitle.substring(0, 10) + "..";
      display.setCursor(5, 45);
      display.print(prevTitle);
      
      // Sonraki öğe (sağ tarafta, küçük)
      String nextTitle = String(settingsMenu[nextIndex]);
      if (nextTitle.length() > 10) nextTitle = nextTitle.substring(0, 10) + "..";
      int nextWidth = nextTitle.length() * 6;
      int nextX = 128 - nextWidth - 5;
      display.setCursor(nextX, 45);
      display.print(nextTitle);
    }
    
    display.display();
    
    handleButtons();
    
    if (isButtonPressed(RIGHT_BTN, 0)) {
      settingsPosition = (settingsPosition + 1) % menuSize;
    }
    
    if (isButtonPressed(LEFT_BTN, 1)) {
      settingsPosition = (settingsPosition - 1 + menuSize) % menuSize;
    }
    
    if (isButtonPressed(OK_BTN, 2)) {
      switch (settingsPosition) {
        case 0: // Language
          submenuPosition = 0; // Reset language selection position
          showLanguageMenu();
          break;
        case 1: // Info
          showInfoScreen();
          break;
        case 2: // Back to Menu
          inSettings = false;
          break;
      }
    }
    
    delay(50);
    yield(); // ESP8266 watchdog reset
  }
}

