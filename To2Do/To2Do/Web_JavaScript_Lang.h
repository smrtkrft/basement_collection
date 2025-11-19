#ifndef WEB_JAVASCRIPT_LANG_H
#define WEB_JAVASCRIPT_LANG_H

/*
 * Language Translations for To2Do
 * 
 * This file contains ALL UI translations in a global LANG object.
 * Translations are stored client-side only (not in ESP32 memory).
 * 
 * Supported Languages:
 * - EN (English) - Default
 * - DE (Deutsch/German)  
 * - TR (Türkçe/Turkish)
 * 
 * Usage in JavaScript:
 *   let currentLang = "EN"; // or "DE" or "TR"
 *   button.textContent = LANG[currentLang].save_btn;
 * 
 * IMPORTANT: User-generated content is NOT translated:
 *   - Project names
 *   - Task titles
 *   - Category names (user can customize)
 *   - Descriptions
 *   - App title (user can customize)
 */

const char* getLanguageJS() {
  return R"rawliteral(
const LANG = {
  EN: {
    // Stats
    stat_total: "TOTAL",
    stat_active: "ACTIVE",
    stat_projects: "PROJECTS",
    
    // Sidebar
    search_projects: "Search projects...",
    new_project_btn: "+ NEW PROJECT",
    settings_btn: "SETTINGS",
    
    // Panel filters
    filter_all: "ALL",
    filter_active: "ACTIVE",
    filter_completed: "COMPLETED",
    filter_task: "TASK",
    filter_plan: "PLAN",
    filter_note: "NOTE",
    filter_reminder: "REMINDER",
    
    // Project list
    projects_header: "PROJECTS",
    archive_header: "ARCHIVE",
    
    // Empty states
    empty_no_project: "NO PROJECT SELECTED",
    empty_select_project: "Select a project from the left",
    empty_no_tasks: "No tasks yet",
    empty_create_task: "Click + to create your first task",
    
    // Task modal
    modal_new_task: "NEW TASK",
    modal_edit_task: "EDIT TASK",
    task_title_placeholder: "Title...",
    task_desc_placeholder: "Description, details, notes...",
    task_type_task: "Task",
    task_type_plan: "Plan",
    task_type_note: "Note",
    task_type_reminder: "Reminder",
    priority_low: "Low",
    priority_medium: "Medium",
    priority_high: "High",
    
    // Project modal
    modal_new_project: "NEW PROJECT",
    modal_edit_project: "EDIT PROJECT",
    project_name_placeholder: "Project name...",
    project_desc_placeholder: "Project description...",
    priority_low_label: "Low Priority",
    priority_medium_label: "Medium Priority",
    priority_high_label: "High Priority",
    
    // Buttons
    btn_cancel: "CANCEL",
    btn_create: "CREATE",
    btn_save: "SAVE",
    btn_update: "UPDATE",
    btn_delete: "DELETE",
    btn_close: "CLOSE",
    btn_add_dependency: "+ DEPENDENCY",
    btn_export: "EXPORT",
    btn_import: "IMPORT",
    btn_test: "TEST CONNECTION",
    
    // Empty state
    empty_title: "EMPTY",
    empty_desc: "Click the + button at the bottom right",
    
    // Notifications
    notif_title: "NOTIFICATIONS",
    notif_today: "TODAY",
    notif_tomorrow: "TOMORROW",
    notif_week: "THIS WEEK",
    notif_overdue: "OVERDUE",
    notif_loading: "Loading...",
    notif_project: "project",
    notif_projects: "projects",
    notif_task: "task",
    notif_tasks: "tasks",
    
    // Settings
    settings_title: "SETTINGS",
    settings_tab_gui: "GUI",
    settings_tab_connection: "CONNECTION",
    settings_tab_info: "INFO",
    
    // GUI Settings
    gui_theme_language: "THEME & LANGUAGE",
    gui_theme_light: "LIGHT",
    gui_theme_dark: "DARK",
    gui_app_title_label: "APP TITLE",
    gui_app_title_placeholder: "To2Do - SmartKraft",
    gui_categories_label: "CATEGORY NAMES",
    gui_category_placeholder: "Category",
    gui_datetime_label: "DATE / TIME",
    gui_date_label: "DATE",
    gui_time_label: "TIME",
    
    // Connection Settings
    conn_primary_wifi: "PRIMARY WIFI",
    conn_backup_wifi: "BACKUP WIFI",
    conn_ap_mode: "AP MODE SETTINGS",
    conn_status_connected: "Connected",
    conn_status_not_connected: "Not Connected",
    conn_status_not_configured: "Not Configured",
    conn_ssid_label: "WiFi SSID (Network Name)",
    conn_password_label: "WiFi PASSWORD",
    conn_show_password: "Show Password",
    conn_static_ip_label: "STATIC IP ADDRESS",
    conn_static_ip_hint: "Leave empty for DHCP (automatic)",
    conn_mdns_label: "mDNS HOSTNAME",
    conn_mdns_hint: "Access via: http://",
    conn_mdns_ap_hint: "Access via: http://",
    conn_ap_ssid_label: "AP SSID (Access Point Name)",
    conn_ap_mdns_label: "mDNS HOSTNAME (AP Mode)",
    conn_ap_no_password: "No password required. Open network for easy first-time setup.",
    conn_current_status: "CURRENT CONNECTION STATUS",
    conn_mode_label: "Mode:",
    conn_connected_to: "Connected To:",
    conn_ip_address: "IP Address:",
    conn_signal_strength: "Signal Strength:",
    
    // Info
    info_version: "Version:",
    info_wifi: "WiFi:",
    info_ip: "IP:",
    info_mdns: "mDNS:",
    info_quick_start: "QUICK START",
    info_quick_1: "1. Select a category from the left",
    info_quick_2: "2. Create a project with '+ NEW PROJECT' button",
    info_quick_3: "3. Click on project to select it",
    info_quick_4: "4. Add tasks with the '+' button on bottom right",
    info_project_management: "PROJECT MANAGEMENT",
    info_project_create: "Create Project:",
    info_project_create_desc: "Use '+ NEW PROJECT' button at the bottom left",
    info_project_edit: "Edit Project:",
    info_project_edit_desc: "Right-click on project and select 'EDIT'",
    info_project_copy: "Copy Project:",
    info_project_copy_desc: "Right-click > 'COPY' to duplicate project",
    info_project_archive: "Archive Project:",
    info_project_archive_desc: "Right-click > 'ARCHIVE' to move to archive",
    info_project_delete: "Delete Project:",
    info_project_delete_desc: "Right-click > 'DELETE' to permanently remove",
    info_task_management: "TASK MANAGEMENT",
    info_task_add: "Add Task:",
    info_task_add_desc: "Click the white '+' button on bottom right",
    info_task_complete: "Complete Task:",
    info_task_complete_desc: "Click the checkbox on the left of the task",
    info_task_types: "Task Types:",
    info_task_types_desc: "Task, Plan, Note, Reminder - 4 types available",
    info_priority_levels: "Priority Levels:",
    info_priority_levels_desc: "Low (Green), Medium (Yellow), High (Red)",
    info_subtasks: "Subtasks:",
    info_subtasks_desc: "Add checklists inside each task",
    add_subtask: "+ Add subtask...",
    date_no_date: "NO DATE",
    info_dependencies: "Dependencies:",
    info_dependencies_desc: "Link tasks to manage workflow",
    info_filtering: "FILTERING",
    info_filter_all: "ALL:",
    info_filter_all_desc: "Show all tasks",
    info_filter_active: "ACTIVE:",
    info_filter_active_desc: "Show only incomplete tasks",
    info_filter_completed: "COMPLETED:",
    info_filter_completed_desc: "Show only completed tasks",
    info_filter_types: "TASK/PLAN/NOTE/REMINDER:",
    info_filter_types_desc: "Filter by type",
    info_search_nav: "SEARCH & NAVIGATION",
    info_search: "Project Search:",
    info_search_desc: "Use the search box at the top left",
    info_category_switch: "Switch Category:",
    info_category_switch_desc: "Select from the top tabs",
    info_shortcuts: "SHORTCUTS",
    info_right_click: "Right Click:",
    info_right_click_desc: "Context menu on projects",
    info_double_click: "Double Click:",
    info_double_click_desc: "Edit task title or description",
    info_fab_button: "FAB Button:",
    info_fab_button_desc: "Always visible '+' button for quick task creation",
    info_customization: "CUSTOMIZATION",
    info_gui_tab: "GUI Tab:",
    info_gui_tab_desc: "Change app title, category names, and theme",
    info_theme: "Theme:",
    info_theme_desc: "Switch between Light/Dark theme",
    info_storage: "Storage:",
    info_storage_desc: "All settings are saved automatically in browser",
    info_data_storage: "DATA STORAGE",
    info_data_storage_desc: "All projects, tasks and settings are stored in ESP32 SPIFFS memory.",
    info_data_persistent: "Data persists even after power loss or firmware updates.",
    info_permanent_storage: "Permanent Storage:",
    info_permanent_storage_desc: "WiFi settings, theme, categories are always protected.",
    info_backup_restore: "BACKUP / RESTORE",
    info_backup_safe: "Safe",
    info_backup_desc: "Backup your data (WiFi settings excluded)",
    info_backup_included: "Included: Projects, Tasks, GUI Settings",
    info_backup_excluded: "Excluded: WiFi credentials (for security)",
    info_factory_reset: "FACTORY RESET",
    info_factory_dangerous: "Dangerous",
    info_factory_warning: "This will permanently delete ALL data:",
    info_factory_projects: "✗ All projects and tasks",
    info_factory_gui: "✗ GUI settings (theme, categories)",
    info_factory_wifi: "✗ WiFi network settings",
    info_factory_irreversible: "This action cannot be undone!",
    info_factory_button: "FACTORY RESET",
    info_more_info: "For more information:",
    
    // Context menu
    ctx_edit: "EDIT",
    ctx_copy: "COPY",
    ctx_archive: "ARCHIVE",
    ctx_delete: "DELETE",
    
    // Alerts & Confirmations
    alert_confirm_delete: "Are you sure you want to delete?",
    alert_confirm_factory: "FACTORY RESET - Are you absolutely sure? All data will be lost!",
    alert_success: "Success!",
    alert_error: "Error!",
    alert_saved: "Saved successfully",
    alert_deleted: "Deleted successfully",
    alert_backup_restored: "Backup restored!",
    alert_backup_failed: "Failed:",
    alert_restore_confirm: "Restore backup? Current data will be overwritten.",
    
    // OLED Display (no Turkish characters)
    oled_today: "Bugun",
    oled_tomorrow: "Yarin",
    oled_week: "Bu Hafta"
  },
  
  DE: {
    // Stats
    stat_total: "GESAMT",
    stat_active: "AKTIV",
    stat_projects: "PROJEKTE",
    
    // Sidebar
    search_projects: "Projekte suchen...",
    new_project_btn: "+ NEUES PROJEKT",
    settings_btn: "EINSTELLUNGEN",
    
    // Panel filters
    filter_all: "ALLE",
    filter_active: "AKTIV",
    filter_completed: "ABGESCHLOSSEN",
    filter_task: "AUFGABE",
    filter_plan: "PLAN",
    filter_note: "NOTIZ",
    filter_reminder: "ERINNERUNG",
    
    // Project list
    projects_header: "PROJEKTE",
    archive_header: "ARCHIV",
    
    // Empty states
    empty_no_project: "KEIN PROJEKT AUSGEWAHLT",
    empty_select_project: "Wahlen Sie ein Projekt von links",
    empty_no_tasks: "Noch keine Aufgaben",
    empty_create_task: "Klicken Sie auf +, um Ihre erste Aufgabe zu erstellen",
    
    // Task modal
    modal_new_task: "NEUE AUFGABE",
    modal_edit_task: "AUFGABE BEARBEITEN",
    task_title_placeholder: "Titel...",
    task_desc_placeholder: "Beschreibung, Details, Notizen...",
    task_type_task: "Aufgabe",
    task_type_plan: "Plan",
    task_type_note: "Notiz",
    task_type_reminder: "Erinnerung",
    priority_low: "Niedrig",
    priority_medium: "Mittel",
    priority_high: "Hoch",
    
    // Project modal
    modal_new_project: "NEUES PROJEKT",
    modal_edit_project: "PROJEKT BEARBEITEN",
    project_name_placeholder: "Projektname...",
    project_desc_placeholder: "Projektbeschreibung...",
    priority_low_label: "Niedrige Prioritat",
    priority_medium_label: "Mittlere Prioritat",
    priority_high_label: "Hohe Prioritat",
    
    // Buttons
    btn_cancel: "ABBRECHEN",
    btn_create: "ERSTELLEN",
    btn_save: "SPEICHERN",
    btn_update: "AKTUALISIEREN",
    btn_delete: "LOSCHEN",
    btn_close: "SCHLIESSEN",
    btn_add_dependency: "+ ABHANGIGKEIT",
    btn_export: "EXPORTIEREN",
    btn_import: "IMPORTIEREN",
    btn_test: "VERBINDUNG TESTEN",
    
    // Empty state
    empty_title: "LEER",
    empty_desc: "Klicken Sie auf die + Schaltflache unten rechts",
    
    // Notifications
    notif_title: "BENACHRICHTIGUNGEN",
    notif_today: "HEUTE",
    notif_tomorrow: "MORGEN",
    notif_week: "DIESE WOCHE",
    notif_overdue: "UBERFALLIG",
    notif_loading: "Wird geladen...",
    notif_project: "Projekt",
    notif_projects: "Projekte",
    notif_task: "Aufgabe",
    notif_tasks: "Aufgaben",
    
    // Settings
    settings_title: "EINSTELLUNGEN",
    settings_tab_gui: "GUI",
    settings_tab_connection: "VERBINDUNG",
    settings_tab_info: "INFO",
    
    // GUI Settings
    gui_theme_language: "THEME & SPRACHE",
    gui_theme_light: "HELL",
    gui_theme_dark: "DUNKEL",
    gui_app_title_label: "APP-TITEL",
    gui_app_title_placeholder: "To2Do - SmartKraft",
    gui_categories_label: "KATEGORIENAMEN",
    gui_category_placeholder: "Kategorie",
    gui_datetime_label: "DATUM / ZEIT",
    gui_date_label: "DATUM",
    gui_time_label: "ZEIT",
    
    // Connection Settings
    conn_primary_wifi: "PRIMARES WLAN",
    conn_backup_wifi: "BACKUP-WLAN",
    conn_ap_mode: "AP-MODUS EINSTELLUNGEN",
    conn_status_connected: "Verbunden",
    conn_status_not_connected: "Nicht verbunden",
    conn_status_not_configured: "Nicht konfiguriert",
    conn_ssid_label: "WLAN SSID (Netzwerkname)",
    conn_password_label: "WLAN PASSWORT",
    conn_show_password: "Passwort anzeigen",
    conn_static_ip_label: "STATISCHE IP-ADRESSE",
    conn_static_ip_hint: "Leer lassen fur DHCP (automatisch)",
    conn_mdns_label: "mDNS HOSTNAME",
    conn_mdns_hint: "Zugriff uber: http://",
    conn_mdns_ap_hint: "Zugriff uber: http://",
    conn_ap_ssid_label: "AP SSID (Access Point Name)",
    conn_ap_mdns_label: "mDNS HOSTNAME (AP-Modus)",
    conn_ap_no_password: "Kein Passwort erforderlich. Offenes Netzwerk fur einfache Ersteinrichtung.",
    conn_current_status: "AKTUELLER VERBINDUNGSSTATUS",
    conn_mode_label: "Modus:",
    conn_connected_to: "Verbunden mit:",
    conn_ip_address: "IP-Adresse:",
    conn_signal_strength: "Signalstarke:",
    
    // Info
    info_version: "Version:",
    info_wifi: "WLAN:",
    info_ip: "IP:",
    info_mdns: "mDNS:",
    info_quick_start: "SCHNELLSTART",
    info_quick_1: "1. Wahlen Sie eine Kategorie von links",
    info_quick_2: "2. Erstellen Sie ein Projekt mit '+ NEUES PROJEKT'",
    info_quick_3: "3. Klicken Sie auf das Projekt, um es auszuwahlen",
    info_quick_4: "4. Fugen Sie Aufgaben mit dem '+' Button unten rechts hinzu",
    info_project_management: "PROJEKTVERWALTUNG",
    info_project_create: "Projekt erstellen:",
    info_project_create_desc: "Verwenden Sie '+ NEUES PROJEKT' unten links",
    info_project_edit: "Projekt bearbeiten:",
    info_project_edit_desc: "Rechtsklick auf Projekt und 'BEARBEITEN' wahlen",
    info_project_copy: "Projekt kopieren:",
    info_project_copy_desc: "Rechtsklick > 'KOPIEREN' zum Duplizieren",
    info_project_archive: "Projekt archivieren:",
    info_project_archive_desc: "Rechtsklick > 'ARCHIVIEREN' zum Verschieben",
    info_project_delete: "Projekt loschen:",
    info_project_delete_desc: "Rechtsklick > 'LOSCHEN' zum dauerhaften Entfernen",
    info_task_management: "AUFGABENVERWALTUNG",
    info_task_add: "Aufgabe hinzufugen:",
    info_task_add_desc: "Klicken Sie auf den weissen '+' Button unten rechts",
    info_task_complete: "Aufgabe abschliessen:",
    info_task_complete_desc: "Klicken Sie auf das Kontrollkastchen links neben der Aufgabe",
    info_task_types: "Aufgabentypen:",
    info_task_types_desc: "Aufgabe, Plan, Notiz, Erinnerung - 4 Typen verfugbar",
    info_priority_levels: "Prioritatsstufen:",
    info_priority_levels_desc: "Niedrig (Grun), Mittel (Gelb), Hoch (Rot)",
    info_subtasks: "Unteraufgaben:",
    info_subtasks_desc: "Checklisten in jeder Aufgabe hinzufugen",
    add_subtask: "+ Unteraufgabe hinzufugen...",
    date_no_date: "KEIN DATUM",
    info_dependencies: "Abhangigkeiten:",
    info_dependencies_desc: "Aufgaben verknupfen, um den Workflow zu verwalten",
    info_filtering: "FILTERUNG",
    info_filter_all: "ALLE:",
    info_filter_all_desc: "Alle Aufgaben anzeigen",
    info_filter_active: "AKTIV:",
    info_filter_active_desc: "Nur unvollstandige Aufgaben anzeigen",
    info_filter_completed: "ABGESCHLOSSEN:",
    info_filter_completed_desc: "Nur abgeschlossene Aufgaben anzeigen",
    info_filter_types: "AUFGABE/PLAN/NOTIZ/ERINNERUNG:",
    info_filter_types_desc: "Nach Typ filtern",
    info_search_nav: "SUCHE & NAVIGATION",
    info_search: "Projektsuche:",
    info_search_desc: "Verwenden Sie das Suchfeld oben links",
    info_category_switch: "Kategorie wechseln:",
    info_category_switch_desc: "Aus den oberen Tabs auswahlen",
    info_shortcuts: "TASTENKURZEL",
    info_right_click: "Rechtsklick:",
    info_right_click_desc: "Kontextmenu bei Projekten",
    info_double_click: "Doppelklick:",
    info_double_click_desc: "Aufgabentitel oder Beschreibung bearbeiten",
    info_fab_button: "FAB Button:",
    info_fab_button_desc: "Immer sichtbarer '+' Button fur schnelle Aufgabenerstellung",
    info_customization: "ANPASSUNG",
    info_gui_tab: "GUI Tab:",
    info_gui_tab_desc: "App-Titel, Kategorienamen und Theme andern",
    info_theme: "Theme:",
    info_theme_desc: "Zwischen Hell/Dunkel Theme wechseln",
    info_storage: "Speicherung:",
    info_storage_desc: "Alle Einstellungen werden automatisch im Browser gespeichert",
    info_data_storage: "DATENSPEICHERUNG",
    info_data_storage_desc: "Alle Projekte, Aufgaben und Einstellungen werden im ESP32 SPIFFS-Speicher gespeichert.",
    info_data_persistent: "Daten bleiben auch nach Stromausfall oder Firmware-Updates erhalten.",
    info_permanent_storage: "Permanente Speicherung:",
    info_permanent_storage_desc: "WLAN-Einstellungen, Theme, Kategorien sind immer geschutzt.",
    info_backup_restore: "BACKUP / WIEDERHERSTELLUNG",
    info_backup_safe: "Sicher",
    info_backup_desc: "Sichern Sie Ihre Daten (WLAN-Einstellungen ausgeschlossen)",
    info_backup_included: "Enthalten: Projekte, Aufgaben, GUI-Einstellungen",
    info_backup_excluded: "Ausgeschlossen: WLAN-Zugangsdaten (aus Sicherheitsgrunden)",
    info_factory_reset: "WERKSEINSTELLUNGEN",
    info_factory_dangerous: "Gefahrlich",
    info_factory_warning: "Dies wird ALLE Daten dauerhaft loschen:",
    info_factory_projects: "✗ Alle Projekte und Aufgaben",
    info_factory_gui: "✗ GUI-Einstellungen (Theme, Kategorien)",
    info_factory_wifi: "✗ WLAN-Netzwerkeinstellungen",
    info_factory_irreversible: "Diese Aktion kann nicht ruckgangig gemacht werden!",
    info_factory_button: "WERKSEINSTELLUNGEN",
    info_more_info: "Weitere Informationen:",
    
    // Context menu
    ctx_edit: "BEARBEITEN",
    ctx_copy: "KOPIEREN",
    ctx_archive: "ARCHIVIEREN",
    ctx_delete: "LOSCHEN",
    
    // Alerts & Confirmations
    alert_confirm_delete: "Mochten Sie wirklich loschen?",
    alert_confirm_factory: "WERKSEINSTELLUNGEN - Sind Sie absolut sicher? Alle Daten gehen verloren!",
    alert_success: "Erfolg!",
    alert_error: "Fehler!",
    alert_saved: "Erfolgreich gespeichert",
    alert_deleted: "Erfolgreich geloscht",
    alert_backup_restored: "Backup wiederhergestellt!",
    alert_backup_failed: "Fehlgeschlagen:",
    alert_restore_confirm: "Backup wiederherstellen? Aktuelle Daten werden uberschrieben.",
    
    // OLED Display (no special characters)
    oled_today: "Heute",
    oled_tomorrow: "Morgen",
    oled_week: "Diese Woche"
  },
  
  TR: {
    // Stats
    stat_total: "TOPLAM",
    stat_active: "AKTIF",
    stat_projects: "PROJE",
    
    // Sidebar
    search_projects: "Proje ara...",
    new_project_btn: "+ YENI PROJE",
    settings_btn: "AYARLAR",
    
    // Panel filters
    filter_all: "TUMU",
    filter_active: "AKTIF",
    filter_completed: "TAMAMLANAN",
    filter_task: "GOREV",
    filter_plan: "PLAN",
    filter_note: "NOT",
    filter_reminder: "HATIRLATMA",
    
    // Project list
    projects_header: "PROJELER",
    archive_header: "ARSIV",
    
    // Empty states
    empty_no_project: "PROJE SECILMEDI",
    empty_select_project: "Sol taraftan bir proje secin",
    empty_no_tasks: "Henuz gorev yok",
    empty_create_task: "Ilk gorevinizi olusturmak icin + tiklayin",
    
    // Task modal
    modal_new_task: "YENI GOREV",
    modal_edit_task: "GOREVI DUZENLE",
    task_title_placeholder: "Baslik...",
    task_desc_placeholder: "Aciklama, detaylar, notlar...",
    task_type_task: "Gorev",
    task_type_plan: "Plan",
    task_type_note: "Not",
    task_type_reminder: "Hatirlatma",
    priority_low: "Dusuk",
    priority_medium: "Orta",
    priority_high: "Yuksek",
    
    // Project modal
    modal_new_project: "YENI PROJE",
    modal_edit_project: "PROJEYI DUZENLE",
    project_name_placeholder: "Proje adi...",
    project_desc_placeholder: "Proje aciklamasi...",
    priority_low_label: "Dusuk Oncelik",
    priority_medium_label: "Orta Oncelik",
    priority_high_label: "Yuksek Oncelik",
    
    // Buttons
    btn_cancel: "IPTAL",
    btn_create: "OLUSTUR",
    btn_save: "KAYDET",
    btn_update: "GUNCELLE",
    btn_delete: "SIL",
    btn_close: "KAPAT",
    btn_add_dependency: "+ BAGLANTI",
    btn_export: "DISA AKTAR",
    btn_import: "ICE AKTAR",
    btn_test: "BAGLANTI TEST",
    
    // Empty state
    empty_title: "BOS",
    empty_desc: "Sag alttaki + butonuna tiklayin",
    
    // Notifications
    notif_title: "BILDIRIMLER",
    notif_today: "BUGUN",
    notif_tomorrow: "YARIN",
    notif_week: "BU HAFTA",
    notif_overdue: "GECMIS",
    notif_loading: "Yukleniyor...",
    notif_project: "proje",
    notif_projects: "proje",
    notif_task: "gorev",
    notif_tasks: "gorev",
    
    // Settings
    settings_title: "AYARLAR",
    settings_tab_gui: "GUI",
    settings_tab_connection: "BAGLANTI",
    settings_tab_info: "INFO",
    
    // GUI Settings
    gui_theme_language: "TEMA & DIL",
    gui_theme_light: "ACIK",
    gui_theme_dark: "KOYU",
    gui_app_title_label: "UYGULAMA BASLIGI",
    gui_app_title_placeholder: "To2Do - SmartKraft",
    gui_categories_label: "KATEGORI ISIMLERI",
    gui_category_placeholder: "Kategori",
    gui_datetime_label: "TARIH / SAAT",
    gui_date_label: "TARIH",
    gui_time_label: "SAAT",
    
    // Connection Settings
    conn_primary_wifi: "PRIMARY WIFI",
    conn_backup_wifi: "BACKUP WIFI",
    conn_ap_mode: "AP MODU AYARLARI",
    conn_status_connected: "Bagli",
    conn_status_not_connected: "Bagli Degil",
    conn_status_not_configured: "Yapilandirilmamis",
    conn_ssid_label: "WiFi SSID (Ag Adi)",
    conn_password_label: "WiFi SIFRESI",
    conn_show_password: "Sifreyi Goster",
    conn_static_ip_label: "STATIK IP ADRESI",
    conn_static_ip_hint: "DHCP (otomatik) icin bos birakin",
    conn_mdns_label: "mDNS HOSTNAME",
    conn_mdns_hint: "Erisim: http://",
    conn_mdns_ap_hint: "Erisim: http://",
    conn_ap_ssid_label: "AP SSID (Access Point Adi)",
    conn_ap_mdns_label: "mDNS HOSTNAME (AP Modu)",
    conn_ap_no_password: "Sifre gerekmez. Kolay ilk kurulum icin acik ag.",
    conn_current_status: "GUNCEL BAGLANTI DURUMU",
    conn_mode_label: "Mod:",
    conn_connected_to: "Baglanti:",
    conn_ip_address: "IP Adresi:",
    conn_signal_strength: "Sinyal Gucu:",
    
    // Info
    info_version: "Versiyon:",
    info_wifi: "WiFi:",
    info_ip: "IP:",
    info_mdns: "mDNS:",
    info_quick_start: "HIZLI BASLANGIC",
    info_quick_1: "1. Sol taraftan bir kategori secin",
    info_quick_2: "2. '+ YENI PROJE' butonu ile proje olusturun",
    info_quick_3: "3. Proje ustune tiklayarak secin",
    info_quick_4: "4. Sag alttaki '+' butonu ile gorev ekleyin",
    info_project_management: "PROJE YONETIMI",
    info_project_create: "Proje Olusturma:",
    info_project_create_desc: "Sol alttaki '+ YENI PROJE' butonunu kullanin",
    info_project_edit: "Proje Duzenleme:",
    info_project_edit_desc: "Proje uzerine sag tiklayip 'DUZENLE' secin",
    info_project_copy: "Proje Kopyalama:",
    info_project_copy_desc: "Sag tikla > 'KOPYALA' ile ayni projeden kopya olusturun",
    info_project_archive: "Proje Arsivleme:",
    info_project_archive_desc: "Sag tikla > 'ARSIVLE' ile projeyi arsive tasiyin",
    info_project_delete: "Proje Silme:",
    info_project_delete_desc: "Sag tikla > 'SIL' ile projeyi kalici olarak silin",
    info_task_management: "GOREV YONETIMI",
    info_task_add: "Gorev Ekleme:",
    info_task_add_desc: "Sag alttaki beyaz '+' butonuna tiklayin",
    info_task_complete: "Gorev Tamamlama:",
    info_task_complete_desc: "Gorevin solundaki kutuya tiklayarak tamamlandi isareti koyun",
    info_task_types: "Gorev Turleri:",
    info_task_types_desc: "Gorev, Plan, Not, Hatirlatma olarak 4 turde olusturabilirsiniz",
    info_priority_levels: "Oncelik Seviyeleri:",
    info_priority_levels_desc: "Dusuk (Yesil), Orta (Sari), Yuksek (Kirmizi)",
    info_subtasks: "Alt Gorevler:",
    info_subtasks_desc: "Her gorevin icine checklist ekleyebilirsiniz",
    add_subtask: "+ Alt gorev ekle...",
    date_no_date: "TARIH YOK",
    info_dependencies: "Bagimlilik:",
    info_dependencies_desc: "Gorevler arasi bagimlilik ekleyerek is akisini yonetin",
    info_filtering: "FILTRELEME",
    info_filter_all: "TUMU:",
    info_filter_all_desc: "Tum gorevleri gosterir",
    info_filter_active: "AKTIF:",
    info_filter_active_desc: "Sadece tamamlanmamis gorevler",
    info_filter_completed: "TAMAMLANAN:",
    info_filter_completed_desc: "Sadece tamamlanmis gorevler",
    info_filter_types: "GOREV/PLAN/NOT/HATIRLATMA:",
    info_filter_types_desc: "Turune gore filtreleme",
    info_search_nav: "ARAMA VE NAVIGASYON",
    info_search: "Proje Arama:",
    info_search_desc: "Sol ust kosedeki arama kutusunu kullanin",
    info_category_switch: "Kategori Degistirme:",
    info_category_switch_desc: "Ust sekmelerden birini secin",
    info_shortcuts: "KISAYOLLAR",
    info_right_click: "Sag Tik:",
    info_right_click_desc: "Projeler uzerinde sag tik menusu",
    info_double_click: "Cift Tik:",
    info_double_click_desc: "Gorev baslik veya aciklama uzerinde duzenlemek icin",
    info_fab_button: "FAB Butonu:",
    info_fab_button_desc: "Her zaman gorunen '+' butonu ile hizli gorev ekleme",
    info_customization: "KISISELLESTIRME",
    info_gui_tab: "GUI Sekmesi:",
    info_gui_tab_desc: "Uygulama basligini, kategori isimlerini ve temayi degistirin",
    info_theme: "Tema:",
    info_theme_desc: "Acik/Koyu tema arasi gecis yapabilirsiniz",
    info_storage: "Kayit:",
    info_storage_desc: "Tum ayarlar otomatik olarak tarayicida saklanir",
    info_data_storage: "VERI SAKLAMA",
    info_data_storage_desc: "Tum proje, gorev ve ayarlar ESP32 cihazinda SPIFFS hafizasinda saklanir.",
    info_data_persistent: "Elektrik kesilse ve firmware tekrar yuklenme yapilsa bile verileriniz korunur.",
    info_permanent_storage: "Kalici Saklama:",
    info_permanent_storage_desc: "WiFi ayarlari, tema, kategoriler her zaman korunur.",
    info_backup_restore: "BACKUP / RESTORE",
    info_backup_safe: "Guvenli",
    info_backup_desc: "Verilerinizi yedekleyin (WiFi ayarlari haric)",
    info_backup_included: "Dahil: Projeler, Gorevler, GUI Ayarlari",
    info_backup_excluded: "Haric: WiFi kimlik bilgileri (guvenlik icin)",
    info_factory_reset: "FABRIKA AYARLARI",
    info_factory_dangerous: "Tehlikeli",
    info_factory_warning: "Bu islem TUM verileri kalici olarak silecektir:",
    info_factory_projects: "✗ Tum projeler ve gorevler",
    info_factory_gui: "✗ GUI ayarlari (tema, kategoriler)",
    info_factory_wifi: "✗ WiFi network ayarlari",
    info_factory_irreversible: "Bu islem geri alinamaz!",
    info_factory_button: "FABRIKA AYARLARI",
    info_more_info: "Daha fazla bilgi icin:",
    
    // Context menu
    ctx_edit: "DUZENLE",
    ctx_copy: "KOPYALA",
    ctx_archive: "ARSIVLE",
    ctx_delete: "SIL",
    
    // Alerts & Confirmations
    alert_confirm_delete: "Silmek istediginizden emin misiniz?",
    alert_confirm_factory: "FABRIKA AYARLARI - Kesinlikle emin misiniz? Tum veriler kaybolacak!",
    alert_success: "Basarili!",
    alert_error: "Hata!",
    alert_saved: "Basariyla kaydedildi",
    alert_deleted: "Basariyla silindi",
    alert_backup_restored: "Yedek geri yuklendi!",
    alert_backup_failed: "Basarisiz:",
    alert_restore_confirm: "Yedeği geri yukle? Mevcut veriler uzerine yazilacak.",
    
    // OLED Display (no Turkish special characters)
    oled_today: "Bugun",
    oled_tomorrow: "Yarin",
    oled_week: "Bu Hafta"
  }
};

// Current language - will be loaded from settings
let currentLang = "EN";

// Helper function to get translation
function t(key) {
  return LANG[currentLang][key] || LANG.EN[key] || key;
}

// Apply translations to the page
function applyTranslations() {
  // This function will be called when language changes
  // Implementation will be added when integrating with the system
  console.log('[Language] Applied:', currentLang);
}
)rawliteral";
}

#endif
