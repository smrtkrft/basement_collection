# Interactive Toy Enhancement System

![Interactive Toy Example](https://github.com/smrtkrft/basement_collection/raw/main/dw_getSound/assets/darth_vader_pop.jpg)

## 🎮 About Interactive Toys

Modern collectible toys and action figures (like Funko POP!, statues, and display figures) can be enhanced with smart electronics to create truly interactive experiences. This project transforms static collectibles into dynamic, internet-connected devices that respond to user interaction.

**How it works:**
- Press the button on your toy
- Custom audio plays through built-in speaker
- HTTP requests trigger smart home devices, webhooks, or APIs
- LED indicators show system status and audio playback
- Fully configurable via serial commands

**Perfect for:**
- Smart home integration (trigger lights, scenes, routines)
- IoT projects and automation
- Collectible display enhancement
- Interactive art installations
- Custom sound effects for figurines
- Webhook notifications and logging

This system uses an ESP32-C6 microcontroller, DFPlayer Mini audio module, and a simple button interface to add intelligence to any toy or collectible figure.

## 🎬 See It In Action

Watch the complete demonstration below - button press triggers audio playback, LED animations, and HTTP requests in real-time:

https://github.com/user-attachments/assets/63fdfd88-6d4f-4fe7-b599-d426373d5c30


---

# ESP32-C6 HTTP Audio Player

HTTP request triggering system controlled by DFPlayer Mini, activated by button press, and plays audio.

## 🎯 Features

- Button-triggered HTTP GET requests
- MP3 playback with DFPlayer Mini
- Full configuration via Serial port
- Persistent WiFi settings storage (EEPROM)
- Multiple URL management (max 10)
- LED status indicators
- Sequential URL execution
- Volume control

## 🔌 Hardware Connections

### Xiao ESP32-C6 Pin Table

| Digital Pin | GPIO | Usage              |
|-------------|------|--------------------|
| D0          | 0    | -                  |
| D1          | 1    | -                  |
| D2          | 2    | LED1 (Status)      |
| D3          | 21   | -                  |
| D4          | 22   | -                  |
| D5          | 23   | -                  |
| D6          | 16   | DFPlayer RX (TX)   |
| D7          | 17   | DFPlayer TX (RX)   |
| D8          | 19   | -                  |
| D9          | 20   | Button             |
| D10         | 18   | -                  |

### Connection Diagram

```
┌────────────────────────────────────┐
│       Xiao ESP32-C6                │
├────────────────────────────────────┤
│ D9  (GPIO20) ─── Button ─── GND    │
│ D2  (GPIO2)  ─── LED1 (Status) ─ 220Ω┤GND
│ D3  (GPIO21) ─── LED2 (Audio) ─ 220Ω┤GND
│ D6  (GPIO16) ─── LED3 (Optional) ─220Ω┤GND
│ D6  (GPIO16) ─── DFPlayer RX       │
│ D7  (GPIO17) ─── DFPlayer TX       │
│ 5V           ─── DFPlayer VCC      │
│ GND          ─── DFPlayer GND      │
└────────────────────────────────────┘

DFPlayer Mini:
┌────────────────────────┐
│  [VCC] [RX] [TX] [GND] │
│    │     │    │     │  │
│    5V   16   17   GND  │
│                        │
│  [SPK1] [SPK2]         │
│     │      │           │
│  Speaker (+/-)         │
│                        │
│  [MicroSD Card]        │
│  001.mp3, 002.mp3...   │
└────────────────────────┘
```

## 🛠️ Hardware Components

![Hardware Components](https://github.com/smrtkrft/basement_collection/raw/main/dw_getSound/assets/hardware_components.jpg)

### Essential Components (Left 3 items)

Perfect for toys with built-in button and speaker:

| Component | Description | Purpose |
|-----------|-------------|---------|
| **Xiao ESP32-C6** | Tiny microcontroller with WiFi | Brain of the system, handles WiFi and logic |
| **DFPlayer Mini** | MP3 audio player module | Plays audio files from SD card |
| **MicroSD Card** | 8GB storage (FAT32) | Stores MP3 sound files (001.mp3, 002.mp3...) |

These three components are sufficient for toys that already have:
- ✅ A button you can wire to
- ✅ A speaker you can replace/connect
- ✅ Battery/power source

### Optional Components (Right 2 items)

For toys without speaker or button:

| Component | Description | Use Case |
|-----------|-------------|----------|
| **PN532 NFC/RFID Reader** | Contactless card reader | Alternative to button - trigger with NFC card/tag |
| **3W 8Ω Speaker** | Small speaker with cables | Add audio to toys without built-in speaker |

**When to use optional components:**
- 🔊 **Speaker**: Your toy has no speaker or broken speaker
- 🏷️ **PN532 RFID**: Your toy has no button, or you want NFC card activation instead

### Minimal Setup Options

**Option 1 - With Button & Speaker (Basic):**
- ESP32-C6 + DFPlayer Mini + MicroSD Card = **3 items**

**Option 2 - With RFID, Without Button:**
- ESP32-C6 + DFPlayer Mini + MicroSD Card + PN532 = **4 items**

**Option 3 - Without Speaker:**
- ESP32-C6 + DFPlayer Mini + MicroSD Card + External Speaker = **4 items**

**Option 4 - Full Setup:**
- All components = **5 items** (maximum flexibility)

### Required Materials

- 1x Xiao ESP32-C6
- 1x DFPlayer Mini (6-pin module)
- 1x MicroSD Card (FAT32, with MP3 files)
- 1x Mini Speaker (3W, 4-8Ω)
- 1x Button (Push Button)
- 3x LED (red, green, blue)
- 3x 220Ω Resistor (for LEDs)
- Breadboard and jumper wires

**Note:** Only 4 pins + 2 speaker pins are used on DFPlayer (VCC, GND, RX, TX, SPK1, SPK2)

## 📦 Required Libraries

Arduino IDE → Sketch → Include Library → Manage Libraries:

1. **DFRobotDFPlayerMini** by DFRobot (v1.0.6+)
   - Search: "DFPlayer Mini"
   - Install

2. **ESP32 Board Support**
   - File → Preferences
   - Additional Board Manager URLs: 
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Board Manager → ESP32 by Espressif → Install (v3.0.0+)

3. Built-in libraries (pre-installed):
   - WiFi.h
   - HTTPClient.h
   - Preferences.h
   - HardwareSerial.h

## 🎵 SD Card Preparation

1. Format MicroSD card as FAT32
2. Name MP3 files as follows:
   ```
   001.mp3
   002.mp3
   003.mp3
   ...
   ```
3. Insert SD card into DFPlayer Mini

**Important:** DFPlayer Mini uses the number in the filename!

## 🚀 Installation

### 1. Arduino IDE Settings

```
Tools → Board → ESP32 Arduino → XIAO_ESP32C6
Tools → Upload Speed → 921600
Tools → USB CDC On Boot → Enabled
Tools → Port → (port where ESP32-C6 is connected)
```

### 2. Upload Code

1. Open `dw_getSound.ino` file in Arduino IDE
2. Press Upload button (or Ctrl+U)
3. Wait for upload to complete

### 3. Open Serial Monitor

```
Tools → Serial Monitor
Baud Rate: 115200
```

## 📝 Usage Guide

### Initial Setup

1. **Set WiFi:**
   ```
   wifi MyWiFi password123
   ```

2. **Add URLs:**
   ```
   add url1 https://api.example.com/sensor/data
   add url2 https://webhook.site/xxxxx
   ```

3. **Check:**
   ```
   list url
   status
   ```

4. **Test:**
   ```
   test
   ```

5. **Press Button!**

### Serial Commands

#### WiFi Management
```bash
wifi SSID PASSWORD       # Save WiFi settings
                         # Example: wifi HomeWiFi 12345678
```

#### URL Management
```bash
add url1 URL             # Add URL to list 1 (ON state - max 10)
                         # Example: add url1 https://api.example.com/data

add url2 URL             # Add URL to list 2 (OFF state - max 10)

del url1 INDEX           # Delete URL from list 1
                         # Example: del url1 2

del url2 INDEX           # Delete URL from list 2

list url                 # List all URLs
```

#### Audio Control
```bash
volume 0-30              # Set volume level (default: 20)
                         # Example: volume 25

play TRACK               # Play track manually
                         # Example: play 1
```

#### System
```bash
status                   # Show system status
test led                 # LED test
test uart                # UART communication test
reset                    # Reset all settings (caution!)
help                     # Command list
```

## 🎮 Operation Logic

### When Button Pressed:

```
1. Get next URL
   ↓
2. Send HTTP GET request
   ↓
3. Response successful?
   ├─ Yes → Go to 4
   └─ No → Status LED blinks (error)
   ↓
4. Play corresponding MP3 (LED Audio blinks randomly)
   ↓
5. Move to next URL
   ↓
6. Ready (wait for button)
```

### LED Indicators:

- 💡 **LED GPIO2 (Status)**: System state and WiFi connection indicator
- 🔊 **LED GPIO21 (Audio)**: Random blinking during audio playback (configure duration based on audio file length)
- 🔧 **LED GPIO16 (Optional)**: Additional indicator for custom use

## 🔧 Example Scenarios

### Scenario 1: Webhook Test
```bash
# Get test URL from Webhook.site
wifi MyWiFi password123
add url1 https://webhook.site/xxxxx-xxxx-xxxx
test
# Press button and see request on webhook.site
```

### Scenario 2: IoT Control
```bash
wifi HomeWiFi mypassword
add url1 https://iot.example.com/api/light/on
add url2 https://iot.example.com/api/temp/read
list url
# Each button press triggers URLs sequentially
```

### Scenario 3: Data Sending
```bash
add url1 https://thingspeak.com/update?api_key=XXXXX&field1=1
add url1 https://thingspeak.com/update?api_key=XXXXX&field1=2
# Each press sends different value
```

## 🐛 Troubleshooting

### Problem: Cannot initialize DFPlayer
**Solution:**
- Check connections (RX/TX crossed?)
- Is SD card inserted?
- Is SD card formatted as FAT32?
- Are MP3 files named correctly? (001.mp3, 002.mp3...)

### Problem: Cannot connect to WiFi
**Solution:**
```bash
status              # Check connection status
wifi SSID PASS      # Re-enter settings
reset               # Reset if needed
```

### Problem: HTTP request fails
**Solution:**
- Is URL correct? (must start with http:// or https://)
- Is internet connection available?
- Check error code in Serial Monitor
- Use webhook.site for testing

### Problem: No audio playing
**Solution:**
```bash
volume 25           # Increase volume
play 1              # Manual test
```
- Is speaker connected correctly?
- Does 001.mp3 exist on SD card?

### Problem: Button not working
**Solution:**
- Is button connected between GPIO 20 and GND?
- Button might be faulty, test it
- Check for "Button pressed!" message in Serial Monitor

## 📊 Technical Details

### Memory Usage
- Program: ~300KB Flash
- SRAM: ~50KB
- Preferences: ~4KB NVS

### Limitations
- Maximum 10 URLs per list
- URL length: 256 characters
- DFPlayer: Maximum 255 files
- HTTP timeout: 5 seconds

### Power Consumption
- Active (WiFi+Audio): ~200mA
- WiFi connected (idle): ~80mA
- Deep sleep: ~20µA (can be added in future)

## 🔮 Future Improvements

Features that can be added:

- [ ] OTA (Over-The-Air) wireless updates
- [ ] Web interface (configuration over WiFi)
- [ ] MQTT support
- [ ] POST requests + JSON body
- [ ] Multiple button support
- [ ] RTC with scheduled requests
- [ ] Deep sleep mode (battery saving)
- [ ] SD card logging

## 📄 License

AGPL-3.0 License - You can use and modify as you wish.

## 🤝 Contributing

Feel free to send pull requests for suggestions and improvements!

---

**Project Date:** November 2025  
**Platform:** Arduino IDE + ESP32-C6  
**Version:** 1.0.0



