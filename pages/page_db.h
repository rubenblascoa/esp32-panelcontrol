// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod
//
// This file is auto-extracted from web_pages.cpp.
#pragma once

#include <Arduino.h>

const char db_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZenithMC | Base de Datos</title>
    <link rel="icon" type="image/png" href="https://cdn-icons-png.flaticon.com/512/8463/8463850.png">
    <script src="https://unpkg.com/lucide@latest"></script>
    <link href="https://fonts.googleapis.com/css2?family=Playfair+Display:wght@700&family=Inter:wght@400;600;900&display=swap" rel="stylesheet">

    <style>
        :root { 
            --bg-color: #05070a; --bg-gradient: radial-gradient(circle at 50% 0%, #161b22 0%, #05070a 80%);
            --card-bg: #0d1117; --border-color: #1f2530; --text-main: #ffffff; --text-muted: #8b949e;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.3); --term-bg: #000000; --term-header: #161a21;
            --accent: #00d4ff; --safe: #00ff88; --warn: #ffcc00; --crit: #ff4b4b;
        }
        body.light-theme {
            --bg-color: #f3f4f6; --bg-gradient: radial-gradient(circle at 50% 0%, #ffffff 0%, #e5e7eb 100%);
            --card-bg: #ffffff; --border-color: #d1d5db; --text-main: #111827; --text-muted: #6b7280;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.05); --term-bg: #f9fafb; --term-header: #e5e7eb;
        }
        ::-webkit-scrollbar { width: 8px; height: 8px; }
        ::-webkit-scrollbar-track { background: var(--bg-color); }
        ::-webkit-scrollbar-thumb { background: #30363d; border-radius: 10px; border: 2px solid var(--bg-color); }
        ::-webkit-scrollbar-thumb:hover { background: #484f58; }

        body { font-family: 'Inter', system-ui, sans-serif; background-color: var(--bg-color); background-image: var(--bg-gradient); background-attachment: fixed; color: var(--text-main); margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; min-height: 100vh; transition: background-color 0.4s, color 0.4s; }
        .back-nav { position: sticky; top: 0; width: 100%; max-width: 100% !important; padding: 30px 5% 20px 5%; display: grid; grid-template-columns: 1fr auto 1fr; align-items: center; z-index: 1000; transition: all 0.4s ease; box-sizing: border-box; background: transparent; }
        .back-nav.scrolled { padding: 15px 5%; background: rgba(13, 17, 23, 0.85); backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px); border-bottom: 1px solid var(--border-color); box-shadow: 0 4px 20px rgba(0,0,0,0.4); }
        body.light-theme .back-nav.scrolled { background: rgba(255, 255, 255, 0.85); }
        .nav-left { justify-self: start; display: flex; gap: 20px; align-items: center; }
        #nav-status { display: flex; align-items: center; justify-content: center; height: 100%; }
        .btn-back { display: flex; align-items: center; gap: 10px; color: var(--text-main); text-decoration: none; font-weight: 700; font-size: 0.95rem; transition: 0.3s; text-transform: uppercase; opacity: 0.7; }
        .btn-back:hover { opacity: 1; color: var(--accent); transform: translateX(-4px); }
        .theme-toggle { justify-self: end; background: transparent; border: none; color: var(--text-main); cursor: pointer; opacity: 0.7; transition: 0.3s; display: flex; align-items: center; justify-content: center; }
        .theme-toggle:hover { opacity: 1; color: var(--accent); transform: scale(1.1) rotate(15deg); }
        .theme-toggle svg { width: 18px; height: 18px; }

        .header-badge { padding: 4px 12px; border-radius: 20px; font-size: 0.75rem; font-weight: 800; letter-spacing: 1px; text-transform: uppercase; display: inline-flex; align-items: center; gap: 6px; transition: all 0.3s ease; }
        .badge-online { background: rgba(0, 255, 136, 0.1); color: var(--safe); border: 1px solid rgba(0, 255, 136, 0.3); }
        .badge-online::before { content: ''; width: 8px; height: 8px; background: var(--safe); border-radius: 50%; box-shadow: 0 0 10px var(--safe); animation: pulse 2s infinite; }
        .badge-offline { background: rgba(255, 75, 75, 0.1); color: var(--crit); border: 1px solid rgba(255, 75, 75, 0.3); }
        .badge-offline::before { content: ''; width: 8px; height: 8px; background: var(--crit); border-radius: 50%; box-shadow: 0 0 5px var(--crit); }
        .badge-connecting { background: rgba(255, 204, 0, 0.1); color: var(--warn); border: 1px solid rgba(255, 204, 0, 0.3); }
        .badge-connecting::before { content: ''; width: 8px; height: 8px; background: var(--warn); border-radius: 50%; box-shadow: 0 0 10px var(--warn); animation: pulse 1s infinite; }
        @keyframes pulse { 0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0, 255, 136, 0.7); } 70% { transform: scale(1); box-shadow: 0 0 0 6px rgba(0, 255, 136, 0); } 100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0, 255, 136, 0); } }

        .master-wrap { width: 95%; max-width: 1400px; display: flex; flex-direction: column; gap: 30px; padding-bottom: 40px; flex: 1; }
        .header-box { text-align: center; margin-top: 10px; display: flex; flex-direction: column; align-items: center; gap: 8px; }
        .brand-container { display: flex; align-items: center; justify-content: center; gap: 20px; }
        .brand-container h1 { font-size: clamp(2rem, 5vw, 3.5rem); font-weight: 900; text-transform: uppercase; letter-spacing: 2px; margin: 0; color: var(--text-main); text-shadow: 0 0 15px rgba(255, 255, 255, 0.15); }
        body.light-theme .brand-container h1 { text-shadow: none; }
        .header-subtitle { color: var(--text-muted); font-size: 0.95rem; letter-spacing: 2px; text-transform: uppercase; font-weight: 600; margin: 0; }
        
        .btn-volver-blanco { background: transparent; color: #ffffff; border: 1px solid #ffffff; padding: 8px 20px; border-radius: 8px; font-weight: 800; text-decoration: none; font-size: 0.85rem; text-transform: uppercase; transition: 0.3s; margin-top: 10px; display: inline-flex; align-items: center; gap: 8px; }
        .btn-volver-blanco:hover { background: #ffffff; color: #000000; box-shadow: 0 0 15px rgba(255, 255, 255, 0.4); transform: scale(1.05); }
        body.light-theme .btn-volver-blanco { color: #111827; border-color: #111827; }
        body.light-theme .btn-volver-blanco:hover { background: #111827; color: #ffffff; }

        .section-title { font-size: 1rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 2px; margin-bottom: 20px; text-align: left; border-bottom: 1px solid var(--border-color); padding-bottom: 10px; }

        .db-container { background: var(--card-bg); border-radius: 15px; border: 1px solid var(--border-color); box-shadow: var(--card-shadow); overflow: hidden; }
        .table-wrapper { width: 100%; overflow-x: auto; }
        table { width: 100%; border-collapse: collapse; text-align: left; white-space: nowrap; }
        thead { background: var(--term-header); position: sticky; top: 0; z-index: 10; box-shadow: 0 2px 10px rgba(0,0,0,0.2); }
        th { padding: 10px 10px; color: var(--text-muted); font-weight: 800; font-size: 0.72rem; text-transform: uppercase; letter-spacing: 0.5px; border-bottom: 1px solid var(--border-color); white-space: nowrap; }
        td { padding: 9px 10px; border-bottom: 1px solid var(--border-color); color: var(--text-main); font-family: 'Consolas', monospace; font-size: 0.85rem; font-weight: 600; }
        tr:last-child td { border-bottom: none; }
        tr:hover td { background-color: var(--term-bg); }

        .d-crit { color: var(--crit); text-shadow: 0 0 10px rgba(255,75,75,0.3); }
        .d-warn { color: var(--warn); }
        .d-ok { color: var(--safe); }
        .d-id { color: var(--text-muted); font-size: 0.8rem; }

        .pagination-wrapper { display: flex; justify-content: space-between; align-items: center; margin-top: 15px; padding: 0 5px; }
        .pagination { display: inline-flex; align-items: center; background: var(--card-bg); border: 1px solid var(--border-color); border-radius: 8px; overflow: hidden; box-shadow: var(--card-shadow); }
        .pagination button { background: transparent; border: none; border-right: 1px solid var(--border-color); color: var(--text-main); padding: 10px 16px; font-size: 0.9rem; cursor: pointer; display: inline-flex; align-items: center; gap: 5px; transition: 0.3s; font-family: 'Inter', sans-serif; font-weight: 600; }
        .pagination button:last-child { border-right: none; }
        .pagination button:hover:not(:disabled) { background: rgba(255,255,255,0.05); }
        body.light-theme .pagination button:hover:not(:disabled) { background: rgba(0,0,0,0.05); }
        
        .pagination button.active { background: #ffffff !important; color: #000000 !important; font-weight: 900; box-shadow: 0 0 10px rgba(255, 255, 255, 0.3); }
        
        .pagination button:disabled { color: var(--text-muted); cursor: not-allowed; opacity: 0.5; }
        .pagination svg { width: 16px; height: 16px; }

        .btn-accion { background: transparent; padding: 6px 12px; border-radius: 6px; font-weight: 900 !important; font-size: 0.75rem; text-transform: uppercase; transition: 0.3s; display: inline-flex; align-items: center; gap: 5px; cursor: pointer; border: 1px solid; margin: 0; text-decoration: none; }
        .btn-accion:hover { transform: scale(1.05); filter: brightness(1.2); }

        .footer-zenith { width: 100%; background: var(--card-bg); padding: 30px 0; border-top: 1px solid var(--border-color); text-align: center; margin-top: auto; transition: 0.4s; }
        .footer-zenith p { color: var(--text-muted); font-size: 0.9rem; margin: 4px 0; }
        .footer-zenith a { color: var(--text-main); text-decoration: none; font-weight: 700; }

        .btn-accion-logout {
            background: transparent;
            padding: 8px 18px;
            border-radius: 10px;
            font-weight: 900;
            font-size: 0.7rem;
            text-transform: uppercase;
            transition: 0.3s;
            display: inline-flex;
            align-items: center;
            gap: 8px;
            cursor: pointer;
            border: 1px solid var(--crit);
            color: var(--crit);
            text-decoration: none;
            letter-spacing: 1px;
        }
        .btn-accion-logout:hover {
            background: var(--crit);
            color: #fff;
            box-shadow: 0 0 15px var(--crit);
            transform: scale(1.05);
        }
        .btn-accion-logout svg { width: 14px; height: 14px; }

        .nav-right { justify-self: end; display: flex; align-items: center; gap: 20px; }
        .user-profile { display: flex; align-items: center; gap: 10px; color: var(--text-muted); font-weight: 800; font-size: 0.75rem; letter-spacing: 1px; text-transform: uppercase; transition: 0.3s; }
        .user-profile:hover { color: var(--text-main); }
        .user-profile img { width: 18px; height: 18px; border-radius: 50%; filter: grayscale(1) opacity(0.7); border: 1px solid var(--border-color); }
        .nav-icon-link { display: flex; align-items: center; color: var(--text-muted); opacity: 0.7; transition: 0.3s; }
        .nav-icon-link:hover { opacity: 1; color: var(--accent); }
        .nav-icon-link svg { width: 18px; height: 18px; }

        .modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.85); backdrop-filter: blur(10px); display: none; justify-content: center; align-items: center; z-index: 9999; }
        .modal-card { background: var(--card-bg); border: 2px solid var(--border-color); border-radius: 30px; padding: 40px; width: 90%; max-width: 400px; text-align: center; box-shadow: 0 10px 30px rgba(0,0,0,0.5); }
        .modal-btns { display: flex; gap: 15px; justify-content: center; margin-top: 25px; }
        .modal-btn { padding: 12px 25px; border-radius: 12px; font-weight: 800; cursor: pointer; border: none; text-transform: uppercase; flex: 1; transition: 0.3s; }

        @media (max-width: 600px) { 
            body { zoom: 1; }
            .back-nav { padding: 15px 5%; grid-template-columns: 1fr 1fr; } 
            #nav-status { display: none; } 
            th, td { padding: 10px 15px; font-size: 0.85rem; }
            .pagination-wrapper { flex-direction: column; gap: 15px; justify-content: center; }
        }
        .toast-container { position: fixed; bottom: 30px; right: 30px; display: flex; flex-direction: column; gap: 10px; z-index: 9998; pointer-events: none; }
        .toast { display: flex; align-items: center; gap: 10px; padding: 14px 20px; border-radius: 12px; font-size: 0.82rem; font-weight: 600; border: 1px solid; backdrop-filter: blur(10px); min-width: 280px; max-width: 420px; transform: translateX(120%); opacity: 0; transition: transform 0.4s cubic-bezier(0.22,1,0.36,1), opacity 0.4s ease; box-shadow: 0 8px 32px rgba(0,0,0,0.4); pointer-events: auto; }
        .toast.show { transform: translateX(0); opacity: 1; }
        .toast.hide { transform: translateX(120%); opacity: 0; }
        .toast-ok   { background: rgba(0,255,136,0.12); color: var(--safe); border-color: rgba(0,255,136,0.3); }
        .toast-err  { background: rgba(255,75,75,0.12);  color: var(--crit); border-color: rgba(255,75,75,0.3); }
        .toast-warn { background: rgba(255,204,0,0.12);  color: var(--warn); border-color: rgba(255,204,0,0.3); }
        .toast svg  { width: 18px; height: 18px; flex-shrink: 0; }
        @media (max-width:600px) { .toast-container { bottom: 15px; right: 15px; left: 15px; } .toast { min-width: unset; } }
    </style>
</head>
<body>

    <nav class="back-nav" id="main-nav">
        <div class="nav-left">
            <a href="https://github.com/rubenblascoa/esp32-panelcontrol" target="_blank" class="btn-back" style="color: var(--text-main); opacity: 0.7; display: flex; align-items: center; gap: 8px; transition: 0.3s;" onmouseover="this.style.opacity='1'; this.style.color='var(--accent)'" onmouseout="this.style.opacity='0.7'; this.style.color='var(--text-main)'">
                  <div style="width: 22px; height: 22px; background-color: currentColor; -webkit-mask: url('https://icones.pro/wp-content/uploads/2021/06/icone-github-grise.png') center/contain no-repeat; mask: url('https://icones.pro/wp-content/uploads/2021/06/icone-github-grise.png') center/contain no-repeat;"></div> GITHUB
            </a>
        </div>
        <div id="nav-status"><div class="header-badge badge-connecting" id="status-badge-nav">CONECTANDO...</div></div>
        <div class="nav-right">
            <div class="user-profile">
                <i data-lucide="circle-user"></i>
                <span id="nav-username">admin</span>
            </div>
            <a href="/config" class="nav-icon-link">
                <i data-lucide="settings"></i>
            </a>
            <button class="theme-toggle" id="theme-btn" onclick="toggleTheme()" title="Cambiar Tema">
                <i data-lucide="sun"></i>
            </button>
        </div>
    </nav>
    
    <div class="master-wrap">
        <div class="header-box">
            <div class="brand-container">
                <i data-lucide="database" style="width: 45px; height: 45px; color: var(--text-main);"></i>
                <h1>DB BLASCO</h1>
            </div>
            <p class="header-subtitle">Historial de Registros de Telemetría (LittleFS)</p>
            <a href="/dashboard" class="btn-volver-blanco"><i data-lucide="arrow-left" style="width: 16px; height: 16px;"></i> Volver al panel principal</a>
        </div>
        
        <section>
            <div class="section-title">Archivos de Memoria Flash (N16R8)</div>
            <div class="db-container">
                <div class="table-wrapper">
                    <table id="tablaDatos">
                        <thead>
                            <tr>
                                <th>#</th>
                                <th><i data-lucide="clock" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>Fecha/Hora</th>
                                <th><i data-lucide="thermometer" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>T.CPU °C</th>
                                <th><i data-lucide="cpu" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>CPU%</th>
                                <th><i data-lucide="activity" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>C0</th>
                                <th><i data-lucide="activity" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>C1</th>
                                <th><i data-lucide="database" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>RAM%</th>
                                <th><i data-lucide="hard-drive" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>Flash%</th>
                                <th><i data-lucide="wifi" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>WiFi%</th>
                                <th><i data-lucide="thermometer-snowflake" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>T.Amb °C</th>
                                <th><i data-lucide="droplet" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>Hum%</th>
                                <th><i data-lucide="alert-triangle" style="width: 13px; margin-right: 3px; vertical-align: middle;"></i>Alerta</th>
                            </tr>
                        </thead>
                        <tbody id="tablaCuerpo"></tbody>
                    </table>
                </div>
            </div>
            
            <div class="pagination-wrapper" style="display: flex; justify-content: space-between; align-items: center; margin-top: 15px;">
                <div style="display: flex; gap: 8px; flex-wrap: wrap;">
                    <a href="/datos.csv" download="historial.csv" class="btn-accion btn-exportar" style="border-color: var(--safe); color: var(--safe);">
                        <i data-lucide="download"></i> EXPORTAR
                    </a>
                    <button onclick="exportarSQL()" class="btn-accion" style="border-color: var(--safe); color: var(--safe); background: transparent;">
                        <i data-lucide="database-backup"></i> EXPORTAR SQL
                    </button>
                    <button onclick="document.getElementById('importarCSVInput').click()" class="btn-accion" style="border-color: #a78bfa; color: #a78bfa; background: transparent;">
                        <i data-lucide="upload"></i> IMPORTAR CSV
                    </button>
                    <input type="file" id="importarCSVInput" accept=".csv" style="display:none;" onchange="importarCSV(this)">
                    <button onclick="confirmarBorradoDB()" class="btn-accion btn-borrar" style="border-color: var(--crit); color: var(--crit);">
                        <i data-lucide="trash-2"></i> BORRAR TODO
                    </button>
                </div>
                
                <div id="pagination-container"></div>
            </div>
        </section>
    </div>

    <footer class="footer-zenith">
        <p>© 2026 ZenithMC Network. Todos los derechos reservados.</p>
        <p><a href="mailto:soporte@zenithmc.es">soporte@zenithmc.es</a></p>

        <div style="margin-top: 20px;">
            <a href="/logout" class="btn-accion-logout">
                <i data-lucide="log-out"></i> CERRAR SESIÓN
            </a>
        </div>
    </footer>

<script>

(function() {
    const params = new URLSearchParams(window.location.search);
    if (params.get('auth') === 'true') {
        sessionStorage.setItem('zenith_active', '1');
        window.history.replaceState({}, document.title, window.location.pathname);
    }
    if (!sessionStorage.getItem('zenith_active')) {
        window.location.href = '/logout';
    }
})();

lucide.createIcons();

window.onscroll = function() {
    const nav = document.getElementById("main-nav");
    if (document.body.scrollTop > 50 || document.documentElement.scrollTop > 50) nav.classList.add("scrolled");
    else nav.classList.remove("scrolled");
};

function toggleTheme() {
    document.body.classList.toggle('light-theme');
    const isLight = document.body.classList.contains('light-theme');
    document.getElementById('theme-btn').innerHTML = isLight ? '<i data-lucide="moon"></i>' : '<i data-lucide="sun"></i>';
    lucide.createIcons();
}

function setStatusBadge(status) {
    const badgeNav = document.getElementById('status-badge-nav');
    if (status === 'online') {
        badgeNav.className = 'header-badge badge-online';
        badgeNav.innerText = 'ONLINE';
    } else if (status === 'offline') {
        badgeNav.className = 'header-badge badge-offline';
        badgeNav.innerText = 'OFFLINE';
    } else if (status === 'connecting') {
        badgeNav.className = 'header-badge badge-connecting';
        badgeNav.innerText = 'RECONECTANDO...';
    }
}

let gateway = `${window.location.protocol === 'https:' ? 'wss:' : 'ws:'}//${window.location.hostname}/ws`;
let websocket;
let csvDataRecibido = false;

function initWebSocket() {
    setStatusBadge('connecting');
    websocket = new WebSocket(gateway);
    websocket.onopen = () => setStatusBadge('online');
    websocket.onclose = () => {
        setStatusBadge('offline');
        setTimeout(initWebSocket, 3000); 
    };
    websocket.onmessage = (e) => {
        try {
            const d = JSON.parse(e.data);
            if (d.type === 'csv_data' && d.text && d.text !== '__TOO_LARGE__') {
                csvDataRecibido = true;
                cargarBaseDatosDesdeTexto(d.text);
            }
        } catch(_){}
    };
}

let currentPage = 1;
let totalRows = 0;
const rowsPerPage = 15;

function cargarBaseDatos(forzar) {
    cargarPagina(1);
}

function cargarPagina(pagina) {
    currentPage = pagina;
    const url = `/api/csv?page=${pagina - 1}&size=${rowsPerPage}`;
    fetch(url)
        .then(response => {
            if (!response.ok) throw new Error("No hay datos");
            totalRows = parseInt(response.headers.get('X-Total-Lines')) || 0;
            return response.text();
        })
        .then(textoCSV => {
            const filas = textoCSV.split('\n');
            const pageData = [];
            for(let i = 0; i < filas.length; i++) {
                const linea = filas[i].trim();
                if (linea === '') continue;
                const cols = linea.split(',');
                while (cols.length < 11) cols.push('');
                pageData.push(cols);
            }
            renderPageData(pageData);
        })
        .catch(error => {
            document.getElementById('tablaCuerpo').innerHTML = `<tr><td colspan="12" style="text-align:center; padding: 30px; color: var(--text-main);"><i data-lucide="alert-circle" style="width: 24px; height: 24px; vertical-align: middle; margin-right: 10px;"></i>Sin datos registrados</td></tr>`;
            document.getElementById('pagination-container').innerHTML = '';
            lucide.createIcons();
        });
}

function cargarBaseDatosDesdeTexto(textoCSV) {
    // Si recibimos el CSV completo vía WS, rellenamos caché local
    // pero la paginación se hace desde /api/csv
    cargarPagina(1);
}

function renderPageData(pageData) {
    if(pageData.length === 0) {
        document.getElementById('tablaCuerpo').innerHTML = `<tr><td colspan="12" style="text-align:center; padding: 30px; color: var(--text-main);"><i data-lucide="alert-circle" style="width: 24px; height: 24px; vertical-align: middle; margin-right: 10px;"></i>Sin datos registrados</td></tr>`;
        document.getElementById('pagination-container').innerHTML = '';
        lucide.createIcons();
        return;
    }

    const tbody = document.getElementById('tablaCuerpo');
    tbody.innerHTML = '';

    const n = (v) => v !== null && !isNaN(v) && v !== '';
    const f = (v, d) => { let p = parseFloat(v); return (!isNaN(p) && v !== '' && v !== null) ? p : null; };
    const i = (v) => { let p = parseInt(v); return (!isNaN(p) && v !== '' && v !== null) ? p : null; };

    pageData.forEach((cols, idx) => {
        const rowId = totalRows - (currentPage - 1) * rowsPerPage - idx;
        const fechaHora = cols[0] || '—';
        const temp  = f(cols[1]);
        const cpu   = i(cols[2]);
        const c0    = i(cols[3]);
        const c1    = i(cols[4]);
        const ram   = f(cols[5]);
        const flash = f(cols[6]);
        const wifi  = i(cols[7]);
        const _dhtT = cols.length > 8 && cols[8].trim() !== '' ? parseFloat(cols[8]) : null;
        const _dhtH = cols.length > 9 && cols[9].trim() !== '' ? parseFloat(cols[9]) : null;
        const dhtTemp = (_dhtT !== null && _dhtT > -126.0 && _dhtT !== 0.0) ? _dhtT : null;
        const dhtHum  = (_dhtH !== null && _dhtH >   0.0) ? _dhtH : null;
        const alerta  = cols.length > 10 ? cols[10] : '-';

        let tempClass    = n(temp) ? (temp >= 65 ? 'd-crit' : (temp >= 45 ? 'd-warn' : 'd-ok')) : '';
        let cpuClass     = n(cpu)  ? (cpu  >= 85 ? 'd-crit' : (cpu  >= 60 ? 'd-warn' : 'd-ok')) : '';
        let ramClass     = n(ram)  ? (ram  >= 80 ? 'd-crit' : (ram  >= 50 ? 'd-warn' : 'd-ok')) : '';
        let wifiClass    = n(wifi) ? (wifi <= 40 ? 'd-crit' : (wifi <= 70 ? 'd-warn' : 'd-ok')) : '';
        let dhtTempClass = n(dhtTemp) ? (dhtTemp >= 40 ? 'd-crit' : (dhtTemp >= 30 ? 'd-warn' : 'd-ok')) : '';
        let dhtHumClass  = n(dhtHum)  ? (dhtHum  >= 80 ? 'd-crit' : (dhtHum  >= 60 ? 'd-warn' : 'd-ok')) : '';
        let alertaClass = alerta !== '-' ? 'd-crit' : '';

        tbody.innerHTML += `<tr>
            <td class="d-id">#${String(rowId).padStart(4, '0')}</td>
            <td>${fechaHora}</td>
            <td class="${tempClass}">${n(temp) ? temp.toFixed(1) : '—'}</td>
            <td class="${cpuClass}">${n(cpu) ? cpu : '—'}</td>
            <td>${n(c0) ? c0 : '—'}</td>
            <td>${n(c1) ? c1 : '—'}</td>
            <td class="${ramClass}">${n(ram) ? ram.toFixed(1) : '—'}</td>
            <td>${n(flash) ? flash.toFixed(1) : '—'}</td>
            <td class="${wifiClass}">${n(wifi) ? wifi : '—'}</td>
            <td class="${dhtTempClass}">${n(dhtTemp) ? dhtTemp.toFixed(1) : '—'}</td>
            <td class="${dhtHumClass}">${n(dhtHum) ? dhtHum.toFixed(1) : '—'}</td>
            <td class="${alertaClass}">${alerta}</td>
        </tr>`;
    });

    renderPagination(Math.ceil(totalRows / rowsPerPage), currentPage);
}

function importarCSV(input) {
    const file = input.files[0];
    if (!file) return;
    if (!file.name.endsWith('.csv')) {
        alert('El archivo debe ser un CSV (.csv)');
        input.value = '';
        return;
    }

    const reader = new FileReader();
    reader.onload = function(e) {
        const texto = e.target.result;

        // Subir el CSV al ESP32: reemplaza /datos.csv y el sistema sigue guardando sobre él
        fetch('/import-csv', {
            method: 'POST',
            headers: { 'Content-Type': 'text/plain' },
            credentials: 'same-origin',
            body: texto
        })
        .then(res => {
            if (!res.ok) throw new Error('Error al importar en el servidor');
            // Recargar la vista desde el servidor
            csvDataRecibido = false;
            cargarBaseDatos(true);
            alert('CSV importado correctamente. El ESP32 continuará guardando datos nuevos sobre este archivo.');
        })
        .catch(err => {
            alert('Error al importar el CSV: ' + err.message);
        });

        input.value = '';
        // Recargar desde el servidor para que use paginación
        cargarPagina(1);
    };
    reader.onerror = function() {
        alert('Error al leer el archivo.');
        input.value = '';
    };
    reader.readAsText(file);
}

function exportarSQL() {
    fetch('/datos.csv')
        .then(response => {
            if (!response.ok) throw new Error("Error obteniendo el CSV");
            return response.text();
        })
        .then(textoCSV => {
            const filas = textoCSV.split('\n');
            let sqlContent = `-- Exportación automática desde ZenithMC | ESP32 Blasco OS\n` +
                             `CREATE DATABASE IF NOT EXISTS \`esp32_blasco_os\` DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;\n` +
                             `USE \`esp32_blasco_os\`;\n\n` +
                             `CREATE TABLE IF NOT EXISTS \`telemetria\` (\n` +
                             `  \`id\` int(11) NOT NULL AUTO_INCREMENT,\n` +
                             `  \`fecha_hora\` varchar(50) NOT NULL,\n` +
                             `  \`temp_cpu\` float NOT NULL,\n` +
                             `  \`uso_cpu\` int(11) NOT NULL,\n` +
                             `  \`core0\` int(11) NOT NULL,\n` +
                             `  \`core1\` int(11) NOT NULL,\n` +
                             `  \`psram\` float NOT NULL,\n` +
                             `  \`flash\` float NOT NULL,\n` +
                             `  \`wifi_signal\` int(11) NOT NULL,\n` +
                              `  \`temp_dht\` float,\n` +
                              `  \`hum_dht\` float,\n` +
                              `  \`alerta\` varchar(255) DEFAULT '-',\n` +
                              `  PRIMARY KEY (\`id\`)\n` +
                             `) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n\n`;

            let insertRows = [];

            for(let i = 0; i < filas.length; i++) {
                let linea = filas[i].trim();
                if(linea !== '') {
                    const cols = linea.split(',');
                    if(cols.length < 11) continue;

                    let fechaHora = cols[0];
                    let temp = parseFloat(cols[1]);
                    let cpu = parseInt(cols[2]);
                    let c0 = parseInt(cols[3]);
                    let c1 = parseInt(cols[4]);
                    let ram = parseFloat(cols[5]);
                    let flash = parseFloat(cols[6]);
                    let wifi = parseInt(cols[7]);
                    let dhtT = cols.length > 8 ? parseFloat(cols[8]) : 'NULL';
                    let dhtH = cols.length > 9 ? parseFloat(cols[9]) : 'NULL';
                    let alerta = cols.length > 10 ? `'${cols[10].replace(/'/g, "''")}'` : "DEFAULT";

                    insertRows.push(`('${fechaHora}', ${temp}, ${cpu}, ${c0}, ${c1}, ${ram}, ${flash}, ${wifi}, ${dhtT}, ${dhtH}, ${alerta})`);
                }
            }

            if (insertRows.length > 0) {
                sqlContent += `INSERT INTO \`telemetria\` (\`fecha_hora\`, \`temp_cpu\`, \`uso_cpu\`, \`core0\`, \`core1\`, \`psram\`, \`flash\`, \`wifi_signal\`, \`temp_dht\`, \`hum_dht\`, \`alerta\`) VALUES\n` + insertRows.join(",\n") + ";\n";
                
                const blob = new Blob([sqlContent], { type: 'application/sql' });
                const url = window.URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.style.display = 'none';
                a.href = url;
                a.download = 'respaldo_blasco_os.sql';
                document.body.appendChild(a);
                a.click();
                window.URL.revokeObjectURL(url);
                document.body.removeChild(a);
            } else {
                alert("No hay registros válidos en el historial para estructurar.");
            }
        })
        .catch(err => {
            console.error(err);
            alert("No hay registros en la base de datos para exportar.");
        });
}

function confirmarBorradoDB() {
    document.getElementById('confirmModalDB').style.display = 'flex';
}

function cerrarModalDB() {
    document.getElementById('confirmModalDB').style.display = 'none';
}

function ejecutarBorradoFinal() {
    cerrarModalDB();
    fetch('/delete-db')
        .then(res => {
            if (res.ok) {
                location.reload(); 
            }
        })
        .catch(err => console.error("Error al borrar:", err));
}

function renderPagination(totalPages, curPage) {
    const container = document.getElementById('pagination-container');
    if (totalPages <= 1) {
        container.innerHTML = '';
        return;
    }

    let html = '<div class="pagination">';
    html += `<button ${curPage === 1 ? 'disabled' : ''} onclick="cargarPagina(${curPage - 1})"><i data-lucide="chevron-left"></i> Anterior</button>`;

    let startPage = curPage - 1;
    let endPage = curPage + 1;

    if (curPage === 1) {
        startPage = 1;
        endPage = Math.min(totalPages, 3);
    } else if (curPage === totalPages) {
        startPage = Math.max(1, totalPages - 2);
        endPage = totalPages;
    }

    for (let i = startPage; i <= endPage; i++) {
        html += `<button class="${i === curPage ? 'active' : ''}" onclick="cargarPagina(${i})">${i}</button>`;
    }

    html += `<button ${curPage === totalPages ? 'disabled' : ''} onclick="cargarPagina(${curPage + 1})">Siguiente <i data-lucide="chevron-right"></i></button>`;

    html += '</div>';
    container.innerHTML = html;
    
    lucide.createIcons();
}

function cargarNombreUsuario() {
    fetch('/api/config/info')
        .then(r => r.json())
        .then(d => {
            const el = document.getElementById('nav-username');
            if (el) el.textContent = d.user || 'admin';
        })
        .catch(() => {});
}

// ── TOAST ──
function toast(msg, tipo = 'ok') {
    const iconMap = { ok: 'check-circle', err: 'x-circle', warn: 'alert-triangle' };
    const tc = document.getElementById('toastContainer');
    if (!tc) return;
    const el = document.createElement('div');
    el.className = `toast toast-${tipo}`;
    el.innerHTML = `<i data-lucide="${iconMap[tipo]}" style="width:18px;height:18px;flex-shrink:0;"></i><span>${msg}</span>`;
    tc.appendChild(el);
    lucide.createIcons();
    requestAnimationFrame(() => { requestAnimationFrame(() => el.classList.add('show')); });
    setTimeout(() => {
        el.classList.remove('show');
        el.classList.add('hide');
        setTimeout(() => el.remove(), 400);
    }, 4000);
}

window.addEventListener('load', () => {
    cargarBaseDatos();
    initWebSocket();
    cargarNombreUsuario();
});
</script>

<div class="modal-overlay" id="confirmModalDB">
    <div class="modal-card">
        <i data-lucide="alert-triangle" style="width: 55px; height: 55px; color: var(--crit); margin-bottom: 15px;"></i>
        <h3>¿BORRAR HISTORIAL?</h3>
        <p>Esta acción eliminará todos los registros de la memoria Flash permanentemente.</p>
        <div class="modal-btns">
            <button class="modal-btn" style="background: var(--crit); color: #fff;" onclick="ejecutarBorradoFinal()">SÍ, BORRAR</button>
            <button class="modal-btn" style="background: var(--border-color); color: var(--text-main);" onclick="cerrarModalDB()">CANCELAR</button>
        </div>
    </div>
</div>

<div class="toast-container" id="toastContainer"></div>
</body>
</html>
)rawliteral";
