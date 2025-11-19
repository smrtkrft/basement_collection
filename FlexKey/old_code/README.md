# FlexKey - RFID Access Control System

**Version:** 1.0.0  
**Hardware:** XIAO ESP32-C6, PN532 NFC/RFID Reader  
**Author:** SmartKraft  
**Website:** [smartkraft.ch/FlexKey](https://smartkraft.ch/FlexKey)

---

## 📋 Overview

FlexKey is a complete RFID-based access control system with web configuration interface. Designed for ESP32-C6 microcontrollers with PN532 NFC/RFID readers, it provides flexible group management, WiFi connectivity, and HTTP trigger capabilities.

---

## ✨ Features

- **Multi/Single Group RFID Management**
  - Support for up to 5 groups
  - Up to 200 total UIDs (4 or 7 byte)
  - Single or multi-group activation modes
  - UID conflict detection

- **Web-Based Configuration**
  - Terminal-style minimalist design
  - 4 main sections: ID & Groups, URLs & Relay, Connection, Info
  - Real-time UID display (1 minute)
  - Mobile responsive

- **WiFi Connectivity**
  - AP mode with auto-generated SSID
  - Primary and backup WiFi networks
  - Static IP configuration
  - Auto-reconnect functionality

- **HTTP Trigger System**
  - Up to 15 URLs per group
  - Non-blocking GET requests
  - HTTP/HTTPS support
  - Error handling without system blocking

- **Relay Control**
  - Toggle or pulse modes
  - Configurable pulse duration
  - Per-group or global settings
  - Manual test capability

- **Data Persistence**
  - NVS (Non-Volatile Storage)
  - Factory reset via button (10s hold)
  - Quick restart (short press)

---

## 🔧 Hardware Requirements

### Required Components
- **XIAO ESP32-C6** microcontroller
- **PN532** NFC/RFID reader module (I2C)
- **Push button** (for reset/factory reset)
- **Relay module** (optional, for access control)

### Pin Connections

| Component | ESP32-C6 Pin | GPIO |
|-----------|--------------|------|
| PN532 SDA | D4 | GPIO6 |
| PN532 SCL | D5 | GPIO7 |
| Button | D1 | GPIO2 |
| Relay | D0 | GPIO3 |
| Status LED (optional) | D10 | GPIO10 |

### Wiring Diagram
```
PN532 NFC Reader:
  VCC  → 3.3V
  GND  → GND
  SDA  → D4 (GPIO6)
  SCL  → D5 (GPIO7)

Button:
  One side → D1 (GPIO2)
  Other side → GND
  (Internal pullup enabled)

Relay Module:
  VCC  → 5V or 3.3V (check module specs)
  GND  → GND
  IN   → D0 (GPIO3)
```

---

## 📦 Installation

### Arduino IDE

1. **Install ESP32 Board Support:**
   - File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager
   - Search "esp32" and install "esp32 by Espressif Systems"

2. **Install Required Libraries:**
   - Sketch → Include Library → Manage Libraries
   - Install the following:
     - **Adafruit PN532** (latest version)
     - **ArduinoJson** (version 7.x)

3. **Clone or Download FlexKey:**
   ```bash
   git clone https://github.com/smrtkrft/FlexKey.git
   ```

4. **Open FlexKey.ino:**
   - File → Open → Navigate to FlexKey/FlexKey.ino

5. **Configure Board:**
   - Tools → Board → ESP32 Arduino → XIAO_ESP32C6
   - Tools → Port → (Select your COM port)

6. **Upload:**
   - Click Upload button or Ctrl+U

---

## 🚀 First Boot Setup

### Initial Access

1. **Power on** the device
2. Look for WiFi network: **`FlexKey-XXXXXX`** (last 6 chars of MAC address)
3. Connect (no password required)
4. Open browser: `http://192.168.4.1`

### Configuration Steps

#### 1. **Connection Tab** (BAĞLANTI)
   - Enter primary WiFi credentials
   - (Optional) Configure backup WiFi
   - (Optional) Set static IP
   - Enable/disable AP mode
   - Click **KAYDET & RESTART**

#### 2. **ID & Group Tab** (ID & GRUP)
   - Choose **Single** or **Multi** group mode
   - Add RFID UIDs manually or scan cards
   - Organize UIDs into groups (1-5)
   - Name your groups

#### 3. **URL & Relay Tab** (URL & RELAIS)
   - Configure trigger URLs (HTTP/HTTPS)
   - Set relay mode (toggle/pulse)
   - Adjust pulse duration if needed
   - Test relay operation

---

## 📖 Usage Guide

### Group Modes

**SINGLE GROUP MODE:**
- Only one group is active at a time
- Select active group from dropdown
- UIDs can exist in multiple groups (only active matters)
- Each group has independent relay settings

**MULTI GROUP MODE:**
- All groups are simultaneously active
- UIDs should be unique across groups (warning if duplicated)
- If UID exists in multiple groups, ALL trigger
- Global relay setting applies to all groups

### Adding RFID Cards

**Method 1: Manual Entry**
1. Go to ID & GRUP tab
2. Enter UID in format: `AA:BB:CC:DD` or `AA:BB:CC:DD:EE:FF:GG`
3. Click **UID EKLE**

**Method 2: Scan Card**
1. Hold RFID card near reader
2. UID appears in "Son Okutulan UID" for 60 seconds
3. Copy and paste, or manually enter
4. Click **UID EKLE**

### URL Triggers

- URLs are triggered in sequence when UID is scanned
- System does NOT wait for responses (fire-and-forget)
- Supports HTTP and HTTPS
- Examples:
  ```
  http://192.168.1.50/unlock
  https://api.example.com/access?user=123
  https://api.telegram.org/bot<TOKEN>/sendMessage?...
  ```

### Factory Reset

1. Press and hold button for **10 seconds**
2. Serial monitor shows countdown
3. All settings reset to defaults
4. System restarts in AP mode

### Quick Restart

- Press button briefly (< 10 seconds)
- System restarts with current settings

---

## 🛠️ Advanced Configuration

### Static IP Setup

1. Navigate to BAĞLANTI tab
2. Enable "Statik IP Kullan"
3. Enter:
   - IP Address (e.g., 192.168.1.100)
   - Gateway (e.g., 192.168.1.1)
   - Subnet (e.g., 255.255.255.0)
   - DNS (e.g., 8.8.8.8)
4. Save and restart

### Relay Configuration

**Toggle Mode:**
- First scan: Turn ON
- Second scan: Turn OFF
- Maintains state

**Pulse Mode:**
- Each scan: Turn ON for X milliseconds
- Automatically turns OFF
- Configurable duration (100-10000ms)

### URL Best Practices

- Keep URLs under 256 characters
- Test URLs in browser first
- Use URL encoding for special characters
- For APIs, include authentication in URL or use public endpoints
- Consider timeouts (system doesn't wait for response)

---

## 📊 System Information

### Web Interface Sections

**Tab 1: ID & GRUP**
- Group mode toggle
- Group selection
- UID management
- Last scanned UID display

**Tab 2: URL & RELAIS**
- URL configuration (15 per group)
- Relay settings
- Relay test button

**Tab 3: BAĞLANTI**
- WiFi credentials
- Static IP settings
- AP mode toggle

**Tab 4: INFO**
- Current IP address
- MAC address
- Active UID count
- Active group name
- User guide

---

## 🔍 Troubleshooting

### PN532 Not Found
```
[RFID] PN532 board not found
```
**Solutions:**
- Check I2C wiring (SDA/SCL)
- Verify 3.3V power supply
- Ensure PN532 is in I2C mode (check module switches/jumpers)
- Try different I2C address (some modules use 0x24 or 0x25)

### WiFi Connection Failed
```
[WIFI] Failed to connect to any network
```
**Solutions:**
- Verify SSID and password
- Check signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- System will fall back to AP mode

### Web Interface Not Loading
**Solutions:**
- Check IP address in serial monitor
- Verify you're on same network
- Try accessing via AP mode (192.168.4.1)
- Clear browser cache

### UID Not Recognized
**Solutions:**
- Verify UID is added to a group
- Check group is active (single mode) or multi-group enabled
- Ensure UID format is correct
- Check for typos in manual entry

### Relay Not Triggering
**Solutions:**
- Verify relay is enabled in settings
- Check relay wiring
- Test relay via web interface
- Ensure relay module voltage matches (3.3V or 5V)

---

## 📝 Serial Monitor Output

### Useful Debug Information

```
[SYSTEM] Device ID: FlexKey-AABBCC112233
[WIFI] IP: 192.168.1.100
[RFID] Card detected!
[RFID] UID: AA:BB:CC:DD
[CARD] Match found in Group 1
[TRIGGER] Sending 3 HTTP GET requests...
[HTTP] GET: http://192.168.1.50/unlock
[HTTP] Response code: 200
```

### Error Codes

- `[HTTP] ERROR: connection refused` - Server not reachable
- `[HTTP] Error code: -1` - Connection timeout
- `[STORAGE] Failed to save` - NVS write error
- `[WIFI] Connection timeout` - WiFi not reachable

---

## 🔒 Security Considerations

⚠️ **Important Security Notes:**

1. **AP Mode Password:** Default is OPEN (no password)
   - Modify in `FlexKey_Config.h` if needed
   
2. **Web Interface:** No authentication by default
   - Access control via network segmentation recommended
   - Consider VPN for remote access

3. **HTTPS URLs:** Certificate validation is DISABLED
   - Required for compatibility with self-signed certs
   - Be cautious with sensitive data

4. **RFID Security:** UID-based authentication only
   - Not suitable for high-security applications
   - Consider as convenience feature, not cryptographic security

---

## 📚 Library Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| Adafruit PN532 | Latest | NFC/RFID reader communication |
| ArduinoJson | 7.x | JSON serialization for config |
| WiFi (ESP32) | Built-in | Network connectivity |
| WebServer (ESP32) | Built-in | HTTP server |
| Preferences (ESP32) | Built-in | NVS storage |
| HTTPClient (ESP32) | Built-in | URL triggering |

---

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

---

## 📄 License

This project is licensed under the MIT License.

---

## 📞 Support

- **Website:** [smartkraft.ch/FlexKey](https://smartkraft.ch/FlexKey)
- **GitHub:** [github.com/smrtkrft/FlexKey](https://github.com/smrtkrft/FlexKey)
- **Email:** info@smartkraft.ch

---

## 🙏 Credits

- **Author:** SmartKraft
- **PN532 Library:** Adafruit Industries
- **Terminal Design:** Inspired by DMF project aesthetics

---

**Made with ❤️ by SmartKraft**
