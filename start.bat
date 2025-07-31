@echo off
cd /d "C:\fipnode"
echo Starting FIP Virtual Pet...

:loop
start "" /b node main.js
timeout /t 1200 /nobreak >nul
taskkill /f /im node.exe >nul 2>&1
goto loop
