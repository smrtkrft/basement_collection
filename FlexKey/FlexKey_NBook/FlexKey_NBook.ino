/*
 * ESP32-C6 + PN532 RFID Reader
 * I2C Bağlantısı ile RFID okuma
 * 
 * Bağlantı Şeması:
 * ================
 * PN532    ->  ESP32-C6
 * -------------------- 
 * VCC      ->  3.3V
 * GND      ->  GND
 * SDA      ->  D4 (GPIO22)
 * SCL      ->  D5 (GPIO23)
 * 
 * PN532 Modül Ayarı:
 * - CH1: ON  (I2C modu için)
 * - CH2: OFF (I2C modu için)
 */

#include <Wire.h>
#include <Adafruit_PN532.h>

// ESP32-C6 I2C pinleri (D4=GPIO22, D5=GPIO23)
#define SDA_PIN 22  // D4
#define SCL_PIN 23  // D5

// PN532 I2C objesi oluştur (SDA ve SCL pinlerini constructor'a ver)
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

// Yeni UID değerleri
uint8_t newUid[7] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
uint8_t newUidLength = 0;
bool uidReady = false;

// UID değiştirme fonksiyonu (Chinese Magic Card / CUID için)
bool changeUid(uint8_t* newUid, uint8_t uidLength) {
  Serial.println("\n⚠️  UID değiştirme başlatılıyor...");
  Serial.println("⚠️  Bu sadece UID değiştirilebilir kartlarda çalışır!");
  Serial.println("    (Chinese Magic Card, CUID, Gen2, vb.)");
  
  // Block 0'ı oku
  uint8_t block0[16];
  uint8_t success = nfc.mifareclassic_ReadDataBlock(0, block0);
  
  if (!success) {
    Serial.println("❌ Block 0 okunamadı!");
    return false;
  }
  
  Serial.println("\nMevcut Block 0:");
  for (int i = 0; i < 16; i++) {
    if (block0[i] < 0x10) Serial.print("0");
    Serial.print(block0[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Yeni UID'i Block 0'a kopyala
  for (int i = 0; i < uidLength && i < 4; i++) {
    block0[i] = newUid[i];
  }
  
  // BCC hesapla (Block 0, byte 4)
  block0[4] = block0[0] ^ block0[1] ^ block0[2] ^ block0[3];
  
  Serial.println("\nYeni Block 0:");
  for (int i = 0; i < 16; i++) {
    if (block0[i] < 0x10) Serial.print("0");
    Serial.print(block0[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  
  // Block 0'a yaz (Magic Card özel komut)
  success = nfc.mifareclassic_WriteDataBlock(0, block0);
  
  if (success) {
    Serial.println("✓ UID başarıyla değiştirildi!");
    return true;
  } else {
    Serial.println("❌ UID değiştirilemedi!");
    Serial.println("   Bu kart muhtemelen normal bir kart (UID değişmez)");
    return false;
  }
}

void setup(void) {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("PN532 RFID Okuyucu Başlatılıyor...");
  Serial.println("========================================");
  
  // I2C başlat
  Serial.print("I2C başlatılıyor: SDA=");
  Serial.print(SDA_PIN);
  Serial.print(" (D4), SCL=");
  Serial.print(SCL_PIN);
  Serial.println(" (D5)");
  
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(100000); // 100kHz I2C hızı
  
  // I2C bus taraması yap
  Serial.println("\nI2C bus taranıyor...");
  byte error, address;
  int nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C cihaz bulundu: 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println("❌ Hiç I2C cihaz bulunamadı!");
    Serial.println("Bağlantıları kontrol edin!");
  } else {
    Serial.print("✓ Toplam ");
    Serial.print(nDevices);
    Serial.println(" I2C cihaz bulundu\n");
  }
  
  delay(500);
  
  Serial.println("PN532 modülü aranıyor...");
  nfc.begin();
  delay(1000);

  // Birkaç kez deneme yap
  uint32_t versiondata = 0;
  for (int i = 0; i < 3; i++) {
    versiondata = nfc.getFirmwareVersion();
    if (versiondata) break;
    Serial.print("Deneme ");
    Serial.print(i + 1);
    Serial.println("...");
    delay(500);
  }
  
  if (!versiondata) {
    Serial.println("\n❌ HATA: PN532 bulunamadı!");
    Serial.println("\nKontrol Listesi:");
    Serial.println("1. Bağlantıları kontrol edin:");
    Serial.println("   - VCC -> 3.3V");
    Serial.println("   - GND -> GND");
    Serial.println("   - SDA -> D4 (GPIO22)");
    Serial.println("   - SCL -> D5 (GPIO23)");
    Serial.println("2. PN532 modülü I2C modunda mı?");
    Serial.println("   - CH1: ON");
    Serial.println("   - CH2: OFF");
    Serial.println("3. Modülün güç LED'i yanıyor mu?");
    Serial.println("4. Kabloları çıkarıp tekrar takın");
    while (1) {
      delay(1000);
    }
  }
  
  // PN532 bilgilerini yazdır
  Serial.println("\n✓ PN532 Modülü Bulundu!");
  Serial.print("Firmware Versiyonu: v");
  Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // SAM konfigürasyonu
  Serial.println("SAM konfigürasyonu yapılıyor...");
  nfc.SAMConfig();
  delay(100);
  
  Serial.println("\n✓ Hazır! RFID kartınızı okutun...");
  Serial.println("\nKomutlar:");
  Serial.println("  'r' - Kart okuma modu");
  Serial.println("  'w' - UID değiştirme modu");
  Serial.println("  'u:XX XX XX XX' - Yeni UID gir (HEX format)");
  Serial.println("\nÖrnek: u:DE AD BE EF");
  Serial.println("========================================\n");
}

char mode = 'r'; // Varsayılan: okuma modu
String serialBuffer = "";

// Serial'den UID parse etme fonksiyonu
void parseUidFromSerial(String input) {
  input.trim();
  input.toUpperCase();
  
  // "u:" ile başlamalı
  if (!input.startsWith("U:")) {
    Serial.println("❌ Hatalı format! Örnek: u:DE AD BE EF");
    return;
  }
  
  // "u:" kısmını çıkar
  input = input.substring(2);
  input.trim();
  
  // Boşluklara göre ayır
  int byteCount = 0;
  int startIdx = 0;
  
  for (int i = 0; i <= input.length(); i++) {
    if (i == input.length() || input.charAt(i) == ' ') {
      if (i > startIdx) {
        String byteStr = input.substring(startIdx, i);
        byteStr.trim();
        
        if (byteStr.length() > 0) {
          // HEX'i byte'a çevir
          long val = strtol(byteStr.c_str(), NULL, 16);
          if (val >= 0 && val <= 255) {
            newUid[byteCount] = (uint8_t)val;
            byteCount++;
            if (byteCount >= 7) break; // Maksimum 7 byte
          } else {
            Serial.println("❌ Geçersiz HEX değer: " + byteStr);
            return;
          }
        }
      }
      startIdx = i + 1;
    }
  }
  
  if (byteCount < 4 || byteCount > 7) {
    Serial.println("❌ UID 4-7 byte arasında olmalı!");
    return;
  }
  
  newUidLength = byteCount;
  uidReady = true;
  
  Serial.println("\n✓ Yeni UID kaydedildi:");
  Serial.print("  ");
  for (int i = 0; i < newUidLength; i++) {
    if (newUid[i] < 0x10) Serial.print("0");
    Serial.print(newUid[i], HEX);
    if (i < newUidLength - 1) Serial.print(" ");
  }
  Serial.println();
  Serial.println("\n'w' tuşuna basıp kartı okutarak yazabilirsiniz.");
}

void loop(void) {
  // Serial komut kontrolü
  if (Serial.available() > 0) {
    char c = Serial.read();
    
    if (c == '\n' || c == '\r') {
      // Satır sonu - komutu işle
      if (serialBuffer.length() > 0) {
        serialBuffer.trim();
        
        if (serialBuffer.equalsIgnoreCase("r")) {
          mode = 'r';
          Serial.println("\n📖 Okuma moduna geçildi");
        } 
        else if (serialBuffer.equalsIgnoreCase("w")) {
          if (!uidReady) {
            Serial.println("\n❌ Önce UID girmelisiniz!");
            Serial.println("Örnek: u:DE AD BE EF");
          } else {
            mode = 'w';
            Serial.println("\n✏️  UID değiştirme moduna geçildi");
            Serial.print("Hedef UID: ");
            for (int i = 0; i < newUidLength; i++) {
              if (newUid[i] < 0x10) Serial.print("0");
              Serial.print(newUid[i], HEX);
              Serial.print(" ");
            }
            Serial.println("\nKartı okutun...");
          }
        }
        else if (serialBuffer.startsWith("u:") || serialBuffer.startsWith("U:")) {
          parseUidFromSerial(serialBuffer);
        }
        else {
          Serial.println("❌ Bilinmeyen komut: " + serialBuffer);
          Serial.println("Komutlar: r, w, u:XX XX XX XX");
        }
        
        serialBuffer = "";
      }
    } else {
      serialBuffer += c;
    }
  }
  
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  
  // RFID kartı oku (500ms timeout - daha uzun bekleme)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 500);
  
  if (success) {
    Serial.println("\n========================================");
    Serial.println("✓ RFID Kart Algılandı!");
    Serial.println("========================================");
    
    Serial.print("UID Uzunluğu: ");
    Serial.print(uidLength, DEC);
    Serial.print(" byte ");
    
    // Kart tipi bilgisi
    if (uidLength == 4) {
      Serial.println("(MIFARE Classic 1K/4K - Tek boyutlu UID)");
      Serial.println("⚠️  Bu kart 4 byte'lık bir karttır!");
      Serial.println("    7 byte UID YAZILAMAZ - donanımsal kısıt");
    } else if (uidLength == 7) {
      Serial.println("(MIFARE Classic - Çift boyutlu UID)");
      Serial.println("✓ Bu kart 7 byte UID destekler");
    } else if (uidLength == 10) {
      Serial.println("(MIFARE Classic - Üç boyutlu UID)");
    } else {
      Serial.println("(Bilinmeyen tip)");
    }
    
    // UID HEX formatı (boşluklu)
    Serial.print("UID (HEX): ");
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(uid[i], HEX);
      if (i < uidLength - 1) {
        Serial.print(" ");
      }
    }
    Serial.println();
    
    // UID HEX formatı (bitişik - kopyala yapıştır için)
    Serial.print("UID (HEX bitişik): ");
    for (uint8_t i = 0; i < uidLength; i++) {
      if (uid[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(uid[i], HEX);
    }
    Serial.println();
    
    // UID HEX formatı (0x prefix ile)
    Serial.print("UID (HEX 0x): ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print("0x");
      if (uid[i] < 0x10) {
        Serial.print("0");
      }
      Serial.print(uid[i], HEX);
      if (i < uidLength - 1) {
        Serial.print(" ");
      }
    }
    Serial.println();
    
    // UID Decimal formatı
    Serial.print("UID (DEC): ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(uid[i], DEC);
      if (i < uidLength - 1) {
        Serial.print("-");
      }
    }
    Serial.println();
    
    // UID Decimal tek sayı olarak
    Serial.print("UID (DEC tek sayı): ");
    unsigned long uidNumber = 0;
    for (uint8_t i = 0; i < uidLength && i < 4; i++) {
      uidNumber = (uidNumber << 8) | uid[i];
    }
    Serial.println(uidNumber);
    
    // Ham veriyi de göster (debug için)
    Serial.print("Ham UID Array (tüm 7 byte): ");
    for (int i = 0; i < 7; i++) {
      if (uid[i] < 0x10) Serial.print("0");
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    
    // Mod kontrolü
    if (mode == 'w') {
      // UID uzunluğu kontrolü
      if (newUidLength != uidLength) {
        Serial.println("\n⚠️  UYARI: UID uzunlukları uyuşmuyor!");
        Serial.print("   Kartın UID uzunluğu: ");
        Serial.print(uidLength);
        Serial.println(" byte");
        Serial.print("   Yazmak istediğiniz UID: ");
        Serial.print(newUidLength);
        Serial.println(" byte");
        Serial.println("\n❌ 4 byte karta sadece 4 byte UID yazılabilir!");
        Serial.println("❌ 7 byte karta sadece 7 byte UID yazılabilir!");
        Serial.println("\nUID uzunluğu değiştirilemez - bu donanımsal bir özelliktir.");
      } else {
        Serial.println("\n🔧 UID değiştirme işlemi başlatılıyor...");
        
        // Önce kimlik doğrulama (default key ile)
        uint8_t keyA[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 0, 0, keyA);
        
        if (success) {
          Serial.println("✓ Kimlik doğrulama başarılı");
          if (changeUid(newUid, newUidLength)) {
            Serial.println("\n🎉 İşlem tamamlandı!");
            Serial.println("Kartı çıkarıp tekrar okutarak kontrol edin.");
            mode = 'r'; // Otomatik okuma moduna geç
          }
        } else {
          Serial.println("❌ Kimlik doğrulama başarısız!");
          Serial.println("   Default key (FF FF FF FF FF FF) çalışmadı");
        }
      }
    }
    
    Serial.println("========================================\n");
    
    // Aynı kartın tekrar okunmaması için bekle
    delay(2000);
  } else {
    // Her 5 saniyede bir canlılık mesajı
    static unsigned long lastMsg = 0;
    if (millis() - lastMsg > 5000) {
      Serial.print("[");
      Serial.print(mode == 'r' ? "Okuma" : "Yazma");
      Serial.println(" modu - Kart bekleniyor...]");
      lastMsg = millis();
    }
  }
  
  delay(50);
}
