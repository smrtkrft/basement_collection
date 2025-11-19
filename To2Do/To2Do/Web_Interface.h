#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "Web_CSS.h"

const char* getHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="tr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1.0">
<title id="page-title">To2Do - SmartKraft</title>
<link rel="stylesheet" href="/app.css">
</head>
<body>
<div class="workspace">
<div class="sidebar">
<div class="sidebar-header">
<div class="sidebar-title" id="app-title">To2Do - SmartKraft</div>
<div class="sidebar-stats">
<div class="stat"><div class="stat-value" id="total-tasks">0</div><div class="stat-label">TOPLAM</div></div>
<div class="stat"><div class="stat-value" id="active-tasks">0</div><div class="stat-label">AKTIF</div></div>
<div class="stat"><div class="stat-value" id="total-projects">0</div><div class="stat-label">PROJE</div></div>
</div>
</div>
<div class="sidebar-tabs">
<button class="sidebar-tab" data-category="Work" id="tab-cat1">WORK</button>
<button class="sidebar-tab active" data-category="Personal" id="tab-cat2">PERSONAL</button>
<button class="sidebar-tab" data-category="Projects" id="tab-cat3">PROJECTS</button>
</div>
<div class="sidebar-search"><input type="text" class="project-search" id="project-search" placeholder="Proje ara..."></div>
<div class="sidebar-body"><div class="projects-list" id="projects-list"></div></div>
<div class="sidebar-footer">
<button class="sidebar-btn" id="new-project-btn">+ YENI PROJE</button>
<button class="sidebar-btn" id="settings-btn">AYARLAR</button>
</div>
</div>
<div class="panel">
<div class="panel-header">
<div class="panel-title-row">
<div style="text-align:center;width:100%;"><div class="panel-title" id="current-project">PROJE SEC</div><div class="panel-subtitle" id="current-project-desc"></div></div>
</div>
<div class="panel-filters" style="justify-content:center;">
<button class="panel-filter-btn active" data-filter="all">TUMU</button>
<button class="panel-filter-btn" data-filter="active">AKTIF</button>
<button class="panel-filter-btn" data-filter="completed">TAMAMLANAN</button>
<button class="panel-filter-btn" data-filter="task">GOREV</button>
<button class="panel-filter-btn" data-filter="plan">PLAN</button>
<button class="panel-filter-btn" data-filter="note">NOT</button>
<button class="panel-filter-btn" data-filter="reminder">HATIRLATMA</button>
</div>
</div>
<div class="panel-body"><div class="tasks-container" id="tasks-container"><div class="empty-state"><h3 id="initial-no-project">PROJE SECILMEDI</h3><p id="initial-select-project">Sol taraftan bir proje secin</p></div></div></div>
</div>
</div>
<button class="fab notification-fab" id="notification-btn" style="right:110px;">!<span id="notification-badge" class="notification-badge"></span></button>
<button class="fab" id="fab-new-task">+</button>
<div class="modal" id="task-modal">
<div class="modal-content">
<div class="modal-title">YENI GOREV</div>
<input type="text" class="modal-input" id="modal-task-title" placeholder="Baslik...">
<textarea class="modal-textarea" id="modal-task-desc" placeholder="Aciklama, detaylar, notlar..."></textarea>
<div class="modal-form-row">
<select class="modal-select" id="modal-task-type"><option value="task">Gorev</option><option value="plan">Plan</option><option value="note">Not</option><option value="reminder">Hatirlatma</option></select>
<select class="modal-select" id="modal-task-priority"><option value="low">Dusuk</option><option value="medium" selected>Orta</option><option value="high">Yuksek</option></select>
<input type="date" class="modal-date" id="modal-task-date">
</div>
<div class="modal-actions">
<button class="modal-btn" id="modal-task-cancel">IPTAL</button>
<button class="modal-btn primary" id="modal-task-create">OLUSTUR</button>
</div>
</div>
</div>
<div class="modal" id="edit-task-modal">
<div class="modal-content">
<div class="modal-title">GOREVI DUZENLE</div>
<input type="text" class="modal-input" id="edit-task-title" placeholder="Baslik...">
<textarea class="modal-textarea" id="edit-task-desc" placeholder="Aciklama, detaylar, notlar..."></textarea>
<div class="modal-form-row">
<select class="modal-select" id="edit-task-type"><option value="task">Gorev</option><option value="plan">Plan</option><option value="note">Not</option><option value="reminder">Hatirlatma</option></select>
<select class="modal-select" id="edit-task-priority"><option value="low">Dusuk</option><option value="medium">Orta</option><option value="high">Yuksek</option></select>
<input type="date" class="modal-date" id="edit-task-date">
</div>
<div class="modal-actions">
<button class="modal-btn" id="edit-task-cancel">IPTAL</button>
<button class="modal-btn primary" id="edit-task-save">KAYDET</button>
</div>
</div>
</div>
<div class="modal" id="project-modal">
<div class="modal-content">
<div class="modal-title">YENI PROJE</div>
<input type="text" class="modal-input" id="modal-project-name" placeholder="Proje adi...">
<textarea class="modal-textarea" id="modal-project-desc" placeholder="Proje aciklamasi..."></textarea>
<select class="modal-select" id="modal-project-priority"><option value="low">Dusuk Oncelik</option><option value="medium" selected>Orta Oncelik</option><option value="high">Yuksek Oncelik</option></select>
<div class="modal-actions">
<button class="modal-btn" id="modal-project-cancel">IPTAL</button>
<button class="modal-btn primary" id="modal-project-create">KAYDET</button>
<button class="modal-btn primary" id="modal-project-update" style="display:none;">GUNCELLE</button>
</div>
</div>
</div>
<div class="modal" id="notification-modal">
<div class="modal-content">
<div class="modal-title">BILDIRIMLER</div>
<div class="settings-tabs">
<button class="settings-tab active" data-tab="notif-today">BUGUN</button>
<button class="settings-tab" data-tab="notif-tomorrow">YARIN</button>
<button class="settings-tab" data-tab="notif-week">BU HAFTA</button>
<button class="settings-tab" data-tab="notif-overdue">GECMIS</button>
</div>
<div class="settings-content">
<div class="settings-tab-content active" data-content="notif-today" id="notif-today-content">
<p style="text-align:center;color:#666;">Loading...</p>
</div>
<div class="settings-tab-content" data-content="notif-tomorrow" id="notif-tomorrow-content">
<p style="text-align:center;color:#666;">Loading...</p>
</div>
<div class="settings-tab-content" data-content="notif-week" id="notif-week-content">
<p style="text-align:center;color:#666;">Loading...</p>
</div>
<div class="settings-tab-content" data-content="notif-overdue" id="notif-overdue-content">
<p style="text-align:center;color:#666;">Loading...</p>
</div>
</div>
<div class="modal-actions">
<button class="modal-btn" onclick="closeNotificationModal()">KAPAT</button>
</div>
</div>
</div>
<div class="modal" id="settings-modal">
<div class="modal-content">
<div class="modal-title">AYARLAR</div>
<div class="settings-tabs">
<button class="settings-tab active" data-tab="gui">GUI</button>
<button class="settings-tab" data-tab="connection">BAGLANTI</button>
<button class="settings-tab" data-tab="info">INFO</button>
</div>
<div class="settings-content">
<div class="settings-tab-content active" data-content="gui">
<div class="settings-section" style="text-align:center;">
<label class="settings-label">TEMA & DIL</label>
<div style="display:flex;justify-content:space-between;align-items:center;gap:20px;">
<div class="theme-toggle" style="flex:1;">
<span class="theme-label">ACIK</span>
<label class="toggle-switch">
<input type="checkbox" id="theme-toggle" checked>
<span class="toggle-slider"></span>
</label>
<span class="theme-label">KOYU</span>
</div>
<div class="language-selector" style="display:flex;gap:8px;">
<button class="lang-btn active" data-lang="EN" id="lang-btn-en">EN</button>
<button class="lang-btn" data-lang="DE" id="lang-btn-de">DE</button>
<button class="lang-btn" data-lang="TR" id="lang-btn-tr">TR</button>
</div>
</div>
</div>
<div class="settings-section">
<label class="settings-label">UYGULAMA BASLIGI</label>
<input type="text" class="settings-input" id="setting-app-title" placeholder="To2Do - SmartKraft">
</div>
<div class="settings-section">
<label class="settings-label">KATEGORI ISIMLERI</label>
<input type="text" class="settings-input" id="setting-cat1" placeholder="Kategori 1">
<input type="text" class="settings-input" id="setting-cat2" placeholder="Kategori 2">
<input type="text" class="settings-input" id="setting-cat3" placeholder="Kategori 3">
</div>
<div class="settings-section">
<label class="settings-label" id="datetime-label">TARIH / SAAT</label>
<div style="display:grid;grid-template-columns:1fr 1fr;gap:15px;align-items:center;padding:20px;background:var(--datetime-bg);border:1px solid var(--datetime-border);border-radius:8px;">
<div style="text-align:center;">
<span class="datetime-sublabel" id="date-sublabel" style="display:block;font-size:11px;color:var(--datetime-label);margin-bottom:8px;letter-spacing:1px;">TARIH</span>
<span id="current-date" style="display:block;font-size:18px;color:var(--datetime-text);font-weight:bold;font-family:'Courier New',monospace;">--.--.----</span>
</div>
<div style="text-align:center;">
<span class="datetime-sublabel" id="time-sublabel" style="display:block;font-size:11px;color:var(--datetime-label);margin-bottom:8px;letter-spacing:1px;">SAAT</span>
<span id="current-time" style="display:block;font-size:18px;color:var(--datetime-text);font-weight:bold;font-family:'Courier New',monospace;">--:--</span>
</div>
</div>
</div>
</div>

<div class="settings-tab-content" data-content="connection">
<div class="accordion">
<div class="accordion-item">
<div class="accordion-header" data-accordion="ap-mode">
<span class="accordion-title">▼ AP MODE SETTINGS</span>
<span class="accordion-status" id="ap-status">Active</span>
</div>
<div class="accordion-content active" data-content="ap-mode">
<div class="settings-section">
<label class="settings-label" id="label-ap-ssid">AP SSID (Access Point Name)</label>
            <input type="text" class="settings-input" id="setting-ap-ssid" placeholder="SmartKraft-To2Do" value="SmartKraft-To2Do">
<p class="settings-hint" id="hint-ap-no-password">No password required. Open network for easy first-time setup.</p>
</div>
<div class="settings-section">
<label class="settings-label" id="label-ap-mdns">mDNS HOSTNAME (AP Mode)</label>
            <input type="text" class="settings-input" id="setting-ap-mdns" placeholder="smartkraft-to2do" value="smartkraft-to2do">
            <p class="settings-hint" id="hint-ap-mdns-access">Access via: http://<span id="ap-mdns-preview">smartkraft-to2do</span>.local</p>
</div>
</div>
</div>
<div class="accordion-item">
<div class="accordion-header" data-accordion="primary-wifi">
<span class="accordion-title">▶ PRIMARY WIFI</span>
<span class="accordion-status" id="primary-status">Not Connected</span>
</div>
<div class="accordion-content" data-content="primary-wifi">
<div class="settings-section">
<label class="settings-label" id="label-primary-ssid">WiFi SSID (Network Name)</label>
<input type="text" class="settings-input" id="setting-primary-ssid" placeholder="Your WiFi Name">
</div>
<div class="settings-section">
<label class="settings-label" id="label-primary-password">WiFi PASSWORD</label>
<input type="password" class="settings-input" id="setting-primary-password" placeholder="••••••••">
<label class="settings-checkbox">
<input type="checkbox" id="show-primary-password">
<span id="checkbox-show-password-primary">Show Password</span>
</label>
</div>
<div class="settings-section">
<label class="settings-label" id="label-primary-static-ip">STATIC IP ADDRESS</label>
<input type="text" class="settings-input" id="setting-primary-ip" placeholder="192.168.1.100">
<p class="settings-hint" id="hint-primary-dhcp">Leave empty for DHCP (automatic)</p>
</div>
<div class="settings-section">
<label class="settings-label" id="label-primary-mdns">mDNS HOSTNAME</label>
            <input type="text" class="settings-input" id="setting-primary-mdns" placeholder="smartkraft-to2do">
            <p class="settings-hint" id="hint-primary-mdns-access">Access via: http://<span id="primary-mdns-preview">smartkraft-to2do</span>.local</p>
</div>
<button class="settings-btn" id="test-primary-wifi">TEST CONNECTION</button>
</div>
</div>
<div class="accordion-item">
<div class="accordion-header" data-accordion="backup-wifi">
<span class="accordion-title">▶ BACKUP WIFI</span>
<span class="accordion-status" id="backup-status">Not Configured</span>
</div>
<div class="accordion-content" data-content="backup-wifi">
<div class="settings-section">
<label class="settings-label" id="label-backup-ssid">WiFi SSID (Network Name)</label>
<input type="text" class="settings-input" id="setting-backup-ssid" placeholder="Backup WiFi Name">
</div>
<div class="settings-section">
<label class="settings-label" id="label-backup-password">WiFi PASSWORD</label>
<input type="password" class="settings-input" id="setting-backup-password" placeholder="••••••••">
<label class="settings-checkbox">
<input type="checkbox" id="show-backup-password">
<span id="checkbox-show-password-backup">Show Password</span>
</label>
</div>
<div class="settings-section">
<label class="settings-label" id="label-backup-static-ip">STATIC IP ADDRESS</label>
<input type="text" class="settings-input" id="setting-backup-ip" placeholder="192.168.2.100">
<p class="settings-hint" id="hint-backup-dhcp">Leave empty for DHCP (automatic)</p>
</div>
<div class="settings-section">
<label class="settings-label" id="label-backup-mdns">mDNS HOSTNAME</label>
            <input type="text" class="settings-input" id="setting-backup-mdns" placeholder="smartkraft-to2do-backup">
            <p class="settings-hint" id="hint-backup-mdns-access">Access via: http://<span id="backup-mdns-preview">smartkraft-to2do-backup</span>.local</p>
</div>
<button class="settings-btn" id="test-backup-wifi">TEST CONNECTION</button>
</div>
</div>
</div>
<div class="connection-info">
<h4 class="connection-info-title" id="conn-status-title">CURRENT CONNECTION STATUS</h4>
<div class="connection-status-grid">
<div class="connection-status-item">
<span class="status-label" id="status-label-mode">Mode:</span>
<span class="status-value" id="current-mode">AP Mode</span>
</div>
<div class="connection-status-item">
<span class="status-label" id="status-label-connected">Connected To:</span>
<span class="status-value" id="current-ssid">-</span>
</div>
<div class="connection-status-item">
<span class="status-label" id="status-label-ip">IP Address:</span>
<span class="status-value" id="current-ip">192.168.4.1</span>
</div>
<div class="connection-status-item">
<span class="status-label" id="status-label-signal">Signal Strength:</span>
<span class="status-value" id="current-signal">-</span>
</div>
</div>
</div>
</div>
<div class="settings-tab-content" data-content="info">
<div class="info-status-bar">
<div class="info-status-item">
<span class="info-status-label">Versiyon:</span>
<span class="info-status-value">SmartKraft-To2Do V1.1</span>
</div>
<div class="info-status-item">
<span class="info-status-label">WiFi:</span>
<span class="info-status-value" id="info-wifi-status">AP Mode</span>
</div>
<div class="info-status-item">
<span class="info-status-label">IP:</span>
<span class="info-status-value" id="info-ip">192.168.4.1</span>
</div>
<div class="info-status-item">
<span class="info-status-label">mDNS:</span>
<span class="info-status-value" id="info-mdns">smartkraft-to2do.local</span>
</div>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-quick">HIZLI BASLANGIC</h3>
<p class="info-text" id="info-text-quick-1">1. Sol taraftan bir kategori secin (<span id="info-cat1">KISISEL</span>, <span id="info-cat2">SMARTKRAFT</span>, <span id="info-cat3">GENEL</span>)</p>
<p class="info-text" id="info-text-quick-2">2. "+ YENI PROJE" butonu ile proje olusturun</p>
<p class="info-text" id="info-text-quick-3">3. Proje ustune tiklayarak secin</p>
<p class="info-text" id="info-text-quick-4">4. Sag alttaki "+" butonu ile gorev ekleyin</p>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-project">PROJE YONETIMI</h3>
<p class="info-text" id="info-text-project-create"><strong>Proje Olusturma:</strong> Sol alttaki "+ YENI PROJE" butonunu kullanin</p>
<p class="info-text" id="info-text-project-edit"><strong>Proje Duzenleme:</strong> Proje uzerine sag tiklayip "DUZENLE" secin</p>
<p class="info-text" id="info-text-project-copy"><strong>Proje Kopyalama:</strong> Sag tikla > "KOPYALA" ile ayni projeden kopya olusturun</p>
<p class="info-text" id="info-text-project-archive"><strong>Proje Arsivleme:</strong> Sag tikla > "ARSIVLE" ile projeyi arsive tasiyin</p>
<p class="info-text" id="info-text-project-delete"><strong>Proje Silme:</strong> Sag tikla > "SIL" ile projeyi kalici olarak silin</p>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-task">GOREV YONETIMI</h3>
<p class="info-text" id="info-text-task-add"><strong>Gorev Ekleme:</strong> Sag alttaki beyaz "+" butonuna tiklayin</p>
<p class="info-text" id="info-text-task-complete"><strong>Gorev Tamamlama:</strong> Gorevin solundaki kutuya tiklayarak tamamlandi isareti koyun</p>
<p class="info-text" id="info-text-task-types"><strong>Gorev Turleri:</strong> Gorev, Plan, Not, Hatirlatma olarak 4 turde olusturabilirsiniz</p>
<p class="info-text" id="info-text-task-priority"><strong>Oncelik Seviyeleri:</strong> Dusuk (Yesil), Orta (Sari), Yuksek (Kirmizi)</p>
<p class="info-text" id="info-text-task-subtasks"><strong>Alt Gorevler:</strong> Her gorevin icine checklist ekleyebilirsiniz</p>
<p class="info-text" id="info-text-task-dependencies"><strong>Bagimlilik:</strong> Gorevler arasi bagimlilik ekleyerek is akisini yonetin</p>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-filter">FILTRELEME</h3>
<p class="info-text" id="info-text-filter-all"><strong>TUMU:</strong> Tum gorevleri gosterir</p>
<p class="info-text" id="info-text-filter-active"><strong>AKTIF:</strong> Sadece tamamlanmamis gorevler</p>
<p class="info-text" id="info-text-filter-completed"><strong>TAMAMLANAN:</strong> Sadece tamamlanmis gorevler</p>
<p class="info-text" id="info-text-filter-types"><strong>GOREV/PLAN/NOT/HATIRLATMA:</strong> Turune gore filtreleme</p>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-search">ARAMA VE NAVIGASYON</h3>
<p class="info-text" id="info-text-search"><strong>Proje Arama:</strong> Sol ust kosedeki arama kutusunu kullanin</p>
<p class="info-text" id="info-text-category-switch"><strong>Kategori Degistirme:</strong> Ust sekmelerden (<span id="info-cat1-nav">KISISEL</span>, <span id="info-cat2-nav">SMARTKRAFT</span>, <span id="info-cat3-nav">GENEL</span>) birini secin</p>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-shortcuts">KISAYOLLAR</h3>
<p class="info-text" id="info-text-right-click"><strong>Sag Tik:</strong> Projeler uzerinde sag tik menusu</p>
<p class="info-text" id="info-text-double-click"><strong>Cift Tik:</strong> Gorev baslik veya aciklama uzerinde duzenlemek icin</p>
<p class="info-text" id="info-text-fab-button"><strong>FAB Butonu:</strong> Her zaman gorunen "+" butonu ile hizli gorev ekleme</p>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-customization">KISISELLESTIRME</h3>
<p class="info-text" id="info-text-gui-tab"><strong>GUI Sekmesi:</strong> Uygulama basligini, kategori isimlerini ve temayi degistirin</p>
<p class="info-text" id="info-text-theme"><strong>Tema:</strong> Acik/Koyu tema arasi gecis yapabilirsiniz</p>
<p class="info-text" id="info-text-storage"><strong>Kayit:</strong> Tum ayarlar otomatik olarak tarayicida saklanir</p>
</div>
<div class="info-section">
<h3 class="info-title" id="info-title-data-storage">VERI SAKLAMA</h3>
<p class="info-text" id="info-text-data-desc">Tum proje, gorev ve ayarlar ESP32 cihazinda SPIFFS hafizasinda saklanir.</p>
<p class="info-text" id="info-text-data-persistent">Elektrik kesilse ve firmware tekrar yuklenme yapilsa bile verileriniz korunur.</p>
<p class="info-text" id="info-text-permanent-storage"><strong>Kalici Saklama:</strong> WiFi ayarlari, tema, kategoriler her zaman korunur.</p>
</div>
<div class="accordion">
<div class="accordion-item">
<div class="accordion-header" data-accordion="backup">
<span class="accordion-title" id="accordion-title-backup">▶ BACKUP / RESTORE</span>
<span class="accordion-status" id="accordion-status-backup-safe">Safe</span>
</div>
<div class="accordion-content" data-content="backup">
<div class="info-section" style="border-left:3px solid #444;margin:0;">
<p class="info-text" id="info-backup-desc"><strong>Backup your data (WiFi settings excluded)</strong></p>
<p class="info-text" id="info-backup-included">Included: Projects, Tasks, GUI Settings</p>
<p class="info-text" id="info-backup-excluded">Excluded: WiFi credentials (for security)</p>
<div style="display:grid;grid-template-columns:1fr 1fr;gap:10px;margin-top:15px;">
<button class="settings-btn" onclick="fetch('/api/backup/export').then(r=>r.text()).then(data=>{const blob=new Blob([data],{type:'application/json'});const url=window.URL.createObjectURL(blob);const a=document.createElement('a');a.href=url;a.download='To2Do_Backup.json';document.body.appendChild(a);a.click();document.body.removeChild(a);window.URL.revokeObjectURL(url);})" style="margin:0;" id="btn-backup-export">EXPORT</button>
<button class="settings-btn" onclick="document.getElementById('backup-upload').click()" style="margin:0;" id="btn-backup-import">IMPORT</button>
</div>
<input type="file" id="backup-upload" accept=".json" style="display:none" onchange="if(this.files[0]){const f=new FileReader();f.onload=e=>{if(confirm('Restore backup? Current data will be overwritten.')){fetch('/api/backup/import',{method:'POST',headers:{'Content-Type':'application/json'},body:e.target.result}).then(r=>r.json()).then(d=>{alert(d.success?'Backup restored!':'Failed: '+d.error);if(d.success)location.reload();})}};f.readAsText(this.files[0])}">
</div>
</div>
</div>
<div class="accordion-item">
<div class="accordion-header" data-accordion="factory-reset">
<span class="accordion-title" id="accordion-title-factory">▶ FACTORY RESET</span>
<span class="accordion-status error" id="accordion-status-factory-danger">Tehlikeli</span>
</div>
<div class="accordion-content" data-content="factory-reset">
<div class="info-section" style="border-left:3px solid #dc3545;margin:0;">
<p class="info-text" id="info-factory-warning">Bu islem <strong>TUM verileri</strong> kalici olarak silecektir:</p>
<p class="info-text" id="info-factory-projects">✗ Tum projeler ve gorevler</p>
<p class="info-text" id="info-factory-gui">✗ GUI ayarlari (tema, kategoriler)</p>
<p class="info-text" id="info-factory-wifi">✗ WiFi network ayarlari</p>
<p class="info-text" style="margin-top:15px;color:#dc3545;" id="info-factory-irreversible"><strong>Bu islem geri alinamaz!</strong></p>
<button onclick="app.confirmFactoryReset()" style="margin-top:15px;padding:12px 24px;background:#dc3545;color:#fff;border:2px solid #dc3545;border-radius:4px;cursor:pointer;font-weight:bold;font-size:14px;width:100%;" id="btn-factory-reset">FACTORY RESET</button>
</div>
</div>
</div>
</div>
<div class="info-footer">
<p class="info-footer-text" id="info-footer-more">Daha fazla bilgi icin:</p>
<a href="https://smartkraft.ch/to2do" target="_blank" class="info-footer-link">smartkraft.ch/to2do →</a>
</div>
</div>
</div>
<div class="modal-actions">
<button class="modal-btn" id="modal-settings-cancel">IPTAL</button>
<button class="modal-btn primary" id="modal-settings-save">KAYDET</button>
</div>
</div>
</div>
<div class="context-menu" id="context-menu">
<div class="context-menu-item" data-action="edit">DUZENLE</div>
<div class="context-menu-item" data-action="duplicate">KOPYALA</div>
<div class="context-menu-item" data-action="archive">ARSIVLE</div>
<div class="context-menu-item danger" data-action="delete">SIL</div>
</div>
<script>
function openNotificationModal(){
const modal=document.getElementById('notification-modal');
modal.classList.add('active');

// Sync browser date before loading notifications to ensure accuracy
console.log('[Notification] Syncing browser date before loading...');
syncBrowserDateToDevice();

// Small delay to ensure date is synced before loading notifications
setTimeout(() => {
  document.querySelectorAll('[data-tab^="notif-"]').forEach(t=>t.classList.remove('active'));
  document.querySelector('[data-tab="notif-today"]')?.classList.add('active');
  document.querySelectorAll('[id^="notif-"][id$="-content"]').forEach(c=>c.style.display='none');
  document.getElementById('notif-today-content').style.display='block';
  loadNotifications('today');
}, 300);
}
function closeNotificationModal(){
document.getElementById('notification-modal').classList.remove('active');
}
function loadNotifications(type){
const contentId='notif-'+type+'-content';
const contentEl=document.getElementById(contentId);
if(!contentEl)return;
contentEl.innerHTML='<p style="text-align:center;color:#666;">Loading...</p>';
fetch('/api/notifications/'+type)
.then(r=>r.json())
.then(d=>{
console.log('Notification response:',d);
if(d.error){
contentEl.innerHTML='<p style="text-align:center;color:#f00;">'+d.error+'</p>';
return;
}
if(d.count===0){
contentEl.innerHTML='<p style="text-align:center;color:#888;padding:40px;">No tasks found</p>';
return;
}

// Category mapping: Convert internal category names to user-defined display names
// Internal categories: "Work", "Personal", "Projects"
// User can customize these in settings as category1, category2, category3
const categoryMapping = {
  'Work': app.settings.category1,
  'Personal': app.settings.category2,
  'Projects': app.settings.category3
};

let html='';
const cats=d.categories||{};
console.log('Categories:',cats);
console.log('Category settings:', app.settings);
for(const catName in cats){
const projects=cats[catName];
if(!projects||typeof projects!=='object'){
console.warn('Invalid projects for category:',catName,projects);
continue;
}
const projectCount=Object.keys(projects).length;

// Calculate total task count for this category
let totalTaskCount = 0;
for(const projName in projects){
const tasks=projects[projName];
if(tasks && Array.isArray(tasks)){
totalTaskCount += tasks.length;
}
}

// Map category name to user-defined name
const displayCatName = categoryMapping[catName] || catName;
console.log('Category mapping:', catName, '->', displayCatName);

const projectLabel = projectCount === 1 ? t('notif_project') : t('notif_projects');
const taskLabel = totalTaskCount === 1 ? t('notif_task') : t('notif_tasks');
html+='<div class="accordion-item"><div class="accordion-header" data-accordion="notif-'+catName+'"><span class="accordion-title">▶ '+displayCatName+'</span><span class="accordion-status">'+projectCount+' '+projectLabel+' · '+totalTaskCount+' '+taskLabel+'</span></div><div class="accordion-content" data-content="notif-'+catName+'">';
for(const projName in projects){
const tasks=projects[projName];
if(!tasks||!Array.isArray(tasks)){
console.warn('Invalid tasks for project:',projName,tasks);
continue;
}
html+='<div style="margin-bottom:15px;"><strong>'+projName+'</strong> ('+tasks.length+')<ul style="margin:5px 0 0 20px;padding:0;">';
tasks.forEach(t=>{
html+='<li style="margin:5px 0;color:#aaa;">'+t.title+' <span style="font-size:11px;color:#666;">('+t.date+')</span></li>';
});
html+='</ul></div>';
}
html+='</div></div>';
}
contentEl.innerHTML='<div class="accordion">'+html+'</div>';
setTimeout(()=>{
contentEl.querySelectorAll('.accordion-header').forEach(header=>{
header.addEventListener('click',e=>{
const accordionName=e.currentTarget.dataset.accordion;
const content=contentEl.querySelector('.accordion-content[data-content="'+accordionName+'"]');
if(!content)return;
const isActive=content.classList.contains('active');
if(isActive){
content.classList.remove('active');
header.classList.remove('active');
const titleEl=header.querySelector('.accordion-title');
if(titleEl)titleEl.textContent=titleEl.textContent.replace('▼','▶');
}else{
content.classList.add('active');
header.classList.add('active');
const titleEl=header.querySelector('.accordion-title');
if(titleEl)titleEl.textContent=titleEl.textContent.replace('▶','▼');
}
});
});
},100);
updateNotificationBadge();
})
.catch(e=>{
console.error('Notification error:',e);
contentEl.innerHTML='<p style="text-align:center;color:#f00;">Error loading notifications</p>';
});
}
function updateNotificationBadge(){
fetch('/api/notifications/today').then(r=>r.json()).then(d=>{
const badge=document.getElementById('notification-badge');
if(d.count>0){
badge.textContent=d.count;
badge.style.display='flex';
}else{
badge.style.display='none';
}
});
}
document.getElementById('notification-btn')?.addEventListener('click',openNotificationModal);
document.querySelectorAll('[data-tab^="notif-"]').forEach(tab=>{
tab.addEventListener('click',e=>{
const type=e.target.dataset.tab.replace('notif-','');
document.querySelectorAll('[data-tab^="notif-"]').forEach(t=>t.classList.remove('active'));
e.target.classList.add('active');
document.querySelectorAll('[id^="notif-"][id$="-content"]').forEach(c=>c.style.display='none');
document.getElementById('notif-'+type+'-content').style.display='block';
loadNotifications(type);
});
});
setTimeout(()=>{
updateNotificationBadge();
const offset=new Date().getTimezoneOffset()/-60;
fetch('/api/notifications/timezone',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({offset:offset})});

// Sync browser date/time to device on every page load
syncBrowserDateToDevice();

loadDateTime();
},2000);

function syncBrowserDateToDevice(){
// Sync browser's current date/time to device and save to SPIFFS
const now = new Date();
const browserDate = {
  year: now.getFullYear(),
  month: now.getMonth() + 1,
  day: now.getDate(),
  hour: now.getHours(),
  minute: now.getMinutes()
};
console.log('[Time] Syncing browser date to device:', browserDate);
fetch('/api/time', {
  method: 'POST',
  headers: {'Content-Type': 'application/json'},
  body: JSON.stringify(browserDate)
}).then(response => response.json())
  .then(data => {
    console.log('[Time] Device date updated and saved to SPIFFS:', data);
  })
  .catch(error => {
    console.error('[Time] Failed to sync browser date:', error);
  });
}

function loadDateTime(){
// Simple browser date/time display
const now = new Date();
const day = now.getDate().toString().padStart(2, '0');
const month = (now.getMonth() + 1).toString().padStart(2, '0');
const year = now.getFullYear();
const hour = now.getHours().toString().padStart(2, '0');
const minute = now.getMinutes().toString().padStart(2, '0');

const dateStr = `${day}.${month}.${year}`;
const timeStr = `${hour}:${minute}`;

document.getElementById('current-date').textContent = dateStr;
document.getElementById('current-time').textContent = timeStr;

// Also sync to device periodically (every minute when time updates)
syncBrowserDateToDevice();
}

// Load time immediately on page load
setTimeout(() => {
  console.log('[Time] Loading browser date/time...');
  loadDateTime();
}, 100);

// Update time every minute (and sync to device)
setInterval(loadDateTime, 60000);
</script>
<script src="/lang.js"></script>
<script src="/lang-handler.js"></script>
<script src="/app.js"></script>
</body>
</html>
)rawliteral";
}

#endif
