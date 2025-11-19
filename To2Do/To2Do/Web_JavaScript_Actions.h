#ifndef WEB_JAVASCRIPT_ACTIONS_H
#define WEB_JAVASCRIPT_ACTIONS_H

// Actions Library - Butonlar, Popup'lar ve Modal Yönetimi
const char* getJavaScriptActions() {
  return R"XJSACTX(
// ==================== MODAL & POPUP MANAGEMENT ====================

function showTaskModal() {
    if (!app.currentProject) {
        showToast('SELECT PROJECT FIRST', 'error');
        return;
    }

    document.getElementById('modal-task-title').value = '';
    document.getElementById('modal-task-desc').value = '';
    document.getElementById('modal-task-type').value = 'task';
    document.getElementById('modal-task-priority').value = 'medium';
    document.getElementById('modal-task-date').value = '';
    document.getElementById('task-modal').classList.add('active');
    document.getElementById('modal-task-title').focus();
}

function hideTaskModal() {
    document.getElementById('task-modal').classList.remove('active');
}

function showEditTaskModal(taskId) {
    const task = app.tasks.find(t => t.id === taskId);
    if (!task) return;
    
    document.getElementById('edit-task-title').value = task.title;
    document.getElementById('edit-task-desc').value = task.description;
    document.getElementById('edit-task-type').value = task.type;
    document.getElementById('edit-task-priority').value = task.priority;
    document.getElementById('edit-task-date').value = task.date;
    document.getElementById('edit-task-modal').classList.add('active');
    document.getElementById('edit-task-title').focus();
    
    // Store task ID for saving
    document.getElementById('edit-task-modal').dataset.taskId = taskId;
}

function hideEditTaskModal() {
    document.getElementById('edit-task-modal').classList.remove('active');
}

function showProjectModal() {
    document.getElementById('modal-project-name').value = '';
    document.getElementById('modal-project-desc').value = '';
    document.getElementById('modal-project-priority').value = 'medium';
    document.getElementById('project-modal').classList.add('active');
    document.getElementById('modal-project-name').focus();
}

function hideProjectModal() {
    document.getElementById('project-modal').classList.remove('active');
    // Reset modal state
    document.getElementById('modal-project-create').style.display = '';
    document.getElementById('modal-project-update').style.display = 'none';
    document.getElementById('modal-project-name').value = '';
    document.getElementById('modal-project-desc').value = '';
    document.getElementById('modal-project-priority').value = 'medium';
}

function showSettingsModal() {
    // Load current GUI settings to modal
    document.getElementById('setting-app-title').value = app.settings.appTitle;
    document.getElementById('setting-cat1').value = app.settings.category1;
    document.getElementById('setting-cat2').value = app.settings.category2;
    document.getElementById('setting-cat3').value = app.settings.category3;
    document.getElementById('theme-toggle').checked = (app.settings.theme === 'dark');
    

    
    // Load network settings
    loadNetworkSettingsToModal();
    
    // Update INFO tab with current network status
    updateNetworkInfo();
    
    // Start periodic network status updates (every 5 seconds while modal is open)
    if (app.networkStatusInterval) {
        clearInterval(app.networkStatusInterval);
    }
    app.networkStatusInterval = setInterval(() => {
        updateNetworkInfo();
    }, 5000);
    
    // Open modal with GUI tab active by default
    const modal = document.getElementById('settings-modal');
    modal.classList.add('active');
    
    // Activate GUI tab
    document.querySelectorAll('.settings-tab').forEach(t => t.classList.remove('active'));
    document.querySelector('[data-tab="gui"]')?.classList.add('active');
    
    document.querySelectorAll('.settings-tab-content').forEach(c => c.classList.remove('active'));
    document.querySelector('[data-content="gui"]')?.classList.add('active');
}

function hideSettingsModal() {
    document.getElementById('settings-modal').classList.remove('active');
    
    // Stop periodic updates
    if (app.networkStatusInterval) {
        clearInterval(app.networkStatusInterval);
        app.networkStatusInterval = null;
    }
    
    // Restore theme if cancelled
    applySettings();
}

function showContextMenu(event, projectId) {
    const menu = document.getElementById('context-menu');
    menu.style.left = event.pageX + 'px';
    menu.style.top = event.pageY + 'px';
    menu.classList.add('active');
    menu.dataset.currentProjectId = projectId;
}

// ==================== TASK ACTIONS ====================

function createTask() {
    const title = document.getElementById('modal-task-title').value.trim();
    const description = document.getElementById('modal-task-desc').value.trim();
    const type = document.getElementById('modal-task-type').value;
    const priority = document.getElementById('modal-task-priority').value;
    const date = document.getElementById('modal-task-date').value;

    if (!title) {
        showToast('TITLE REQUIRED', 'error');
        return;
    }

    const newTask = {
        id: app.nextTaskId++,
        projectId: app.currentProject,
        title,
        description,
        type,
        priority,
        date,
        completed: false,
        checklist: [],
        dependencies: []
    };

    app.tasks.push(newTask);
    hideTaskModal();
    renderTasks();
    renderProjects();
    updateStats();
    showToast('TASK CREATED');
    app.saveToServer();
}

function saveEditedTask() {
    const taskId = parseInt(document.getElementById('edit-task-modal').dataset.taskId);
    const task = app.tasks.find(t => t.id === taskId);
    if (!task) return;
    
    const title = document.getElementById('edit-task-title').value.trim();
    const description = document.getElementById('edit-task-desc').value.trim();
    const type = document.getElementById('edit-task-type').value;
    const priority = document.getElementById('edit-task-priority').value;
    const date = document.getElementById('edit-task-date').value;

    if (!title) {
        showToast('TITLE REQUIRED', 'error');
        return;
    }

    task.title = title;
    task.description = description;
    task.type = type;
    task.priority = priority;
    task.date = date;

    hideEditTaskModal();
    renderTasks();
    renderProjects();
    updateStats();
    showToast('TASK UPDATED');
    app.saveToServer();
}

function toggleTask(taskId) {
    const task = app.tasks.find(t => t.id === taskId);
    if (task) {
        task.completed = !task.completed;
        renderTasks();
        updateStats();
        app.saveToServer();
    }
}

function deleteTask(taskId) {
    const task = app.tasks.find(t => t.id === taskId);
    if (task && confirm('DELETE: ' + task.title + '?')) {
        app.tasks.forEach(t => {
            if (t.dependencies) {
                t.dependencies = t.dependencies.filter(d => d !== taskId);
            }
        });

        app.tasks = app.tasks.filter(t => t.id !== taskId);
        renderTasks();
        renderProjects();
        updateStats();
        showToast('DELETED', 'error');
        app.saveToServer();
    }
}

// Inline date editing disabled - use edit modal instead
// function editTaskDate(taskId) {
//     const task = app.tasks.find(t => t.id === taskId);
//     if (!task) return;
//
//     const currentDate = task.date || '';
//     const newDate = prompt('Date (dd.mm.yyyy or leave empty):', currentDate);
//     
//     if (newDate === null) return;
//     
//     task.date = newDate.trim();
//     renderTasks();
//     showToast('DATE UPDATED');
//     app.saveToServer();
// }

function startInlineEdit(taskId, field, element) {
    // Inline editing disabled - use edit modal instead
    return;
}

// ==================== CHECKLIST ACTIONS ====================

function toggleChecklist(taskId, checklistId) {
    const task = app.tasks.find(t => t.id === taskId);
    if (task && task.checklist) {
        const item = task.checklist.find(c => c.id === checklistId);
        if (item) {
            item.completed = !item.completed;
            renderTasks();
            app.saveToServer();
        }
    }
}

function addChecklistItem(taskId, text) {
    const task = app.tasks.find(t => t.id === taskId);
    if (task) {
        if (!task.checklist) task.checklist = [];
        const newId = task.checklist.length > 0 
            ? Math.max(...task.checklist.map(c => c.id)) + 1 
            : 1;
        task.checklist.push({ id: newId, text, completed: false });
        renderTasks();
        showToast('SUBTASK ADDED');
        app.saveToServer();
    }
}

function removeChecklistItem(taskId, checklistId) {
    const task = app.tasks.find(t => t.id === taskId);
    if (task && task.checklist) {
        task.checklist = task.checklist.filter(c => c.id !== checklistId);
        renderTasks();
        showToast('SUBTASK DELETED');
        app.saveToServer();
    }
}

// ==================== DEPENDENCY ACTIONS ====================

function addDependency(taskId) {
    const task = app.tasks.find(t => t.id === taskId);
    if (!task) return;

    const availableTasks = app.tasks
        .filter(t => t.projectId === app.currentProject && t.id !== taskId)
        .map(t => ({ id: t.id, title: t.title, completed: t.completed }));

    if (availableTasks.length === 0) {
        showToast('NO OTHER TASKS', 'error');
        return;
    }

    showDependencyPicker(taskId, availableTasks);
}

function showDependencyPicker(taskId, availableTasks) {
    // Create modal HTML
    const modalHTML = `
        <div class="modal active" id="depPickerModal" onclick="if(event.target===this) closeDependencyPicker()">
            <div class="modal-content" style="max-width: 500px; max-height: 70vh;">
                <div class="modal-title">SELECT DEPENDENCY</div>
                <div style="padding: 0 30px; margin-bottom: 20px; color: #888; font-size: 13px;">
                    Which task must be completed first?
                </div>
                <div style="flex: 1; overflow-y: auto; padding: 0 30px; margin-bottom: 20px;">
                    ${availableTasks.map(t => `
                        <div class="dep-task-item" data-task-id="${t.id}" onclick="selectDependency(${taskId}, ${t.id})">
                            <div style="display: flex; align-items: center; gap: 10px;">
                                <div style="font-weight: bold; color: #fff; min-width: 30px;">#${t.id}</div>
                                <div style="flex: 1; ${t.completed ? 'text-decoration: line-through; color: #666;' : ''}">${t.title}</div>
                                ${t.completed ? '<div style="color: #0f0; font-size: 11px;">✓ DONE</div>' : ''}
                            </div>
                        </div>
                    `).join('')}
                </div>
                <div class="modal-actions">
                    <button type="button" class="modal-btn" onclick="closeDependencyPicker()">Cancel</button>
                </div>
            </div>
        </div>
    `;

    // Inject modal
    document.body.insertAdjacentHTML('beforeend', modalHTML);

    // Add CSS for dep-task-item
    if (!document.getElementById('depPickerCSS')) {
        const style = document.createElement('style');
        style.id = 'depPickerCSS';
        style.textContent = `
            .dep-task-item {
                padding: 15px;
                background: #111;
                border: 1px solid #333;
                margin-bottom: 10px;
                cursor: pointer;
                transition: all 0.2s;
            }
            .dep-task-item:hover {
                background: #1a1a1a;
                border-color: #fff;
                transform: translateX(5px);
            }
            body[data-theme="light"] .dep-task-item {
                background: #f5f5f5;
                border-color: #ddd;
            }
            body[data-theme="light"] .dep-task-item:hover {
                background: #fff;
                border-color: #000;
            }
        `;
        document.head.appendChild(style);
    }
}

function selectDependency(taskId, depId) {
    const task = app.tasks.find(t => t.id === taskId);
    const depTask = app.tasks.find(t => t.id === depId);

    if (!task || !depTask) {
        showToast('TASK NOT FOUND', 'error');
        closeDependencyPicker();
        return;
    }

    if (task.dependencies.includes(depId)) {
        showToast('ALREADY ADDED', 'error');
        closeDependencyPicker();
        return;
    }

    task.dependencies.push(depId);
    renderTasks();
    showToast('DEPENDENCY ADDED');
    app.saveToServer();
    closeDependencyPicker();
}

function closeDependencyPicker() {
    const modal = document.getElementById('depPickerModal');
    if (modal) {
        modal.remove();
    }
}

function removeDependency(taskId, depId) {
    const task = app.tasks.find(t => t.id === taskId);
    if (task) {
        task.dependencies = task.dependencies.filter(d => d !== depId);
        renderTasks();
        showToast('LINK REMOVED');
        app.saveToServer();
    }
}

// ==================== PROJECT ACTIONS ====================

function createProject() {
    const name = document.getElementById('modal-project-name').value.trim();
    const desc = document.getElementById('modal-project-desc').value.trim();
    const priority = document.getElementById('modal-project-priority').value;

    if (!name) {
        showToast('PROJECT NAME REQUIRED', 'error');
        return;
    }

    const newProject = {
        id: app.nextProjectId++,
        name,
        description: desc,
        category: app.currentCategory,
        priority,
        archived: false
    };

    app.projects.push(newProject);
    hideProjectModal();
    renderProjects();
    updateStats();
    showToast('PROJECT CREATED');
    selectProject(newProject.id);
    app.saveToServer();
}

function editProjectDetails() {
    const project = app.projects.find(p => p.id === app.currentProject);
    if (!project) return;

    document.getElementById('modal-project-name').value = project.name;
    document.getElementById('modal-project-desc').value = project.description || '';
    document.getElementById('modal-project-priority').value = project.priority;
    
    document.getElementById('modal-project-create').style.display = 'none';
    document.getElementById('modal-project-update').style.display = '';
    
    document.getElementById('project-modal').classList.add('active');
    document.getElementById('modal-project-name').focus();
}

function updateProject() {
    const project = app.projects.find(p => p.id === app.currentProject);
    if (!project) return;
    
    const name = document.getElementById('modal-project-name').value.trim();
    const desc = document.getElementById('modal-project-desc').value.trim();
    const priority = document.getElementById('modal-project-priority').value;

    if (!name) {
        showToast('PROJECT NAME REQUIRED', 'error');
        return;
    }

    project.name = name;
    project.description = desc;
    project.priority = priority;

    hideProjectModal();
    renderProjects();
    renderTasks();
    showToast('PROJECT UPDATED');
    app.saveToServer();
}

function handleContextAction(action, projectId) {
    const project = app.projects.find(p => p.id === projectId);
    if (!project) return;

    switch (action) {
        case 'edit':
            app.currentProject = projectId;
            selectProject(projectId);
            editProjectDetails();
            break;
        case 'duplicate':
            const copy = {...project, id: app.nextProjectId++, name: project.name + ' (Copy)'};
            app.projects.push(copy);
            renderProjects();
            showToast('DUPLICATED');
            app.saveToServer();
            break;
        case 'archive':
            project.archived = !project.archived;
            renderProjects();
            showToast(project.archived ? 'ARCHIVED' : 'UNARCHIVED');
            app.saveToServer();
            break;
        case 'delete':
            if (confirm('DELETE PROJECT: ' + project.name + '?')) {
                app.projects = app.projects.filter(p => p.id !== projectId);
                app.tasks = app.tasks.filter(t => t.projectId !== projectId);
                if (app.currentProject === projectId) {
                    app.currentProject = null;
                }
                renderProjects();
                renderTasks();
                updateStats();
                showToast('DELETED', 'error');
                app.saveToServer();
            }
            break;
    }
}

// ==================== SETTINGS ACTIONS ====================

async function saveSettings() {
    // Show immediate feedback
    showToast('KAYDEDILIYOR...');
    
    app.settings.appTitle = document.getElementById('setting-app-title').value.trim() || 'To2Do - SmartKraft';
    app.settings.category1 = document.getElementById('setting-cat1').value.trim() || 'WORK';
    app.settings.category2 = document.getElementById('setting-cat2').value.trim() || 'PERSONAL';
    app.settings.category3 = document.getElementById('setting-cat3').value.trim() || 'PROJECTS';
    app.settings.theme = document.getElementById('theme-toggle').checked ? 'dark' : 'light';
    
    // Only collect network settings if they exist (may not be on GUI tab)
    const hasNetworkSettings = document.getElementById('setting-ap-ssid');
    
    if (hasNetworkSettings) {
    app.networkSettings.apSSID = document.getElementById('setting-ap-ssid')?.value.trim() || 'SmartKraft-To2Do';
    app.networkSettings.apMDNS = document.getElementById('setting-ap-mdns')?.value.trim() || 'smartkraft-to2do';
        app.networkSettings.primarySSID = document.getElementById('setting-primary-ssid')?.value.trim() || '';
        app.networkSettings.primaryPassword = document.getElementById('setting-primary-password')?.value || '';
        app.networkSettings.primaryIP = document.getElementById('setting-primary-ip')?.value.trim() || '';
    app.networkSettings.primaryMDNS = document.getElementById('setting-primary-mdns')?.value.trim() || 'smartkraft-to2do';
        app.networkSettings.backupSSID = document.getElementById('setting-backup-ssid')?.value.trim() || '';
        app.networkSettings.backupPassword = document.getElementById('setting-backup-password')?.value || '';
        app.networkSettings.backupIP = document.getElementById('setting-backup-ip')?.value.trim() || '';
    app.networkSettings.backupMDNS = document.getElementById('setting-backup-mdns')?.value.trim() || 'smartkraft-to2do-backup';
        localStorage.setItem('networkSettings', JSON.stringify(app.networkSettings));
    }
    
    // Save to localStorage immediately (instant)
    localStorage.setItem('to2doSettings', JSON.stringify(app.settings));
    
    // Apply settings immediately (instant)
    applySettings();
    
    // Show success and close modal immediately
    showToast('AYARLAR KAYDEDILDI');
    hideSettingsModal();
    
    // Save to server in background (don't wait for it)
    if (hasNetworkSettings) {
        Promise.all([
            app.saveSettingsToServer(),
            app.saveNetworkSettings()
        ]).catch(error => {
            console.error('Background save error:', error);
        });
    } else {
        app.saveSettingsToServer().catch(error => {
            console.error('Background save error:', error);
        });
    }
}

function loadNetworkSettingsToModal() {
    const apSSID = document.getElementById('setting-ap-ssid');
    const apMDNS = document.getElementById('setting-ap-mdns');
    const primarySSID = document.getElementById('setting-primary-ssid');
    const primaryPassword = document.getElementById('setting-primary-password');
    const primaryIP = document.getElementById('setting-primary-ip');
    const primaryMDNS = document.getElementById('setting-primary-mdns');
    const backupSSID = document.getElementById('setting-backup-ssid');
    const backupPassword = document.getElementById('setting-backup-password');
    const backupIP = document.getElementById('setting-backup-ip');
    const backupMDNS = document.getElementById('setting-backup-mdns');
    
    if (apSSID) apSSID.value = app.networkSettings.apSSID;
    if (apMDNS) apMDNS.value = app.networkSettings.apMDNS;
    if (primarySSID) primarySSID.value = app.networkSettings.primarySSID;
    if (primaryPassword) primaryPassword.value = app.networkSettings.primaryPassword;
    if (primaryIP) primaryIP.value = app.networkSettings.primaryIP;
    if (primaryMDNS) primaryMDNS.value = app.networkSettings.primaryMDNS;
    if (backupSSID) backupSSID.value = app.networkSettings.backupSSID;
    if (backupPassword) backupPassword.value = app.networkSettings.backupPassword;
    if (backupIP) backupIP.value = app.networkSettings.backupIP;
    if (backupMDNS) backupMDNS.value = app.networkSettings.backupMDNS;
}

async function testWiFiConnection(type) {
    const ssid = type === 'primary' 
        ? document.getElementById('setting-primary-ssid').value 
        : document.getElementById('setting-backup-ssid').value;
    
    if (!ssid) {
        showToast('ENTER WIFI SSID FIRST', 'error');
        return;
    }
    
    showToast(`TESTING ${type.toUpperCase()} WIFI...`);
    
    try {
        const response = await fetch('/api/network/test', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            credentials: 'include',
            body: JSON.stringify({ type, ssid })
        });
        
        if (response.ok) {
            const data = await response.json();
            showToast(data.success ? 'CONNECTION OK' : 'CONNECTION FAILED', data.success ? '' : 'error');
        } else {
            showToast('TEST FAILED', 'error');
        }
    } catch (error) {
        console.error('WiFi test error:', error);
        showToast('TEST FEATURE NOT AVAILABLE', 'error');
    }
}

// ==================== TASK EVENT BINDING ====================

function bindTaskEvents() {
    const container = document.getElementById('tasks-container');

    container.querySelectorAll('.task-checkbox').forEach(el => {
        el.addEventListener('click', () => {
            const taskId = parseInt(el.dataset.taskId);
            toggleTask(taskId);
        });
    });

    container.querySelectorAll('.checklist-checkbox').forEach(el => {
        el.addEventListener('click', () => {
            const taskId = parseInt(el.dataset.taskId);
            const checklistId = parseInt(el.dataset.checklistId);
            toggleChecklist(taskId, checklistId);
        });
    });

    container.querySelectorAll('.checklist-input').forEach(el => {
        el.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                const taskId = parseInt(el.dataset.taskId);
                const text = el.value.trim();
                if (text) {
                    addChecklistItem(taskId, text);
                    el.value = '';
                }
            }
        });
    });

    container.querySelectorAll('.task-action-btn.edit').forEach(el => {
        el.addEventListener('click', () => {
            const taskId = parseInt(el.dataset.taskId);
            showEditTaskModal(taskId);
        });
    });

    container.querySelectorAll('.task-action-btn.delete').forEach(el => {
        el.addEventListener('click', () => {
            const taskId = parseInt(el.dataset.taskId);
            deleteTask(taskId);
        });
    });

    container.querySelectorAll('.task-action-btn.add-dep').forEach(el => {
        el.addEventListener('click', () => {
            const taskId = parseInt(el.dataset.taskId);
            addDependency(taskId);
        });
    });

    container.querySelectorAll('.dependency-remove').forEach(el => {
        el.addEventListener('click', () => {
            const taskId = parseInt(el.dataset.taskId);
            const depId = parseInt(el.dataset.depId);
            removeDependency(taskId, depId);
        });
    });

    container.querySelectorAll('.checklist-remove').forEach(el => {
        el.addEventListener('click', () => {
            const taskId = parseInt(el.dataset.taskId);
            const checklistId = parseInt(el.dataset.checklistId);
            removeChecklistItem(taskId, checklistId);
        });
    });

    // Date click removed - use edit button instead
    // container.querySelectorAll('.task-date-display').forEach(el => {
    //     el.addEventListener('click', () => {
    //         const taskId = parseInt(el.dataset.taskId);
    //         editTaskDate(taskId);
    //     });
    // });
}
)XJSACTX";
}

#endif
