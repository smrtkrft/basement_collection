@echo off
REM Joy_BSA - Serial Monitor Only
call "C:\Espressif\frameworks\esp-idf-v5.5.2\export.bat"
cd /d "%~dp0"
idf.py -p COM13 monitor
