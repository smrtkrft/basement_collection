#ifndef WEB_JAVASCRIPT_LANG_HANDLER_H
#define WEB_JAVASCRIPT_LANG_HANDLER_H

/*
 * Language Handler JavaScript
 * Handles language switching and translation application
 */

const char* getLanguageHandlerJS() {
  return R"rawliteral(
// Language Handler - Initialized after LANG object is loaded

// Load saved language from server
async function loadLanguage() {
  try {
    const response = await fetch('/api/language');
    const data = await response.json();
    if (data.current) {
      currentLang = data.current;
      updateLanguageButtons();
      applyAllTranslations();
    }
  } catch (error) {
    console.error('[Language] Failed to load:', error);
    currentLang = "EN"; // Fallback to English
  }
}

// Update language button states
function updateLanguageButtons() {
  document.querySelectorAll('.lang-btn').forEach(btn => {
    btn.classList.remove('active');
    if (btn.dataset.lang === currentLang) {
      btn.classList.add('active');
    }
  });
}

// Save language to server
async function saveLanguage(lang) {
  try {
    const response = await fetch('/api/language', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ language: lang })
    });
    const data = await response.json();
    if (data.success) {
      console.log('[Language] Saved:', lang);
      return true;
    }
  } catch (error) {
    console.error('[Language] Failed to save:', error);
  }
  return false;
}

// Apply all translations to the page
function applyAllTranslations() {
  console.log('[Language] Applying translations:', currentLang);
  
  // === STATS ===
  const statLabels = document.querySelectorAll('.stat-label');
  if (statLabels[0]) statLabels[0].textContent = t('stat_total');
  if (statLabels[1]) statLabels[1].textContent = t('stat_active');
  if (statLabels[2]) statLabels[2].textContent = t('stat_projects');
  
  // === SIDEBAR ===
  const projectSearch = document.getElementById('project-search');
  if (projectSearch) projectSearch.placeholder = t('search_projects');
  
  // Initial empty state in HTML
  const initialNoProject = document.getElementById('initial-no-project');
  if (initialNoProject) initialNoProject.textContent = t('empty_no_project');
  const initialSelectProject = document.getElementById('initial-select-project');
  if (initialSelectProject) initialSelectProject.textContent = t('empty_select_project');
  
  const newProjectBtn = document.getElementById('new-project-btn');
  if (newProjectBtn) newProjectBtn.textContent = t('new_project_btn');
  
  const settingsBtn = document.getElementById('settings-btn');
  if (settingsBtn) settingsBtn.textContent = t('settings_btn');
  
  // === PANEL FILTERS ===
  const filters = document.querySelectorAll('.panel-filter-btn');
  if (filters[0]) filters[0].textContent = t('filter_all');
  if (filters[1]) filters[1].textContent = t('filter_active');
  if (filters[2]) filters[2].textContent = t('filter_completed');
  if (filters[3]) filters[3].textContent = t('filter_task');
  if (filters[4]) filters[4].textContent = t('filter_plan');
  if (filters[5]) filters[5].textContent = t('filter_note');
  if (filters[6]) filters[6].textContent = t('filter_reminder');
  
  // === EMPTY STATES ===
  const panelTitle = document.getElementById('current-project');
  if (panelTitle && (panelTitle.textContent.includes('PROJE SEC') || panelTitle.textContent.includes('NO PROJECT') || panelTitle.textContent.includes('KEIN PROJEKT'))) {
    panelTitle.textContent = t('empty_no_project');
  }
  
  // === MODALS ===
  // Task Modal
  const taskModalTitle = document.querySelector('#task-modal .modal-title');
  if (taskModalTitle) taskModalTitle.textContent = t('modal_new_task');
  
  const taskTitle = document.getElementById('modal-task-title');
  if (taskTitle) taskTitle.placeholder = t('task_title_placeholder');
  
  const taskDesc = document.getElementById('modal-task-desc');
  if (taskDesc) taskDesc.placeholder = t('task_desc_placeholder');
  
  // Task type options
  const taskTypeSelect = document.getElementById('modal-task-type');
  if (taskTypeSelect) {
    taskTypeSelect.options[0].text = t('task_type_task');
    taskTypeSelect.options[1].text = t('task_type_plan');
    taskTypeSelect.options[2].text = t('task_type_note');
    taskTypeSelect.options[3].text = t('task_type_reminder');
  }
  
  // Priority options
  const taskPrioritySelect = document.getElementById('modal-task-priority');
  if (taskPrioritySelect) {
    taskPrioritySelect.options[0].text = t('priority_low');
    taskPrioritySelect.options[1].text = t('priority_medium');
    taskPrioritySelect.options[2].text = t('priority_high');
  }
  
  // Task modal buttons
  const taskCancelBtn = document.getElementById('modal-task-cancel');
  if (taskCancelBtn) taskCancelBtn.textContent = t('btn_cancel');
  
  const taskCreateBtn = document.getElementById('modal-task-create');
  if (taskCreateBtn) taskCreateBtn.textContent = t('btn_create');
  
  // Edit Task Modal
  const editTaskModalTitle = document.querySelector('#edit-task-modal .modal-title');
  if (editTaskModalTitle) editTaskModalTitle.textContent = t('modal_edit_task');
  
  const editTaskTitle = document.getElementById('edit-task-title');
  if (editTaskTitle) editTaskTitle.placeholder = t('task_title_placeholder');
  
  const editTaskDesc = document.getElementById('edit-task-desc');
  if (editTaskDesc) editTaskDesc.placeholder = t('task_desc_placeholder');
  
  const editTaskTypeSelect = document.getElementById('edit-task-type');
  if (editTaskTypeSelect) {
    editTaskTypeSelect.options[0].text = t('task_type_task');
    editTaskTypeSelect.options[1].text = t('task_type_plan');
    editTaskTypeSelect.options[2].text = t('task_type_note');
    editTaskTypeSelect.options[3].text = t('task_type_reminder');
  }
  
  const editTaskPrioritySelect = document.getElementById('edit-task-priority');
  if (editTaskPrioritySelect) {
    editTaskPrioritySelect.options[0].text = t('priority_low');
    editTaskPrioritySelect.options[1].text = t('priority_medium');
    editTaskPrioritySelect.options[2].text = t('priority_high');
  }
  
  const editTaskCancelBtn = document.getElementById('edit-task-cancel');
  if (editTaskCancelBtn) editTaskCancelBtn.textContent = t('btn_cancel');
  
  const editTaskSaveBtn = document.getElementById('edit-task-save');
  if (editTaskSaveBtn) editTaskSaveBtn.textContent = t('btn_save');
  
  // Project Modal
  const projectModalTitle = document.querySelector('#project-modal .modal-title');
  if (projectModalTitle) projectModalTitle.textContent = t('modal_new_project');
  
  const projectName = document.getElementById('modal-project-name');
  if (projectName) projectName.placeholder = t('project_name_placeholder');
  
  const projectDesc = document.getElementById('modal-project-desc');
  if (projectDesc) projectDesc.placeholder = t('project_desc_placeholder');
  
  const projectPrioritySelect = document.getElementById('modal-project-priority');
  if (projectPrioritySelect) {
    projectPrioritySelect.options[0].text = t('priority_low_label');
    projectPrioritySelect.options[1].text = t('priority_medium_label');
    projectPrioritySelect.options[2].text = t('priority_high_label');
  }
  
  const projectCancelBtn = document.getElementById('modal-project-cancel');
  if (projectCancelBtn) projectCancelBtn.textContent = t('btn_cancel');
  
  const projectCreateBtn = document.getElementById('modal-project-create');
  if (projectCreateBtn) projectCreateBtn.textContent = t('btn_save');
  
  const projectUpdateBtn = document.getElementById('modal-project-update');
  if (projectUpdateBtn) projectUpdateBtn.textContent = t('btn_update');
  
  // === NOTIFICATION MODAL ===
  const notifModalTitle = document.querySelector('#notification-modal .modal-title');
  if (notifModalTitle) notifModalTitle.textContent = t('notif_title');
  
  const notifTabs = document.querySelectorAll('#notification-modal .settings-tab');
  if (notifTabs[0]) notifTabs[0].textContent = t('notif_today');
  if (notifTabs[1]) notifTabs[1].textContent = t('notif_tomorrow');
  if (notifTabs[2]) notifTabs[2].textContent = t('notif_week');
  if (notifTabs[3]) notifTabs[3].textContent = t('notif_overdue');
  
  // Notification close button
  const notifCloseBtn = document.querySelector('#notification-modal .modal-btn');
  if (notifCloseBtn) notifCloseBtn.textContent = t('btn_close');
  
  // === SETTINGS MODAL ===
  const settingsModalTitle = document.querySelector('#settings-modal .modal-title');
  if (settingsModalTitle) settingsModalTitle.textContent = t('settings_title');
  
  const settingsTabs = document.querySelectorAll('#settings-modal .settings-tab');
  if (settingsTabs[0]) settingsTabs[0].textContent = t('settings_tab_gui');
  if (settingsTabs[1]) settingsTabs[1].textContent = t('settings_tab_connection');
  if (settingsTabs[2]) settingsTabs[2].textContent = t('settings_tab_info');
  
  // GUI Settings Labels
  const labels = document.querySelectorAll('.settings-label');
  labels.forEach(label => {
    const text = label.textContent.trim();
    if (text.includes('TEMA') || text.includes('THEME')) {
      label.textContent = t('gui_theme_language');
    } else if (text.includes('UYGULAMA') || text.includes('APP')) {
      label.textContent = t('gui_app_title_label');
    } else if (text.includes('KATEGORI') || text.includes('CATEGORY') || text.includes('KATEGORIEN')) {
      label.textContent = t('gui_categories_label');
    } else if (text.includes('TARIH') || text.includes('DATE') || text.includes('DATUM')) {
      label.textContent = t('gui_datetime_label');
    } else if (text.includes('WiFi SSID')) {
      label.textContent = t('conn_ssid_label');
    } else if (text.includes('PASSWORD') || text.includes('SIFRE')) {
      label.textContent = t('conn_password_label');
    } else if (text.includes('STATIC IP') || text.includes('STATIK IP')) {
      label.textContent = t('conn_static_ip_label');
    } else if (text.includes('mDNS')) {
      label.textContent = t('conn_mdns_label');
    }
  });
  
  // Theme labels
  const themeLabels = document.querySelectorAll('.theme-label');
  if (themeLabels[0]) themeLabels[0].textContent = t('gui_theme_light');
  if (themeLabels[1]) themeLabels[1].textContent = t('gui_theme_dark');
  
  // Date/Time section
  const datetimeLabel = document.getElementById('datetime-label');
  if (datetimeLabel) datetimeLabel.textContent = t('gui_datetime_label');
  
  const dateSublabel = document.getElementById('date-sublabel');
  if (dateSublabel) dateSublabel.textContent = t('gui_date_label');
  
  const timeSublabel = document.getElementById('time-sublabel');
  if (timeSublabel) timeSublabel.textContent = t('gui_time_label');
  
  // Settings buttons
  const settingsCancelBtn = document.getElementById('modal-settings-cancel');
  if (settingsCancelBtn) settingsCancelBtn.textContent = t('btn_cancel');
  
  const settingsSaveBtn = document.getElementById('modal-settings-save');
  if (settingsSaveBtn) settingsSaveBtn.textContent = t('btn_save');
  
  // Connection test buttons
  const testButtons = document.querySelectorAll('.settings-btn');
  testButtons.forEach(btn => {
    if (btn.textContent.includes('TEST') || btn.textContent.includes('BAGLANTI')) {
      btn.textContent = t('btn_test');
    } else if (btn.textContent.includes('EXPORT') || btn.textContent.includes('DISA')) {
      btn.textContent = t('btn_export');
    } else if (btn.textContent.includes('IMPORT') || btn.textContent.includes('ICE')) {
      btn.textContent = t('btn_import');
    }
  });
  
  // Accordion headers (Connection section)
  const accordionTitles = document.querySelectorAll('.accordion-title');
  accordionTitles.forEach(title => {
    const text = title.textContent.trim();
    if (text.includes('PRIMARY WIFI')) {
      title.textContent = '▶ ' + t('conn_primary_wifi');
    } else if (text.includes('BACKUP WIFI')) {
      title.textContent = '▶ ' + t('conn_backup_wifi');
    } else if (text.includes('AP MODE')) {
      title.textContent = '▶ ' + t('conn_ap_mode');
    } else if (text.includes('BACKUP') && text.includes('RESTORE')) {
      title.textContent = '▶ ' + t('info_backup_restore');
    } else if (text.includes('FACTORY RESET') || text.includes('FABRIKA')) {
      title.textContent = '▶ ' + t('info_factory_reset');
    }
  });
  
  // Connection section labels and hints
  const labelApSsid = document.getElementById('label-ap-ssid');
  if (labelApSsid) labelApSsid.textContent = t('conn_ap_ssid_label');
  const labelApMdns = document.getElementById('label-ap-mdns');
  if (labelApMdns) labelApMdns.textContent = t('conn_ap_mdns_label');
  const hintApNoPassword = document.getElementById('hint-ap-no-password');
  if (hintApNoPassword) hintApNoPassword.textContent = t('conn_ap_no_password');
  const hintApMdnsAccess = document.getElementById('hint-ap-mdns-access');
  if (hintApMdnsAccess) hintApMdnsAccess.innerHTML = t('conn_mdns_ap_hint') + '<span id="ap-mdns-preview">' + document.getElementById('ap-mdns-preview').textContent + '</span>.local';
  
  const labelPrimarySsid = document.getElementById('label-primary-ssid');
  if (labelPrimarySsid) labelPrimarySsid.textContent = t('conn_ssid_label');
  const labelPrimaryPassword = document.getElementById('label-primary-password');
  if (labelPrimaryPassword) labelPrimaryPassword.textContent = t('conn_password_label');
  const checkboxShowPasswordPrimary = document.getElementById('checkbox-show-password-primary');
  if (checkboxShowPasswordPrimary) checkboxShowPasswordPrimary.textContent = t('conn_show_password');
  const labelPrimaryStaticIp = document.getElementById('label-primary-static-ip');
  if (labelPrimaryStaticIp) labelPrimaryStaticIp.textContent = t('conn_static_ip_label');
  const hintPrimaryDhcp = document.getElementById('hint-primary-dhcp');
  if (hintPrimaryDhcp) hintPrimaryDhcp.textContent = t('conn_static_ip_hint');
  const labelPrimaryMdns = document.getElementById('label-primary-mdns');
  if (labelPrimaryMdns) labelPrimaryMdns.textContent = t('conn_mdns_label');
  const hintPrimaryMdnsAccess = document.getElementById('hint-primary-mdns-access');
  if (hintPrimaryMdnsAccess) hintPrimaryMdnsAccess.innerHTML = t('conn_mdns_hint') + '<span id="primary-mdns-preview">' + document.getElementById('primary-mdns-preview').textContent + '</span>.local';
  
  const labelBackupSsid = document.getElementById('label-backup-ssid');
  if (labelBackupSsid) labelBackupSsid.textContent = t('conn_ssid_label');
  const labelBackupPassword = document.getElementById('label-backup-password');
  if (labelBackupPassword) labelBackupPassword.textContent = t('conn_password_label');
  const checkboxShowPasswordBackup = document.getElementById('checkbox-show-password-backup');
  if (checkboxShowPasswordBackup) checkboxShowPasswordBackup.textContent = t('conn_show_password');
  const labelBackupStaticIp = document.getElementById('label-backup-static-ip');
  if (labelBackupStaticIp) labelBackupStaticIp.textContent = t('conn_static_ip_label');
  const hintBackupDhcp = document.getElementById('hint-backup-dhcp');
  if (hintBackupDhcp) hintBackupDhcp.textContent = t('conn_static_ip_hint');
  const labelBackupMdns = document.getElementById('label-backup-mdns');
  if (labelBackupMdns) labelBackupMdns.textContent = t('conn_mdns_label');
  const hintBackupMdnsAccess = document.getElementById('hint-backup-mdns-access');
  if (hintBackupMdnsAccess) hintBackupMdnsAccess.innerHTML = t('conn_mdns_hint') + '<span id="backup-mdns-preview">' + document.getElementById('backup-mdns-preview').textContent + '</span>.local';
  
  // Connection status labels
  const connStatusTitle = document.getElementById('conn-status-title');
  if (connStatusTitle) connStatusTitle.textContent = t('conn_current_status');
  const statusLabelMode = document.getElementById('status-label-mode');
  if (statusLabelMode) statusLabelMode.textContent = t('conn_mode_label');
  const statusLabelConnected = document.getElementById('status-label-connected');
  if (statusLabelConnected) statusLabelConnected.textContent = t('conn_connected_to');
  const statusLabelIp = document.getElementById('status-label-ip');
  if (statusLabelIp) statusLabelIp.textContent = t('conn_ip_address');
  const statusLabelSignal = document.getElementById('status-label-signal');
  if (statusLabelSignal) statusLabelSignal.textContent = t('conn_signal_strength');
  
  // Info section titles
  const infoTitles = document.querySelectorAll('.info-title');
  infoTitles.forEach(title => {
    const id = title.id;
    if (id === 'info-title-quick') title.textContent = t('info_quick_start');
    else if (id === 'info-title-project') title.textContent = t('info_project_management');
    else if (id === 'info-title-task') title.textContent = t('info_task_management');
    else if (id === 'info-title-filter') title.textContent = t('info_filtering');
    else if (id === 'info-title-search') title.textContent = t('info_search_nav');
    else if (id === 'info-title-shortcuts') title.textContent = t('info_shortcuts');
    else if (id === 'info-title-customization') title.textContent = t('info_customization');
    else if (id === 'info-title-data-storage') title.textContent = t('info_data_storage');
  });
  
  // Info section detailed texts
  const quickStep1 = document.getElementById('info-text-quick-1');
  if (quickStep1) quickStep1.innerHTML = t('info_quick_1');
  const quickStep2 = document.getElementById('info-text-quick-2');
  if (quickStep2) quickStep2.textContent = t('info_quick_2');
  const quickStep3 = document.getElementById('info-text-quick-3');
  if (quickStep3) quickStep3.textContent = t('info_quick_3');
  const quickStep4 = document.getElementById('info-text-quick-4');
  if (quickStep4) quickStep4.textContent = t('info_quick_4');
  
  const projectCreate = document.getElementById('info-text-project-create');
  if (projectCreate) projectCreate.innerHTML = '<strong>' + t('info_project_create') + '</strong> ' + t('info_project_create_desc');
  const projectEdit = document.getElementById('info-text-project-edit');
  if (projectEdit) projectEdit.innerHTML = '<strong>' + t('info_project_edit') + '</strong> ' + t('info_project_edit_desc');
  const projectCopy = document.getElementById('info-text-project-copy');
  if (projectCopy) projectCopy.innerHTML = '<strong>' + t('info_project_copy') + '</strong> ' + t('info_project_copy_desc');
  const projectArchive = document.getElementById('info-text-project-archive');
  if (projectArchive) projectArchive.innerHTML = '<strong>' + t('info_project_archive') + '</strong> ' + t('info_project_archive_desc');
  const projectDelete = document.getElementById('info-text-project-delete');
  if (projectDelete) projectDelete.innerHTML = '<strong>' + t('info_project_delete') + '</strong> ' + t('info_project_delete_desc');
  
  const taskAdd = document.getElementById('info-text-task-add');
  if (taskAdd) taskAdd.innerHTML = '<strong>' + t('info_task_add') + '</strong> ' + t('info_task_add_desc');
  const taskComplete = document.getElementById('info-text-task-complete');
  if (taskComplete) taskComplete.innerHTML = '<strong>' + t('info_task_complete') + '</strong> ' + t('info_task_complete_desc');
  const taskTypes = document.getElementById('info-text-task-types');
  if (taskTypes) taskTypes.innerHTML = '<strong>' + t('info_task_types') + '</strong> ' + t('info_task_types_desc');
  const taskPriority = document.getElementById('info-text-task-priority');
  if (taskPriority) taskPriority.innerHTML = '<strong>' + t('info_priority_levels') + '</strong> ' + t('info_priority_levels_desc');
  const taskSubtasks = document.getElementById('info-text-task-subtasks');
  if (taskSubtasks) taskSubtasks.innerHTML = '<strong>' + t('info_subtasks') + '</strong> ' + t('info_subtasks_desc');
  const taskDependencies = document.getElementById('info-text-task-dependencies');
  if (taskDependencies) taskDependencies.innerHTML = '<strong>' + t('info_dependencies') + '</strong> ' + t('info_dependencies_desc');
  
  const filterAll = document.getElementById('info-text-filter-all');
  if (filterAll) filterAll.innerHTML = '<strong>' + t('info_filter_all') + '</strong> ' + t('info_filter_all_desc');
  const filterActive = document.getElementById('info-text-filter-active');
  if (filterActive) filterActive.innerHTML = '<strong>' + t('info_filter_active') + '</strong> ' + t('info_filter_active_desc');
  const filterCompleted = document.getElementById('info-text-filter-completed');
  if (filterCompleted) filterCompleted.innerHTML = '<strong>' + t('info_filter_completed') + '</strong> ' + t('info_filter_completed_desc');
  const filterTypes = document.getElementById('info-text-filter-types');
  if (filterTypes) filterTypes.innerHTML = '<strong>' + t('info_filter_types') + '</strong> ' + t('info_filter_types_desc');
  
  const searchText = document.getElementById('info-text-search');
  if (searchText) searchText.innerHTML = '<strong>' + t('info_search') + '</strong> ' + t('info_search_desc');
  const categorySwitch = document.getElementById('info-text-category-switch');
  if (categorySwitch) categorySwitch.innerHTML = '<strong>' + t('info_category_switch') + '</strong> ' + t('info_category_switch_desc');
  
  const rightClick = document.getElementById('info-text-right-click');
  if (rightClick) rightClick.innerHTML = '<strong>' + t('info_right_click') + '</strong> ' + t('info_right_click_desc');
  const doubleClick = document.getElementById('info-text-double-click');
  if (doubleClick) doubleClick.innerHTML = '<strong>' + t('info_double_click') + '</strong> ' + t('info_double_click_desc');
  const fabButton = document.getElementById('info-text-fab-button');
  if (fabButton) fabButton.innerHTML = '<strong>' + t('info_fab_button') + '</strong> ' + t('info_fab_button_desc');
  
  const guiTab = document.getElementById('info-text-gui-tab');
  if (guiTab) guiTab.innerHTML = '<strong>' + t('info_gui_tab') + '</strong> ' + t('info_gui_tab_desc');
  const themeText = document.getElementById('info-text-theme');
  if (themeText) themeText.innerHTML = '<strong>' + t('info_theme') + '</strong> ' + t('info_theme_desc');
  const storageText = document.getElementById('info-text-storage');
  if (storageText) storageText.innerHTML = '<strong>' + t('info_storage') + '</strong> ' + t('info_storage_desc');
  
  const dataDesc = document.getElementById('info-text-data-desc');
  if (dataDesc) dataDesc.textContent = t('info_data_storage_desc');
  const dataPersistent = document.getElementById('info-text-data-persistent');
  if (dataPersistent) dataPersistent.textContent = t('info_data_persistent');
  const permanentStorage = document.getElementById('info-text-permanent-storage');
  if (permanentStorage) permanentStorage.innerHTML = '<strong>' + t('info_permanent_storage') + '</strong> ' + t('info_permanent_storage_desc');
  
  // Backup/Restore section
  const backupTitle = document.getElementById('accordion-title-backup');
  if (backupTitle) backupTitle.textContent = '▶ ' + t('info_backup_restore');
  const backupSafe = document.getElementById('accordion-status-backup-safe');
  if (backupSafe) backupSafe.textContent = t('info_backup_safe');
  const backupDesc = document.getElementById('info-backup-desc');
  if (backupDesc) backupDesc.innerHTML = '<strong>' + t('info_backup_desc') + '</strong>';
  const backupIncluded = document.getElementById('info-backup-included');
  if (backupIncluded) backupIncluded.textContent = t('info_backup_included');
  const backupExcluded = document.getElementById('info-backup-excluded');
  if (backupExcluded) backupExcluded.textContent = t('info_backup_excluded');
  const exportBtn = document.getElementById('btn-backup-export');
  if (exportBtn) exportBtn.textContent = t('btn_export');
  const importBtn = document.getElementById('btn-backup-import');
  if (importBtn) importBtn.textContent = t('btn_import');
  
  // Factory Reset section
  const factoryTitle = document.getElementById('accordion-title-factory');
  if (factoryTitle) factoryTitle.textContent = '▶ ' + t('info_factory_reset');
  const factoryDanger = document.getElementById('accordion-status-factory-danger');
  if (factoryDanger) factoryDanger.textContent = t('info_factory_dangerous');
  const factoryWarning = document.getElementById('info-factory-warning');
  if (factoryWarning) factoryWarning.innerHTML = t('info_factory_warning');
  const factoryProjects = document.getElementById('info-factory-projects');
  if (factoryProjects) factoryProjects.textContent = t('info_factory_projects');
  const factoryGui = document.getElementById('info-factory-gui');
  if (factoryGui) factoryGui.textContent = t('info_factory_gui');
  const factoryWifi = document.getElementById('info-factory-wifi');
  if (factoryWifi) factoryWifi.textContent = t('info_factory_wifi');
  const factoryIrreversible = document.getElementById('info-factory-irreversible');
  if (factoryIrreversible) factoryIrreversible.innerHTML = '<strong>' + t('info_factory_irreversible') + '</strong>';
  const factoryBtn = document.getElementById('btn-factory-reset');
  if (factoryBtn) factoryBtn.textContent = t('info_factory_button');
  
  // Footer
  const footerMore = document.getElementById('info-footer-more');
  if (footerMore) footerMore.textContent = t('info_more_info');
  
  // Context menu
  const contextMenu = document.getElementById('context-menu');
  if (contextMenu) {
    const items = contextMenu.querySelectorAll('.context-menu-item');
    if (items[0]) items[0].textContent = t('ctx_edit');
    if (items[1]) items[1].textContent = t('ctx_copy');
    if (items[2]) items[2].textContent = t('ctx_archive');
    if (items[3]) items[3].textContent = t('ctx_delete');
  }
  
  console.log('[Language] Translations applied successfully');
}

// Override context menu creation to translate it
function translateContextMenu() {
  // Translate existing context menu immediately
  const contextMenu = document.getElementById('context-menu');
  if (contextMenu) {
    const items = contextMenu.querySelectorAll('.context-menu-item');
    if (items[0]) items[0].textContent = t('ctx_edit');
    if (items[1]) items[1].textContent = t('ctx_copy');
    if (items[2]) items[2].textContent = t('ctx_archive');
    if (items[3]) items[3].textContent = t('ctx_delete');
  }
  
  // Watch for dynamically added context menus
  const observer = new MutationObserver((mutations) => {
    mutations.forEach((mutation) => {
      mutation.addedNodes.forEach((node) => {
        if (node.classList && node.classList.contains('context-menu')) {
          const items = node.querySelectorAll('.context-menu-item');
          if (items[0]) items[0].textContent = t('ctx_edit');
          if (items[1]) items[1].textContent = t('ctx_copy');
          if (items[2]) items[2].textContent = t('ctx_archive');
          if (items[3]) items[3].textContent = t('ctx_delete');
        }
      });
    });
  });
  observer.observe(document.body, { childList: true, subtree: true });
}

// Setup language button handlers
function setupLanguageButtons() {
  document.querySelectorAll('.lang-btn').forEach(btn => {
    btn.addEventListener('click', async () => {
      const newLang = btn.dataset.lang;
      if (newLang !== currentLang) {
        currentLang = newLang;
        updateLanguageButtons();
        applyAllTranslations();
        await saveLanguage(newLang);
        
        // Refresh current content
        if (window.app && window.app.selectedProjectId) {
          window.app.loadTasksForProject(window.app.selectedProjectId);
        }
      }
    });
  });
}

// Initialize language system on page load
document.addEventListener('DOMContentLoaded', async () => {
  await loadLanguage();
  setupLanguageButtons();
  translateContextMenu();
});
)rawliteral";
}

#endif
