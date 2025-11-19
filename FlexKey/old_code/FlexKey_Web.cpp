#include "FlexKey_Web.h"
#include "FlexKey_Storage.h"
#include "FlexKey_RFID.h"
#include "FlexKey_Relay.h"
#include "FlexKey_HTTP.h"
#include <ArduinoJson.h>
#include <WiFi.h>

FlexKeyWeb::FlexKeyWeb(SystemConfig_t* config, FlexKeyStorage* stor, 
                       FlexKeyRFID* rfidReader, FlexKeyRelay* relayCtrl, 
                       FlexKeyHTTP* httpClient, LastUID_t* lastUIDPtr)
    : server(nullptr), sysConfig(config), storage(stor), 
      rfid(rfidReader), relay(relayCtrl), http(httpClient), lastUID(lastUIDPtr) {
}

void FlexKeyWeb::begin() {
    server = new WebServer(WEB_SERVER_PORT);
    
    // Page routes
    server->on("/", HTTP_GET, [this]() { this->handleRoot(); });
    server->on("/style.css", HTTP_GET, [this]() { this->handleCSS(); });
    server->on("/script.js", HTTP_GET, [this]() { this->handleJS(); });
    
    // API routes
    server->on("/api/config", HTTP_GET, [this]() { this->handleAPIGetConfig(); });
    server->on("/api/groupmode", HTTP_POST, [this]() { this->handleAPISetGroupMode(); });
    server->on("/api/activegroup", HTTP_POST, [this]() { this->handleAPISetActiveGroup(); });
    server->on("/api/groups", HTTP_GET, [this]() { this->handleAPIGetGroups(); });
    server->on("/api/group/save", HTTP_POST, [this]() { this->handleAPISaveGroup(); });
    server->on("/api/group/deleteuid", HTTP_POST, [this]() { this->handleAPIDeleteUID(); });
    server->on("/api/group/adduid", HTTP_POST, [this]() { this->handleAPIAddUID(); });
    server->on("/api/wifi", HTTP_GET, [this]() { this->handleAPIGetWiFiConfig(); });
    server->on("/api/wifi/save", HTTP_POST, [this]() { this->handleAPISaveWiFiConfig(); });
    server->on("/api/system", HTTP_GET, [this]() { this->handleAPIGetSystemInfo(); });
    server->on("/api/relay/test", HTTP_POST, [this]() { this->handleAPITestRelay(); });
    server->on("/api/lastuid", HTTP_GET, [this]() { this->handleAPIGetLastUID(); });
    server->on("/api/globalrelay", HTTP_POST, [this]() { this->handleAPISaveGlobalRelay(); });
    server->on("/api/uidconflicts", HTTP_GET, [this]() { this->handleAPICheckUIDConflicts(); });
    
    server->onNotFound([this]() { this->handleNotFound(); });
    
    server->begin();
    Serial.println("[WEB] Server started on port " + String(WEB_SERVER_PORT));
}

void FlexKeyWeb::handleClient() {
    if (server) {
        server->handleClient();
    }
}

bool FlexKeyWeb::isRunning() {
    return server != nullptr;
}

String FlexKeyWeb::generateHTML() {
    String html = R"rawliteral(<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>)rawliteral";
    html += SYSTEM_NAME;
    html += R"rawliteral(</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>FlexKey RFID Access Control</h1>
            <p class="device-id">Device ID: <span id="deviceID">Loading...</span></p>
        </div>

        <div class="tabs">
            <button class="tab active" data-tab="tab1">ID & GRUP</button>
            <button class="tab" data-tab="tab2">URL & RÖLE</button>
            <button class="tab" data-tab="tab3">BAĞLANTI</button>
            <button class="tab" data-tab="tab4">BİLGİ</button>
        </div>

        <div id="tab1" class="tab-content active">
            <div class="section">
                <h2>GRUP MODU</h2>
                <div class="toggle-container">
                    <span>TEKLİ GRUP</span>
                    <label class="switch">
                        <input type="checkbox" id="multiGroupMode">
                        <span class="slider"></span>
                    </label>
                    <span>ÇOKLU GRUP</span>
                </div>
                <div id="multiGroupWarning" class="warning" style="display:none;">
                    Çoklu grup modunda aynı UID birden fazla grupta bulundu! 
                    Bu UID okutulduğunda tüm grupların URL'leri tetiklenecektir.
                </div>
            </div>

            <div class="section" id="relaySection" style="display:none;">
                <h2>GLOBAL RÖLE AYARLARI</h2>
                <div class="form-group">
                    <label><input type="checkbox" id="globalRelayEnabled"> Röle Etkin</label>
                </div>
                <div class="form-group">
                    <label><input type="radio" name="globalRelayMode" value="toggle" checked> Toggle</label>
                    <label><input type="radio" name="globalRelayMode" value="pulse"> Pulse</label>
                </div>
                <div class="form-group">
                    <label>Pulse Süresi (ms)</label>
                    <input type="number" id="globalRelayPulse" value="500" min="100" max="10000">
                </div>
                <button class="btn" onclick="saveGlobalRelay()">KAYDET</button>
            </div>

            <div class="section" id="singleGroupSelect" style="display:none;">
                <h2>AKTİF GRUP</h2>
                <select id="activeGroup" onchange="changeActiveGroup()">
                    <option value="0">Grup 1</option>
                    <option value="1">Grup 2</option>
                    <option value="2">Grup 3</option>
                    <option value="3">Grup 4</option>
                    <option value="4">Grup 5</option>
                </select>
            </div>

            <div class="section">
                <h2>GRUP YÖNETİMİ</h2>
                <div class="group-tabs" id="groupTabs"></div>
                <div id="groupContent"></div>
            </div>
        </div>

        <!-- Tab 2: URLs & Relay -->
        <div id="tab2" class="tab-content">
            <div class="section">
                <h2>URL & RELAIS AYARLARI</h2>
                <div class="group-tabs" id="groupTabsURL"></div>
                <div id="groupURLContent"></div>
            </div>
        </div>

        <!-- Tab 3: Connection -->
        <div id="tab3" class="tab-content">
            <div class="section">
                <h2>AP MODE AYARLARI</h2>
                <div class="form-group">
                    <div class="toggle-container">
                        <span>AP Modu Kapalı</span>
                        <label class="switch">
                            <input type="checkbox" id="apModeEnabled">
                            <span class="slider"></span>
                        </label>
                        <span>AP Modu Açık</span>
                    </div>
                    <p style="color:#888; font-size:12px; margin-top:10px;">
                        AP modu açık olduğunda cihaz kendi WiFi ağını yayınlar (FlexKey-XXXXXX).
                        Kapatmak için en az bir WiFi SSID yapılandırılmalıdır.
                    </p>
                </div>
            </div>

            <div class="section">
                <h2>PRIMARY WiFi</h2>
                <div class="form-group">
                    <label>SSID</label>
                    <input type="text" id="primarySSID" placeholder="WiFi adı">
                </div>
                <div class="form-group">
                    <label>Şifre</label>
                    <input type="password" id="primaryPassword" placeholder="WiFi şifresi">
                </div>
                
                <div class="form-group">
                    <label><input type="checkbox" id="primaryUseStaticIP" onchange="togglePrimaryStaticIP()"> Statik IP Kullan</label>
                </div>
                
                <div id="primaryStaticIPFields" style="display:none;">
                    <div class="form-group">
                        <label>IP Adresi</label>
                        <input type="text" id="primaryStaticIP" placeholder="192.168.1.100">
                    </div>
                    <div class="form-group">
                        <label>Gateway</label>
                        <input type="text" id="primaryGateway" placeholder="192.168.1.1">
                    </div>
                    <div class="form-group">
                        <label>Subnet</label>
                        <input type="text" id="primarySubnet" placeholder="255.255.255.0">
                    </div>
                    <div class="form-group">
                        <label>DNS</label>
                        <input type="text" id="primaryDNS" placeholder="8.8.8.8">
                    </div>
                </div>
            </div>

            <div class="section">
                <h2>BACKUP WiFi</h2>
                <div class="form-group">
                    <label>SSID</label>
                    <input type="text" id="backupSSID" placeholder="Yedek WiFi">
                </div>
                <div class="form-group">
                    <label>Şifre</label>
                    <input type="password" id="backupPassword" placeholder="Yedek şifre">
                </div>
                
                <div class="form-group">
                    <label><input type="checkbox" id="backupUseStaticIP" onchange="toggleBackupStaticIP()"> Statik IP Kullan</label>
                </div>
                
                <div id="backupStaticIPFields" style="display:none;">
                    <div class="form-group">
                        <label>IP Adresi</label>
                        <input type="text" id="backupStaticIP" placeholder="192.168.2.100">
                    </div>
                    <div class="form-group">
                        <label>Gateway</label>
                        <input type="text" id="backupGateway" placeholder="192.168.2.1">
                    </div>
                    <div class="form-group">
                        <label>Subnet</label>
                        <input type="text" id="backupSubnet" placeholder="255.255.255.0">
                    </div>
                    <div class="form-group">
                        <label>DNS</label>
                        <input type="text" id="backupDNS" placeholder="8.8.8.8">
                    </div>
                </div>
            </div>

            <button class="btn" onclick="saveWiFiConfig()">KAYDET & RESTART</button>
        </div>

        <div id="tab4" class="tab-content">
            <div class="section">
                <h2>SİSTEM BİLGİLERİ</h2>
                <div class="info-item">
                    <span>IP Adresi:</span>
                    <span id="ipAddress">-</span>
                </div>
                <div class="info-item">
                    <span>MAC Adresi:</span>
                    <span id="macAddress">-</span>
                </div>
                <div class="info-item">
                    <span>Chip ID:</span>
                    <span id="chipID">-</span>
                </div>
                <div class="info-item">
                    <span>Toplam UID:</span>
                    <span id="totalUIDs">-</span>
                </div>
                <div class="info-item">
                    <span>Aktif Grup:</span>
                    <span id="activeGroupName">-</span>
                </div>
                <div class="info-item">
                    <span>Versiyon:</span>
                    <span>)rawliteral";
    html += FLEXKEY_VERSION;
    html += R"rawliteral(</span>
                </div>
            </div>

            <div class="section">
                <h2>SON OKUTULAN UID (60 saniye)</h2>
                <div id="lastUIDDisplay" class="last-uid-box">
                    <div id="lastUIDValue" style="font-size:20px; font-weight:bold; margin-bottom:10px;">Henüz kart okutulmadı</div>
                    <div id="lastUIDTime" style="font-size:12px; color:#888;"></div>
                </div>
            </div>

            <div class="section">
                <h2>KULLANIM KLAVUZU</h2>
                <p>
                    <strong>FlexKey RFID Access Control System</strong><br><br>
                    1. ID & GRUP sekmesinden RFID UID'leri ekleyin<br>
                    2. URL & RÖLE sekmesinden tetikleme URL'lerini ayarlayın<br>
                    3. BAĞLANTI sekmesinden WiFi yapılandırın<br><br>
                    Detaylı bilgi: <a href="https://smartkraft.ch/FlexKey" target="_blank">smartkraft.ch/FlexKey</a>
                </p>
            </div>

            <div class="section">
                <button onclick="testRelay()" style="width:100%; padding:15px; margin-bottom:10px;">RÖLE TEST</button>
                <button onclick="confirmRestart()" style="width:100%; padding:15px;">YENİDEN BAŞLAT</button>
            </div>
        </div>
    </div>
    <script src="/script.js"></script>
</body>
</html>)rawliteral";
    
    return html;
}

void FlexKeyWeb::handleRoot() {
    server->send(200, "text/html", generateHTML());
}

void FlexKeyWeb::handleCSS() {
    server->send(200, "text/css", generateCSS());
}

void FlexKeyWeb::handleJS() {
    server->send(200, "application/javascript", generateJS());
}

void FlexKeyWeb::handleNotFound() {
    server->send(404, "text/plain", "404: Not Found");
}

// CSS Generator - Modern Dark Theme (SmartKraft Style)
String FlexKeyWeb::generateCSS() {
    return R"rawliteral(
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Roboto', 'Helvetica Neue', Arial, sans-serif;
    background: #0a0a0a;
    color: #ffffff;
    padding: 20px;
    line-height: 1.6;
    min-height: 100vh;
}

.container {
    max-width: 1000px;
    margin: 0 auto;
    background: #1a1a1a;
    border-radius: 12px;
    box-shadow: 0 8px 32px rgba(0, 0, 0, 0.6);
    padding: 40px;
    border: 1px solid #2a2a2a;
}

.header {
    text-align: center;
    border-bottom: 2px solid #00d4ff;
    padding-bottom: 25px;
    margin-bottom: 35px;
}

.header h1 {
    font-size: 32px;
    color: #ffffff;
    margin-bottom: 10px;
    font-weight: 600;
    letter-spacing: -0.5px;
}

.device-id {
    font-size: 14px;
    color: #888;
    font-weight: 400;
}

.device-id span {
    color: #00d4ff;
    font-weight: 600;
}

/* Buttons */
.btn {
    background: linear-gradient(135deg, #00d4ff 0%, #0099cc 100%);
    color: #000;
    border: none;
    padding: 12px 28px;
    cursor: pointer;
    font-size: 14px;
    font-weight: 600;
    border-radius: 8px;
    transition: all 0.3s ease;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.btn:hover {
    background: linear-gradient(135deg, #00e5ff 0%, #00b8e6 100%);
    transform: translateY(-2px);
    box-shadow: 0 6px 20px rgba(0, 212, 255, 0.4);
}

.btn-secondary {
    background: #2a2a2a;
    color: #ffffff;
    border: 1px solid #3a3a3a;
    padding: 10px 20px;
    cursor: pointer;
    font-size: 14px;
    font-weight: 500;
    border-radius: 8px;
    transition: all 0.3s ease;
    margin-top: 10px;
}

.btn-secondary:hover {
    background: #3a3a3a;
    border-color: #00d4ff;
    color: #00d4ff;
}

.btn-secondary:disabled {
    background: #1a1a1a;
    color: #555;
    border-color: #2a2a2a;
    cursor: not-allowed;
}

.btn-danger {
    background: #ff3b3b;
    color: #fff;
    border: none;
    padding: 8px 14px;
    cursor: pointer;
    font-size: 18px;
    font-weight: bold;
    border-radius: 6px;
    transition: all 0.3s ease;
    min-width: 40px;
    height: 40px;
}

.btn-danger:hover {
    background: #ff5555;
    transform: scale(1.1);
    box-shadow: 0 4px 12px rgba(255, 59, 59, 0.4);
}

.btn.danger {
    background: #ff3b3b;
}

.btn.danger:hover {
    background: #ff5555;
}

.btn.warning {
    background: #ffa500;
    color: #000;
    font-weight: 600;
}

.btn.warning:hover {
    background: #ffb733;
}

/* Tabs */
.tabs {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 8px;
    margin-bottom: 25px;
    background: transparent;
}

.tab {
    background: #1a1a1a;
    color: #888;
    border: 1px solid #2a2a2a;
    padding: 14px;
    cursor: pointer;
    font-size: 13px;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 1px;
    transition: all 0.3s ease;
    border-radius: 8px;
}

.tab:hover {
    background: #2a2a2a;
    color: #00d4ff;
    border-color: #00d4ff;
}

.tab.active {
    background: linear-gradient(135deg, #00d4ff 0%, #0099cc 100%);
    color: #000;
    border-color: #00d4ff;
    font-weight: 700;
}

/* Tab Content */
.tab-content {
    display: none;
}

.tab-content.active {
    display: block;
}

/* Section */
.section {
    border: 1px solid #2a2a2a;
    padding: 25px;
    margin-bottom: 20px;
    background: #141414;
    border-radius: 10px;
}

.section h2 {
    font-size: 20px;
    color: #ffffff;
    margin-bottom: 20px;
    border-bottom: 2px solid #00d4ff;
    padding-bottom: 12px;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.section h3 {
    font-size: 16px;
    color: #ffffff;
    margin-bottom: 15px;
    font-weight: 600;
}

/* Form Elements */
.form-group {
    margin-bottom: 18px;
}

.form-group label {
    display: block;
    margin-bottom: 8px;
    font-size: 13px;
    color: #aaa;
    font-weight: 500;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

input[type="text"],
input[type="password"],
input[type="number"],
select,
textarea {
    width: 100%;
    background: #0a0a0a;
    color: #ffffff;
    border: 1px solid #2a2a2a;
    padding: 12px 16px;
    font-size: 14px;
    border-radius: 8px;
    transition: all 0.3s ease;
}

input[type="text"]:focus,
input[type="password"]:focus,
input[type="number"]:focus,
select:focus,
textarea:focus {
    outline: none;
    border-color: #00d4ff;
    background: #1a1a1a;
    box-shadow: 0 0 0 3px rgba(0, 212, 255, 0.1);
}

input[type="text"]::placeholder,
input[type="password"]::placeholder,
textarea::placeholder {
    color: #555;
}

textarea {
    resize: vertical;
    min-height: 100px;
}

/* Checkbox & Radio */
input[type="checkbox"],
input[type="radio"] {
    margin-right: 8px;
    width: 18px;
    height: 18px;
    cursor: pointer;
}

/* Toggle Switch */
.toggle-container {
    display: flex;
    align-items: center;
    gap: 15px;
    margin: 15px 0;
    padding: 15px;
    background: #f8f9fa;
    border: 1px solid #dee2e6;
    border-radius: 4px;
}

.toggle-container span {
    font-size: 14px;
}

.switch {
    position: relative;
    display: inline-block;
    width: 60px;
    height: 30px;
}

.switch input {
    opacity: 0;
    width: 0;
    height: 0;
}

.slider {
    position: absolute;
    cursor: pointer;
    top: 0;
    left: 0;
    right: 0;
    bottom: 0;
    background-color: #ccc;
    border-radius: 30px;
    transition: .3s;
}

.slider:before {
    position: absolute;
    content: "";
    height: 22px;
    width: 22px;
    left: 4px;
    bottom: 4px;
    background-color: white;
    border-radius: 50%;
    transition: .3s;
}

input:checked + .slider {
    background-color: #007bff;
}

input:checked + .slider:before {
    transform: translateX(30px);
}

/* Warning Box */
.warning {
    background: #fff3cd;
    border: 1px solid #ffc107;
    color: #856404;
    padding: 12px;
    margin: 15px 0;
    font-size: 14px;
    border-radius: 4px;
}

.warning::before {
    content: "⚠️ ";
}

/* Info Box */
.info-box {
    background: #e7f3ff;
    border: 1px solid #b3d7ff;
    padding: 15px;
    margin-bottom: 15px;
    border-radius: 4px;
}

.info-item {
    display: flex;
    justify-content: space-between;
    padding: 8px 0;
    border-bottom: 1px solid #d0e7ff;
    font-size: 14px;
}

.info-item:last-child {
    border-bottom: none;
}

.info-item span:first-child {
    color: #495057;
    font-weight: 500;
}

.info-item span:last-child {
.info-item span:last-child {
    color: #007bff;
    font-weight: 600;
}

/* Info Text */
.info-text {
    padding: 15px;
    background: #e7f3ff;
    border: 1px solid #b3d7ff;
    border-radius: 4px;
    color: #004085;
    font-size: 14px;
    line-height: 1.6;
}

.info-text .highlight {
    color: #dc3545;
    font-weight: 600;
}

/* UID List */
.uid-list {
    max-height: 300px;
    overflow-y: auto;
    border: 1px solid #dee2e6;
    padding: 10px;
    margin-bottom: 15px;
    background: #f8f9fa;
    border-radius: 4px;
}

.uid-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px;
    border-bottom: 1px solid #dee2e6;
    transition: background 0.2s;
    background: white;
    margin-bottom: 5px;
    border-radius: 3px;
}

.uid-item:hover {
    background: #e9ecef;
}

.uid-item:last-child {
    border-bottom: none;
}

.uid-item .mono {
    font-family: 'Consolas', 'Monaco', monospace;
    color: #007bff;
    font-size: 14px;
    font-weight: 600;
}

/* Group Tabs */
.group-tabs {
    display: flex;
    gap: 5px;
    margin-bottom: 15px;
    flex-wrap: wrap;
}

.group-tab {
    background: white;
    color: #495057;
    border: 1px solid #dee2e6;
    padding: 8px 16px;
    cursor: pointer;
    font-size: 13px;
    font-weight: 600;
    border-radius: 8px;
    transition: all 0.3s ease;
}

.group-tab:hover {
    background: #2a2a2a;
    border-color: #00d4ff;
    color: #00d4ff;
}

.group-tab.active {
    background: linear-gradient(135deg, #00d4ff 0%, #0099cc 100%);
    color: #000;
    border-color: #00d4ff;
    font-weight: 700;
}

/* Scrollbar */
::-webkit-scrollbar {
    width: 8px;
    height: 8px;
}

::-webkit-scrollbar-track {
    background: #0a0a0a;
    border-radius: 4px;
}

::-webkit-scrollbar-thumb {
    background: #2a2a2a;
    border-radius: 4px;
}

::-webkit-scrollbar-thumb:hover {
    background: #00d4ff;
}

/* Select Dropdown */
select {
    appearance: none;
    background-image: url('data:image/svg+xml;utf8,<svg fill="%23ffffff" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M7 10l5 5 5-5z"/></svg>');
    background-repeat: no-repeat;
    background-position: right 10px center;
    background-size: 20px;
    padding-right: 40px;
}

select option {
    background: #1a1a1a;
    color: #ffffff;
}

/* Responsive */
@media (max-width: 768px) {
    .container {
        padding: 20px;
    }
    
    .tabs {
        grid-template-columns: repeat(2, 1fr);
    }
    
    .header h1 {
        font-size: 24px;
    }
}

.mono {
    font-family: 'Consolas', 'Monaco', monospace;
    color: #00d4ff;
    font-weight: 600;
}
)rawliteral";
}

// JavaScript generation
String FlexKeyWeb::generateJS() {
    return R"rawliteral(
let currentGroupTab = 0;
let currentURLTab = 0;
let config = {};

// Initialize on load
document.addEventListener('DOMContentLoaded', function() {
    initTabs();
    loadConfig();
    loadSystemInfo();
    startLastUIDPolling();
    checkUIDConflicts();
});

// Tab switching
function initTabs() {
    const tabs = document.querySelectorAll('.tab');
    tabs.forEach(tab => {
        tab.addEventListener('click', function() {
            const tabId = this.getAttribute('data-tab');
            switchTab(tabId);
        });
    });
}

function switchTab(tabId) {
    document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
    document.querySelectorAll('.tab-content').forEach(t => t.classList.remove('active'));
    
    document.querySelector(`[data-tab="${tabId}"]`).classList.add('active');
    document.getElementById(tabId).classList.add('active');
}

// Load configuration
async function loadConfig() {
    try {
        const response = await fetch('/api/config');
        config = await response.json();
        
        document.getElementById('deviceID').textContent = config.deviceID;
        document.getElementById('multiGroupMode').checked = config.multiGroupMode;
        updateGroupModeUI();
        
        if (config.multiGroupMode) {
            loadGlobalRelay();
        }
        
        renderGroupTabs();
        renderGroupURLTabs();
        loadWiFiConfig();
    } catch (error) {
        console.error('Config load error:', error);
    }
}

// Multi-group mode toggle
document.getElementById('multiGroupMode').addEventListener('change', async function() {
    const enabled = this.checked;
    try {
        const response = await fetch('/api/groupmode', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ enabled })
        });
        if (response.ok) {
            config.multiGroupMode = enabled;
            updateGroupModeUI();
            if (enabled) {
                checkUIDConflicts();
            }
        }
    } catch (error) {
        console.error('Group mode error:', error);
    }
});

function updateGroupModeUI() {
    const singleSelect = document.getElementById('singleGroupSelect');
    const relaySection = document.getElementById('relaySection');
    
    if (config.multiGroupMode) {
        singleSelect.style.display = 'none';
        relaySection.style.display = 'block';
    } else {
        singleSelect.style.display = 'block';
        relaySection.style.display = 'none';
        document.getElementById('activeGroup').value = config.activeGroupIndex;
    }
}

async function changeActiveGroup() {
    const groupIndex = parseInt(document.getElementById('activeGroup').value);
    try {
        await fetch('/api/activegroup', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex })
        });
        config.activeGroupIndex = groupIndex;
    } catch (error) {
        console.error('Active group error:', error);
    }
}

// Group tabs rendering
function renderGroupTabs() {
    const container = document.getElementById('groupTabs');
    container.innerHTML = '';
    
    for (let i = 0; i < 5; i++) {
        const btn = document.createElement('button');
        btn.className = 'group-tab' + (i === 0 ? ' active' : '');
        btn.textContent = `GRUP ${i + 1}`;
        btn.onclick = () => selectGroupTab(i);
        container.appendChild(btn);
    }
    
    selectGroupTab(0);
}

function renderGroupURLTabs() {
    const container = document.getElementById('groupTabsURL');
    container.innerHTML = '';
    
    for (let i = 0; i < 5; i++) {
        const btn = document.createElement('button');
        btn.className = 'group-tab' + (i === 0 ? ' active' : '');
        btn.textContent = `GRUP ${i + 1}`;
        btn.onclick = () => selectURLTab(i);
        container.appendChild(btn);
    }
    
    selectURLTab(0);
}

function selectGroupTab(index) {
    currentGroupTab = index;
    document.querySelectorAll('#groupTabs .group-tab').forEach((tab, i) => {
        tab.classList.toggle('active', i === index);
    });
    renderGroupContent(index);
}

function selectURLTab(index) {
    currentURLTab = index;
    document.querySelectorAll('#groupTabsURL .group-tab').forEach((tab, i) => {
        tab.classList.toggle('active', i === index);
    });
    renderURLContent(index);
}

// Group content rendering
async function renderGroupContent(groupIndex) {
    const container = document.getElementById('groupContent');
    const response = await fetch(`/api/groups`);
    const groups = await response.json();
    const group = groups[groupIndex];
    
    let html = `<h3>${group.name}</h3>`;
    html += `<div class="form-group">
                <label>Grup Adı</label>
                <input type="text" id="groupName${groupIndex}" value="${group.name}">
             </div>`;
    
    html += `<div class="uid-list">`;
    if (group.uids.length === 0) {
        html += '<div style="color:#666;padding:10px;">UID yok</div>';
    } else {
        group.uids.forEach((uid, i) => {
            html += `<div class="uid-item">
                        <span class="mono">${uid}</span>
                        <button class="danger" onclick="deleteUID(${groupIndex}, ${i})">SİL</button>
                     </div>`;
        });
    }
    html += `</div>`;
    
    html += `<div class="form-group">
                <label>UID Ekle (manuel)</label>
                <input type="text" id="newUID${groupIndex}" placeholder="AA:BB:CC:DD veya AA:BB:CC:DD:EE:FF:GG">
             </div>`;
    html += `<button onclick="addUID(${groupIndex})">UID EKLE</button>`;
    html += `<button onclick="saveGroup(${groupIndex})">KAYDET</button>`;
    
    container.innerHTML = html;
}

async function renderURLContent(groupIndex) {
    const container = document.getElementById('groupURLContent');
    const response = await fetch(`/api/groups`);
    const groups = await response.json();
    const group = groups[groupIndex];
    
    let html = `<h3>${group.name} - URL & RÖLE AYARLARI</h3>`;
    
    // Relay settings (only in single group mode)
    if (!config.multiGroupMode) {
        html += `<div class="section">
                    <h3>RÖLE AYARLARI</h3>
                    <div class="form-group">
                        <label><input type="checkbox" id="relayEnabled${groupIndex}" ${group.relayEnabled ? 'checked' : ''}> Röle Etkin</label>
                    </div>
                    <div class="form-group">
                        <label><input type="radio" name="relayMode${groupIndex}" value="toggle" ${group.relayToggle ? 'checked' : ''}> Toggle</label>
                        <label><input type="radio" name="relayMode${groupIndex}" value="pulse" ${!group.relayToggle ? 'checked' : ''}> Pulse</label>
                    </div>
                    <div class="form-group">
                        <label>Pulse Süresi (ms)</label>
                        <input type="number" id="relayPulse${groupIndex}" value="${group.relayPulseDuration}" min="100" max="10000">
                    </div>
                    <button onclick="testRelay()">TEST RÖLE</button>
                 </div>`;
    }
    
    // URLs - Dynamic list
    html += `<div class="section">
                <h3>URL LİSTESİ (Max 15)</h3>
                <div id="urlList${groupIndex}"></div>
                <button class="btn-secondary" onclick="addNewURLField(${groupIndex})" id="addURLBtn${groupIndex}">+ YENİ URL EKLE</button>
                <br><br>
                <button class="btn" onclick="saveURLs(${groupIndex})">KAYDET</button>
             </div>`;
    
    container.innerHTML = html;
    
    // Render existing URLs or show one empty field
    renderURLFields(groupIndex, group.urls || []);
}

function renderURLFields(groupIndex, urls) {
    const container = document.getElementById(`urlList${groupIndex}`);
    const addButton = document.getElementById(`addURLBtn${groupIndex}`);
    
    if (!container) return;
    
    // Filter out empty URLs to count actual URLs
    const actualURLs = urls.filter(url => url && url.trim() !== '');
    const urlCount = actualURLs.length;
    
    // Show at least 1 field, or show all saved URLs
    const fieldsToShow = Math.max(1, urlCount);
    
    let html = '';
    for (let i = 0; i < fieldsToShow; i++) {
        const url = urls[i] || '';
        html += `<div class="url-field-group" id="urlField${groupIndex}_${i}">
                    <div style="display: flex; gap: 10px; margin-bottom: 10px;">
                        <div style="flex: 1;">
                            <label>URL ${i + 1}</label>
                            <input type="text" id="url${groupIndex}_${i}" value="${url}" placeholder="http:// veya https://">
                        </div>
                        <button class="btn-danger" onclick="removeURLField(${groupIndex}, ${i})" style="align-self: flex-end; ${i === 0 && fieldsToShow === 1 ? 'visibility: hidden;' : ''}">×</button>
                    </div>
                 </div>`;
    }
    
    container.innerHTML = html;
    
    // Disable add button if we have 15 URLs
    if (fieldsToShow >= 15 && addButton) {
        addButton.disabled = true;
        addButton.textContent = 'MAKSIMUM 15 URL';
    } else if (addButton) {
        addButton.disabled = false;
        addButton.textContent = '+ YENİ URL EKLE';
    }
}

function addNewURLField(groupIndex) {
    const container = document.getElementById(`urlList${groupIndex}`);
    if (!container) return;
    
    const currentFields = container.querySelectorAll('.url-field-group');
    const currentCount = currentFields.length;
    
    if (currentCount >= 15) {
        alert('Maksimum 15 URL ekleyebilirsiniz');
        return;
    }
    
    const newIndex = currentCount;
    const newField = document.createElement('div');
    newField.className = 'url-field-group';
    newField.id = `urlField${groupIndex}_${newIndex}`;
    newField.innerHTML = `<div style="display: flex; gap: 10px; margin-bottom: 10px;">
                            <div style="flex: 1;">
                                <label>URL ${newIndex + 1}</label>
                                <input type="text" id="url${groupIndex}_${newIndex}" value="" placeholder="http:// veya https://">
                            </div>
                            <button class="btn-danger" onclick="removeURLField(${groupIndex}, ${newIndex})" style="align-self: flex-end;">×</button>
                          </div>`;
    
    container.appendChild(newField);
    
    // Update add button state
    const addButton = document.getElementById(`addURLBtn${groupIndex}`);
    if (currentCount + 1 >= 15 && addButton) {
        addButton.disabled = true;
        addButton.textContent = 'MAKSIMUM 15 URL';
    }
    
    // Show remove button on first field if we now have more than 1
    if (currentCount === 0) {
        const firstRemoveBtn = container.querySelector('.btn-danger');
        if (firstRemoveBtn) {
            firstRemoveBtn.style.visibility = 'visible';
        }
    }
}

function removeURLField(groupIndex, fieldIndex) {
    const field = document.getElementById(`urlField${groupIndex}_${fieldIndex}`);
    if (!field) return;
    
    const container = document.getElementById(`urlList${groupIndex}`);
    const currentFields = container.querySelectorAll('.url-field-group');
    
    // Don't allow removing if only 1 field remains
    if (currentFields.length <= 1) {
        // Just clear the input instead
        const input = document.getElementById(`url${groupIndex}_${fieldIndex}`);
        if (input) input.value = '';
        return;
    }
    
    field.remove();
    
    // Re-number remaining fields
    const remainingFields = container.querySelectorAll('.url-field-group');
    remainingFields.forEach((field, index) => {
        const label = field.querySelector('label');
        if (label) {
            label.textContent = `URL ${index + 1}`;
        }
        
        // Update IDs
        const input = field.querySelector('input[type="text"]');
        const oldId = input.id;
        const newId = `url${groupIndex}_${index}`;
        input.id = newId;
        
        field.id = `urlField${groupIndex}_${index}`;
        
        // Update remove button onclick
        const removeBtn = field.querySelector('.btn-danger');
        if (removeBtn) {
            removeBtn.onclick = function() { removeURLField(groupIndex, index); };
            
            // Hide remove button if only 1 field remains
            if (remainingFields.length === 1) {
                removeBtn.style.visibility = 'hidden';
            } else {
                removeBtn.style.visibility = 'visible';
            }
        }
    });
    
    // Update add button state
    const addButton = document.getElementById(`addURLBtn${groupIndex}`);
    if (addButton) {
        addButton.disabled = false;
        addButton.textContent = '+ YENİ URL EKLE';
    }
}

async function saveGroup(groupIndex) {
    const name = document.getElementById(`groupName${groupIndex}`).value;
    try {
        await fetch('/api/group/save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex, name })
        });
        alert('Grup kaydedildi');
        renderGroupContent(groupIndex);
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

async function addUID(groupIndex) {
    const uidInput = document.getElementById(`newUID${groupIndex}`).value.trim();
    if (!uidInput) {
        alert('UID giriniz');
        return;
    }
    
    try {
        const response = await fetch('/api/group/adduid', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex, uid: uidInput })
        });
        
        const result = await response.json();
        if (result.success) {
            alert('UID eklendi');
            document.getElementById(`newUID${groupIndex}`).value = '';
            renderGroupContent(groupIndex);
            checkUIDConflicts();
        } else {
            alert('Hata: ' + result.message);
        }
    } catch (error) {
        alert('Ekleme hatası: ' + error);
    }
}

async function deleteUID(groupIndex, uidIndex) {
    if (!confirm('UID silinsin mi?')) return;
    
    try {
        await fetch('/api/group/deleteuid', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ groupIndex, uidIndex })
        });
        alert('UID silindi');
        renderGroupContent(groupIndex);
        checkUIDConflicts();
    } catch (error) {
        alert('Silme hatası: ' + error);
    }
}

async function saveURLs(groupIndex) {
    const urls = [];
    const container = document.getElementById(`urlList${groupIndex}`);
    
    if (container) {
        // Get all URL input fields dynamically
        const urlFields = container.querySelectorAll('input[type="text"]');
        urlFields.forEach(input => {
            const url = input.value.trim();
            if (url) urls.push(url);
        });
    }
    
    // Pad with empty strings to maintain array size (max 15)
    while (urls.length < 15) {
        urls.push('');
    }
    
    // Get relay settings if single mode
    let relayEnabled = false;
    let relayToggle = true;
    let relayPulse = 500;
    
    if (!config.multiGroupMode) {
        relayEnabled = document.getElementById(`relayEnabled${groupIndex}`).checked;
        relayToggle = document.querySelector(`input[name="relayMode${groupIndex}"]:checked`).value === 'toggle';
        relayPulse = parseInt(document.getElementById(`relayPulse${groupIndex}`).value);
    }
    
    try {
        await fetch('/api/group/save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ 
                groupIndex, 
                urls, 
                relayEnabled, 
                relayToggle, 
                relayPulseDuration: relayPulse 
            })
        });
        alert('Ayarlar kaydedildi');
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

async function loadGlobalRelay() {
    try {
        const response = await fetch('/api/config');
        const data = await response.json();
        document.getElementById('globalRelayEnabled').checked = data.globalRelay.enabled;
        document.querySelector(`input[name="globalRelayMode"][value="${data.globalRelay.toggle ? 'toggle' : 'pulse'}"]`).checked = true;
        document.getElementById('globalRelayPulse').value = data.globalRelay.pulseDuration;
    } catch (error) {
        console.error('Global relay load error:', error);
    }
}

async function saveGlobalRelay() {
    const enabled = document.getElementById('globalRelayEnabled').checked;
    const toggle = document.querySelector('input[name="globalRelayMode"]:checked').value === 'toggle';
    const pulse = parseInt(document.getElementById('globalRelayPulse').value);
    
    try {
        await fetch('/api/globalrelay', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ enabled, toggle, pulseDuration: pulse })
        });
        alert('Global röle ayarları kaydedildi');
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

async function testRelay() {
    try {
        await fetch('/api/relay/test', { method: 'POST' });
        alert('Röle test edildi');
    } catch (error) {
        alert('Test hatası: ' + error);
    }
}

// WiFi configuration
async function loadWiFiConfig() {
    try {
        const response = await fetch('/api/wifi');
        const wifi = await response.json();
        
        // AP Mode
        document.getElementById('apModeEnabled').checked = wifi.apModeEnabled !== false;
        
        // Primary Network
        document.getElementById('primarySSID').value = wifi.primarySSID || '';
        document.getElementById('primaryPassword').value = wifi.primaryPassword || '';
        document.getElementById('primaryUseStaticIP').checked = wifi.primaryUseStaticIP || false;
        document.getElementById('primaryStaticIP').value = wifi.primaryStaticIP || '192.168.1.100';
        document.getElementById('primaryGateway').value = wifi.primaryGateway || '192.168.1.1';
        document.getElementById('primarySubnet').value = wifi.primarySubnet || '255.255.255.0';
        document.getElementById('primaryDNS').value = wifi.primaryDNS || '8.8.8.8';
        
        // Backup Network
        document.getElementById('backupSSID').value = wifi.backupSSID || '';
        document.getElementById('backupPassword').value = wifi.backupPassword || '';
        document.getElementById('backupUseStaticIP').checked = wifi.backupUseStaticIP || false;
        document.getElementById('backupStaticIP').value = wifi.backupStaticIP || '192.168.2.100';
        document.getElementById('backupGateway').value = wifi.backupGateway || '192.168.2.1';
        document.getElementById('backupSubnet').value = wifi.backupSubnet || '255.255.255.0';
        document.getElementById('backupDNS').value = wifi.backupDNS || '8.8.8.8';
        
        // Toggle static IP fields visibility
        togglePrimaryStaticIP();
        toggleBackupStaticIP();
        
    } catch (error) {
        console.error('WiFi config load error:', error);
    }
}

function togglePrimaryStaticIP() {
    const checkbox = document.getElementById('primaryUseStaticIP');
    const fields = document.getElementById('primaryStaticIPFields');
    if (checkbox && fields) {
        fields.style.display = checkbox.checked ? 'block' : 'none';
    }
}

function toggleBackupStaticIP() {
    const checkbox = document.getElementById('backupUseStaticIP');
    const fields = document.getElementById('backupStaticIPFields');
    if (checkbox && fields) {
        fields.style.display = checkbox.checked ? 'block' : 'none';
    }
}

async function saveWiFiConfig() {
    if (!confirm('WiFi ayarları kaydedilecek ve sistem yeniden başlatılacak. Devam edilsin mi?')) {
        return;
    }
    
    const primarySSID = document.getElementById('primarySSID').value.trim();
    const backupSSID = document.getElementById('backupSSID').value.trim();
    const apModeEnabled = document.getElementById('apModeEnabled').checked;
    
    // Validation: If AP mode disabled, WiFi must be configured
    if (!apModeEnabled && primarySSID === '' && backupSSID === '') {
        alert('⚠️ UYARI: AP modu kapatılamaz! En az bir WiFi SSID ayarlanmalıdır.\n\nAP modunu kapatmak için Primary veya Backup WiFi SSID girmelisiniz.');
        return;
    }
    
    const wifi = {
        // Primary Network
        primarySSID: primarySSID,
        primaryPassword: document.getElementById('primaryPassword').value,
        primaryUseStaticIP: document.getElementById('primaryUseStaticIP').checked,
        primaryStaticIP: document.getElementById('primaryStaticIP').value,
        primaryGateway: document.getElementById('primaryGateway').value,
        primarySubnet: document.getElementById('primarySubnet').value,
        primaryDNS: document.getElementById('primaryDNS').value,
        
        // Backup Network
        backupSSID: backupSSID,
        backupPassword: document.getElementById('backupPassword').value,
        backupUseStaticIP: document.getElementById('backupUseStaticIP').checked,
        backupStaticIP: document.getElementById('backupStaticIP').value,
        backupGateway: document.getElementById('backupGateway').value,
        backupSubnet: document.getElementById('backupSubnet').value,
        backupDNS: document.getElementById('backupDNS').value,
        
        // AP Mode
        apModeEnabled: apModeEnabled
    };
    
    try {
        await fetch('/api/wifi/save', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(wifi)
        });
        alert('Ayarlar kaydedildi. Sistem yeniden başlatılıyor...');
        setTimeout(() => location.reload(), 3000);
    } catch (error) {
        alert('Kayıt hatası: ' + error);
    }
}

// System info
async function loadSystemInfo() {
    try {
        const response = await fetch('/api/system');
        const info = await response.json();
        
        document.getElementById('ipAddress').textContent = info.ip;
        document.getElementById('macAddress').textContent = info.mac;
        document.getElementById('totalUIDs').textContent = info.totalUIDs;
        document.getElementById('activeGroupName').textContent = info.activeGroupName;
    } catch (error) {
        console.error('System info error:', error);
    }
}

// Last UID polling
function startLastUIDPolling() {
    setInterval(async () => {
        try {
            const response = await fetch('/api/lastuid');
            const data = await response.json();
            
            const valueElem = document.getElementById('lastUIDValue');
            const timeElem = document.getElementById('lastUIDTime');
            
            if (data.valid && valueElem && timeElem) {
                valueElem.textContent = data.uid;
                valueElem.style.color = '#0f0';
                
                // Show timestamp
                const now = new Date();
                const timeStr = now.toLocaleTimeString('tr-TR');
                timeElem.textContent = 'Son okuma: ' + timeStr;
            } else if (valueElem && timeElem) {
                valueElem.textContent = 'Henüz kart okutulmadı';
                valueElem.style.color = '#666';
                timeElem.textContent = '';
            }
        } catch (error) {
            console.error('Last UID error:', error);
        }
    }, 1000);
}

// UID conflict checking
async function checkUIDConflicts() {
    if (!config.multiGroupMode) {
        document.getElementById('multiGroupWarning').style.display = 'none';
        return;
    }
    
    try {
        const response = await fetch('/api/uidconflicts');
        const data = await response.json();
        
        const warning = document.getElementById('multiGroupWarning');
        if (data.hasConflicts) {
            warning.style.display = 'block';
        } else {
            warning.style.display = 'none';
        }
    } catch (error) {
        console.error('UID conflict check error:', error);
    }
}
)rawliteral";
}

// API Handlers implementation

void FlexKeyWeb::handleAPIGetConfig() {
    JsonDocument doc;
    
    doc["multiGroupMode"] = sysConfig->multiGroupMode;
    doc["activeGroupIndex"] = sysConfig->activeGroupIndex;
    doc["deviceID"] = sysConfig->deviceID;
    
    // Global relay
    JsonObject globalRelay = doc["globalRelay"].to<JsonObject>();
    globalRelay["enabled"] = sysConfig->globalRelay.enabled;
    globalRelay["toggle"] = sysConfig->globalRelay.toggle;
    globalRelay["pulseDuration"] = sysConfig->globalRelay.pulseDuration;
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISetGroupMode() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    bool enabled = doc["enabled"];
    sysConfig->multiGroupMode = enabled;
    storage->saveMultiGroupMode(enabled);
    
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPISetActiveGroup() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    if (groupIndex < MAX_GROUPS) {
        sysConfig->activeGroupIndex = groupIndex;
        storage->saveActiveGroup(groupIndex);
        server->send(200, "application/json", "{\"success\":true}");
    } else {
        server->send(400, "application/json", "{\"error\":\"Invalid group\"}");
    }
}

void FlexKeyWeb::handleAPIGetGroups() {
    JsonDocument doc;
    JsonArray groupsArray = doc.to<JsonArray>();
    
    for (uint8_t i = 0; i < MAX_GROUPS; i++) {
        JsonObject group = groupsArray.add<JsonObject>();
        group["name"] = sysConfig->groups[i].name;
        group["active"] = sysConfig->groups[i].active;
        group["relayEnabled"] = sysConfig->groups[i].relayEnabled;
        group["relayToggle"] = sysConfig->groups[i].relayToggle;
        group["relayPulseDuration"] = sysConfig->groups[i].relayPulseDuration;
        
        // UIDs
        JsonArray uids = group["uids"].to<JsonArray>();
        for (uint16_t j = 0; j < sysConfig->groups[i].uidCount; j++) {
            uids.add(sysConfig->groups[i].uids[j].toString());
        }
        
        // URLs
        JsonArray urls = group["urls"].to<JsonArray>();
        for (uint8_t j = 0; j < sysConfig->groups[i].urlCount; j++) {
            urls.add(sysConfig->groups[i].urls[j]);
        }
    }
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISaveGroup() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    if (groupIndex >= MAX_GROUPS) {
        server->send(400, "application/json", "{\"error\":\"Invalid group\"}");
        return;
    }
    
    // Update group name if provided
    if (doc.containsKey("name")) {
        sysConfig->groups[groupIndex].name = doc["name"].as<String>();
    }
    
    // Update URLs if provided
    if (doc.containsKey("urls")) {
        JsonArray urlsArray = doc["urls"];
        sysConfig->groups[groupIndex].urlCount = 0;
        for (uint8_t i = 0; i < urlsArray.size() && i < MAX_URLS_PER_GROUP; i++) {
            sysConfig->groups[groupIndex].urls[i] = urlsArray[i].as<String>();
            sysConfig->groups[groupIndex].urlCount++;
        }
    }
    
    // Update relay settings if provided (single group mode)
    if (doc.containsKey("relayEnabled")) {
        sysConfig->groups[groupIndex].relayEnabled = doc["relayEnabled"];
    }
    if (doc.containsKey("relayToggle")) {
        sysConfig->groups[groupIndex].relayToggle = doc["relayToggle"];
    }
    if (doc.containsKey("relayPulseDuration")) {
        sysConfig->groups[groupIndex].relayPulseDuration = doc["relayPulseDuration"];
    }
    
    storage->saveGroup(groupIndex, sysConfig->groups[groupIndex]);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIDeleteUID() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    uint16_t uidIndex = doc["uidIndex"];
    
    if (groupIndex >= MAX_GROUPS || uidIndex >= sysConfig->groups[groupIndex].uidCount) {
        server->send(400, "application/json", "{\"error\":\"Invalid indices\"}");
        return;
    }
    
    // Shift UIDs down
    for (uint16_t i = uidIndex; i < sysConfig->groups[groupIndex].uidCount - 1; i++) {
        sysConfig->groups[groupIndex].uids[i] = sysConfig->groups[groupIndex].uids[i + 1];
    }
    sysConfig->groups[groupIndex].uidCount--;
    
    storage->saveGroup(groupIndex, sysConfig->groups[groupIndex]);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIAddUID() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    uint8_t groupIndex = doc["groupIndex"];
    String uidStr = doc["uid"].as<String>();
    
    if (groupIndex >= MAX_GROUPS) {
        server->send(400, "application/json", "{\"error\":\"Invalid group\"}");
        return;
    }
    
    if (sysConfig->groups[groupIndex].uidCount >= MAX_UIDS) {
        server->send(400, "application/json", "{\"error\":\"UID limit reached\"}");
        return;
    }
    
    UID_t newUID;
    if (!newUID.fromString(uidStr)) {
        server->send(400, "application/json", "{\"error\":\"Invalid UID format\"}");
        return;
    }
    
    // Check for duplicates in same group
    for (uint16_t i = 0; i < sysConfig->groups[groupIndex].uidCount; i++) {
        if (sysConfig->groups[groupIndex].uids[i].equals(newUID)) {
            server->send(400, "application/json", "{\"error\":\"UID already exists in this group\"}");
            return;
        }
    }
    
    // Add UID
    sysConfig->groups[groupIndex].uids[sysConfig->groups[groupIndex].uidCount] = newUID;
    sysConfig->groups[groupIndex].uidCount++;
    
    storage->saveGroup(groupIndex, sysConfig->groups[groupIndex]);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIGetWiFiConfig() {
    JsonDocument doc;
    
    // Primary Network
    doc["primarySSID"] = sysConfig->wifi.primarySSID;
    doc["primaryPassword"] = sysConfig->wifi.primaryPassword;
    doc["primaryUseStaticIP"] = sysConfig->wifi.primaryUseStaticIP;
    doc["primaryStaticIP"] = sysConfig->wifi.primaryStaticIP.toString();
    doc["primaryGateway"] = sysConfig->wifi.primaryGateway.toString();
    doc["primarySubnet"] = sysConfig->wifi.primarySubnet.toString();
    doc["primaryDNS"] = sysConfig->wifi.primaryDNS.toString();
    
    // Backup Network
    doc["backupSSID"] = sysConfig->wifi.backupSSID;
    doc["backupPassword"] = sysConfig->wifi.backupPassword;
    doc["backupUseStaticIP"] = sysConfig->wifi.backupUseStaticIP;
    doc["backupStaticIP"] = sysConfig->wifi.backupStaticIP.toString();
    doc["backupGateway"] = sysConfig->wifi.backupGateway.toString();
    doc["backupSubnet"] = sysConfig->wifi.backupSubnet.toString();
    doc["backupDNS"] = sysConfig->wifi.backupDNS.toString();
    
    // AP Mode
    doc["apModeEnabled"] = sysConfig->wifi.apModeEnabled;
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISaveWiFiConfig() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    // Primary Network
    sysConfig->wifi.primarySSID = doc["primarySSID"].as<String>();
    sysConfig->wifi.primaryPassword = doc["primaryPassword"].as<String>();
    sysConfig->wifi.primaryUseStaticIP = doc["primaryUseStaticIP"];
    sysConfig->wifi.primaryStaticIP.fromString(doc["primaryStaticIP"].as<String>());
    sysConfig->wifi.primaryGateway.fromString(doc["primaryGateway"].as<String>());
    sysConfig->wifi.primarySubnet.fromString(doc["primarySubnet"].as<String>());
    sysConfig->wifi.primaryDNS.fromString(doc["primaryDNS"].as<String>());
    
    // Backup Network
    sysConfig->wifi.backupSSID = doc["backupSSID"].as<String>();
    sysConfig->wifi.backupPassword = doc["backupPassword"].as<String>();
    sysConfig->wifi.backupUseStaticIP = doc["backupUseStaticIP"];
    sysConfig->wifi.backupStaticIP.fromString(doc["backupStaticIP"].as<String>());
    sysConfig->wifi.backupGateway.fromString(doc["backupGateway"].as<String>());
    sysConfig->wifi.backupSubnet.fromString(doc["backupSubnet"].as<String>());
    sysConfig->wifi.backupDNS.fromString(doc["backupDNS"].as<String>());
    
    // AP Mode
    sysConfig->wifi.apModeEnabled = doc["apModeEnabled"];
    
    storage->saveWiFiConfig(sysConfig->wifi);
    server->send(200, "application/json", "{\"success\":true}");
    
    delay(1000);
    ESP.restart();
}

void FlexKeyWeb::handleAPIGetSystemInfo() {
    JsonDocument doc;
    
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    
    // Count total UIDs
    uint16_t totalUIDs = 0;
    for (uint8_t i = 0; i < MAX_GROUPS; i++) {
        totalUIDs += sysConfig->groups[i].uidCount;
    }
    doc["totalUIDs"] = totalUIDs;
    
    // Active group name
    if (sysConfig->multiGroupMode) {
        doc["activeGroupName"] = "Çoklu Grup Modu";
    } else {
        doc["activeGroupName"] = sysConfig->groups[sysConfig->activeGroupIndex].name;
    }
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPITestRelay() {
    relay->pulse(500);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPIGetLastUID() {
    JsonDocument doc;
    
    if (lastUID->isExpired() || !lastUID->uid.isValid) {
        doc["valid"] = false;
        doc["uid"] = "";
    } else {
        doc["valid"] = true;
        doc["uid"] = lastUID->uid.toString();
    }
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

void FlexKeyWeb::handleAPISaveGlobalRelay() {
    if (!server->hasArg("plain")) {
        server->send(400, "application/json", "{\"error\":\"No data\"}");
        return;
    }
    
    JsonDocument doc;
    deserializeJson(doc, server->arg("plain"));
    
    sysConfig->globalRelay.enabled = doc["enabled"];
    sysConfig->globalRelay.toggle = doc["toggle"];
    sysConfig->globalRelay.pulseDuration = doc["pulseDuration"];
    
    storage->saveGlobalRelay(sysConfig->globalRelay);
    server->send(200, "application/json", "{\"success\":true}");
}

void FlexKeyWeb::handleAPICheckUIDConflicts() {
    JsonDocument doc;
    String message;
    bool hasConflicts = checkUIDConflicts(message);
    
    doc["hasConflicts"] = hasConflicts;
    doc["message"] = message;
    
    String output;
    serializeJson(doc, output);
    server->send(200, "application/json", output);
}

// Helper functions

String FlexKeyWeb::jsonEscape(const String& str) {
    String escaped = str;
    escaped.replace("\\", "\\\\");
    escaped.replace("\"", "\\\"");
    escaped.replace("\n", "\\n");
    escaped.replace("\r", "\\r");
    escaped.replace("\t", "\\t");
    return escaped;
}

bool FlexKeyWeb::checkUIDConflicts(String& conflictMessage) {
    if (!sysConfig->multiGroupMode) {
        return false;
    }
    
    bool hasConflicts = false;
    conflictMessage = "";
    
    // Check each UID across all groups
    for (uint8_t g1 = 0; g1 < MAX_GROUPS; g1++) {
        for (uint16_t u1 = 0; u1 < sysConfig->groups[g1].uidCount; u1++) {
            int conflictCount = 0;
            
            for (uint8_t g2 = 0; g2 < MAX_GROUPS; g2++) {
                if (g1 == g2) continue;
                
                for (uint16_t u2 = 0; u2 < sysConfig->groups[g2].uidCount; u2++) {
                    if (sysConfig->groups[g1].uids[u1].equals(sysConfig->groups[g2].uids[u2])) {
                        conflictCount++;
                        hasConflicts = true;
                    }
                }
            }
            
            if (conflictCount > 0) {
                conflictMessage += sysConfig->groups[g1].uids[u1].toString() + " (" + String(conflictCount + 1) + " grup), ";
            }
        }
    }
    
    return hasConflicts;
}

