# FlexKey - Sistem Özet

**Versiyon:** v1.0.3  
**Son Güncelleme:** Separate Static IP + Smart AP Mode Control

## 📁 Oluşturulan Dosyalar:

**Ana Dosya:**
- ✅ `FlexKey.ino` - Ana program (v1.0.3: Akıllı AP mode kontrolü)

**Konfigürasyon:**
- ✅ `FlexKey_Config.h` - Tüm sabitler ve yapılar (v1.0.3: Her WiFi için ayrı statik IP + apModeEnabled)

**Modüller:**
- ✅ `FlexKey_Storage.cpp/h` - NVS veri yönetimi (v1.0.3: AP mode storage eklendi)
- ✅ `FlexKey_Button.cpp/h` - Buton ve factory reset
- ✅ `FlexKey_WiFi.cpp/h` - WiFi ve AP modu (v1.0.3: Akıllı WiFi mode seçimi)
- ✅ `FlexKey_RFID.cpp/h` - PN532 RFID okuyucu
- ✅ `FlexKey_Relay.cpp/h` - Röle kontrolü
- ✅ `FlexKey_HTTP.cpp/h` - HTTP GET istekleri
- ✅ `FlexKey_Web.cpp/h` - Web server ve arayüz (v1.0.3: AP mode checkbox + validasyon)

**Metadata:**
- ✅ `library.properties` - Kütüphane bilgileri
- ✅ `keywords.txt` - Arduino IDE syntax highlighting
- ✅ `README.md` - Detaylı kullanım kılavuzu
- ✅ `LIBRARIES.txt` - Gerekli kütüphaneler listesi

---

## 🔄 VERSİYON GEÇMİŞİ:

### v1.0.3 (Tamamlandı)
**Değişiklikler:**
- 🆕 Her WiFi ağı için AYRI statik IP ayarları
- 🆕 Primary WiFi: Kendi IP/Gateway/Subnet/DNS
- 🆕 Backup WiFi: Kendi IP/Gateway/Subnet/DNS
- 🆕 **AKILLI AP MODE KONTROLÜ**: WiFi SSID varsa kapatılabilir
- 🔧 WiFiConfig_t yapısı: apModeEnabled field geri eklendi
- 🔧 WiFi mode seçimi: AP / STA / AP+STA otomatik
- 🔧 Web UI: AP mode checkbox + validasyon
- 🔧 JavaScript: WiFi SSID kontrolü ile AP kapatma engelleme
- 🔧 Storage: AP mode ayarı kaydediliyor

**AP Mode Mantığı:**
```
WiFi SSID YOK + AP Kapalı → AP ZORLA AÇIK (güvenlik)
WiFi SSID YOK + AP Açık → AP Açık
WiFi SSID VAR + AP Kapalı → Sadece WiFi (AP kapalı)
WiFi SSID VAR + AP Açık → AP + WiFi (dual mode)
```

**Dosya Değişiklikleri:**
- `FlexKey_Config.h`: apModeEnabled field eklendi
- `FlexKey_WiFi.cpp`: WIFI_AP / WIFI_STA / WIFI_AP_STA akıllı seçim
- `FlexKey_Storage.cpp`: apModeEnabled kaydetme/yükleme
- `FlexKey_Web.cpp`: AP checkbox + SSID validasyonu
- `FlexKey.ino`: AP başlatma koşullu hale getirildi

**Sorun Çözümü:**
- ✅ Primary ve backup farklı subnet'te olabilir
- ✅ Statik IP düzgün çalışıyor
- ✅ AP mode kapatılabilir (WiFi varsa)
- ✅ WiFi yoksa AP otomatik açılıyor (güvenlik)

### v1.0.2 (Tamamlandı)
**Değişiklikler:**
- 🆕 Device ID artık Chip ID kullanıyor (MAC yerine)
- 🆕 Web UI DMF terminal stiline dönüştürüldü
- 🆕 Yeşil terminal teması (#0f0 on #000)
- 🆕 4-kolonlu status bar

### v1.0.1 (Tamamlandı)
**Bug Fixes:**
- 🐛 WiFi.h include hataları düzeltildi

### v1.0.0 (İlk Release)
**Özellikler:**
- ✅ Tüm modüller oluşturuldu
- ✅ Modüler mimari
- ✅ Web interface

---

## 🚀 SONRAKİ ADIMLAR:

### 1. **Kütüphaneleri Yükle:**
Arduino IDE Library Manager'dan şunları yükle:
- **Adafruit PN532** (latest)
- **ArduinoJson** (v7.x)

### 2. **Board Ayarları:**
```
Tools → Board → ESP32 Arduino → XIAO_ESP32C6
Tools → Port → (COM port seç)
```

### 3. **Derleme ve Yükleme:**
```
Sketch → Verify/Compile (Ctrl+R)
Sketch → Upload (Ctrl+U)
```

### 4. **İlk Çalıştırma:**
1. Seri monitor aç (115200 baud)
2. WiFi ağı ara: **FlexKey-XXXXXX**
3. Bağlan (şifre yok)
4. Tarayıcıda aç: **http://192.168.4.1**

---

## 🎯 SİSTEM ÖZELLİKLERİ:

### ✅ Tamamlanan Özellikler:
- ✅ 5 grup, 200 UID kapasitesi
- ✅ 4 veya 7 byte UID desteği
- ✅ Tekli/Çoklu grup modu
- ✅ UID çakışma uyarı sistemi
- ✅ **AKILLI AP MODE**: WiFi SSID varsa kapatılabilir, yoksa otomatik açık
- ✅ WiFi Mode: AP / STA / AP+STA (otomatik seçim)
- ✅ **Her WiFi ağı için AYRI statik IP ayarları**
- ✅ Primary WiFi: Kendi IP/Gateway/Subnet/DNS
- ✅ Backup WiFi: Kendi IP/Gateway/Subnet/DNS
- ✅ Farklı subnet'ler destekleniyor (örn: 192.168.1.x ve 192.168.2.x)
- ✅ Her grup için 15 URL
- ✅ HTTP/HTTPS destek
- ✅ Non-blocking GET istekleri
- ✅ Röle toggle/pulse modu
- ✅ Global röle (çoklu grup)
- ✅ **DMF-style terminal arayüz (yeşil/siyah tema)**
- ✅ **Chip ID bazlı cihaz kimliği (MAC yerine)**
- ✅ Son okutulan UID (1 dk)
- ✅ Factory reset (10sn buton)
- ✅ Restart (kısa basış)
- ✅ NVS veri saklama

### 🎨 Web Arayüz (DMF Terminal Style):
- ✅ **Header:** Büyük SMARTKRAFT FLEXKEY başlığı, Chip ID gösterimi
- ✅ **Status Bar:** 4 kolonlu (Son UID, Toplam UID, Sonraki Alarm, WiFi Durumu)
- ✅ **Tab 1:** ID & Grup ayarları
- ✅ **Tab 2:** URL & Relais ayarları
- ✅ **Tab 3:** Bağlantı ayarları (AP mode her zaman aktif uyarısı)
- ✅ **Tab 4:** Info & kullanım kılavuzu
- ✅ **Tema:** Yeşil (#0f0) metin, siyah (#000) arkaplan, glowing efektler

---

## ⚙️ PIN BAĞLANTILARI:

```
PN532 NFC:
  VCC → 3.3V
  GND → GND
  SDA → D4 (GPIO6)
  SCL → D5 (GPIO7)

Buton:
  → D1 (GPIO2) + GND

Röle:
  → D0 (GPIO3)
```

---

## 📖 KULLANIM ÖRNEĞİ:

1. **Sistem açılır** → AP mode: `FlexKey-XXXXXX` (Chip ID'nin son 6 karakteri)
2. **AP'ye bağlan** → Şifresiz WiFi ağı
3. **Web'e gir** → http://192.168.4.1 (AP her zaman aktif)
4. **WiFi ayarla** → Primary/Backup SSID (opsiyonel)
5. **Grup oluştur** → UID ekle (manuel veya okut)
6. **URL ekle** → Telegram/API/Local IP
7. **RFID oku** → URL'ler tetiklenir
8. **Röle çalışır** → Kapı açılır

**ÖNEMLİ:** WiFi ayarlarını yapsanız da yapmassanız da AP mode **HER ZAMAN AKTİF** kalır!

---

## 🔧 GELİŞTİRME NOTLARI:

### Ağ Davranışı (v1.0.2):
- ✅ WIFI_AP_STA modu kalıcı olarak aktif
- ✅ AP SSID: "FlexKey-" + Chip ID son 6 hex karakter
- ✅ AP IP: 192.168.4.1 (değiştirilemez)
- ✅ WiFi bağlanamazsa sadece AP modu çalışır
- ✅ Her iki modda da web interface erişilebilir

### Device ID (v1.0.2):
- ✅ ESP.getEfuseMac() kullanılıyor
- ✅ MAC address yerine Chip ID tercih edildi
- ✅ Bazı ESP32-C6 boardlarda MAC 00:00:00:00:00:00 sorunu giderildi

### Sistem Hata Yönetimi:
- ✅ HTTP hataları bloke etmez
- ✅ RFID bulunamazsa sistem çalışır
- ✅ WiFi bağlanamazsa AP moda geçer
- ✅ Tüm hatalar Serial'e loglanır

### Performans:
- Non-blocking HTTP
- Non-blocking RFID okuma (100ms timeout)
- Watchdog koruması (10ms loop delay)
- UID duplicate detection

---

## 💡 ÖNEMLİ NOKTLAR:

1. **PN532 I2C Modu:** Modülün I2C modda olduğundan emin ol
2. **ESP32-C6 Board:** En güncel ESP32 board support gerekli
3. **ArduinoJson v7:** Eski versiyonlar çalışmaz
4. **Web Tarayıcı:** Modern browser önerilir (Chrome/Edge/Firefox)
5. **Seri Monitor:** İlk kurulumda debug için açık tut
6. **AP Mode:** Asla kapatılamaz, bu tasarım gereği bir özelliktir

---

## 📊 SİSTEM MİMARİSİ:

### Modüler Yapı:
```
FlexKey.ino (Ana Program)
    ├── FlexKey_Config.h (Yapılar & Sabitler)
    ├── FlexKey_Storage (NVS Yönetimi)
    ├── FlexKey_Button (Restart/Factory Reset)
    ├── FlexKey_WiFi (Ağ Bağlantısı)
    ├── FlexKey_RFID (Kart Okuma)
    ├── FlexKey_Relay (Röle Kontrolü)
    ├── FlexKey_HTTP (URL Tetikleme)
    └── FlexKey_Web (Web Arayüz)
```

### Veri Akışı:
```
RFID Kart → UID Okuma → Grup Kontrolü → URL Tetikleme + Röle Aktivasyonu
                                      ↓
                                Web Arayüz ← → NVS Storage
```

---

## 🔄 GÜNCELLEME GEÇMİŞİ:

### v1.0.1 (Bug Fix)
- WiFi.h include eklendi (FlexKey_HTTP.cpp, FlexKey_Web.cpp)
- Derleme hataları düzeltildi
- WiFiClient ve WiFi class erişim sorunları çözüldü

### v1.0.0 (İlk Sürüm)
- Tüm temel özellikler tamamlandı
- Modüler yapı oluşturuldu
- Web arayüz terminal tasarımı
- NVS storage entegrasyonu
- Multi/single grup modu
- HTTP/HTTPS trigger sistemi

---

**Son Güncelleme:** 8 Ekim 2025  
**Durum:** Production Ready ✅  
**Versiyon:** 1.0.1
