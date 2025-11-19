# FlexKey - RFID Access Control System

## Project Overview

FlexKey is a production-ready, embedded IoT access control system built for ESP32-C6 microcontrollers. It provides RFID/NFC-based authentication with web configuration, WiFi connectivity, HTTP trigger capabilities, and relay control for physical access management.

**Status**: Production Ready (v1.0.3)
**Author**: SmartKraft
**License**: MIT
**Platform**: ESP32-C6 (specifically XIAO ESP32-C6)
**Language**: C++ (Arduino Framework)

---

## Architecture

### System Type
- **Embedded IoT Device**: Firmware running on ESP32-C6 microcontroller
- **Real-time System**: Non-blocking operations, watchdog protection
- **Persistent Configuration**: NVS (Non-Volatile Storage) for settings retention
- **Web-Enabled**: HTTP server with RESTful API and web interface

### Hardware Components
- **XIAO ESP32-C6**: Main microcontroller
- **PN532 NFC/RFID Reader**: I2C communication (supports 4-byte and 7-byte UIDs)
- **Relay Module**: Access control actuator (GPIO control)
- **Push Button**: Factory reset and restart functionality
- **Status LED**: Optional visual feedback (GPIO 10)

### Pin Configuration (XIAO ESP32-C6)
```
PN532 I2C:  SDA → GPIO22 (D4), SCL → GPIO23 (D5)
Button:     GPIO2 (D1) - Internal pullup enabled
Relay:      GPIO3 (D0) - Active high/low configurable
Status LED: GPIO10 (D10) - Optional
```

---

## Project Structure

```
FlexKey/
├── old_code/                    # Main source code (v1.0.3)
│   ├── FlexKey.ino             # Entry point - setup() and loop()
│   ├── FlexKey_Config.h        # System configuration and data structures
│   │
│   ├── FlexKey_Storage.cpp/h   # NVS storage management
│   ├── FlexKey_Button.cpp/h    # Button handler with debouncing
│   ├── FlexKey_WiFi.cpp/h      # Network management (AP + STA modes)
│   ├── FlexKey_RFID.cpp/h      # PN532 communication and card reading
│   ├── FlexKey_Relay.cpp/h     # Relay control (toggle/pulse modes)
│   ├── FlexKey_HTTP.cpp/h      # Non-blocking HTTP/HTTPS client
│   └── FlexKey_Web.cpp/h       # Web server and UI generation
│   │
│   ├── library.properties      # Arduino library metadata
│   ├── keywords.txt            # Arduino IDE syntax highlighting
│   ├── LIBRARIES.txt           # Dependency installation guide
│   ├── README.md               # Comprehensive documentation (415 lines)
│   └── sistem-ozet.md          # System summary in Turkish (263 lines)
│
└── FlexKey_NBook/              # Development notes
    └── Geri_Cekildi!!!.txt     # RFID frequency issue documentation
```

---

## Core Modules (Modular Architecture)

### 1. FlexKey_Storage (NVS Management)
**Purpose**: Persistent storage of all system configuration
**Key Functions**:
- `begin()`: Initialize NVS partition
- `loadSystemConfig()`: Load complete system state from NVS
- `saveSystemConfig()`: Persist all settings to NVS
- `saveGroups()`, `saveWiFiConfig()`, `saveRelayConfig()`: Granular saves
- `factoryReset()`: Erase all settings and restart
- `isFirstBoot()`: Detect initial startup

**Storage Keys**: Uses ESP32 Preferences library with namespaces for organized storage

### 2. FlexKey_Button (Button Handler)
**Purpose**: Factory reset and quick restart functionality
**Key Functions**:
- `begin()`: Initialize GPIO with internal pullup
- `update()`: Check button state, return true if factory reset triggered
- Debouncing with 50ms delay
- **10-second hold** → Factory reset (with countdown)
- **Short press** → Quick restart

### 3. FlexKey_WiFi (Network Manager)
**Purpose**: Dual WiFi with fallback + Access Point mode
**Key Features**:
- **Primary WiFi Network**: Main connection with optional static IP
- **Backup WiFi Network**: Automatic fallback with separate static IP config
- **Access Point Mode**: `FlexKey-XXXXXX` SSID (based on chip ID)
- **Smart WiFi Mode Selection**:
  - AP only (no WiFi configured)
  - STA only (AP disabled + WiFi configured)
  - AP+STA (both enabled)
- Auto-reconnection logic
- Support for different subnets (e.g., 192.168.1.x and 192.168.2.x)

**Key Functions**:
- `begin()`: Initialize WiFi system
- `startAP()`: Launch access point
- `connectToWiFi()`: Connect to primary/backup networks
- `update()`: Monitor connection status
- `getIPAddress()`, `getMACAddress()`: Network info retrieval

### 4. FlexKey_RFID (Card Reader)
**Purpose**: PN532 NFC/RFID communication
**Key Features**:
- I2C communication on GPIO22/23
- Support for 4-byte and 7-byte UIDs
- Non-blocking reads (100ms timeout)
- Card detection with debouncing (500ms minimum between reads)
- Automatic firmware version detection

**Key Functions**:
- `begin()`: Initialize PN532 and verify communication
- `readCard()`: Non-blocking UID read
- Error handling with periodic diagnostics

**UID Format**: `AA:BB:CC:DD` or `AA:BB:CC:DD:EE:FF:GG` (hex with colons)

### 5. FlexKey_Relay (Access Control)
**Purpose**: Physical relay control for door locks, gates, etc.
**Key Features**:
- **Toggle Mode**: Alternating ON/OFF state
- **Pulse Mode**: Timed activation (100-10000ms) then automatic OFF
- Non-blocking pulse timing (uses millis())
- Manual test capability via web interface

**Key Functions**:
- `begin()`: Initialize relay GPIO
- `toggle()`: Switch relay state
- `pulse(duration)`: Activate for specified milliseconds
- `update()`: Handle pulse timing (must be called in loop)

### 6. FlexKey_HTTP (HTTP Trigger System)
**Purpose**: Send HTTP/HTTPS GET requests to external APIs
**Key Features**:
- Non-blocking "fire-and-forget" approach
- Sequential URL triggering
- HTTP and HTTPS support
- Certificate validation disabled (for self-signed certs)
- Error handling without system blocking

**Key Functions**:
- `sendGET(url)`: Single GET request
- `sendMultipleGET(urls[], count)`: Trigger multiple URLs in sequence

**Use Cases**:
- Telegram bot notifications
- Home automation API calls
- Logging access events to external servers
- Webhook triggers

### 7. FlexKey_Web (Web Server & UI)
**Purpose**: Configuration interface and system management
**Key Features**:
- RESTful API with 13+ endpoints
- Terminal-style UI (DMF-inspired green/black theme)
- 4-tab interface: ID & Groups, URL & Relay, Connection, Info
- Real-time last scanned UID display (60-second window)
- Mobile-responsive design
- JSON-based configuration API

**API Endpoints**:
- `GET /` - Web interface HTML
- `GET /api/config` - Get complete system configuration
- `POST /api/config` - Update system settings
- `GET /api/last-uid` - Get last scanned UID
- `POST /api/test-relay` - Manual relay test
- `POST /api/restart` - System restart

**Web Interface Sections**:
1. **ID & GRUP**: Group mode, UID management, card scanning
2. **URL & RELAIS**: HTTP triggers, relay configuration
3. **BAĞLANTI**: WiFi settings, static IP, AP mode control
4. **INFO**: System status, user guide, troubleshooting

---

## Data Structures (FlexKey_Config.h)

### Core Types

#### `UID_t` - RFID Card Identifier
```cpp
struct UID_t {
    uint8_t data[7];      // UID bytes (4 or 7)
    uint8_t length;       // 4 or 7
    bool isValid;

    bool equals(const UID_t& other);
    String toString();           // "AA:BB:CC:DD"
    bool fromString(const String& str);
};
```

#### `Group_t` - Access Control Group
```cpp
struct Group_t {
    String name;                          // "Office Staff", "Visitors", etc.
    bool active;                          // Group activation state
    UID_t uids[MAX_UIDS];                 // Up to 200 UIDs
    uint16_t uidCount;
    String urls[MAX_URLS_PER_GROUP];      // Up to 15 URLs
    uint8_t urlCount;

    // Relay settings (per-group, single mode only)
    bool relayEnabled;
    bool relayToggle;                     // true=toggle, false=pulse
    uint16_t relayPulseDuration;          // milliseconds
};
```

#### `WiFiConfig_t` - Network Configuration
```cpp
struct WiFiConfig_t {
    // Primary Network
    String primarySSID;
    String primaryPassword;
    bool primaryUseStaticIP;
    IPAddress primaryStaticIP;
    IPAddress primaryGateway;
    IPAddress primarySubnet;
    IPAddress primaryDNS;

    // Backup Network (separate static IP config)
    String backupSSID;
    String backupPassword;
    bool backupUseStaticIP;
    IPAddress backupStaticIP;
    IPAddress backupGateway;
    IPAddress backupSubnet;
    IPAddress backupDNS;

    // AP Mode Control (v1.0.3)
    bool apModeEnabled;
};
```

#### `GlobalRelayConfig_t` - Multi-Group Relay Settings
```cpp
struct GlobalRelayConfig_t {
    bool enabled;
    bool toggle;              // true=toggle, false=pulse
    uint16_t pulseDuration;   // milliseconds
};
```

#### `SystemConfig_t` - Complete System State
```cpp
struct SystemConfig_t {
    bool multiGroupMode;              // false=single, true=multi
    uint8_t activeGroupIndex;         // Active group (single mode, 0-4)
    GlobalRelayConfig_t globalRelay;  // Multi-group relay settings
    Group_t groups[MAX_GROUPS];       // 5 groups
    WiFiConfig_t wifi;                // Network configuration
    String deviceID;                  // "FlexKey-XXXXXX"
};
```

#### `LastUID_t` - Last Scanned Card (Web Display)
```cpp
struct LastUID_t {
    UID_t uid;
    unsigned long timestamp;

    bool isExpired();         // 60-second display window
    void update(const UID_t& newUID);
    void clear();
};
```

---

## System Limits & Constants

```cpp
#define MAX_GROUPS          5       // Maximum access control groups
#define MAX_UIDS            200     // Total UID capacity
#define MAX_URLS_PER_GROUP  15      // HTTP triggers per group
#define MAX_URL_LENGTH      256     // URL character limit
#define UID_DISPLAY_TIME    60000   // 1 minute in milliseconds

#define BUTTON_DEBOUNCE_MS      50
#define FACTORY_RESET_HOLD_MS   10000  // 10 seconds
#define WIFI_CONNECT_TIMEOUT    10000  // 10 seconds
#define RFID_READ_DELAY         500    // Minimum ms between card reads
```

---

## Key Workflows

### 1. System Startup Flow
```
1. Serial console init (115200 baud)
2. NVS storage initialization
3. First boot check → Load/Create default config
4. Generate device ID from chip MAC (if needed)
5. Initialize button (factory reset handler)
6. WiFi initialization
7. Start AP mode (if enabled OR no WiFi configured)
8. Connect to WiFi networks (if configured)
9. RFID reader initialization (graceful failure)
10. Relay controller initialization
11. Web server startup on port 80
12. Display system info (IP, MAC, access URL)
```

### 2. Main Loop Operations
```
loop() {
    1. Check button state (factory reset/restart)
    2. Update WiFi connection status
    3. Handle web server requests
    4. Update relay pulse timing
    5. Poll RFID reader for cards
    6. Process detected cards
    7. Delay 10ms (watchdog protection)
}
```

### 3. RFID Card Processing
```
Card Detected → Update lastReadUID
             ↓
      ┌──────┴──────┐
      │ Multi-Group?│
      └──────┬──────┘
   YES ←─────┴─────→ NO
    │                │
    ├─ Check all     ├─ Check active group only
    │  groups        │
    ↓                ↓
Match Found? ─────→ Trigger URLs (sequential)
    │                ↓
    └─────────────→ Trigger Relay (if enabled)
                     ↓
              Log to Serial Monitor
```

**Group Modes Explained**:
- **Single Group Mode**:
  - Only one group active at a time (selectable via web UI)
  - UIDs can be duplicated across groups (only active group matters)
  - Each group has independent relay settings
  - Ideal for: location-based access, shift-based access

- **Multi-Group Mode**:
  - All groups active simultaneously
  - UIDs should be unique (system warns about duplicates)
  - If UID exists in multiple groups, ALL trigger
  - Global relay setting applies to all groups
  - Ideal for: hierarchical access, overlapping permissions

### 4. Factory Reset Procedure
```
Button Press → Start debounce timer
            ↓
        Hold ≥ 10 seconds?
            ↓ YES
    Serial countdown (10...9...8...)
            ↓
    NVS erase all partitions
            ↓
    System restart (ESP.restart())
            ↓
    Boot with default configuration
    (AP mode enabled, no WiFi, empty groups)
```

---

## Dependencies

### Required Arduino Libraries
```
Adafruit PN532 (latest)  - NFC/RFID reader communication
ArduinoJson (v7.x)       - JSON serialization for config/API
```

### Built-in ESP32 Libraries
```
WiFi                - Network connectivity (AP + STA)
WebServer           - HTTP server on port 80
HTTPClient          - Outgoing HTTP/HTTPS requests
WiFiClientSecure    - HTTPS support
Preferences         - NVS (Non-Volatile Storage)
Wire                - I2C communication with PN532
```

### Board Support
```
ESP32 Arduino Core by Espressif Systems
Board: XIAO_ESP32C6
URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

---

## Configuration & Usage

### Initial Setup (First Boot)
1. Power on device
2. Connect to WiFi network: **`FlexKey-XXXXXX`** (open, no password)
3. Open browser: `http://192.168.4.1`
4. Configure WiFi, groups, URLs, relay settings
5. Save & Restart

### Adding RFID Cards
**Method 1**: Manual entry in web UI
- Format: `AA:BB:CC:DD` or `AA:BB:CC:DD:EE:FF:GG`
- Colon-separated hex values

**Method 2**: Scan card near reader
- UID appears in "Son Okutulan UID" for 60 seconds
- Copy from web UI and add to group

### URL Trigger Examples
```
http://192.168.1.50/api/unlock
https://api.telegram.org/bot<TOKEN>/sendMessage?chat_id=<CHAT_ID>&text=Access%20granted
https://home-automation.local/trigger?device=door&action=open
```

### Static IP Configuration
- Separate static IP settings for primary and backup WiFi
- Supports different subnets (e.g., home network 192.168.1.x, office 192.168.2.x)
- Optional DNS configuration (defaults to 8.8.8.8)

### AP Mode Control (v1.0.3)
**Smart AP Mode Logic**:
```
No WiFi SSID + AP Disabled → AP FORCED ON (safety)
No WiFi SSID + AP Enabled  → AP ON
WiFi SSID + AP Disabled    → WiFi only (no AP)
WiFi SSID + AP Enabled     → Dual mode (AP + WiFi)
```

---

## Technical Details

### Memory Management
- **Dynamic Allocation**: PN532 instance created with `new`
- **String-based Config**: Flexible but memory-intensive (use carefully)
- **NVS Storage**: Wear-leveling built into ESP32 Preferences library
- **Array-based Storage**: Fixed-size arrays with counters for UIDs/URLs

### Non-Blocking Design
- **HTTP Requests**: Fire-and-forget, don't wait for responses
- **RFID Reading**: 100ms timeout, doesn't block loop
- **Relay Pulses**: Millisecond-based timing, non-blocking
- **WiFi Connection**: Timeout-based with fallback

### Error Handling
- **Graceful Degradation**: System continues if RFID fails to initialize
- **Serial Logging**: Detailed debug output (115200 baud)
- **Periodic Reporting**: Errors logged every 5 seconds (RFID diagnostics)
- **WiFi Fallback**: Automatic AP mode if WiFi connection fails

### Security Considerations
**Important**: FlexKey is designed for convenience, not cryptographic security.

⚠️ **Known Security Limitations**:
1. **AP Mode**: No password by default (open network)
2. **Web Interface**: No authentication (anyone on network can configure)
3. **HTTPS**: Certificate validation disabled (for self-signed cert compatibility)
4. **RFID**: UID-based only, not encrypted (easily cloneable)

**Recommendations**:
- Use network segmentation (isolated VLAN)
- Consider VPN for remote access
- Physical security for device and RFID reader
- Suitable for low-security applications (office doors, lockers, etc.)
- NOT suitable for high-security applications (bank vaults, data centers, etc.)

---

## Version History

### v1.0.3 (Current - October 2025)
**Major Changes**:
- Separate static IP configurations for primary and backup WiFi
- Smart AP mode control (can be disabled if WiFi is configured)
- Automatic WiFi mode selection (AP / STA / AP+STA)
- Enhanced web UI validation
- Multi-subnet support (different IP ranges for each network)

**Files Modified**: FlexKey_Config.h, FlexKey_WiFi.cpp, FlexKey_Storage.cpp, FlexKey_Web.cpp, FlexKey.ino

### v1.0.2 (October 2025)
- Device ID now uses Chip ID instead of MAC address (solves 00:00:00:00:00:00 issue on some boards)
- Web UI transformed to DMF terminal style (green on black theme)
- 4-column status bar

### v1.0.1 (October 2025)
- Bug fix: WiFi.h include added to HTTP and Web modules
- Compilation error fixes

### v1.0.0 (Initial Release)
- Production-ready release
- All core features implemented
- Modular architecture
- Comprehensive documentation

---

## Known Issues & Solutions

### Issue: PN532 Not Found
**Symptoms**: `[RFID] PN532 board not found`
**Solutions**:
- Verify I2C wiring (SDA → GPIO22, SCL → GPIO23)
- Check 3.3V power supply
- Ensure PN532 is in I2C mode (check jumpers/switches on module)
- Try different I2C addresses (some modules use 0x24 or 0x25)

### Issue: RFID Frequency Blocking (Documented in FlexKey_NBook)
**Symptoms**: RFID stops working when powered by battery/DC
**Root Cause**: Electromagnetic interference from power supply
**Solution**: Ferrite filter on power lines
**Status**: Temporarily resolved, documented for future reference

### Issue: WiFi Connection Failed
**Symptoms**: `[WIFI] Failed to connect to any network`
**Solutions**:
- Verify SSID and password
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Check signal strength
- System will automatically fall back to AP mode

### Issue: MAC Address Shows 00:00:00:00:00:00 (v1.0.1)
**Fixed in v1.0.2**: Now uses Chip ID instead of MAC address for device identification

---

## Development Guidelines

### Code Style
- **Modular Design**: Each module in separate .cpp/.h files
- **Clear Naming**: Descriptive function and variable names
- **Comments**: Inline documentation for complex logic
- **Error Handling**: Always check return values, log errors
- **Serial Logging**: Use `[MODULE]` prefixes for debug output

### Testing Checklist
- [ ] RFID card detection (4-byte and 7-byte UIDs)
- [ ] Web interface accessibility (AP mode and WiFi mode)
- [ ] URL triggering (HTTP and HTTPS)
- [ ] Relay operation (toggle and pulse modes)
- [ ] WiFi fallback (primary → backup → AP only)
- [ ] Factory reset (10-second button hold)
- [ ] Configuration persistence (NVS save/load)
- [ ] Static IP configuration (both networks)
- [ ] Multi-group and single-group modes
- [ ] UID conflict detection

### Adding New Features
1. **Create Module**: New .cpp/.h pair if substantial functionality
2. **Update Config**: Add structures/constants to FlexKey_Config.h
3. **Update Storage**: Add NVS save/load if persistent state needed
4. **Update Web UI**: Add API endpoints and UI elements
5. **Update Documentation**: Modify README.md and this file
6. **Test Thoroughly**: Verify no regressions in existing features

---

## Common Modification Points

### Changing Pin Assignments
**File**: `FlexKey_Config.h`
```cpp
#define PIN_I2C_SDA         22  // Change if different board
#define PIN_I2C_SCL         23
#define PIN_BUTTON          2
#define PIN_RELAY           3
#define PIN_STATUS_LED      10
```

### Changing System Limits
**File**: `FlexKey_Config.h`
```cpp
#define MAX_GROUPS          5   // Increase if more groups needed
#define MAX_UIDS            200 // Increase if more cards needed
#define MAX_URLS_PER_GROUP  15  // Increase for more HTTP triggers
```

### Changing AP Mode Password
**File**: `FlexKey_Config.h`
```cpp
#define AP_MODE_PASSWORD    ""  // Set password for secure AP
```

### Changing Web UI Theme
**File**: `FlexKey_Web.cpp` - Look for CSS section in `generateHTML()`
```cpp
// Current: DMF terminal style (green on black)
// Modify colors in <style> block
```

---

## API Reference (Web Interface)

### GET /
Returns complete HTML web interface

### GET /api/config
Returns JSON system configuration
```json
{
  "multiGroupMode": false,
  "activeGroupIndex": 0,
  "groups": [...],
  "wifi": {...},
  "globalRelay": {...}
}
```

### POST /api/config
Update system configuration
**Body**: JSON matching SystemConfig_t structure
**Response**: `{"status": "ok"}` or error message

### GET /api/last-uid
Returns last scanned UID (within 60-second window)
```json
{
  "uid": "AA:BB:CC:DD",
  "valid": true
}
```

### POST /api/test-relay
Manual relay test
**Body**: JSON with group index (single mode) or global (multi mode)
**Response**: `{"status": "ok"}`

### POST /api/restart
System restart
**Response**: None (device restarts immediately)

---

## Serial Monitor Commands & Output

### Startup Output
```
========================================
  SmartKraft - FlexKey
  Version: 1.0.3
========================================

[SYSTEM] Device ID: FlexKey-A1B2C3
[WIFI] Starting AP: FlexKey-A1B2C3
[WIFI] AP IP: 192.168.4.1
[WIFI] Connecting to: MyWiFiNetwork
[WIFI] Connected! IP: 192.168.1.100
[RFID] PN532 found! Firmware version: 1.6
[WEB] Web server started

========================================
  SYSTEM READY
========================================
  IP Address: 192.168.1.100
  MAC Address: A1:B2:C3:D4:E5:F6
  Web Interface: http://192.168.1.100
========================================
```

### Card Detection Output
```
[CARD] UID detected: AA:BB:CC:DD
[CARD] Single-group mode - checking Group 1
[CARD] Match found in active group (Office Staff)
[TRIGGER] Sending 3 HTTP GET requests...
[HTTP] GET: http://192.168.1.50/unlock
[HTTP] Response code: 200
[HTTP] GET: https://api.telegram.org/bot.../sendMessage
[HTTP] Response code: 200
[TRIGGER] Relay pulse (500ms)
```

### Error Output
```
[HTTP] ERROR: connection refused
[HTTP] Error code: -1 (connection timeout)
[STORAGE] Failed to save group data
[RFID] PN532 board not found (periodic check)
```

---

## Troubleshooting Guide

### Problem: Web Interface Not Loading
**Diagnostic Steps**:
1. Check Serial Monitor for IP address
2. Verify device on same network
3. Try AP mode access (192.168.4.1)
4. Ping device IP
5. Clear browser cache

### Problem: UID Not Recognized
**Diagnostic Steps**:
1. Verify UID is added to a group (check web UI)
2. Check group is active (single mode) or multi-group enabled
3. Ensure UID format is correct (no typos)
4. Check Serial Monitor for "Card detected" message
5. Verify RFID reader is working (check [RFID] logs)

### Problem: Relay Not Triggering
**Diagnostic Steps**:
1. Verify relay is enabled in group settings
2. Check relay wiring and power supply
3. Test relay manually via web interface
4. Ensure relay module voltage matches (3.3V or 5V logic)
5. Check Serial Monitor for [TRIGGER] messages

### Problem: URLs Not Triggering
**Diagnostic Steps**:
1. Test URLs in browser first
2. Check Serial Monitor for [HTTP] error messages
3. Verify WiFi connectivity
4. Ensure URLs are under 256 characters
5. Check URL encoding for special characters

---

## Contact & Support

- **Website**: [smartkraft.ch/FlexKey](https://smartkraft.ch/FlexKey)
- **GitHub**: [github.com/smrtkrft/FlexKey](https://github.com/smrtkrft/FlexKey)
- **Email**: info@smartkraft.ch

---

## License

MIT License - See project repository for full license text

---

## Credits

- **Author**: SmartKraft
- **PN532 Library**: Adafruit Industries
- **Terminal Design Inspiration**: DMF project aesthetics
- **Documentation**: Comprehensive user and developer guides included

---

**Last Updated**: October 2025
**Document Version**: 1.0
**For FlexKey Version**: 1.0.3
