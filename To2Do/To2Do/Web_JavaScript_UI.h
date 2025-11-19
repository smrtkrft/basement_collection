#ifndef WEB_JAVASCRIPT_UI_H
#define WEB_JAVASCRIPT_UI_H

// UI Library - Arayüz Render Fonksiyonları
const char* getJavaScriptUI() {
  return R"XJSUIX(
// ==================== UI RENDERING FUNCTIONS ====================
// Project and Task Render Functions

function renderProjects(searchTerm = '') {
    const container = document.getElementById('projects-list');
    const filtered = app.projects
        .filter(p => p.category === app.currentCategory)
        .filter(p => !searchTerm || p.name.toLowerCase().includes(searchTerm.toLowerCase()));

    const active = filtered.filter(p => !p.archived);
    const archived = filtered.filter(p => p.archived);

    let html = '';

    if (active.length > 0) {
        html += '<div class="project-group-label">' + t('projects_header') + '</div>';
        active.forEach(project => {
            html += renderProject(project);
        });
    }

    if (archived.length > 0) {
        html += '<div class="project-group-label" style="margin-top: 20px;">' + t('archive_header') + '</div>';
        archived.forEach(project => {
            html += renderProject(project);
        });
    }

    if (filtered.length === 0) {
        html = '<div class="empty-state"><h3>No projects found</h3></div>';
    }

    container.innerHTML = html;

    container.querySelectorAll('.sidebar-project').forEach(el => {
        el.addEventListener('click', () => {
            selectProject(parseInt(el.dataset.projectId));
        });

        el.addEventListener('contextmenu', (e) => {
            e.preventDefault();
            showContextMenu(e, parseInt(el.dataset.projectId));
        });
    });
}

function renderProject(project) {
    const taskCount = app.tasks.filter(t => t.projectId === project.id).length;
    const activeClass = app.currentProject === project.id ? 'active' : '';

    return `
        <div class="sidebar-project ${activeClass}" data-project-id="${project.id}">
            <div class="project-info">
                <div class="project-name">${escapeHtml(project.name)}</div>
                ${project.description ? `<div class="project-desc">${escapeHtml(project.description)}</div>` : ''}
            </div>
            <div class="project-meta">
                <div class="project-priority priority-${project.priority}"></div>
                <div class="project-count">${taskCount}</div>
            </div>
        </div>
    `;
}

function selectProject(projectId) {
    app.currentProject = projectId;
    renderProjects();
    renderTasks();
    
    const project = app.projects.find(p => p.id === projectId);
    if (project) {
        document.getElementById('current-project').textContent = project.name;
        document.getElementById('current-project-desc').textContent = project.description || '';
    }
}

function renderTasks() {
    const container = document.getElementById('tasks-container');
    
    if (!app.currentProject) {
        container.innerHTML = '<div class="empty-state"><h3>' + t('empty_no_project') + '</h3><p>' + t('empty_select_project') + '</p></div>';
        return;
    }

    let filtered = app.tasks.filter(t => t.projectId === app.currentProject);

    switch (app.currentFilter) {
        case 'active':
            filtered = filtered.filter(t => !t.completed);
            break;
        case 'completed':
            filtered = filtered.filter(t => t.completed);
            break;
        case 'task':
        case 'plan':
        case 'note':
        case 'reminder':
            filtered = filtered.filter(t => t.type === app.currentFilter);
            break;
    }

    filtered.sort((a, b) => {
        if (a.completed !== b.completed) return a.completed ? 1 : -1;
        const prio = { high: 3, medium: 2, low: 1 };
        return (prio[b.priority] || 2) - (prio[a.priority] || 2);
    });

    if (filtered.length === 0) {
        container.innerHTML = '<div class="empty-state"><h3>' + t('empty_title') + '</h3><p>' + t('empty_desc') + '</p></div>';
        return;
    }

    container.innerHTML = filtered.map(task => renderTask(task)).join('');
    bindTaskEvents();
}

function renderTask(task) {
    const typeLabels = { task: t('filter_task'), plan: t('filter_plan'), note: t('filter_note'), reminder: t('filter_reminder') };

    const formatDate = (dateStr) => {
        if (!dateStr) return t('date_no_date');
        const date = new Date(dateStr);
        const today = new Date();
        const tomorrow = new Date(today);
        tomorrow.setDate(tomorrow.getDate() + 1);
        
        if (date.toDateString() === today.toDateString()) return t('notif_today');
        if (date.toDateString() === tomorrow.toDateString()) return t('notif_tomorrow');
        
        return date.toLocaleDateString('tr-TR', { day: '2-digit', month: '2-digit', year: 'numeric' });
    };

    const blockedBy = task.dependencies
        .map(depId => app.tasks.find(t => t.id === depId))
        .filter(t => t && !t.completed);

    let html = `
        <div class="task-item ${task.completed ? 'completed' : ''} priority-${task.priority}" data-task-id="${task.id}">
            <div class="task-header">
                <div class="task-checkbox ${task.completed ? 'checked' : ''}" data-task-id="${task.id}"></div>
                <div class="task-content">
                    <div class="task-title-row">
                        <div class="task-title" data-task-id="${task.id}">${escapeHtml(task.title)}</div>
                        <div class="task-type-badge">${typeLabels[task.type]}</div>
                    </div>
                    ${task.description ? `<div class="task-description" data-task-id="${task.id}">${escapeHtml(task.description)}</div>` : ''}
    `;

    // Dependencies with remove button
    if (task.dependencies.length > 0) {
        html += '<div class="task-dependencies">';
        html += '<div class="dependency-label">BEKLIYOR:</div>';
        task.dependencies.forEach(depId => {
            const dep = app.tasks.find(t => t.id === depId);
            if (dep) {
                const completedClass = dep.completed ? ' style="color: #0f0;"' : '';
                html += `
                    <div class="dependency-item">
                        <div class="dependency-text"${completedClass}>${escapeHtml(dep.title)}</div>
                        <button class="dependency-remove" 
                                data-task-id="${task.id}" 
                                data-dep-id="${depId}">KALDIR</button>
                    </div>
                `;
            }
        });
        html += '</div>';
    }

    // Checklist
    if (task.checklist && task.checklist.length > 0) {
        html += '<div class="task-checklist">';
        task.checklist.forEach(item => {
            html += `
                <div class="checklist-item">
                    <div class="checklist-checkbox ${item.completed ? 'checked' : ''}" 
                         data-task-id="${task.id}" 
                         data-checklist-id="${item.id}"></div>
                    <div class="checklist-text ${item.completed ? 'completed' : ''}">${escapeHtml(item.text)}</div>
                    <button class="checklist-remove" 
                            data-task-id="${task.id}" 
                            data-checklist-id="${item.id}">X</button>
                </div>
            `;
        });
        html += `<input type="text" class="checklist-input" placeholder="${t('add_subtask')}" data-task-id="${task.id}">`;
        html += '</div>';
    } else {
        html += `<div class="task-checklist"><input type="text" class="checklist-input" placeholder="${t('add_subtask')}" data-task-id="${task.id}"></div>`;
    }

    html += `
                    <div class="task-footer">
                        <div class="task-meta">
                            <span class="task-date-display" data-task-id="${task.id}">${formatDate(task.date)}</span>
                            <span>|</span>
                            <span>${task.priority.toUpperCase()}</span>
                        </div>
                        <div class="task-actions">
                            <button class="task-action-btn add-dep" data-task-id="${task.id}">${t('btn_add_dependency')}</button>
                            <button class="task-action-btn edit" data-task-id="${task.id}">${t('ctx_edit')}</button>
                            <button class="task-action-btn delete" data-task-id="${task.id}">${t('btn_delete')}</button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    `;

    return html;
}

function updateStats() {
    const totalTasks = app.tasks.length;
    const activeTasks = app.tasks.filter(t => !t.completed).length;
    const totalProjects = app.projects.filter(p => !p.archived).length;

    document.getElementById('total-tasks').textContent = totalTasks;
    document.getElementById('active-tasks').textContent = activeTasks;
    document.getElementById('total-projects').textContent = totalProjects;
}

function applySettings() {
    // Apply app title to both page title and app title
    document.getElementById('app-title').textContent = app.settings.appTitle;
    document.title = app.settings.appTitle;
    
    // Apply category names
    document.getElementById('tab-cat1').textContent = app.settings.category1;
    document.getElementById('tab-cat2').textContent = app.settings.category2;
    document.getElementById('tab-cat3').textContent = app.settings.category3;
    
    // Update INFO section category names (if elements exist)
    const infoCat1 = document.getElementById('info-cat1');
    const infoCat2 = document.getElementById('info-cat2');
    const infoCat3 = document.getElementById('info-cat3');
    const infoCat1Nav = document.getElementById('info-cat1-nav');
    const infoCat2Nav = document.getElementById('info-cat2-nav');
    const infoCat3Nav = document.getElementById('info-cat3-nav');
    
    if (infoCat1) infoCat1.textContent = app.settings.category1;
    if (infoCat2) infoCat2.textContent = app.settings.category2;
    if (infoCat3) infoCat3.textContent = app.settings.category3;
    if (infoCat1Nav) infoCat1Nav.textContent = app.settings.category1;
    if (infoCat2Nav) infoCat2Nav.textContent = app.settings.category2;
    if (infoCat3Nav) infoCat3Nav.textContent = app.settings.category3;
    
    // Apply theme
    if (app.settings.theme === 'light') {
        document.body.setAttribute('data-theme', 'light');
    } else {
        document.body.removeAttribute('data-theme');
    }
    
    // Update network info
    updateNetworkInfo();
}

function updateNetworkInfo() {
    // Fetch current network status from ESP32
    fetch('/api/network/status')
        .then(response => response.json())
        .then(data => {
            // INFO tab - Top status bar
            const infoWiFi = document.getElementById('info-wifi-status');
            const infoIP = document.getElementById('info-ip');
            const infoMDNS = document.getElementById('info-mdns');
            
            if (infoWiFi) {
                infoWiFi.textContent = data.mode === 'AP' ? 'AP Mode' : data.ssid;
            }
            
            if (infoIP) {
                infoIP.textContent = data.ip || '192.168.4.1';
            }
            
            if (infoMDNS) {
                infoMDNS.textContent = data.mdns || 'smartkraft-to2do.local';
            }
            
            // CONNECTION tab - Accordion statuses
            const apStatus = document.getElementById('ap-status');
            const primaryStatus = document.getElementById('primary-status');
            const backupStatus = document.getElementById('backup-status');
            
            if (apStatus) {
                apStatus.textContent = data.mode === 'AP' ? 'Active' : 'Inactive';
                apStatus.className = data.mode === 'AP' ? 'accordion-status active' : 'accordion-status';
            }
            
            if (primaryStatus) {
                if (data.mode === 'STA' && data.connected) {
                    primaryStatus.textContent = 'Connected';
                    primaryStatus.className = 'accordion-status connected';
                } else {
                    primaryStatus.textContent = 'Not Connected';
                    primaryStatus.className = 'accordion-status';
                }
            }
            
            if (backupStatus) {
                // Backup sadece configure edilmişse göster
                backupStatus.textContent = app.networkSettings.backupSSID ? 'Configured' : 'Not Configured';
                backupStatus.className = 'accordion-status';
            }
            
            // CONNECTION tab - Current Connection Status
            const currentMode = document.getElementById('current-mode');
            const currentSSID = document.getElementById('current-ssid');
            const currentIP = document.getElementById('current-ip');
            const currentSignal = document.getElementById('current-signal');
            
            if (currentMode) {
                currentMode.textContent = data.mode === 'AP' ? 'AP Mode' : 'WiFi Connected';
            }
            
            if (currentSSID) {
                currentSSID.textContent = data.mode === 'AP' ? '-' : (data.ssid || '-');
            }
            
            if (currentIP) {
                currentIP.textContent = data.ip || '192.168.4.1';
            }
            
            if (currentSignal) {
                if (data.mode === 'STA' && data.rssi) {
                    currentSignal.textContent = data.rssi + ' dBm';
                } else {
                    currentSignal.textContent = '-';
                }
            }
        })
        .catch(error => {
            // Fallback to AP mode defaults
            const infoWiFi = document.getElementById('info-wifi-status');
            const infoIP = document.getElementById('info-ip');
            const infoMDNS = document.getElementById('info-mdns');
            
            if (infoWiFi) infoWiFi.textContent = 'AP Mode';
            if (infoIP) infoIP.textContent = '192.168.4.1';
            if (infoMDNS) infoMDNS.textContent = 'smartkraft-to2do.local';
            
            const apStatus = document.getElementById('ap-status');
            if (apStatus) {
                apStatus.textContent = 'Active';
                apStatus.className = 'accordion-status active';
            }
            
            const currentMode = document.getElementById('current-mode');
            if (currentMode) currentMode.textContent = 'AP Mode';
        });
}

function showToast(message, type = 'success') {
    const existing = document.querySelector('.toast');
    if (existing) existing.remove();

    const toast = document.createElement('div');
    toast.className = 'toast ' + type;
    toast.textContent = message;
    document.body.appendChild(toast);

    setTimeout(() => {
        if (toast.parentNode) toast.remove();
    }, 2500);
}

function escapeHtml(text) {
    if (!text) return '';
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}
)XJSUIX";
}

#endif
