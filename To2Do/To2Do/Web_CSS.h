#ifndef WEB_CSS_H
#define WEB_CSS_H

const char* getCSS() {
  return R"rawliteral(
/* ==========================
   GLOBAL RESET & BASE STYLES
   ========================== */
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Courier New', monospace;
  background: #000;
  color: #fff;
  overflow: hidden;
  font-size: 14px;
  --datetime-bg: #0a0a0a;
  --datetime-border: #333;
  --datetime-text: #fff;
  --datetime-label: #666;
}

/* ==========================
   LIGHT THEME OVERRIDES
   ========================== */
body[data-theme="light"] {
  background: #f5f5f5;
  color: #000;
  --datetime-bg: #fff;
  --datetime-border: #ddd;
  --datetime-text: #000;
  --datetime-label: #888;
}

body[data-theme="light"] .workspace {
  background: #f5f5f5;
}

body[data-theme="light"] .sidebar {
  background: #fff;
  border-right: 1px solid #ddd;
}

body[data-theme="light"] .sidebar-header {
  border-bottom: 1px solid #ddd;
}

body[data-theme="light"] .stat-label {
  color: #999;
}

body[data-theme="light"] .sidebar-tab {
  background: #fff;
  border-right: 1px solid #ddd;
  color: #999;
}

body[data-theme="light"] .sidebar-tab.active {
  background: #f5f5f5;
  color: #000;
  border-bottom: 2px solid #000;
}

body[data-theme="light"] .project-search {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #000;
}

body[data-theme="light"] .sidebar-project {
  background: #fff;
  border: 1px solid #ddd;
}

body[data-theme="light"] .sidebar-project:hover {
  background: #f9f9f9;
  border-color: #ccc;
}

body[data-theme="light"] .sidebar-project.active {
  background: #f0f0f0;
  border-color: #000;
}

body[data-theme="light"] .project-desc {
  color: #666;
}

body[data-theme="light"] .project-count {
  color: #999;
}

body[data-theme="light"] .sidebar-btn {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #000;
}

body[data-theme="light"] .sidebar-btn:hover {
  background: #e5e5e5;
  border-color: #000;
}

body[data-theme="light"] .panel {
  background: #f5f5f5;
}

body[data-theme="light"] .panel-header {
  border-bottom: 1px solid #ddd;
}

body[data-theme="light"] .panel-subtitle {
  color: #666;
}

body[data-theme="light"] .panel-filter-btn {
  background: #fff;
  border: 1px solid #ddd;
  color: #999;
}

body[data-theme="light"] .panel-filter-btn.active {
  background: #000;
  color: #fff;
  border-color: #000;
}

body[data-theme="light"] .panel-filter-btn:hover:not(.active) {
  background: #f9f9f9;
}

body[data-theme="light"] .task-item {
  background: #fff;
  border: 1px solid #ddd;
}

body[data-theme="light"] .task-item:hover {
  background: #f9f9f9;
  border-color: #ccc;
}

body[data-theme="light"] .task-checkbox {
  border: 2px solid #999;
}

body[data-theme="light"] .task-checkbox.checked {
  background: #000;
}

body[data-theme="light"] .task-checkbox.checked::after {
  border-color: #fff;
}

body[data-theme="light"] .task-type-badge {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #666;
}

body[data-theme="light"] .task-description {
  color: #555;
}

body[data-theme="light"] .task-dependencies {
  background: #f9f9f9;
  border: 1px solid #ddd;
}

body[data-theme="light"] .dependency-label {
  color: #999;
}

body[data-theme="light"] .dependency-item {
  color: #666;
  border-bottom: 1px solid #eee;
}

body[data-theme="light"] .dependency-remove {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #666;
}

body[data-theme="light"] .dependency-remove:hover {
  background: #f00;
  border-color: #f00;
  color: #fff;
}

body[data-theme="light"] .checklist-checkbox {
  border: 1px solid #999;
}

body[data-theme="light"] .checklist-checkbox.checked {
  background: #999;
}

body[data-theme="light"] .checklist-text {
  color: #555;
}

body[data-theme="light"] .checklist-text.completed {
  color: #aaa;
}

body[data-theme="light"] .checklist-remove {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #999;
}

body[data-theme="light"] .checklist-remove:hover {
  background: #f00;
  border-color: #f00;
  color: #fff;
}

body[data-theme="light"] .checklist-input {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #000;
}

body[data-theme="light"] .task-meta {
  color: #999;
}

body[data-theme="light"] .task-action-btn {
  background: #fff;
  border: 1px solid #ddd;
  color: #666;
}

body[data-theme="light"] .task-action-btn:hover {
  background: #f9f9f9;
  border-color: #000;
  color: #000;
}

body[data-theme="light"] .task-action-btn.delete:hover {
  background: #f00;
  border-color: #f00;
  color: #fff;
}

body[data-theme="light"] .empty-state {
  color: #bbb;
}

body[data-theme="light"] .fab {
  background: #000;
  color: #fff;
  box-shadow: 0 4px 12px rgba(0,0,0,.3);
}

body[data-theme="light"] .fab:hover {
  box-shadow: 0 6px 16px rgba(0,0,0,.5);
}

body[data-theme="light"] .modal {
  background: rgba(0,0,0,.7);
}

body[data-theme="light"] .modal-content {
  background: #fff;
  border: 2px solid #ddd;
}

body[data-theme="light"] .modal-input,
body[data-theme="light"] .modal-textarea,
body[data-theme="light"] .modal-select,
body[data-theme="light"] .modal-date {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #000;
}

body[data-theme="light"] .modal-input:focus,
body[data-theme="light"] .modal-textarea:focus,
body[data-theme="light"] .modal-select:focus,
body[data-theme="light"] .modal-date:focus {
  border-color: #000;
}

body[data-theme="light"] .modal-date::-webkit-calendar-picker-indicator {
  filter: invert(0);
}

body[data-theme="light"] .modal-btn {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #000;
}

body[data-theme="light"] .modal-btn.primary {
  background: #000;
  color: #fff;
}

body[data-theme="light"] .modal-btn:hover {
  background: #e5e5e5;
  border-color: #000;
}

body[data-theme="light"] .modal-btn.primary:hover {
  background: #333;
}

body[data-theme="light"] .settings-tab {
  background: #fff;
  color: #999;
}

body[data-theme="light"] .settings-tab.active {
  color: #000;
  border-bottom-color: #000;
}

body[data-theme="light"] .settings-tab:hover:not(.active) {
  background: #f9f9f9;
}

body[data-theme="light"] .settings-label {
  color: #666;
}

body[data-theme="light"] .settings-input {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #000;
}

body[data-theme="light"] .settings-input:focus {
  border-color: #000;
}

body[data-theme="light"] .theme-label {
  color: #666;
}

body[data-theme="light"] .toggle-slider {
  background: #ddd;
}

body[data-theme="light"] .toggle-slider:before {
  background: #000;
}

body[data-theme="light"] .toggle-switch input:checked+.toggle-slider {
  background: #000;
}

body[data-theme="light"] .toggle-switch input:checked+.toggle-slider:before {
  background: #fff;
}

body[data-theme="light"] .context-menu {
  background: #fff;
  border: 1px solid #ddd;
}

body[data-theme="light"] .context-menu-item {
  border-bottom: 1px solid #eee;
}

body[data-theme="light"] .context-menu-item:hover {
  background: #f9f9f9;
}

body[data-theme="light"] .context-menu-item.danger:hover {
  background: #f00;
  color: #fff;
}

body[data-theme="light"] .toast {
  background: #000;
  color: #fff;
  border: 2px solid #000;
}

body[data-theme="light"] .toast.error {
  background: #f00;
  color: #fff;
}

body[data-theme="light"] ::-webkit-scrollbar-track {
  background: #f5f5f5;
}

body[data-theme="light"] ::-webkit-scrollbar-thumb {
  background: #ddd;
}

body[data-theme="light"] ::-webkit-scrollbar-thumb:hover {
  background: #ccc;
}

/* ==========================
   WORKSPACE & LAYOUT
   ========================== */
.workspace {
  display: flex;
  height: 100vh;
}

/* ==========================
   SIDEBAR
   ========================== */
.sidebar {
  width: 450px;
  background: #0a0a0a;
  border-right: 1px solid #222;
  display: flex;
  flex-direction: column;
}

.sidebar-header {
  padding: 20px;
  border-bottom: 1px solid #222;
}

.sidebar-title {
  font-size: 20px;
  letter-spacing: 3px;
  margin-bottom: 15px;
}

.sidebar-stats {
  display: flex;
  justify-content: space-between;
}

.stat {
  text-align: center;
}

.stat-value {
  font-size: 28px;
  font-weight: bold;
}

.stat-label {
  font-size: 12px;
  color: #666;
  margin-top: 5px;
}

.sidebar-tabs {
  display: flex;
  border-bottom: 1px solid #222;
}

.sidebar-tab {
  flex: 1;
  padding: 12px;
  text-align: center;
  background: #0a0a0a;
  border: none;
  border-right: 1px solid #222;
  color: #666;
  cursor: pointer;
  transition: all .2s;
  font-family: 'Courier New', monospace;
  font-size: 14px;
  font-weight: bold;
}

.sidebar-tab:last-child {
  border-right: none;
}

.sidebar-tab.active {
  background: #000;
  color: #fff;
  border-bottom: 2px solid #fff;
}

.sidebar-search {
  padding: 15px;
  border-bottom: 1px solid #222;
}

.project-search {
  width: 100%;
  padding: 10px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  font-family: 'Courier New', monospace;
  font-size: 14px;
}

.sidebar-body {
  flex: 1;
  overflow-y: auto;
}

.projects-list {
  padding: 15px;
  padding-bottom: 100px;  /* Space for context menu */
}

.project-group-label {
  font-size: 11px;
  color: #444;
  margin-bottom: 10px;
  letter-spacing: 2px;
}

.sidebar-project {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px;
  margin-bottom: 8px;
  background: #0a0a0a;
  border: 1px solid #222;
  cursor: pointer;
  transition: all .2s;
}

.sidebar-project:hover {
  background: #111;
  border-color: #333;
}

.sidebar-project.active {
  background: #111;
  border-color: #fff;
}

.project-info {
  flex: 1;
}

.project-name {
  font-size: 15px;
  font-weight: bold;
  margin-bottom: 4px;
}

.project-desc {
  font-size: 12px;
  color: #666;
}

.project-meta {
  display: flex;
  align-items: center;
  gap: 10px;
}

.project-priority {
  width: 8px;
  height: 8px;
  border-radius: 50%;
}

.priority-high {
  background: #f00;
}

.priority-medium {
  background: #ff0;
}

.priority-low {
  background: #0f0;
}

.project-count {
  font-size: 12px;
  color: #666;
}

.sidebar-footer {
  padding: 15px;
  border-top: 1px solid #222;
  display: flex;
  gap: 10px;
}

.sidebar-btn {
  flex: 1;
  padding: 12px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  cursor: pointer;
  transition: all .2s;
  font-family: 'Courier New', monospace;
  font-size: 13px;
}

.sidebar-btn:hover {
  background: #222;
  border-color: #fff;
}

/* ==========================
   MAIN PANEL
   ========================== */
.panel {
  flex: 1;
  display: flex;
  flex-direction: column;
  overflow: hidden;
}

.panel-header {
  padding: 20px;
  border-bottom: 1px solid #222;
}

.panel-title-row {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 5px;
}

.panel-title {
  font-size: 22px;
}

.panel-actions {
  display: flex;
  gap: 8px;
}

.panel-btn {
  padding: 8px 12px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  cursor: pointer;
  font-family: 'Courier New', monospace;
  font-size: 12px;
  transition: all .2s;
}

.panel-btn:hover {
  background: #222;
  border-color: #fff;
}

.panel-subtitle {
  font-size: 13px;
  color: #888;
  font-style: italic;
  margin-bottom: 15px;
}

.panel-filters {
  display: flex;
  gap: 6px;
  flex-wrap: wrap;
}

.panel-filter-btn {
  padding: 6px 10px;
  background: #0a0a0a;
  border: 1px solid #333;
  color: #666;
  cursor: pointer;
  transition: all .2s;
  font-family: 'Courier New', monospace;
  font-size: 12px;
}

.panel-filter-btn.active {
  background: #fff;
  color: #000;
  border-color: #fff;
}

.panel-filter-btn:hover:not(.active) {
  background: #111;
}

.panel-body {
  flex: 1;
  overflow-y: auto;
  padding: 20px;
}

.tasks-container {
  max-width: 1400px;
  padding-bottom: 100px;  /* Space for FAB button */
}

/* ==========================
   TASK ITEMS
   ========================== */
.task-item {
  margin-bottom: 15px;
  padding: 15px;
  background: #0a0a0a;
  border: 1px solid #222;
  border-left: 3px solid #444;
  transition: all .2s;
}

.task-item:hover {
  background: #111;
  border-color: #333;
}

.task-item.completed {
  opacity: .6;
}

.task-item.priority-high {
  border-left-color: #f00;
}

.task-item.priority-medium {
  border-left-color: #ff0;
}

.task-item.priority-low {
  border-left-color: #0f0;
}

.task-header {
  display: flex;
  gap: 15px;
}

.task-checkbox {
  width: 20px;
  height: 20px;
  border: 2px solid #666;
  cursor: pointer;
  flex-shrink: 0;
  margin-top: 2px;
}

.task-checkbox.checked {
  background: #fff;
  position: relative;
}

.task-checkbox.checked::after {
  content: '';
  position: absolute;
  left: 5px;
  top: 2px;
  width: 5px;
  height: 10px;
  border: solid #000;
  border-width: 0 2px 2px 0;
  transform: rotate(45deg);
}

.task-content {
  flex: 1;
}

.task-title-row {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 8px;
}

.task-title {
  font-size: 15px;
  font-weight: bold;
  flex: 1;
}

.task-type-badge {
  padding: 5px 10px;
  background: #111;
  border: 1px solid #333;
  font-size: 11px;
  color: #888;
}

.task-description {
  font-size: 13px;
  color: #aaa;
  margin-bottom: 10px;
}

/* ==========================
   DEPENDENCIES
   ========================== */
.task-dependencies {
  margin: 10px 0;
  padding: 10px;
  background: #050505;
  border: 1px solid #222;
  font-size: 12px;
}

.dependency-label {
  color: #666;
  margin-bottom: 8px;
  font-size: 11px;
  letter-spacing: 1px;
}

.dependency-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 6px 0;
  color: #888;
  border-bottom: 1px solid #111;
}

.dependency-item:last-child {
  border-bottom: none;
}

.dependency-text {
  flex: 1;
}

.dependency-text::before {
  content: '→ ';
  color: #f00;
  font-weight: bold;
}

.dependency-remove {
  padding: 4px 8px;
  background: #111;
  border: 1px solid #333;
  color: #888;
  cursor: pointer;
  font-size: 10px;
  transition: all .2s;
}

.dependency-remove:hover {
  background: #f00;
  border-color: #f00;
  color: #000;
}

/* ==========================
   CHECKLIST
   ========================== */
.task-checklist {
  margin: 10px 0;
  padding-left: 10px;
}

.checklist-item {
  display: flex;
  align-items: flex-start;
  gap: 8px;
  margin-bottom: 6px;
  font-size: 13px;
}

.checklist-checkbox {
  width: 14px;
  height: 14px;
  border: 1px solid #666;
  cursor: pointer;
  flex-shrink: 0;
  margin-top: 2px;
}

.checklist-checkbox.checked {
  background: #666;
}

.checklist-text {
  flex: 1;
  color: #aaa;
}

.checklist-text.completed {
  text-decoration: line-through;
  color: #555;
}

.checklist-remove {
  padding: 2px 6px;
  background: #111;
  border: 1px solid #333;
  color: #666;
  cursor: pointer;
  font-size: 10px;
  margin-left: 8px;
  transition: all .2s;
}

.checklist-remove:hover {
  background: #f00;
  border-color: #f00;
  color: #000;
}

.checklist-input {
  width: 100%;
  padding: 6px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  font-family: 'Courier New', monospace;
  font-size: 12px;
}

/* ==========================
   TASK FOOTER
   ========================== */
.task-footer {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-top: 10px;
}

.task-meta {
  display: flex;
  gap: 10px;
  font-size: 12px;
  color: #666;
}

.task-actions {
  display: flex;
  gap: 8px;
}

.task-action-btn {
  padding: 6px 10px;
  background: #0a0a0a;
  border: 1px solid #333;
  color: #888;
  cursor: pointer;
  transition: all .2s;
  font-family: 'Courier New', monospace;
  font-size: 12px;
}

.task-action-btn:hover {
  background: #111;
  border-color: #fff;
  color: #fff;
}

.task-action-btn.delete:hover {
  background: #f00;
  border-color: #f00;
  color: #000;
}

/* ==========================
   EMPTY STATE
   ========================== */
.empty-state {
  text-align: center;
  padding: 60px 20px;
  color: #444;
}

/* ==========================
   FAB BUTTON
   ========================== */
.fab {
  position: fixed;
  bottom: 30px;
  right: 30px;
  width: 60px;
  height: 60px;
  background: #fff;
  border: none;
  border-radius: 50%;
  color: #000;
  font-size: 28px;
  cursor: pointer;
  box-shadow: 0 4px 12px rgba(255,255,255,.3);
  transition: all .2s;
  z-index: 100;
}

.fab:hover {
  transform: scale(1.1);
  box-shadow: 0 6px 16px rgba(255,255,255,.5);
}

.fab:active {
  transform: scale(.95);
}

.notification-fab {
  font-weight: bold;
  font-size: 32px;
}

.notification-badge {
  position: absolute;
  top: -5px;
  right: -5px;
  background: #000;
  color: #fff;
  border-radius: 50%;
  width: 20px;
  height: 20px;
  font-size: 11px;
  font-weight: bold;
  display: none;
  align-items: center;
  justify-content: center;
}

body[data-theme="light"] .notification-badge {
  background: #fff;
  color: #000;
  border: 2px solid #000;
}

/* ==========================
   MODAL
   ========================== */
.modal {
  display: none;
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: rgba(0,0,0,.85);
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.modal.active {
  display: flex;
}

.modal-content {
  background: #0a0a0a;
  border: 2px solid #333;
  width: 600px;
  max-width: 90%;
  max-height: 90vh;
  display: flex;
  flex-direction: column;
}

.modal-title {
  font-size: 20px;
  margin-bottom: 20px;
  letter-spacing: 2px;
  padding: 30px 30px 0 30px;
}

.modal-form-row {
  display: grid;
  grid-template-columns: 2fr 1fr 1fr;
  gap: 10px;
  margin-bottom: 15px;
  padding: 0 30px;
}

.modal-input,
.modal-textarea,
.modal-select,
.modal-date {
  width: 100%;
  padding: 10px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  font-family: 'Courier New', monospace;
  font-size: 14px;
}

.modal-input:focus,
.modal-textarea:focus,
.modal-select:focus,
.modal-date:focus {
  outline: none;
  border-color: #fff;
}

.modal-textarea {
  min-height: 80px;
  resize: vertical;
  margin-bottom: 15px;
}

.modal-date::-webkit-calendar-picker-indicator {
  filter: invert(1);
}

.modal-actions {
  display: flex;
  gap: 10px;
  padding: 20px 30px;
  border-top: 1px solid #333;
  margin-top: auto;
}

.modal-btn {
  flex: 1;
  padding: 12px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  cursor: pointer;
  font-family: 'Courier New', monospace;
  font-size: 14px;
  transition: all .2s;
}

.modal-btn.primary {
  background: #fff;
  color: #000;
}

.modal-btn:hover {
  background: #222;
  border-color: #fff;
}

.modal-btn.primary:hover {
  background: #ddd;
}

/* ==========================
   SETTINGS
   ========================== */
.settings-tabs {
  display: flex;
  gap: 5px;
  margin-bottom: 20px;
  border-bottom: 1px solid #333;
  padding: 30px 30px 0 30px;
}

.settings-tab {
  flex: 1;
  padding: 12px;
  background: #0a0a0a;
  border: none;
  border-bottom: 2px solid transparent;
  color: #666;
  cursor: pointer;
  transition: all .2s;
  font-family: 'Courier New', monospace;
  font-size: 13px;
  font-weight: bold;
}

.settings-tab.active {
  color: #fff;
  border-bottom-color: #fff;
}

.settings-tab:hover:not(.active) {
  background: #111;
}

.settings-content {
  flex: 1;
  overflow-y: auto;
  padding: 20px 30px;
}

.settings-tab-content {
  display: none;
}

.settings-tab-content.active {
  display: block;
}

.settings-tab-content p {
  margin-bottom: 10px;
  line-height: 1.6;
  color: #aaa;
}

.settings-section {
  margin-bottom: 25px;
}

.settings-label {
  display: block;
  font-size: 12px;
  color: #888;
  margin-bottom: 8px;
  letter-spacing: 1px;
}

.settings-input {
  width: 100%;
  padding: 10px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  font-family: 'Courier New', monospace;
  font-size: 14px;
  margin-bottom: 8px;
}

.settings-input:focus {
  outline: none;
  border-color: #fff;
}

.theme-toggle {
  display: flex;
  align-items: center;
  gap: 15px;
  padding: 10px 0;
}

.theme-label {
  font-size: 13px;
  color: #888;
  min-width: 50px;
}

.toggle-switch {
  position: relative;
  width: 50px;
  height: 24px;
  display: inline-block;
}

.toggle-switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

.toggle-slider {
  position: absolute;
  cursor: pointer;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: #333;
  transition: .3s;
  border-radius: 24px;
}

.toggle-slider:before {
  position: absolute;
  content: "";
  height: 18px;
  width: 18px;
  left: 3px;
  bottom: 3px;
  background: #fff;
  transition: .3s;
  border-radius: 50%;
}

.toggle-switch input:checked+.toggle-slider {
  background: #fff;
}

.toggle-switch input:checked+.toggle-slider:before {
  transform: translateX(26px);
  background: #000;
}

/* ==========================
   LANGUAGE SELECTOR BUTTONS
   ========================== */
.language-selector {
  display: flex;
  gap: 8px;
  align-items: center;
}

.lang-btn {
  width: 40px;
  height: 40px;
  border: 2px solid #fff;
  background: #000;
  color: #fff;
  font-family: 'Courier New', monospace;
  font-size: 12px;
  font-weight: bold;
  cursor: pointer;
  border-radius: 4px;
  transition: all 0.2s ease;
  display: flex;
  align-items: center;
  justify-content: center;
}

.lang-btn:hover {
  opacity: 0.8;
  transform: scale(1.05);
}

.lang-btn.active {
  background: #fff;
  color: #000;
  border-color: #fff;
}

/* Light theme language buttons */
body[data-theme="light"] .lang-btn {
  border: 2px solid #000;
  background: #fff;
  color: #000;
}

body[data-theme="light"] .lang-btn.active {
  background: #000;
  color: #fff;
  border-color: #000;
}

body[data-theme="light"] .lang-btn:hover {
  opacity: 0.8;
}

/* ==========================
   ACCORDION
   ========================== */
.accordion {
  margin-bottom: 20px;
}

.accordion-item {
  margin-bottom: 10px;
  border: 1px solid #333;
  background: #0a0a0a;
}

.accordion-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 15px;
  cursor: pointer;
  transition: all .2s;
  border-bottom: 1px solid transparent;
}

.accordion-header:hover {
  background: #111;
}

.accordion-header.active {
  background: #111;
  border-bottom-color: #333;
}

.accordion-title {
  font-size: 14px;
  font-weight: bold;
  color: #fff;
  letter-spacing: 1px;
}

.accordion-status {
  font-size: 11px;
  padding: 4px 8px;
  background: #222;
  border: 1px solid #333;
  border-radius: 3px;
  color: #888;
}

.accordion-status.active {
  background: #0f0;
  border-color: #0f0;
  color: #000;
}

.accordion-status.connected {
  background: #0ff;
  border-color: #0ff;
  color: #000;
}

.accordion-status.error {
  background: #f00;
  border-color: #f00;
  color: #fff;
}

.accordion-content {
  display: none;
  padding: 20px;
  border-top: 1px solid #333;
}

.accordion-content.active {
  display: block;
}

.settings-hint {
  font-size: 11px;
  color: #666;
  margin-top: 5px;
  font-style: italic;
}

.settings-hint span {
  color: #fff;
  font-weight: bold;
}

.settings-checkbox {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-top: 8px;
  cursor: pointer;
  font-size: 13px;
  color: #888;
}

.settings-checkbox input {
  cursor: pointer;
}

.settings-btn {
  width: 100%;
  padding: 12px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  cursor: pointer;
  font-family: 'Courier New', monospace;
  font-size: 13px;
  font-weight: bold;
  transition: all .2s;
  margin-top: 10px;
}

.settings-btn:hover {
  background: #222;
  border-color: #fff;
}

.connection-info {
  margin-top: 25px;
  padding: 20px;
  background: #0a0a0a;
  border: 2px solid #333;
}

.connection-info-title {
  font-size: 13px;
  font-weight: bold;
  color: #fff;
  margin-bottom: 15px;
  letter-spacing: 1px;
}

.connection-status-grid {
  display: grid;
  grid-template-columns: repeat(2,1fr);
  gap: 15px;
}

.connection-status-item {
  display: flex;
  flex-direction: column;
  gap: 5px;
}

.status-label {
  font-size: 11px;
  color: #666;
}

.status-value {
  font-size: 13px;
  color: #fff;
  font-weight: bold;
}

body[data-theme="light"] .accordion-item {
  background: #fff;
  border: 1px solid #ddd;
}

body[data-theme="light"] .accordion-header:hover {
  background: #f9f9f9;
}

body[data-theme="light"] .accordion-header.active {
  background: #f9f9f9;
  border-bottom-color: #ddd;
}

body[data-theme="light"] .accordion-title {
  color: #000;
}

body[data-theme="light"] .accordion-status {
  background: #f5f5f5;
  border-color: #ddd;
  color: #666;
}

body[data-theme="light"] .accordion-status.active {
  background: #0f0;
  border-color: #0f0;
  color: #000;
}

body[data-theme="light"] .accordion-status.connected {
  background: #0ff;
  border-color: #0ff;
  color: #000;
}

body[data-theme="light"] .accordion-status.error {
  background: #f00;
  border-color: #f00;
  color: #fff;
}

body[data-theme="light"] .accordion-content {
  border-top: 1px solid #ddd;
}

body[data-theme="light"] .settings-hint {
  color: #999;
}

body[data-theme="light"] .settings-hint span {
  color: #000;
}

body[data-theme="light"] .settings-checkbox {
  color: #666;
}

body[data-theme="light"] .settings-btn {
  background: #f5f5f5;
  border-color: #ddd;
  color: #000;
}

body[data-theme="light"] .settings-btn:hover {
  background: #e5e5e5;
  border-color: #000;
}

body[data-theme="light"] .connection-info {
  background: #f9f9f9;
  border-color: #ddd;
}

body[data-theme="light"] .connection-info-title {
  color: #000;
}

body[data-theme="light"] .status-label {
  color: #999;
}

body[data-theme="light"] .status-value {
  color: #000;
}

/* ==========================
   INFO SECTIONS
   ========================== */
.info-status-bar {
  display: grid;
  grid-template-columns: repeat(4,1fr);
  gap: 10px;
  margin-bottom: 20px;
  padding: 15px;
  background: #0a0a0a;
  border: 1px solid #333;
}

.info-status-item {
  text-align: center;
  padding: 8px;
  background: #111;
  border: 1px solid #222;
}

.info-status-label {
  display: block;
  font-size: 10px;
  color: #666;
  margin-bottom: 4px;
  letter-spacing: 1px;
}

.info-status-value {
  display: block;
  font-size: 12px;
  color: #fff;
  font-weight: bold;
}

.info-section {
  margin-bottom: 20px;
  padding: 15px;
  background: #0a0a0a;
  border: 1px solid #222;
  border-left: 3px solid #444;
}

.info-section span {
  color: #fff;
  font-weight: bold;
}

.info-title {
  font-size: 14px;
  font-weight: bold;
  color: #fff;
  margin-bottom: 10px;
  letter-spacing: 1px;
}

.info-text {
  font-size: 13px;
  line-height: 1.8;
  color: #aaa;
  margin-bottom: 6px;
  padding-left: 10px;
}

.info-text strong {
  color: #fff;
}

.info-footer {
  margin-top: 20px;
  padding: 15px;
  text-align: center;
  background: #0a0a0a;
  border: 1px solid #333;
}

.info-footer-text {
  font-size: 12px;
  color: #888;
  margin-bottom: 8px;
}

.info-footer-link {
  display: inline-block;
  padding: 8px 16px;
  background: #111;
  border: 1px solid #333;
  color: #fff;
  text-decoration: none;
  font-size: 12px;
  transition: all .2s;
}

.info-footer-link:hover {
  background: #222;
  border-color: #fff;
  transform: translateX(3px);
}

body[data-theme="light"] .info-status-bar {
  background: #fff;
  border: 1px solid #ddd;
}

body[data-theme="light"] .info-status-item {
  background: #f9f9f9;
  border: 1px solid #ddd;
}

body[data-theme="light"] .info-status-label {
  color: #999;
}

body[data-theme="light"] .info-status-value {
  color: #000;
}

body[data-theme="light"] .info-section {
  background: #fff;
  border: 1px solid #ddd;
  border-left: 3px solid #999;
}

body[data-theme="light"] .info-section span {
  color: #000;
}

body[data-theme="light"] .info-title {
  color: #000;
}

body[data-theme="light"] .info-text {
  color: #555;
}

body[data-theme="light"] .info-text strong {
  color: #000;
}

body[data-theme="light"] .info-footer {
  background: #f9f9f9;
  border: 1px solid #ddd;
}

body[data-theme="light"] .info-footer-text {
  color: #666;
}

body[data-theme="light"] .info-footer-link {
  background: #f5f5f5;
  border: 1px solid #ddd;
  color: #000;
}

body[data-theme="light"] .info-footer-link:hover {
  background: #e5e5e5;
  border-color: #000;
}

/* ==========================
   CONTEXT MENU
   ========================== */
.context-menu {
  display: none;
  position: fixed;
  background: #0a0a0a;
  border: 1px solid #333;
  min-width: 200px;
  z-index: 2000;
}

.context-menu.active {
  display: block;
}

.context-menu-item {
  padding: 12px 15px;
  cursor: pointer;
  border-bottom: 1px solid #222;
  font-size: 13px;
}

.context-menu-item:hover {
  background: #111;
}

.context-menu-item.danger:hover {
  background: #f00;
  color: #000;
}

/* ==========================
   TOAST NOTIFICATIONS
   ========================== */
.toast {
  position: fixed;
  bottom: 30px;
  left: 50%;
  transform: translateX(-50%);
  padding: 15px 25px;
  background: #fff;
  color: #000;
  font-size: 13px;
  border: 2px solid #fff;
  z-index: 3000;
  animation: slideUp .3s ease;
}

.toast.error {
  background: #f00;
  color: #fff;
}

@keyframes slideUp {
  from {
    transform: translateX(-50%) translateY(100px);
    opacity: 0;
  }
  to {
    transform: translateX(-50%) translateY(0);
    opacity: 1;
  }
}

/* ==========================
   SCROLLBAR
   ========================== */
::-webkit-scrollbar {
  width: 8px;
}

::-webkit-scrollbar-track {
  background: #0a0a0a;
}

::-webkit-scrollbar-thumb {
  background: #333;
}

::-webkit-scrollbar-thumb:hover {
  background: #555;
}

/* ==========================
   RESPONSIVE DESIGN
   ========================== */
@media(max-width:768px) {
  body {
    overflow: auto;
  }
  
  .workspace {
    flex-direction: column;
    height: auto;
  }
  
  .sidebar {
    width: 100%;
    border-right: none;
    border-bottom: 1px solid #222;
  }
  
  .sidebar-header {
    padding: 15px;
  }
  
  .sidebar-title {
    font-size: 18px;
  }
  
  .sidebar-stats {
    gap: 10px;
  }
  
  .stat-value {
    font-size: 20px;
  }
  
  .sidebar-tabs {
    flex-wrap: nowrap;
    overflow-x: auto;
  }
  
  .sidebar-tab {
    font-size: 12px;
    padding: 10px;
  }
  
  .sidebar-body {
    max-height: 40vh;
  }
  
  .panel {
    width: 100%;
  }
  
  .panel-header {
    padding: 15px;
  }
  
  .panel-title {
    font-size: 18px;
  }
  
  .panel-filters {
    overflow-x: auto;
    flex-wrap: nowrap;
  }
  
  .panel-body {
    padding: 10px;
  }
  
  .task-item {
    padding: 10px;
  }
  
  .task-title {
    font-size: 14px;
  }
  
  .modal-content {
    padding: 20px;
    width: 95%;
  }
  
  .modal-form-row {
    grid-template-columns: 1fr;
    gap: 8px;
  }
  
  .fab {
    bottom: 20px;
    right: 20px;
    width: 50px;
    height: 50px;
    font-size: 24px;
  }
  
  .info-status-bar {
    grid-template-columns: repeat(2,1fr);
  }
  
  .info-status-item {
    font-size: 11px;
  }
}
)rawliteral";
}

#endif
