@echo off
REM Joy_BSA - Build, Flash & Monitor
REM USB: COM13

call "C:\Espressif\frameworks\esp-idf-v5.5.2\export.bat"

cd /d "%~dp0"

echo.
echo ===================================
echo  Joy_BSA - Build, Flash ^& Monitor
echo  Port: COM13
echo ===================================
echo.

idf.py set-target esp32c6
idf.py build
if %errorlevel% neq 0 (
    echo BUILD FAILED!
    pause
    exit /b 1
)

idf.py -p COM13 flash monitor
pause
