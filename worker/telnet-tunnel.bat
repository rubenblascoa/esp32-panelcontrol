@echo off
:: telnet-tunnel.bat — Inicia el proxy Telnet y abre la sesión automáticamente
:: Coloca este .bat junto a telnet-tunnel.js y edita WORKER_URL abajo.
:: Ejecutar como Administrador (necesario para el puerto 23).

setlocal

:: ── CONFIGURAR AQUÍ ────────────────────────────────────────────────────────
set WORKER_URL=esp32blascoos.rubenblasco.workers.dev
set ESP32_IP=192.168.18.203
:: Si el puerto 23 está bloqueado, cambia a 2323 y haz: telnet localhost 2323
set PORT=23
:: ───────────────────────────────────────────────────────────────────────────

echo.
echo  ESP32 Blasco OS - Telnet Tunnel
echo  Worker: %WORKER_URL%
echo.

:: Verificar Node.js
where node >nul 2>&1
if errorlevel 1 (
  echo [ERROR] Node.js no encontrado. Instalalo desde https://nodejs.org
  pause
  exit /b 1
)

:: Iniciar proxy en segundo plano
echo  Iniciando proxy TCP-WebSocket en puerto %PORT%...
start "Telnet Tunnel Proxy" /min cmd /c "node "%~dp0telnet-tunnel.js" %WORKER_URL% & pause"

:: Esperar a que arranque
timeout /t 2 /nobreak >nul

:: Conectar con telnet
echo  Conectando con Telnet...
telnet %ESP32_IP% %PORT%

endlocal
