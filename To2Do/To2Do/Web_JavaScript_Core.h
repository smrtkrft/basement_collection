#ifndef WEB_JAVASCRIPT_CORE_H
#define WEB_JAVASCRIPT_CORE_H

// Core Library - Veri Yönetimi, Server İletişimi ve Sistem Stabilitesi
const char* getJavaScriptCore() {
  return R"XJSCOREX(
// ==================== CORE APPLICATION CLASS ====================
class WorkspaceFinal {
    constructor() {
        this.projects = [];
        this.tasks = [];
        this.currentCategory = 'Personal';
        this.currentProject = null;
        this.currentFilter = 'all';
        this.nextProjectId = 1;
        this.nextTaskId = 1;
        this.networkStatusInterval = null;
        

        
        // Settings
        this.settings = {
            appTitle: 'To2Do - SmartKraft',
            category1: 'WORK',
            category2: 'PERSONAL',
            category3: 'PROJECTS',
            theme: 'dark'
        };
        
        // Network settings
        this.networkSettings = {
            apSSID: 'SmartKraft-To2Do',
            apMDNS: 'smartkraft-to2do',
            primarySSID: '',
            primaryPassword: '',
            primaryIP: '',
            primaryMDNS: 'smartkraft-to2do',
            backupSSID: '',
            backupPassword: '',
            backupIP: '',
            backupMDNS: 'smartkraft-to2do-backup'
        };
        
        this.init();
    }



    async init() {
        await this.loadSettingsFromServer();
        applySettings();
        await this.loadFromServer();
        
        // Force render after data is loaded
        setTimeout(() => {
            this.bindEvents();
            renderProjects();
            updateStats();
            
            // Auto-select first project if exists
            if (this.projects.length > 0 && !this.currentProject) {
                this.selectProject(this.projects[0].id);
            }
        }, 100);
    }

    // ==================== DATA PERSISTENCE ====================

    async loadFromServer() {
        try {
            const response = await fetch('/api/todos', {credentials: 'include'});
            
            if (!response.ok) {
                console.error('Failed to load todos from server:', response.status);
                return;
            }
            
            const text = await response.text();
            
            // If empty or invalid, keep current state (don't overwrite with demo)
            if (!text || text === '{}' || text.trim() === '' || text === '[]') {
                console.log('No todos found on server, keeping current state');
                return;
            }
            
            const data = JSON.parse(text);
            
            if (data.projects && data.tasks) {
                this.projects = data.projects;
                this.tasks = data.tasks;
                this.nextProjectId = Math.max(...this.projects.map(p => p.id), 0) + 1;
                this.nextTaskId = Math.max(...this.tasks.map(t => t.id), 0) + 1;
                console.log('Loaded from server:', this.projects.length, 'projects,', this.tasks.length, 'tasks');
            } else {
                console.warn('Invalid data structure from server');
            }
        } catch (error) {
            console.error('Load error:', error);
        }
    }

    async saveToServer() {
        try {
            const data = {
                projects: this.projects,
                tasks: this.tasks
            };
            
            console.log('Saving to server:', data.projects.length, 'projects,', data.tasks.length, 'tasks');
            
            const response = await fetch('/api/todos', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                credentials: 'include',
                body: JSON.stringify(data)
            });
            
            if (!response.ok) {
                console.error('Save failed:', response.status);
                showToast('SAVE FAILED!', 'error');
            } else {
                console.log('✓ Saved to server successfully');
            }
        } catch (error) {
            console.error('Server save error:', error);
            showToast('SAVE ERROR!', 'error');
        }
    }

    // ==================== SETTINGS MANAGEMENT ====================

    async saveSettingsToServer() {
        try {
            const response = await fetch('/api/settings', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                credentials: 'include',
                body: JSON.stringify(this.settings)
            });
            
            if (response.ok) {
                console.log('Settings saved to SPIFFS successfully');
            } else {
                console.error('Failed to save settings to SPIFFS');
            }
        } catch (error) {
            console.error('Error saving settings to server:', error);
        }
    }

    async loadSettingsFromServer() {
        try {
            // Load GUI settings
            const settingsResponse = await fetch('/api/settings', {credentials: 'include'});
            if (settingsResponse.ok) {
                const data = await settingsResponse.json();
                this.settings = {
                    ...this.settings,
                    ...data
                };
                console.log('Settings loaded from SPIFFS:', this.settings);
            } else {
                console.log('Using default GUI settings');
            }
            
            // Load network settings
            const networkResponse = await fetch('/api/network/settings', {credentials: 'include'});
            if (networkResponse.ok) {
                const networkData = await networkResponse.json();
                this.networkSettings = {
                    ...this.networkSettings,
                    ...networkData
                };
                console.log('Network settings loaded from SPIFFS:', this.networkSettings);
            } else {
                console.log('Using default network settings');
            }
        } catch (error) {
            console.error('Failed to load settings from server:', error);
            this.loadSettingsFromLocalStorage();
        }
    }
    
    loadSettingsFromLocalStorage() {
    const saved = localStorage.getItem('to2doSettings');
        if (saved) {
            try {
                this.settings = JSON.parse(saved);
            } catch (e) {
                console.error('Settings load error:', e);
            }
        }
        
        const savedNetwork = localStorage.getItem('networkSettings');
        if (savedNetwork) {
            try {
                this.networkSettings = JSON.parse(savedNetwork);
            } catch (e) {
                console.error('Network settings load error:', e);
            }
        }
    }

    // ==================== NETWORK MANAGEMENT ====================

    async saveNetworkSettings() {
        try {
            const response = await fetch('/api/network/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                credentials: 'include',
                body: JSON.stringify(this.networkSettings)
            });
            
            if (!response.ok) {
                console.error('Network settings save failed:', response.status);
                showToast('NETWORK SAVE FAILED', 'error');
            } else {
                showToast('CONNECTING TO WIFI...');
                this.startConnectionMonitoring();
            }
        } catch (error) {
            console.error('Network save error:', error);
        }
    }
    
    startConnectionMonitoring() {
        let attempts = 0;
        const maxAttempts = 10;
        
        const checkConnection = setInterval(() => {
            attempts++;
            
            fetch('/api/network/status', {credentials: 'include'})
                .then(response => response.json())
                .then(data => {
                    if (data.mode === 'STA' && data.connected) {
                        clearInterval(checkConnection);
                        showToast('WIFI CONNECTED!');
                        updateNetworkInfo();
                    } else if (attempts >= maxAttempts) {
                        clearInterval(checkConnection);
                        showToast('CONNECTION FAILED - AP MODE ACTIVE', 'error');
                        updateNetworkInfo();
                    }
                })
                .catch(error => {
                    console.error('Status check error:', error);
                    clearInterval(checkConnection);
                });
        }, 3000);
    }

    // ==================== FACTORY RESET ====================

    confirmFactoryReset() {
        const confirmed = confirm(
            '⚠️ FACTORY RESET WARNING ⚠️\n\n' +
            'This will PERMANENTLY delete:\n' +
            '• All projects and tasks\n' +
            '• All GUI settings (theme, categories)\n' +
            '• All WiFi network settings\n\n' +
            'THIS CANNOT BE UNDONE!\n\n' +
            'Are you sure you want to continue?'
        );
        
        if (confirmed) {
            const doubleCheck = confirm(
                'FINAL CONFIRMATION\n\n' +
                'Type YES in the next prompt to proceed with factory reset.'
            );
            
            if (doubleCheck) {
                const userInput = prompt('Type YES to confirm factory reset:');
                if (userInput === 'YES') {
                    this.performFactoryReset();
                } else {
                    showToast('FACTORY RESET CANCELLED', 'error');
                }
            } else {
                showToast('FACTORY RESET CANCELLED', 'error');
            }
        }
    }

    async performFactoryReset() {
        showToast('PERFORMING FACTORY RESET...');
        
        try {
            const response = await fetch('/api/factory-reset', {
                method: 'POST',
                credentials: 'include'
            });
            
            if (response.ok) {
                const data = await response.json();
                showToast('FACTORY RESET COMPLETE - RESTARTING...');
                
                localStorage.clear();
                
                setTimeout(() => {
                    window.location.reload();
                }, 3000);
            } else {
                showToast('FACTORY RESET FAILED', 'error');
            }
        } catch (error) {
            console.error('Factory reset error:', error);
            showToast('FACTORY RESET ERROR', 'error');
        }
    }

    // ==================== EVENT BINDING ====================

    bindEvents() {
        // FAB
        document.getElementById('fab-new-task').addEventListener('click', () => {
            showTaskModal();
        });

        // Category tabs
        document.querySelectorAll('.sidebar-tab').forEach(tab => {
            tab.addEventListener('click', (e) => {
                this.currentCategory = e.target.dataset.category;
                document.querySelectorAll('.sidebar-tab').forEach(t => t.classList.remove('active'));
                e.target.classList.add('active');
                renderProjects();
            });
        });

        // Project search
        document.getElementById('project-search').addEventListener('input', (e) => {
            renderProjects(e.target.value);
        });

        // New project
        document.getElementById('new-project-btn').addEventListener('click', () => {
            showProjectModal();
        });

        // Settings button
        document.getElementById('settings-btn').addEventListener('click', () => {
            showSettingsModal();
        });

        // Filters
        document.querySelectorAll('.panel-filter-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                this.currentFilter = e.target.dataset.filter;
                document.querySelectorAll('.panel-filter-btn').forEach(b => b.classList.remove('active'));
                e.target.classList.add('active');
                renderTasks();
            });
        });

        // Task modal
        document.getElementById('modal-task-cancel').addEventListener('click', () => {
            hideTaskModal();
        });

        document.getElementById('modal-task-create').addEventListener('click', () => {
            createTask();
        });

        // Project modal buttons
        document.getElementById('modal-project-cancel').addEventListener('click', () => {
            hideProjectModal();
        });

        document.getElementById('modal-project-create').addEventListener('click', () => {
            createProject();
        });
        
        document.getElementById('modal-project-update').addEventListener('click', () => {
            updateProject();
        });

        // Edit task modal buttons
        document.getElementById('edit-task-cancel').addEventListener('click', () => {
            hideEditTaskModal();
        });
        
        document.getElementById('edit-task-save').addEventListener('click', () => {
            saveEditedTask();
        });

        // Close modals on outside click
        document.getElementById('task-modal').addEventListener('click', (e) => {
            if (e.target.id === 'task-modal') hideTaskModal();
        });

        document.getElementById('edit-task-modal').addEventListener('click', (e) => {
            if (e.target.id === 'edit-task-modal') hideEditTaskModal();
        });

        document.getElementById('project-modal').addEventListener('click', (e) => {
            if (e.target.id === 'project-modal') hideProjectModal();
        });

        document.getElementById('settings-modal').addEventListener('click', (e) => {
            if (e.target.id === 'settings-modal') hideSettingsModal();
        });

        // Settings modal buttons
        document.getElementById('modal-settings-cancel').addEventListener('click', () => {
            hideSettingsModal();
        });

        document.getElementById('modal-settings-save').addEventListener('click', () => {
            saveSettings();
        });

        // Settings tabs
        document.querySelectorAll('.settings-tab').forEach(tab => {
            tab.addEventListener('click', (e) => {
                const tabName = e.target.dataset.tab;
                document.querySelectorAll('.settings-tab').forEach(t => t.classList.remove('active'));
                document.querySelectorAll('.settings-tab-content').forEach(c => c.classList.remove('active'));
                e.target.classList.add('active');
                document.querySelector(`.settings-tab-content[data-content="${tabName}"]`).classList.add('active');
            });
        });

        // Theme toggle - live preview
        document.getElementById('theme-toggle').addEventListener('change', (e) => {
            if (e.target.checked) {
                document.body.removeAttribute('data-theme');
            } else {
                document.body.setAttribute('data-theme', 'light');
            }
        });

        // Accordion handlers
        document.querySelectorAll('.accordion-header').forEach(header => {
            header.addEventListener('click', (e) => {
                const accordionName = e.currentTarget.dataset.accordion;
                const content = document.querySelector(`.accordion-content[data-content="${accordionName}"]`);
                const isActive = content.classList.contains('active');
                
                if (isActive) {
                    content.classList.remove('active');
                    header.classList.remove('active');
                    header.querySelector('.accordion-title').textContent = header.querySelector('.accordion-title').textContent.replace('▼', '▶');
                } else {
                    content.classList.add('active');
                    header.classList.add('active');
                    header.querySelector('.accordion-title').textContent = header.querySelector('.accordion-title').textContent.replace('▶', '▼');
                }
            });
        });

        // Password visibility toggles
        document.getElementById('show-primary-password')?.addEventListener('change', (e) => {
            const input = document.getElementById('setting-primary-password');
            input.type = e.target.checked ? 'text' : 'password';
        });

        document.getElementById('show-backup-password')?.addEventListener('change', (e) => {
            const input = document.getElementById('setting-backup-password');
            input.type = e.target.checked ? 'text' : 'password';
        });

        // mDNS preview updates
        document.getElementById('setting-ap-mdns')?.addEventListener('input', (e) => {
            document.getElementById('ap-mdns-preview').textContent = e.target.value || 'smartkraft-to2do';
        });

        document.getElementById('setting-primary-mdns')?.addEventListener('input', (e) => {
            document.getElementById('primary-mdns-preview').textContent = e.target.value || 'smartkraft-to2do';
        });

        document.getElementById('setting-backup-mdns')?.addEventListener('input', (e) => {
            document.getElementById('backup-mdns-preview').textContent = e.target.value || 'smartkraft-to2do-backup';
        });

        // Test connection buttons
        document.getElementById('test-primary-wifi')?.addEventListener('click', () => {
            testWiFiConnection('primary');
        });

        document.getElementById('test-backup-wifi')?.addEventListener('click', () => {
            testWiFiConnection('backup');
        });

        // Close context menu
        document.addEventListener('click', () => {
            document.getElementById('context-menu').classList.remove('active');
        });

        // Context menu items
        document.querySelectorAll('.context-menu-item').forEach(item => {
            item.addEventListener('click', () => {
                const menu = document.getElementById('context-menu');
                const projectId = parseInt(menu.dataset.currentProjectId);
                const action = item.dataset.action;
                handleContextAction(action, projectId);
                menu.classList.remove('active');
            });
        });
    }

    // ==================== DEMO DATA ====================

    loadDemoData() {
        this.projects = [
            { id: 1, name: 'Website Redesign', category: 'Work', description: 'Company website modernization project', priority: 'high', archived: false },
            { id: 2, name: 'Team Meeting Prep', category: 'Work', description: 'Prepare presentation and reports', priority: 'medium', archived: false },
            { id: 3, name: 'Home Improvement', category: 'Personal', description: 'Living room renovation tasks', priority: 'medium', archived: false },
            { id: 4, name: 'Fitness Goals', category: 'Personal', description: 'Weekly workout and diet plan', priority: 'high', archived: false },
            { id: 5, name: 'Smart Home System', category: 'Projects', description: 'IoT devices integration project', priority: 'high', archived: false },
            { id: 6, name: 'Learn New Skills', category: 'Projects', description: 'Online courses and certifications', priority: 'medium', archived: false },
            { id: 7, name: 'Shopping List', category: 'Personal', description: 'Weekly grocery and supplies', priority: 'low', archived: false }
        ];

        this.tasks = [
            { id: 1, projectId: 1, title: 'Design homepage mockup', description: 'Create new homepage design with modern UI/UX principles', type: 'task', priority: 'high', completed: false, date: '2025-10-15', checklist: [{ id: 1, text: 'Research competitor websites', completed: true }, { id: 2, text: 'Create wireframe layout', completed: true }, { id: 3, text: 'Design color scheme', completed: false }, { id: 4, text: 'Get client approval', completed: false }], dependencies: [] },
            { id: 2, projectId: 1, title: 'Setup development environment', description: 'Install required tools and frameworks', type: 'task', priority: 'high', completed: true, date: '2025-10-13', checklist: [{ id: 1, text: 'Install Node.js and npm', completed: true }, { id: 2, text: 'Setup React project', completed: true }, { id: 3, text: 'Configure Git repository', completed: true }], dependencies: [] },
            { id: 3, projectId: 1, title: 'Implement responsive layout', description: 'Make website mobile-friendly and responsive', type: 'task', priority: 'medium', completed: false, date: '2025-10-20', checklist: [{ id: 1, text: 'Mobile layout design', completed: false }, { id: 2, text: 'Tablet optimization', completed: false }, { id: 3, text: 'Cross-browser testing', completed: false }], dependencies: [1] },
            { id: 4, projectId: 2, title: 'Prepare quarterly report', description: 'Compile Q3 performance data and insights', type: 'task', priority: 'high', completed: false, date: '2025-10-14', checklist: [{ id: 1, text: 'Collect sales data', completed: true }, { id: 2, text: 'Create charts and graphs', completed: false }, { id: 3, text: 'Write summary report', completed: false }], dependencies: [] },
            { id: 5, projectId: 2, title: 'Schedule team meeting', description: 'Book conference room and send invites', type: 'reminder', priority: 'medium', completed: true, date: '2025-10-13', checklist: [], dependencies: [] },
            { id: 6, projectId: 3, title: 'Paint living room walls', description: 'Choose color and paint all walls', type: 'task', priority: 'medium', completed: false, date: '2025-10-16', checklist: [{ id: 1, text: 'Buy paint and supplies', completed: true }, { id: 2, text: 'Prepare walls and tape edges', completed: false }, { id: 3, text: 'Apply first coat', completed: false }, { id: 4, text: 'Apply second coat', completed: false }], dependencies: [] },
            { id: 7, projectId: 3, title: 'Buy new furniture', description: 'Shop for sofa and coffee table', type: 'plan', priority: 'low', completed: false, date: '2025-10-25', checklist: [{ id: 1, text: 'Measure available space', completed: true }, { id: 2, text: 'Browse online catalogs', completed: false }, { id: 3, text: 'Visit furniture stores', completed: false }], dependencies: [6] },
            { id: 8, projectId: 4, title: 'Morning workout routine', description: '30 minutes cardio + strength training', type: 'task', priority: 'high', completed: true, date: '2025-10-13', checklist: [{ id: 1, text: '10 min warm-up', completed: true }, { id: 2, text: '15 min running', completed: true }, { id: 3, text: '5 min cool-down', completed: true }], dependencies: [] },
            { id: 9, projectId: 4, title: 'Meal prep for the week', description: 'Prepare healthy meals for Monday-Friday', type: 'task', priority: 'medium', completed: false, date: '2025-10-13', checklist: [{ id: 1, text: 'Plan weekly menu', completed: true }, { id: 2, text: 'Buy ingredients', completed: false }, { id: 3, text: 'Cook and store meals', completed: false }], dependencies: [] },
            { id: 10, projectId: 5, title: 'Setup WiFi sensors', description: 'Install temperature and humidity sensors', type: 'task', priority: 'high', completed: false, date: '2025-10-17', checklist: [{ id: 1, text: 'Order ESP32 modules', completed: true }, { id: 2, text: 'Flash firmware', completed: true }, { id: 3, text: 'Mount sensors in rooms', completed: false }, { id: 4, text: 'Configure WiFi connection', completed: false }], dependencies: [] },
            { id: 11, projectId: 5, title: 'Create dashboard interface', description: 'Build web dashboard to monitor all sensors', type: 'task', priority: 'medium', completed: false, date: '2025-10-22', checklist: [{ id: 1, text: 'Design UI layout', completed: false }, { id: 2, text: 'Setup database', completed: false }, { id: 3, text: 'Implement real-time updates', completed: false }], dependencies: [10] },
            { id: 12, projectId: 5, title: 'Research automation ideas', description: 'Explore home automation scenarios', type: 'note', priority: 'low', completed: false, date: '', checklist: [], dependencies: [] },
            { id: 13, projectId: 6, title: 'Complete Python course', description: 'Finish online Python programming course', type: 'task', priority: 'medium', completed: false, date: '2025-10-30', checklist: [{ id: 1, text: 'Watch module 5 videos', completed: true }, { id: 2, text: 'Complete coding exercises', completed: false }, { id: 3, text: 'Take final exam', completed: false }], dependencies: [] },
            { id: 14, projectId: 6, title: 'Practice JavaScript', description: 'Solve coding challenges daily', type: 'plan', priority: 'medium', completed: false, date: '2025-10-31', checklist: [{ id: 1, text: 'LeetCode easy problems (5/day)', completed: false }, { id: 2, text: 'Build small projects', completed: false }], dependencies: [] },
            { id: 15, projectId: 7, title: 'Weekly grocery shopping', description: 'Buy food and household items', type: 'task', priority: 'medium', completed: false, date: '2025-10-14', checklist: [{ id: 1, text: 'Milk and eggs', completed: false }, { id: 2, text: 'Fruits and vegetables', completed: false }, { id: 3, text: 'Bread and cereals', completed: false }, { id: 4, text: 'Cleaning supplies', completed: false }], dependencies: [] }
        ];

        this.nextProjectId = Math.max(...this.projects.map(p => p.id)) + 1;
        this.nextTaskId = Math.max(...this.tasks.map(t => t.id)) + 1;
    }
}

// ==================== INITIALIZE APPLICATION ====================
let workspace;
let app; // Global reference for HTML onclick handlers

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', () => {
        workspace = new WorkspaceFinal();
        app = workspace;
    });
} else {
    workspace = new WorkspaceFinal();
    app = workspace;
}
)XJSCOREX";
}

#endif
