// MIT License
// 
// Copyright (c) 2026 Ruben Blasco Armengod
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
 * @file web_pages.cpp
 * @brief Definición del contenido web estático completo del ecosistema ZenithMC.
 */
#include "web_pages.h"

// ============================================================================
// PANEL WEB 1: MONITOR PRINCIPAL Y CONSOLE TERMINAL (PROGMEM)
// ============================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZenithMC | Recursos ESP32</title>
    <link rel="icon" type="image/png" href="https://cdn-icons-png.flaticon.com/512/8463/8463850.png">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://unpkg.com/lucide@latest"></script>
    
    <link href="https://fonts.googleapis.com/css2?family=Playfair+Display:wght@700&family=Inter:wght@400;600;900&display=swap" rel="stylesheet">

    <style>
        :root { 
            --bg-color: #05070a; --bg-gradient: radial-gradient(circle at 50% 0%, #161b22 0%, #05070a 80%);
            --card-bg: #0d1117; --border-color: #1f2530; --text-main: #ffffff; --text-muted: #8b949e;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.3); --term-bg: #000000; --term-header: #161a21;
            --term-bar: #050505; --accent: #00d4ff; --safe: #00ff88; --warn: #ffcc00; --crit: #ff4b4b;
            --uptime-glow: rgba(255,255,255,0.6);
            /* Variables para Modales - Tema Oscuro por defecto */
            --modal-border: #ffffff; --modal-icon: #ffffff; --progress-fill: #ffffff;
        }

        body.light-theme {
            --bg-color: #f3f4f6; --bg-gradient: radial-gradient(circle at 50% 0%, #ffffff 0%, #e5e7eb 100%);
            --card-bg: #ffffff; --border-color: #d1d5db; --text-main: #111827; --text-muted: #6b7280;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.05); --term-bg: #f9fafb; --term-header: #e5e7eb;
            --term-bar: #ffffff; --uptime-glow: rgba(0,0,0,0.15);
            /* Variables para Modales - Tema Claro */
            --modal-border: #111827; --modal-icon: #111827; --progress-fill: #111827;
        }

        /* BARRA DE DESPLAZAMIENTO MODERNA (SELECTA) */
        ::-webkit-scrollbar { width: 8px; height: 8px; }
        ::-webkit-scrollbar-track { background: var(--bg-color); }
        ::-webkit-scrollbar-thumb { background: #30363d; border-radius: 10px; border: 2px solid var(--bg-color); }
        ::-webkit-scrollbar-thumb:hover { background: #484f58; }

        body { font-family: 'Inter', system-ui, sans-serif; background-color: var(--bg-color); background-image: var(--bg-gradient); background-attachment: fixed; color: var(--text-main); margin: 0; padding: 0; display: flex; flex-direction: column; align-items: center; min-height: 100vh; zoom: 0.9; transition: background-color 0.4s, color 0.4s; }
        
        /* =========================================
            TOPBAR ADHESIVA (STICKY) MODERNA CENTRADA
           ========================================= */
        .back-nav { 
            position: sticky; top: 0; 
            width: 100%; max-width: 100% !important;
            padding: 30px 5% 20px 5%; 
            display: grid;
            grid-template-columns: 1fr auto 1fr; 
            align-items: center; 
            z-index: 1000;
            transition: all 0.4s ease;
            box-sizing: border-box;
            background: transparent;
        }

        .back-nav.scrolled {
            padding: 15px 5%;
            background: rgba(13, 17, 23, 0.85);
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
            border-bottom: 1px solid var(--border-color);
            box-shadow: 0 4px 20px rgba(0,0,0,0.4);
        }
        body.light-theme .back-nav.scrolled { background: rgba(255, 255, 255, 0.85); }

        #nav-status {
            opacity: 0;
            transform: translateY(-10px);
            transition: all 0.3s ease;
            pointer-events: none;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100%;
        }
        .back-nav.scrolled #nav-status {
            opacity: 1;
            transform: translateY(4px);
        }

        .btn-back { justify-self: start; display: flex; align-items: center; gap: 10px; color: var(--text-main); text-decoration: none; font-weight: 700; font-size: 0.95rem; transition: 0.3s; text-transform: uppercase; opacity: 0.7; }
        .btn-back:hover { opacity: 1; color: var(--accent); transform: translateX(-4px); }
        
        .theme-toggle { justify-self: end; background: transparent; border: none; color: var(--text-main); cursor: pointer; opacity: 0.7; transition: 0.3s; display: flex; align-items: center; justify-content: center; }
        .theme-toggle:hover { opacity: 1; color: var(--accent); transform: scale(1.1) rotate(15deg); }
        .theme-toggle svg { width: 18px; height: 18px; }

        /* CONTENIDO */
        .master-wrap { width: 90%; max-width: 1100px; display: flex; flex-direction: column; gap: 40px; padding-bottom: 60px; flex-grow: 1; }
        .header-box { text-align: center; margin-top: 10px; margin-bottom: 25px; display: flex; flex-direction: column; align-items: center; gap: 8px; }
        
        .brand-container { display: flex; align-items: center; justify-content: center; gap: 20px; }
        .brand-container img { height: 50px; width: auto; filter: invert(1) drop-shadow(0 0 10px rgba(255,255,255,0.3)); transition: filter 0.4s ease; }
        body.light-theme .brand-container img { filter: invert(0) drop-shadow(0 0 5px rgba(0,0,0,0.1)); }

        .brand-container h1 { font-size: clamp(2.2rem, 6vw, 3.8rem); font-weight: 900; text-transform: uppercase; letter-spacing: 2px; margin: 0; line-height: 1.1; color: var(--text-main); text-shadow: 0 0 15px rgba(255, 255, 255, 0.15); }
        body.light-theme .brand-container h1 { text-shadow: none; }
        
        .header-subtitle { color: var(--text-muted); font-size: 0.95rem; letter-spacing: 2px; text-transform: uppercase; font-weight: 600; margin: 0; }
        
        /* BADGES */
        .header-badge { padding: 4px 12px; border-radius: 20px; font-size: 0.75rem; font-weight: 800; letter-spacing: 1px; text-transform: uppercase; display: inline-flex; align-items: center; gap: 6px; margin-bottom: 5px; transition: all 0.3s ease; }
        .badge-online { background: rgba(0, 255, 136, 0.1); color: var(--safe); border: 1px solid rgba(0, 255, 136, 0.3); }
        .badge-online::before { content: ''; width: 8px; height: 8px; background: var(--safe); border-radius: 50%; box-shadow: 0 0 10px var(--safe); animation: pulse 2s infinite; }
        .badge-offline { background: rgba(255, 75, 75, 0.1); color: var(--crit); border: 1px solid rgba(255, 75, 75, 0.3); }
        .badge-offline::before { content: ''; width: 8px; height: 8px; background: var(--crit); border-radius: 50%; box-shadow: 0 0 5px var(--crit); }
        .badge-connecting { background: rgba(255, 204, 0, 0.1); color: var(--warn); border: 1px solid rgba(255, 204, 0, 0.3); }
        .badge-connecting::before { content: ''; width: 8px; height: 8px; background: var(--warn); border-radius: 50%; box-shadow: 0 0 10px var(--warn); animation: pulse 1s infinite; }

        @keyframes pulse { 0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0, 255, 136, 0.7); } 70% { transform: scale(1); box-shadow: 0 0 0 6px rgba(0, 255, 136, 0); } 100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0, 255, 136, 0); } }

        /* MODALES INTELIGENTES */
        .modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.85); backdrop-filter: blur(10px); display: none; justify-content: center; align-items: center; z-index: 9999; }
        .modal-card { background: var(--card-bg); border: 2px solid var(--modal-border); border-radius: 35px; padding: 40px; width: 90%; max-width: 420px; text-align: center; box-shadow: 0 0 30px rgba(0,0,0,0.5); transition: border 0.4s ease; }
        .modal-card h3 { margin: 0 0 10px 0; font-size: 1.6rem; letter-spacing: 2px; font-weight: 900; color: var(--text-main); }
        .modal-card p { color: var(--text-muted); margin-bottom: 20px; }
        
        .progress-container { width: 100%; background: var(--border-color); height: 8px; border-radius: 10px; margin-top: 20px; overflow: hidden; }
        .progress-bar { width: 0%; height: 100%; background: var(--progress-fill); box-shadow: 0 0 15px var(--progress-fill); transition: width 0.3s ease, background 0.4s ease; }

        /* BOTÓN BLANCO ENLACE A DB */
        .btn-volver-blanco { background: transparent; color: #ffffff; border: 1px solid #ffffff; padding: 8px 20px; border-radius: 8px; font-weight: 800; text-decoration: none; font-size: 0.85rem; text-transform: uppercase; transition: 0.3s; margin-top: 10px; display: inline-flex; align-items: center; gap: 8px; }
        .btn-volver-blanco:hover { background: #ffffff; color: #000000; box-shadow: 0 0 15px rgba(255, 255, 255, 0.4); transform: scale(1.05); }
        body.light-theme .btn-volver-blanco { color: #111827; border-color: #111827; }
        body.light-theme .btn-volver-blanco:hover { background: #111827; color: #ffffff; }

        /* TABLAS Y GRÁFICAS */
        .section-title { font-size: 1rem; color: var(--text-muted); text-transform: uppercase; letter-spacing: 2px; margin-bottom: 25px; text-align: center; border-bottom: 1px solid var(--border-color); padding-bottom: 12px; }

        .gauges-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 20px; width: 100%; }
        .stat-card { background: var(--card-bg); border: 1px solid var(--border-color); border-radius: 20px; padding: 20px; display: flex; flex-direction: column; gap: 12px; transition: all 0.3s ease; box-shadow: var(--card-shadow); }
        .stat-card:hover { transform: translateY(-5px); border-color: var(--text-main); box-shadow: 0 8px 25px rgba(255, 255, 255, 0.05); }
        .stat-header { display: flex; align-items: center; gap: 10px; }
        .stat-header svg { width: 22px; height: 22px; stroke-width: 2.5px; transition: color 0.4s ease; color: var(--text-muted); }
        .stat-header .label { font-size: 0.85rem; color: var(--text-muted); font-weight: 800; text-transform: uppercase; letter-spacing: 1px; }
        .stat-val { font-size: 1.6rem; font-weight: 900; font-family: 'Consolas', monospace; color: var(--text-main); }
        .stat-bar-bg { width: 100%; height: 8px; background: var(--border-color); border-radius: 10px; overflow: hidden; margin-top: 5px; }
        .stat-bar-fill { height: 100%; width: 0%; border-radius: 10px; transition: width 0.6s cubic-bezier(0.4, 0, 0.2, 1), background-color 0.4s ease, box-shadow 0.4s ease; }
        #bar-u { width: 100%; background-color: var(--text-main); box-shadow: 0 0 12px var(--uptime-glow); }
        #i-u { color: var(--text-main); }

        /* REGLA MAESTRA PARA CENTRAR LOS DOS ÚLTIMOS CÍRCULOS (En pantallas grandes de 4 columnas) */
        @media (min-width: 901px) {
            .stat-card:nth-child(9) {
                grid-column-start: 2;
            }
        }

        .chart-box { background: var(--card-bg); padding: 30px; border-radius: 35px; border: 1px solid var(--border-color); box-shadow: var(--card-shadow); }
        .chart-h { height: 360px; width: 100%; }

        .cmd-terminal { background: var(--term-bg); border: 1px solid var(--border-color); border-radius: 30px; overflow: hidden; box-shadow: var(--card-shadow); }
        .cmd-header { background: var(--term-header); padding: 14px 30px; font-size: 0.85rem; color: var(--text-muted); border-bottom: 1px solid var(--border-color); font-family: monospace; }
        #logs { height: 300px; overflow-y: auto; padding: 25px; font-family: 'Consolas', monospace; color: var(--text-main); line-height: 1.4; font-size: 0.95rem; white-space: pre-wrap; }
        #logs::-webkit-scrollbar { width: 6px; }
        #logs::-webkit-scrollbar-thumb { background: #30363d; border-radius: 10px; }

        .cmd-bar { display: flex; background: var(--term-bar); padding: 18px 30px; border-top: 1px solid var(--border-color); align-items: center; gap: 15px; flex-wrap: wrap; }
        .cmd-prompt { color: var(--accent); font-weight: bold; font-family: monospace; white-space: nowrap; }
        .cmd-input { flex: 1; min-width: 150px; background: transparent; border: none; color: var(--text-main); outline: none; font-size: 1.1rem; font-family: monospace; }
        .btn { border: none; padding: 12px 28px; border-radius: 18px; font-weight: 900; cursor: pointer; transition: 0.3s; font-size: 0.95rem; text-transform: uppercase; white-space: nowrap; }
        .btn-exe { background: var(--accent); color: #000; }
        .btn-exe:hover { background: var(--text-main); transform: scale(1.05); }
        .btn-reset { background: rgba(255, 75, 75, 0.15); color: var(--crit); border: 1px solid var(--crit); }
        .btn-reset:hover { background: var(--crit); color: #fff; }

        .nav-right {
        justify-self: end;
        display: flex;
        align-items: center;
        gap: 20px;
        }

        .user-profile img {
        width: 18px;
        height: 18px;
        border-radius: 50%;
        filter: grayscale(1) opacity(0.7);
        border: 1px solid var(--border-color);
        }

        .user-profile {
        display: flex;
        align-items: center;
        gap: 10px;
        color: var(--text-muted);
        font-weight: 800;
        font-size: 0.75rem;
        letter-spacing: 1px;
        text-transform: uppercase;
        transition: 0.3s;
        }

        .user-profile:hover {
        color: var(--text-main);
        }

        .nav-icon-link {
        display: flex;
        align-items: center;
        color: var(--text-main);
        opacity: 0.7;
        transition: 0.3s;
        }
        .nav-icon-link:hover {
        opacity: 1;
        color: var(--accent);
        }
        .nav-icon-link svg {
        width: 18px;
        height: 18px;
        }
        .theme-toggle svg { width: 18px; height: 18px; }

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
        .btn-accion-logout svg {
            width: 14px;
            height: 14px;
        }


        .modal-btns { display: flex; gap: 15px; justify-content: center; flex-direction: row; }
        .modal-btn { padding: 12px 35px; border-radius: 15px; font-weight: 900; cursor: pointer; transition: 0.3s; border: none; text-transform: uppercase; flex: 1; }
        
        /* BOTÓN DE REINICIO TEMATIZADO */
        .btn-si { background: var(--progress-fill); color: var(--bg-color); }
        .btn-no { background: var(--border-color); color: var(--text-main); }

        .footer-zenith { width: 100%; background: var(--card-bg); padding: 40px 0; border-top: 1px solid var(--border-color); text-align: center; transition: 0.4s; }
        .footer-zenith p { color: var(--text-muted); font-size: 0.95rem; margin: 6px 0; }
        .footer-zenith a { color: var(--text-main); text-decoration: none; font-weight: 700; }

        @media (max-width: 900px) { .gauges-grid { grid-template-columns: repeat(2, 1fr); } }
        @media (max-width: 600px) { 
            body { zoom: 1; } .back-nav { padding: 15px 5%; grid-template-columns: 1fr 1fr; } 
            #nav-status { display: none; } 
            .brand-container { flex-direction: column; gap: 10px; }
            .stat-card { padding: 15px; } .stat-val { font-size: 1.4rem; } .chart-box { padding: 15px; border-radius: 20px; }
            .chart-h { height: 250px; } .cmd-bar { padding: 15px; flex-direction: column; align-items: stretch; gap: 10px; }
            .cmd-prompt { text-align: center; margin-bottom: 5px; } .cmd-input { text-align: center; border-bottom: 1px solid var(--border-color); padding-bottom: 10px; }
            .btn { width: 100%; } .modal-card { padding: 25px; } .modal-btns { flex-direction: column; } 
        }
    </style>
</head>
<body>

    <div class="modal-overlay" id="confirmModal">
        <div class="modal-card">
            <i data-lucide="help-circle" style="width: 50px; height: 50px; color: var(--modal-icon); margin-bottom: 15px; transition: color 0.4s ease;"></i>
            <h3>¿REINICIAR?</h3>
            <p>Se perderá la conexión temporalmente.</p>
            <div class="modal-btns">
                <button class="modal-btn btn-si" onclick="startReset()">SÍ, REINICIAR</button>
                <button class="modal-btn btn-no" onclick="closeModals()">CANCELAR</button>
            </div>
        </div>
    </div>
    <div class="modal-overlay" id="loadingModal">
        <div class="modal-card">
            <i data-lucide="refresh-cw" class="spin" style="width: 50px; height: 50px; color: var(--modal-icon); margin-bottom: 15px; transition: color 0.4s ease;"></i>
            <h3 id="loadTitle">REINICIANDO</h3>
            <p id="loadText">Esperando a que la placa esté disponible...</p>
            <div class="progress-container"><div class="progress-bar" id="pbar"></div></div>
        </div>
    </div>

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
            <div class="header-badge badge-offline" id="status-badge">Conectando...</div>
            
            <div class="brand-container">
                <img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="Chip ESP32">
                <h1>ESP32 BLASCO</h1>
            </div>
            
            <p class="header-subtitle">Panel de Control Multimódulo</p>

            <a href="/db" class="btn-volver-blanco"><i data-lucide="database" style="width: 16px; height: 16px;"></i> VER BASE DE DATOS</a>
        </div>
        
        <section>
            <div class="section-title">Monitor de Estado en Tiempo Real</div>
            <div class="gauges-grid">
                <div class="stat-card"><div class="stat-header"><i data-lucide="thermometer" id="i-t"></i><span class="label">Temp CPU</span></div><span class="stat-val" id="v-t">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-t"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="cpu" id="i-c"></i><span class="label">Uso CPU</span></div><span class="stat-val" id="v-c">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-c"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="activity" id="i-c0"></i><span class="label">Core 0</span></div><span class="stat-val" id="v-c0">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-c0"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="activity" id="i-c1"></i><span class="label">Core 1</span></div><span class="stat-val" id="v-c1">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-c1"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="database" id="i-r"></i><span class="label">PSRAM</span></div><span class="stat-val" id="v-r">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-r"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="hard-drive" id="i-f"></i><span class="label">Flash</span></div><span class="stat-val" id="v-f">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-f"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="wifi" id="i-w"></i><span class="label">Señal WiFi</span></div><span class="stat-val" id="v-w">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-w"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="clock" id="i-u"></i><span class="label">Uptime</span></div><span class="stat-val" id="v-u">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-u"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="thermometer-snowflake" id="i-ta"></i><span class="label">Temp Amb</span></div><span class="stat-val" id="v-ta">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-ta"></div></div></div>
                <div class="stat-card"><div class="stat-header"><i data-lucide="droplet" id="i-h"></i><span class="label">Humedad</span></div><span class="stat-val" id="v-h">--</span><div class="stat-bar-bg"><div class="stat-bar-fill" id="bar-h"></div></div></div>
            </div>
        </section>

        <section>
            <div class="section-title">Historial de Rendimiento</div>
            <div class="chart-box"><div class="chart-h"><canvas id="mainChart"></canvas></div></div>
        </section>

        <section>
            <div class="section-title">Consola de Control Directa</div>
            <div class="cmd-terminal">
                <div class="cmd-header">ZENITH_BLASCO // CONEXIÓN EN DIRECTO</div>
                <div id="logs"></div>
                <form class="cmd-bar" onsubmit="exe(event)">
                    <span class="cmd-prompt">esp32@blasco:~$</span>
                    <input type="text" id="cmd-in" class="cmd-input" placeholder="Comando..." autocomplete="off">
                    <button type="button" class="btn btn-reset" onclick="openConfirm()">REINICIAR</button>
                    <button type="submit" class="btn btn-exe">ENVIAR</button>
                </form>
            </div>
        </section>
    </div>

    <footer class="footer-zenith">
        <p>© 2024 ZenithMC Network. Todos los derechos reservados.</p>
        <p><a href="mailto:soporte@zenithmc.es">soporte@zenithmc.es</a></p>
        <div style="margin-top: 20px;">
            <a href="/logout" class="btn-accion-logout">
                <i data-lucide="log-out"></i> CERRAR SESIÓN
            </a>
        </div>
    </footer>

<script>

// --- GUARDIA DE SESIÓN DE PESTAÑA ---
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
    if (document.body.scrollTop > 50 || document.documentElement.scrollTop > 50) {
        nav.classList.add("scrolled");
    } else {
        nav.classList.remove("scrolled");
    }
};

function toggleTheme() {
    document.body.classList.toggle('light-theme');
    const isLight = document.body.classList.contains('light-theme');
    document.getElementById('theme-btn').innerHTML = isLight ? '<i data-lucide="moon"></i>' : '<i data-lucide="sun"></i>';
    lucide.createIcons();

    const gridColor = isLight ? 'rgba(0,0,0,0.05)' : 'rgba(255,255,255,0.05)';
    const textColor = isLight ? '#6b7280' : '#8b949e';
    chart.options.scales.y.grid.color = gridColor;
    chart.options.scales.x.ticks.color = textColor;
    chart.options.scales.y.ticks.color = textColor;
    chart.options.plugins.legend.labels.color = textColor;
    chart.update();
}

const ctx = document.getElementById('mainChart').getContext('2d');
const chart = new Chart(ctx, {
    type: 'line',
    data: { 
        labels: [], 
        datasets: [
            { label: 'Temp', data: [], borderColor: '#ff4b4b', tension: 0.3, pointRadius: 0 },
            { label: 'CPU', data: [], borderColor: '#00d4ff', tension: 0.3, pointRadius: 0 },
            { label: 'C0', data: [], borderColor: '#9900ff', tension: 0.3, borderDash: [5, 5], pointRadius: 0 },
            { label: 'C1', data: [], borderColor: '#ff00ff', tension: 0.3, borderDash: [5, 5], pointRadius: 0 },
            { label: 'RAM', data: [], borderColor: '#00ff88', tension: 0.3, pointRadius: 0 },
            { label: 'Flash', data: [], borderColor: '#ffcc00', tension: 0.3, pointRadius: 0 },
            { label: 'WiFi', data: [], borderColor: '#00ff88', tension: 0.3, pointRadius: 0 },
            { label: 'Temp Amb', data: [], borderColor: '#ff7b00', tension: 0.3, pointRadius: 0 }, 
            { label: 'Humedad', data: [], borderColor: '#00bfff', tension: 0.3, pointRadius: 0 }  
        ] 
    },
	options: {
        responsive: true, maintainAspectRatio: false,
        animation: { duration: 400, easing: 'easeOutQuart' }, 
        interaction: { mode: 'index', intersect: false },
        plugins: { 
            legend: { position: 'bottom', labels: { color: '#8b949e', usePointStyle: true, font: { size: 11, weight: 'bold' } } },
            tooltip: {
                backgroundColor: 'rgba(13, 17, 23, 0.95)',
                titleColor: '#00d4ff',
                bodyColor: '#ffffff',
                borderColor: '#1f2530',
                borderWidth: 1,
                padding: 12,
                callbacks: {
                    title: function(tooltipItems) {
                        return 'Tiempo Activo: ' + tooltipItems[0].label;
                    }
                }
            }
        },
        scales: { 
            y: { min: 0, max: 100, grid: { color: 'rgba(255,255,255,0.05)' }, ticks: { color: '#8b949e', font: { size: 11 } } },
            x: { grid: { display: false }, ticks: { color: '#8b949e', maxTicksLimit: 12 } } 
        }
    }
});

function openConfirm() { document.getElementById('confirmModal').style.display = 'flex'; }
function closeModals() { document.getElementById('confirmModal').style.display = 'none'; document.getElementById('loadingModal').style.display = 'none'; }

async function startReset() {
    document.getElementById('confirmModal').style.display = 'none';
    document.getElementById('loadingModal').style.display = 'flex';
    document.getElementById('pbar').style.width = '20%';
    if(websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send("reboot");
    }

    const checkAvailability = setInterval(async () => {
        try {
            const res = await fetch('/', { signal: AbortSignal.timeout(800) });
            if (res.ok) {
                document.getElementById('pbar').style.width = '100%';
                document.getElementById('loadTitle').innerText = '¡REESTABLECIDO!';
                document.getElementById('loadText').innerText = 'Recargando panel...';
                clearInterval(checkAvailability);
                setTimeout(() => { window.location.reload(true); }, 1000);
            }
        } catch (e) {
            let currentWidth = parseFloat(document.getElementById('pbar').style.width);
            if (currentWidth < 95) document.getElementById('pbar').style.width = (currentWidth + 2) + '%';
        }
    }, 2000);
}

function getColor(v, t) {
    if(t==='temp') return v<45 ? '#00ff88' : (v<65 ? '#ffcc00' : '#ff4b4b');
    if(t==='temp_amb') return v<30 ? '#00ff88' : (v<38 ? '#ffcc00' : '#ff4b4b');
    if(t==='wifi') return v>70 ? '#00ff88' : (v>40 ? '#ffcc00' : '#ff4b4b'); 
    if(t==='hum') return (v>=35 && v<=65) ? '#00ff88' : '#ffcc00'; 
    return v<60 ? '#00ff88' : (v<85 ? '#ffcc00' : '#ff4b4b'); 
}

function setStatusBadge(status) {
    const badge = document.getElementById('status-badge');
    const badgeNav = document.getElementById('status-badge-nav');
    
    const update = (el) => {
        if(!el) return;
        if (status === 'online') {
            el.className = 'header-badge badge-online';
            el.innerText = 'ONLINE';
        } else if (status === 'offline') {
            el.className = 'header-badge badge-offline';
            el.innerText = 'OFFLINE';
        } else if (status === 'connecting') {
            el.className = 'header-badge badge-connecting';
            el.innerText = 'RECONECTANDO...';
        }
    };
    
    update(badge);
    update(badgeNav);
}

let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

function initWebSocket() {
    setStatusBadge('connecting');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    setStatusBadge('online');
    document.getElementById('logs').innerHTML += `<span style="color:var(--safe)">[EN DIRECTO] Túnel bidireccional WebSocket establecido.</span>\n`;
}

function onClose(event) {
    setStatusBadge('offline');
    document.getElementById('logs').innerHTML += `<span style="color:var(--crit)">[SISTEMA] Conexión perdida. Reconectando en breve...</span>\n`;
    setTimeout(initWebSocket, 3000);
}

function onMessage(event) {
    try {
        let d = JSON.parse(event.data);
        if(d.type === 'telemetry') {
            actualizarDashboard(d);
            return;
        }
    } catch(e) {}
    
    const logs = document.getElementById('logs');
    logs.innerHTML += event.data;
    logs.scrollTop = logs.scrollHeight;
}

function cargarHistorialGrafica() {
    fetch('/datos.csv')
        .then(response => {
            if (!response.ok) throw new Error("Sin datos previos");
            return response.text();
        })
        .then(textoCSV => {
            const filas = textoCSV.split('\n');
            for(let i = 0; i < filas.length; i++) {
                if(filas[i].trim() !== '') {
                    const cols = filas[i].split(',');
                    while (cols.length < 10) cols.push('');
                    
                    chart.data.labels.push(cols[0]); 
                    chart.data.datasets[0].data.push(parseFloat(cols[1]) || null); 
                    chart.data.datasets[1].data.push(parseInt(cols[2]) || null);   
                    chart.data.datasets[2].data.push(parseInt(cols[3]) || null);   
                    chart.data.datasets[3].data.push(parseInt(cols[4]) || null);   
                    chart.data.datasets[4].data.push(parseFloat(cols[5]) || null); 
                    chart.data.datasets[5].data.push(parseFloat(cols[6]) || null); 
                    chart.data.datasets[6].data.push(parseInt(cols[7]) || null);   
                    chart.data.datasets[7].data.push(parseFloat(cols[8]) || null); 
                    chart.data.datasets[8].data.push(parseFloat(cols[9]) || null); 
                }
            }
            chart.update();
        })
        .catch(err => console.log("Historial no cargado:", err));
}

function actualizarDashboard(d) {
    let wifiQ = Math.min(Math.max(2 * (d.wifi + 100), 0), 100);
    
    const map = { 
        'v-t': d.temp+'°', 
        'v-c': d.cpu+'%', 
        'v-c0': d.c0+'%', 
        'v-c1': d.c1+'%', 
        'v-r': d.ram+'%', 
        'v-f': d.flash+'%', 
        'v-w': wifiQ+'%', 
        'v-u': d.uptime,
        'v-ta': (d.dhtTemp !== undefined ? d.dhtTemp : '--')+'°', 
        'v-h': (d.dhtHum !== undefined ? d.dhtHum : '--')+'%'         
    };
    for(let id in map) { let el = document.getElementById(id); if(el) el.innerText = map[id]; }

    const barValues = { 
        't': d.temp, 
        'c': d.cpu, 
        'c0': d.c0, 
        'c1': d.c1, 
        'r': d.ram, 
        'f': d.flash, 
        'w': wifiQ,
        'ta': d.dhtTemp !== undefined ? d.dhtTemp : 0,
        'h': d.dhtHum !== undefined ? d.dhtHum : 0
    };
    
    for(let key in barValues) {
        let val = barValues[key];
        let type = key === 't' ? 'temp' : (key === 'ta' ? 'temp_amb' : (key === 'w' ? 'wifi' : (key === 'h' ? 'hum' : 'perc')));
        let color = getColor(val, type);
        let barEl = document.getElementById('bar-' + key);
        if(barEl) {
            barEl.style.width = val + '%';
            barEl.style.backgroundColor = color;
            barEl.style.boxShadow = `0 0 12px ${color}, 0 0 4px ${color}`;
        }
        let iconEl = document.getElementById('i-' + key);
        if(iconEl) iconEl.style.color = color;
    }
    
    if(chart.data.labels.length > 2000) { 
        chart.data.labels.shift();
        chart.data.datasets.forEach(s => s.data.shift()); 
    }
    
    chart.data.labels.push(d.uptime);
    chart.data.datasets[0].data.push(d.temp);
    chart.data.datasets[1].data.push(d.cpu);
    chart.data.datasets[2].data.push(d.c0);
    chart.data.datasets[3].data.push(d.c1);
    chart.data.datasets[4].data.push(d.ram);
    chart.data.datasets[5].data.push(d.flash);
    chart.data.datasets[6].data.push(wifiQ);
    chart.data.datasets[7].data.push(d.dhtTemp !== undefined ? d.dhtTemp : null);
    chart.data.datasets[8].data.push(d.dhtHum !== undefined ? d.dhtHum : null);
    chart.update('none');
}

function exe(e) {
    e.preventDefault();
    const i = document.getElementById('cmd-in');
    if(!i.value) return;
    
    document.getElementById('logs').innerHTML += `<span style="color:var(--accent)">esp32@blasco:~$</span> ${i.value}\n`;
    if(websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(i.value);
    }
    
    i.value = "";
    document.getElementById('logs').scrollTop = 99999;
}

initWebSocket();

function cargarNombreUsuario() {
    fetch('/api/config/info')
        .then(r => r.json())
        .then(d => {
            const el = document.getElementById('nav-username');
            if (el) el.textContent = d.user || 'admin';
        })
        .catch(() => {});
}

window.addEventListener('load', async () => {
    cargarHistorialGrafica();
    cargarNombreUsuario();
});
</script>
</body>
</html>
)rawliteral";

// ============================================================================
// PANEL WEB 2: HISTORIAL DE TELEMETRÍA DESDE LA BASE DE DATOS (PROGMEM)
// ============================================================================
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
            <a href="/" class="btn-volver-blanco"><i data-lucide="arrow-left" style="width: 16px; height: 16px;"></i> Volver al panel principal</a>
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

let gateway = `ws://${window.location.hostname}/ws`;
let websocket;

function initWebSocket() {
    setStatusBadge('connecting');
    websocket = new WebSocket(gateway);
    websocket.onopen = () => setStatusBadge('online');
    websocket.onclose = () => {
        setStatusBadge('offline');
        setTimeout(initWebSocket, 3000); 
    };
}

let allDataRows = [];
let currentPage = 1;
const rowsPerPage = 10;

function cargarBaseDatos() {
    fetch('/datos.csv')
        .then(response => {
            if (!response.ok) throw new Error("No hay datos");
            return response.text();
        })
        .then(textoCSV => {
            const filas = textoCSV.split('\n');
            allDataRows = []; 
            let id = filas.length; 
            for(let i = filas.length - 1; i >= 0; i--) {
                if(filas[i].trim() !== '') {
                    const cols = filas[i].split(',');
                    while (cols.length < 10) cols.push('');
                    allDataRows.push({ id: id, cols: cols });
                    id--;
                }
            }
            if(allDataRows.length === 0) throw new Error("Sin datos");
            renderPage(1);
        })
        .catch(error => {
            document.getElementById('tablaCuerpo').innerHTML = `<tr><td colspan="11" style="text-align:center; padding: 30px; color: var(--text-main);"><i data-lucide="alert-circle" style="width: 24px; height: 24px; vertical-align: middle; margin-right: 10px;"></i>Sin datos registrados (Esperando al primer guardado)</td></tr>`;
            document.getElementById('pagination-container').innerHTML = '';
            lucide.createIcons();
        });
}

function renderPage(page) {
    currentPage = page;
    const totalPages = Math.ceil(allDataRows.length / rowsPerPage);
    const startIndex = (page - 1) * rowsPerPage;
    const endIndex = startIndex + rowsPerPage;
    const pageData = allDataRows.slice(startIndex, endIndex);

    const tbody = document.getElementById('tablaCuerpo');
    tbody.innerHTML = ''; 

    const n = (v) => v !== null && !isNaN(v) && v !== '';
    const f = (v, d) => { let p = parseFloat(v); return (!isNaN(p) && v !== '' && v !== null) ? p : null; };
    const i = (v) => { let p = parseInt(v); return (!isNaN(p) && v !== '' && v !== null) ? p : null; };

    pageData.forEach(row => {
        const cols = row.cols;
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

        let tempClass    = n(temp) ? (temp >= 65 ? 'd-crit' : (temp >= 45 ? 'd-warn' : 'd-ok')) : '';
        let cpuClass     = n(cpu)  ? (cpu  >= 85 ? 'd-crit' : (cpu  >= 60 ? 'd-warn' : 'd-ok')) : '';
        let ramClass     = n(ram)  ? (ram  >= 80 ? 'd-crit' : (ram  >= 50 ? 'd-warn' : 'd-ok')) : '';
        let wifiClass    = n(wifi) ? (wifi <= 40 ? 'd-crit' : (wifi <= 70 ? 'd-warn' : 'd-ok')) : '';
        let dhtTempClass = n(dhtTemp) ? (dhtTemp >= 40 ? 'd-crit' : (dhtTemp >= 30 ? 'd-warn' : 'd-ok')) : '';
        let dhtHumClass  = n(dhtHum)  ? (dhtHum  >= 80 ? 'd-crit' : (dhtHum  >= 60 ? 'd-warn' : 'd-ok')) : '';

        tbody.innerHTML += `<tr>
            <td class="d-id">#${String(row.id).padStart(4, '0')}</td>
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
        </tr>`;
    });

    renderPagination(totalPages, page);
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
            cargarBaseDatos();
            alert('CSV importado correctamente. El ESP32 continuará guardando datos nuevos sobre este archivo.');
        })
        .catch(err => {
            alert('Error al importar el CSV: ' + err.message);
        });

        // Vista previa local con manejo de campos en blanco
        const filas = texto.split('\n');
        allDataRows = [];
        let id = 1;

        for (let i = 0; i < filas.length; i++) {
            const linea = filas[i].trim();
            if (linea === '') continue;
            const cols = linea.split(',');
            while (cols.length < 10) cols.push('');
            allDataRows.push({ id: id, cols: cols });
            id++;
        }

        allDataRows.reverse();
        allDataRows.forEach((row, i) => row.id = allDataRows.length - i);

        input.value = '';

        if (allDataRows.length > 0) renderPage(1);
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
                             `  PRIMARY KEY (\`id\`)\n` +
                             `) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;\n\n`;

            let insertRows = [];

            for(let i = 0; i < filas.length; i++) {
                let linea = filas[i].trim();
                if(linea !== '') {
                    const cols = linea.split(',');
                    if(cols.length < 10) continue;

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

                    insertRows.push(`('${fechaHora}', ${temp}, ${cpu}, ${c0}, ${c1}, ${ram}, ${flash}, ${wifi}, ${dhtT}, ${dhtH})`);
                }
            }

            if (insertRows.length > 0) {
                sqlContent += `INSERT INTO \`telemetria\` (\`fecha_hora\`, \`temp_cpu\`, \`uso_cpu\`, \`core0\`, \`core1\`, \`psram\`, \`flash\`, \`wifi_signal\`, \`temp_dht\`, \`hum_dht\`) VALUES\n` + insertRows.join(",\n") + ";\n";
                
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

function renderPagination(totalPages, currentPage) {
    const container = document.getElementById('pagination-container');
    if (totalPages <= 1) {
        container.innerHTML = '';
        return;
    }

    let html = '<div class="pagination">';
    html += `<button ${currentPage === 1 ? 'disabled' : ''} onclick="renderPage(${currentPage - 1})"><i data-lucide="chevron-left"></i> Anterior</button>`;

    let startPage = currentPage - 1;
    let endPage = currentPage + 1;

    if (currentPage === 1) {
        startPage = 1;
        endPage = Math.min(totalPages, 3);
    } else if (currentPage === totalPages) {
        startPage = Math.max(1, totalPages - 2);
        endPage = totalPages;
    }

    for (let i = startPage; i <= endPage; i++) {
        html += `<button class="${i === currentPage ? 'active' : ''}" onclick="renderPage(${i})">${i}</button>`;
    }

    html += `<button ${currentPage === totalPages ? 'disabled' : ''} onclick="renderPage(${currentPage + 1})">Siguiente <i data-lucide="chevron-right"></i></button>`;

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

</body>
</html>
)rawliteral";


// ============================================================================
// PANEL WEB 3: INTERFAZ SEGURA DE ACCESO RESTRINGIDO (PROGMEM)
// ============================================================================
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZenithMC | Login</title>
    <script src="https://unpkg.com/lucide@latest"></script>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;800;900&display=swap" rel="stylesheet">
    <style>
        :root { 
            --bg-color: #05070a; --bg-gradient: radial-gradient(circle at 50% 0%, #161b22 0%, #05070a 80%);
            --card-bg: #0d1117; --border-color: #1f2530; --text-main: #ffffff; --text-muted: #8b949e;
            --accent: #00d4ff; --safe: #00ff88; --warn: #ffcc00; --crit: #ff4b4b;
        }

        body.light-theme {
            --bg-color: #f3f4f6;
            --bg-gradient: radial-gradient(circle at 50% 0%, #ffffff 0%, #e5e7eb 100%);
            --card-bg: #ffffff; --border-color: #d1d5db; --text-main: #111827; --text-muted: #6b7280;
        }

        html, body { height: 100%; margin: 0; padding: 0; overflow-x: hidden; }
        body { 
            font-family: 'Inter', sans-serif;
            background-color: var(--bg-color); background-image: var(--bg-gradient); 
            background-attachment: fixed; color: var(--text-main); display: flex; flex-direction: column; zoom: 0.9;
            transition: background-color 0.4s, color 0.4s;
        }

        .back-nav { 
            position: sticky;
            top: 0; width: 100%; padding: 30px 5% 20px 5%; 
            display: grid; grid-template-columns: 1fr auto 1fr; 
            align-items: center; z-index: 1000;
            transition: all 0.4s ease; box-sizing: border-box; background: transparent; 
        }
        .back-nav.scrolled {
            padding: 15px 5%;
            background: rgba(13, 17, 23, 0.85); backdrop-filter: blur(10px);
            border-bottom: 1px solid var(--border-color);
        }
        body.light-theme .back-nav.scrolled { background: rgba(255, 255, 255, 0.85); }

        .nav-terminal { justify-self: start;
        display: flex; align-items: center; gap: 8px; color: var(--text-muted); font-weight: 800; font-size: 0.8rem; letter-spacing: 1px;
        }
        .nav-status { justify-self: center; opacity: 0; transform: translateY(-10px); transition: 0.3s ease;
        }
        .back-nav.scrolled .nav-status { opacity: 1; transform: translateY(0);
        }
        .nav-theme { justify-self: end;
        }

        .header-badge { padding: 4px 12px;
        border-radius: 20px; font-size: 0.75rem; font-weight: 800; text-transform: uppercase; display: inline-flex; align-items: center; gap: 6px;
        }
        .badge-online { background: rgba(0, 255, 136, 0.1); color: var(--safe);
        border: 1px solid rgba(0, 255, 136, 0.3); }
        .badge-online::before { content: '';
        width: 8px; height: 8px; background: var(--safe); border-radius: 50%; box-shadow: 0 0 10px var(--safe);
        animation: pulse 2s infinite;}
        .badge-offline { background: rgba(255, 75, 75, 0.1); color: var(--crit);
        border: 1px solid rgba(255, 75, 75, 0.3); }
        .badge-offline::before { content: '';
        width: 8px; height: 8px; background: var(--crit); border-radius: 50%; box-shadow: 0 0 5px var(--crit);
        }
        .badge-connecting { background: rgba(255, 204, 0, 0.1); color: var(--warn);
        border: 1px solid rgba(255, 204, 0, 0.3); }
        .badge-connecting::before { content: '';
        width: 8px; height: 8px; background: var(--warn); border-radius: 50%; box-shadow: 0 0 10px var(--warn); animation: pulse 1s infinite;
        }

        @keyframes pulse { 0% { transform: scale(0.95);
        box-shadow: 0 0 0 0 rgba(0, 255, 136, 0.7); } 70% { transform: scale(1);
        box-shadow: 0 0 0 6px rgba(0, 255, 136, 0); } 100% { transform: scale(0.95);
        box-shadow: 0 0 0 0 rgba(0, 255, 136, 0); } }

        .master-wrap { 
            flex: 1;
            display: flex; flex-direction: column; align-items: center; justify-content: flex-start; 
            padding-top: 8vh; width: 95%; max-width: 1400px; align-self: center;
        }
        
        .header-box { text-align: center; margin-bottom: 30px; }
        .brand-container { display: flex; align-items: center; justify-content: center; gap: 15px; margin: 10px 0; }
        h1 { font-size: 3rem; font-weight: 900; letter-spacing: -1px; margin: 0; }
        .header-subtitle { color: var(--text-muted); font-size: 0.85rem; letter-spacing: 4px; text-transform: uppercase; font-weight: 600; }

        .login-card { 
            background: var(--card-bg);
            border: 1px solid var(--border-color); border-radius: 35px; 
            padding: 50px 40px; width: 100%; max-width: 400px; box-shadow: 0 10px 30px rgba(0,0,0,0.5);
        }
        
        .input-group { text-align: left; margin-bottom: 25px; position: relative; }
        .input-group svg { position: absolute; left: 16px; top: 38px; color: var(--text-muted); width: 20px; height: 20px; transition: 0.3s; }
        label { display: block; margin-bottom: 10px; font-weight: 800; font-size: 0.75rem; text-transform: uppercase; color: var(--text-muted); padding-left: 5px; }
        input { 
            width: 100%;
            background: #000; border: 1px solid var(--border-color); border-radius: 12px; 
            padding: 14px 15px 14px 48px; color: #fff; font-family: 'Inter'; transition: 0.3s;
            box-sizing: border-box; font-size: 1rem;
        }
        body.light-theme input { background: #fff; color: #000; }
        
        input:focus { border-color: var(--text-main); outline: none; box-shadow: 0 0 15px rgba(255, 255, 255, 0.15); }
        input:focus + svg, .input-group svg:focus-within { color: var(--text-main); }
        
        .btn-login { 
            width: 100%;
            background: transparent; color: var(--text-main); border: 2px solid var(--text-main); 
            padding: 16px; border-radius: 15px; font-weight: 900; font-size: 1rem; cursor: pointer; 
            text-transform: uppercase; transition: 0.3s; margin-top: 10px;
        }
        .btn-login:hover { background: var(--text-main); color: #000; box-shadow: 0 0 20px rgba(255, 255, 255, 0.4); transform: scale(1.02); }
        body.light-theme .btn-login:hover { color: #fff; }

        .footer-zenith { width: 100%;
        background: var(--card-bg); padding: 40px 0; border-top: 1px solid var(--border-color); text-align: center; margin-top: auto; transition: 0.4s;
        }
        .footer-zenith p { color: var(--text-muted); font-size: 0.95rem; margin: 6px 0; font-weight: 600; }
        .footer-zenith a { color: var(--text-main); text-decoration: none; font-weight: 700; transition: 0.3s; }
        .footer-zenith a:hover { color: var(--accent); }
        
        .theme-btn { background: none; border: none; color: var(--text-main); cursor: pointer; display: flex; align-items: center; justify-content: center; transition: 0.3s; }
        .theme-btn:hover { color: var(--accent); transform: rotate(20deg); }
    </style>
</head>
<body>

    <nav class="back-nav" id="mainNav">
        <div class="nav-terminal">
            <i data-lucide="shield-check" style="width: 18px; color: var(--safe);"></i> TERMINAL PROTEGIDA
        </div>
        <div class="nav-status" id="nav-status">
            <div class="header-badge badge-connecting" id="status-badge-nav">CONECTANDO...</div>
        </div>
        <div class="nav-theme">
            <button class="theme-btn" onclick="toggleTheme()" id="theme-btn">
                <i data-lucide="sun"></i>
            </button>
        </div>
    </nav>

    <div class="master-wrap">
        <header class="header-box">
            <div class="header-badge badge-connecting" id="status-badge">CONECTANDO...</div>
            <div class="brand-container">
                <i data-lucide="fingerprint" style="width: 45px; height: 45px; color: var(--text-main);"></i>
                <h1>ACCESO</h1>
            </div>
            <p class="header-subtitle">ZENITHMC NETWORK</p>
        </header>

        <div class="login-card">
            <form action="/login" method="POST">
                <div class="input-group">
                    <label>Usuario</label>
                    <input type="text" name="user" placeholder="Admin" required autocomplete="off">
                    <i data-lucide="user"></i>
                </div>
                <div class="input-group">
                    <label>Código de Acceso</label>
                    <input type="password" name="password" placeholder="••••••••" required>
                    <i data-lucide="key-round"></i>
                </div>
                <button type="submit" class="btn-login">Verificar Identidad</button>
            </form>
        </div>
    </div>

    <footer class="footer-zenith">
        <p>© 2026 ZenithMC Network. Todos los derechos reservados.</p>
        <p><a href="mailto:soporte@zenithmc.es">soporte@zenithmc.es</a></p>
    </footer>

    <script>
        lucide.createIcons();
        window.addEventListener('scroll', () => {
            const nav = document.getElementById('mainNav');
            if (window.scrollY > 20) {
                nav.classList.add('scrolled');
            } else {
                nav.classList.remove('scrolled');
            }
        });

        function toggleTheme() {
            document.body.classList.toggle('light-theme');
            const isLight = document.body.classList.contains('light-theme');
            document.getElementById('theme-btn').innerHTML = isLight ? '<i data-lucide="moon"></i>' : '<i data-lucide="sun"></i>';
            lucide.createIcons();
        }

        function setStatusBadge(status) {
            const badge = document.getElementById('status-badge');
            const badgeNav = document.getElementById('status-badge-nav');
            
            const update = (el) => {
                if(!el) return;
                if (status === 'online') {
                    el.className = 'header-badge badge-online';
                    el.innerText = 'ONLINE';
                } else if (status === 'offline') {
                    el.className = 'header-badge badge-offline';
                    el.innerText = 'OFFLINE';
                } else if (status === 'connecting') {
                    el.className = 'header-badge badge-connecting';
                    el.innerText = 'RECONECTANDO...';
                }
            };
            
            update(badge);
            update(badgeNav);
        }

        let gateway = `ws://${window.location.hostname}/ws`;
        let websocket;

        function initWebSocket() {
            setStatusBadge('connecting');
            websocket = new WebSocket(gateway);
            websocket.onopen = () => setStatusBadge('online');
            websocket.onclose = () => {
                setStatusBadge('offline');
                setTimeout(initWebSocket, 3000);
            };
        }

        window.addEventListener('load', () => {
            initWebSocket();
        });
    </script>
</body>
</html>
)rawliteral";

// ============================================================================
// PANEL WEB 4: CONFIGURACIÓN INICIAL WIFI (Captive Portal)
// ============================================================================
const char wifi_setup_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZenithMC | Configuraci&oacute;n WiFi</title>
    <link rel="icon" href="data:,">
    <script>
    (function(){var w={
    "alert-triangle":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0z"/><line x1="12" y1="9" x2="12" y2="13"/><line x1="12" y1="17" x2="12.01" y2="17"/></svg>',
    "sun":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="5"/><line x1="12" y1="1" x2="12" y2="3"/><line x1="12" y1="21" x2="12" y2="23"/><line x1="4.22" y1="4.22" x2="5.64" y2="5.64"/><line x1="18.36" y1="18.36" x2="19.78" y2="19.78"/><line x1="1" y1="12" x2="3" y2="12"/><line x1="21" y1="12" x2="23" y2="12"/><line x1="4.22" y1="19.78" x2="5.64" y2="18.36"/><line x1="18.36" y1="5.64" x2="19.78" y2="4.22"/></svg>',
    "moon":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79z"/></svg>',
    "wifi":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/><path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><circle cx="12" cy="20" r="1"/></svg>',
    "arrow-right":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="5" y1="12" x2="19" y2="12"/><polyline points="12 5 19 12 12 19"/></svg>',
    "info":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" y1="16" x2="12" y2="12"/><line x1="12" y1="8" x2="12.01" y2="8"/></svg>',
    "lock":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="3" y="11" width="18" height="11" rx="2" ry="2"/><path d="M7 11V7a5 5 0 0 1 10 0v4"/></svg>',
    "save":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M19 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h11l5 5v11a2 2 0 0 1-2 2z"/><polyline points="17 21 17 13 7 13 7 21"/><polyline points="7 3 7 8 15 8"/></svg>',
    "arrow-left":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="19" y1="12" x2="5" y2="12"/><polyline points="12 19 5 12 12 5"/></svg>',
    "shield":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>',
    "refresh-cw":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="23 4 23 10 17 10"/><polyline points="1 20 1 14 7 14"/><path d="M3.51 9a9 9 0 0 1 14.85-3.36L23 10M1 14l4.64 4.36A9 9 0 0 0 20.49 15"/></svg>',
    "check":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="20 6 9 17 4 12"/></svg>',
    "external-link":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M18 13v6a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h6"/><polyline points="15 3 21 3 21 9"/><line x1="10" y1="14" x2="21" y2="3"/></svg>',
    "rotate-ccw":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="1 4 1 10 7 10"/><path d="M3.51 15a9 9 0 1 0 2.13-9.36L1 10"/></svg>',
    "cpu":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><path d="M15 2v2M15 20v2M2 9h2M20 9h2M2 15h2M20 15h2M9 2v2M9 20v2"/></svg>',
    "user":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M20 21v-2a4 4 0 0 0-4-4H8a4 4 0 0 0-4 4v2"/><circle cx="12" cy="7" r="4"/></svg>',
    "smartphone":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="5" y="2" width="14" height="20" rx="2" ry="2"/><line x1="12" y1="18" x2="12.01" y2="18"/></svg>',
    "thermometer":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 14.76V3.5a2.5 2.5 0 0 0-5 0v11.26a4.5 4.5 0 1 0 5 0z"/></svg>',
    "github":'<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="currentColor"><path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z"/></svg>'
    };
    window.lucide={createIcons:function(){document.querySelectorAll('[data-lucide]').forEach(function(e){var n=e.getAttribute('data-lucide'),s=w[n];if(s){var d=document.createElement('span');d.innerHTML=s;var v=d.firstElementChild;if(e.getAttribute('style'))v.setAttribute('style',e.getAttribute('style'));if(e.getAttribute('class'))v.setAttribute('class',e.getAttribute('class'));e.parentNode.replaceChild(v,e)}})}}})();
    </script>
    <style>
        :root {
            --bg-color: #05070a;
            --bg-gradient: radial-gradient(circle at 50% 0%, #161b22 0%, #05070a 80%);
            --card-bg: #0d1117;
            --border-color: #1f2530;
            --text-main: #ffffff;
            --text-muted: #8b949e;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.3);
            --accent: #00d4ff;
            --safe: #00ff88;
            --warn: #ffcc00;
            --crit: #ff4b4b;
        }
        body.light-theme {
            --bg-color: #f3f4f6;
            --bg-gradient: radial-gradient(circle at 50% 0%, #ffffff 0%, #e5e7eb 100%);
            --card-bg: #ffffff;
            --border-color: #d1d5db;
            --text-main: #111827;
            --text-muted: #6b7280;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.05);
        }
        ::-webkit-scrollbar { width: 8px; height: 8px; }
        ::-webkit-scrollbar-track { background: var(--bg-color); }
        ::-webkit-scrollbar-thumb { background: #30363d; border-radius: 10px; border: 2px solid var(--bg-color); }
        ::-webkit-scrollbar-thumb:hover { background: #484f58; }
        html, body { height: 100%; margin: 0; padding: 0; overflow-x: hidden; }
        body {
            font-family: system-ui, -apple-system, 'Segoe UI', Roboto, sans-serif;
            background-color: var(--bg-color);
            background-image: var(--bg-gradient);
            background-attachment: fixed;
            color: var(--text-main);
            display: flex;
            flex-direction: column;
            zoom: 0.9;
            transition: background-color 0.4s, color 0.4s;
        }
        .back-nav {
            position: sticky; top: 0;
            width: 100%; padding: 30px 5% 20px 5%;
            display: grid; grid-template-columns: 1fr auto 1fr;
            align-items: center; z-index: 1000;
            transition: all 0.4s ease; box-sizing: border-box;
            background: transparent;
        }
        .back-nav.scrolled {
            padding: 15px 5%;
            background: rgba(13, 17, 23, 0.85);
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
            border-bottom: 1px solid var(--border-color);
            box-shadow: 0 4px 20px rgba(0,0,0,0.4);
        }
        body.light-theme .back-nav.scrolled { background: rgba(255, 255, 255, 0.85); }
        .btn-back { justify-self: start; display: flex; align-items: center; gap: 10px; color: var(--text-main); text-decoration: none; font-weight: 700; font-size: 0.95rem; transition: 0.3s; text-transform: uppercase; opacity: 0.7; }
        .btn-back:hover { opacity: 1; color: var(--accent); transform: translateX(-4px); }
        body.light-theme .back-nav { background: transparent; }
        .nav-left { justify-self: start; display: flex; align-items: center; gap: 10px; }
        .nav-center { justify-self: center; }
        .nav-right-wrap { justify-self: end; }
        .theme-btn {
            background: none; border: none;
            color: var(--text-main); cursor: pointer;
            display: flex; align-items: center;
            justify-content: center; transition: 0.3s; opacity: 0.7;
        }
        .theme-btn:hover { color: var(--accent); opacity: 1; transform: scale(1.1) rotate(15deg); }
        .header-badge {
            padding: 4px 12px; border-radius: 20px;
            font-size: 0.75rem; font-weight: 800;
            letter-spacing: 1px; text-transform: uppercase;
            display: inline-flex; align-items: center; gap: 6px;
            transition: all 0.3s ease;
        }
        .badge-online { background: rgba(0,255,136,0.1); color: var(--safe); border: 1px solid rgba(0,255,136,0.3); }
        .badge-online::before { content: ''; width: 8px; height: 8px; background: var(--safe); border-radius: 50%; box-shadow: 0 0 10px var(--safe); animation: pulse 2s infinite; }
        .badge-offline { background: rgba(255,75,75,0.1); color: var(--crit); border: 1px solid rgba(255,75,75,0.3); }
        .badge-offline::before { content: ''; width: 8px; height: 8px; background: var(--crit); border-radius: 50%; box-shadow: 0 0 5px var(--crit); }
        .badge-connecting { background: rgba(255,204,0,0.1); color: var(--warn); border: 1px solid rgba(255,204,0,0.3); }
        .badge-connecting::before { content: ''; width: 8px; height: 8px; background: var(--warn); border-radius: 50%; box-shadow: 0 0 10px var(--warn); animation: pulse 1s infinite; }
        .badge-ap { background: rgba(0,255,136,0.1); color: var(--safe); border: 1px solid rgba(0,255,136,0.3); }
        .badge-ap::before { content: ''; width: 8px; height: 8px; background: var(--safe); border-radius: 50%; box-shadow: 0 0 10px var(--safe); animation: pulse 1.5s infinite; }
        @keyframes pulse {
            0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0,255,136,0.7); }
            70% { transform: scale(1); box-shadow: 0 0 0 6px rgba(0,255,136,0); }
            100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0,255,136,0); }
        }
        @keyframes spin { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }
        .spin { animation: spin 1.2s linear infinite; }
        .master-wrap {
            flex: 1;
            display: flex; flex-direction: column;
            align-items: center; justify-content: flex-start;
            padding-top: 4vh;
            width: 95%; max-width: 1400px;
            align-self: center;
            padding-bottom: 60px;
        }
        .header-box { text-align: center; margin-bottom: 30px; }
        .brand-container { display: flex; align-items: center; justify-content: center; gap: 15px; margin: 10px 0; }
        .brand-container h1 { font-size: 3rem; font-weight: 900; letter-spacing: -1px; margin: 0; text-transform: uppercase; }
        .header-subtitle { color: var(--text-muted); font-size: 0.85rem; letter-spacing: 4px; text-transform: uppercase; font-weight: 600; }
        .steps-wrap {
            display: flex; align-items: center; justify-content: center;
            gap: 0; margin-bottom: 35px; width: 100%; max-width: 560px;
        }
        .step-item {
            display: flex; flex-direction: column; align-items: center; gap: 8px;
            flex: 1;
        }
        .step-circle {
            width: 36px; height: 36px; border-radius: 50%;
            border: 2px solid var(--border-color);
            display: flex; align-items: center; justify-content: center;
            font-size: 0.8rem; font-weight: 900; color: var(--text-muted);
            transition: all 0.3s;
        }
        .step-item.active .step-circle { border-color: var(--text-main); color: var(--text-main); box-shadow: 0 0 15px rgba(255,255,255,0.15); }
        .step-item.done .step-circle { border-color: var(--safe); background: rgba(0,255,136,0.1); color: var(--safe); box-shadow: 0 0 10px rgba(0,255,136,0.2); }
        .step-label { font-size: 0.65rem; font-weight: 800; letter-spacing: 1px; text-transform: uppercase; color: var(--text-muted); transition: 0.3s; }
        .step-item.active .step-label { color: var(--text-main); }
        .step-item.done .step-label { color: var(--safe); }
        .step-line { flex: 1; height: 2px; background: var(--border-color); transition: background 0.3s; margin-bottom: 20px; max-width: 60px; }
        .step-line.done { background: var(--safe); }
        .setup-card {
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 35px;
            padding: 50px 40px;
            width: 100%; max-width: 420px;
            box-shadow: var(--card-shadow);
        }
        .section-label {
            font-size: 0.72rem; font-weight: 800;
            text-transform: uppercase; letter-spacing: 1.5px;
            color: var(--text-muted); margin-bottom: 14px;
        }
        .network-list { display: flex; flex-direction: column; gap: 8px; margin-bottom: 20px; max-height: 200px; overflow-y: auto; }
        .net-item {
            display: flex; align-items: center; justify-content: space-between;
            background: rgba(255,255,255,0.03);
            border: 1px solid var(--border-color);
            border-radius: 14px; padding: 12px 16px;
            cursor: pointer; transition: all 0.2s;
        }
        .net-item:hover { border-color: rgba(255,255,255,0.2); background: rgba(255,255,255,0.06); }
        .net-item.selected { border-color: var(--text-main); background: rgba(255,255,255,0.05); box-shadow: 0 0 15px rgba(255,255,255,0.05); }
        body.light-theme .net-item.selected { border-color: #111827; }
        .net-left { display: flex; align-items: center; gap: 12px; }
        .net-name { font-weight: 800; font-size: 0.9rem; }
        .net-sec { font-size: 0.72rem; font-weight: 600; color: var(--text-muted); text-transform: uppercase; letter-spacing: 0.5px; }
        .signal-bars { display: flex; gap: 3px; align-items: flex-end; height: 16px; }
        .bar { width: 4px; border-radius: 2px; background: var(--border-color); }
        .bar.on { background: var(--safe); box-shadow: 0 0 4px var(--safe); }
        .manual-toggle {
            font-size: 0.75rem; font-weight: 700; color: var(--text-muted);
            text-align: center; cursor: pointer; text-decoration: underline;
            text-underline-offset: 3px; letter-spacing: 0.5px; transition: color 0.2s;
            margin-bottom: 20px;
        }
        .manual-toggle:hover { color: var(--accent); }
        .input-group { text-align: left; margin-bottom: 25px; position: relative; }
        .input-group label { display: block; margin-bottom: 10px; font-weight: 800; font-size: 0.75rem; text-transform: uppercase; color: var(--text-muted); padding-left: 5px; }
        .input-group input {
            width: 100%;
            background: #000; border: 1px solid var(--border-color); border-radius: 12px;
            padding: 14px 15px 14px 48px; color: #fff; font-family: inherit;
            transition: 0.3s; box-sizing: border-box; font-size: 1rem;
        }
        body.light-theme .input-group input { background: #f9fafb; color: #111827; }
        .input-group input:focus { border-color: var(--text-main); outline: none; box-shadow: 0 0 15px rgba(255,255,255,0.1); }
        .input-group input::placeholder { color: #444; }
        body.light-theme .input-group input::placeholder { color: #9ca3af; }
        .btn-main {
            width: 100%;
            background: transparent; color: var(--text-main);
            border: 2px solid var(--text-main);
            padding: 16px; border-radius: 15px; font-weight: 900;
            font-size: 1rem; cursor: pointer;
            text-transform: uppercase; transition: 0.3s;
            display: flex; align-items: center; justify-content: center; gap: 10px;
        }
        .btn-main:hover { background: var(--text-main); color: #000; box-shadow: 0 0 20px rgba(255,255,255,0.3); transform: scale(1.02); }
        .btn-main:disabled { border-color: var(--border-color); color: var(--text-muted); cursor: not-allowed; transform: none; box-shadow: none; }
        body.light-theme .btn-main:hover { color: #fff; }
        .btn-sec {
            width: 100%;
            background: transparent; color: var(--text-muted);
            border: 1px solid var(--border-color);
            padding: 12px; border-radius: 12px; font-weight: 800;
            font-size: 0.85rem; cursor: pointer; text-transform: uppercase;
            transition: 0.3s; margin-top: 12px;
            display: flex; align-items: center; justify-content: center; gap: 8px;
        }
        .btn-sec:hover { border-color: var(--text-muted); color: var(--text-main); background: rgba(255,255,255,0.04); }
        .btn-crit { border-color: var(--crit); color: var(--crit); }
        .btn-crit:hover { background: rgba(255,75,75,0.15); border-color: var(--crit); box-shadow: 0 0 15px rgba(255,75,75,0.2); color: var(--crit); }
        .divider { border: none; border-top: 1px solid var(--border-color); margin: 22px 0; }
        .progress-container { width: 100%; background: var(--border-color); height: 8px; border-radius: 10px; overflow: hidden; margin-bottom: 20px; }
        .progress-bar { width: 0%; height: 100%; background: var(--text-main); box-shadow: 0 0 15px rgba(255,255,255,0.4); transition: width 0.3s ease; }
        body.light-theme .progress-bar { box-shadow: 0 0 10px rgba(0,0,0,0.2); }
        .log-box {
            background: #000;
            border: 1px solid var(--border-color);
            border-radius: 16px; padding: 18px 20px;
            font-family: 'Consolas', monospace; font-size: 0.85rem;
            color: var(--text-main); line-height: 1.8;
            min-height: 120px; max-height: 180px;
            overflow-y: auto; margin-bottom: 22px;
            white-space: pre-wrap;
        }
        body.light-theme .log-box { background: #f9fafb; }
        .log-ok { color: var(--safe); }
        .log-info { color: var(--accent); }
        .log-warn { color: var(--warn); }
        .log-err { color: var(--crit); }
        .screen { display: none; }
        .screen.active { display: block; }
        .success-icon {
            width: 64px; height: 64px; border-radius: 50%;
            border: 2px solid var(--safe);
            background: rgba(0,255,136,0.08);
            display: flex; align-items: center; justify-content: center;
            margin: 0 auto 20px;
            box-shadow: 0 0 20px rgba(0,255,136,0.2);
        }
        .success-title { font-size: 1.8rem; font-weight: 900; text-align: center; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 10px; }
        .success-sub { color: var(--text-muted); font-size: 0.9rem; text-align: center; line-height: 1.8; margin-bottom: 25px; }
        .ip-display {
            background: #000; border: 1px solid var(--border-color);
            border-radius: 12px; padding: 14px 20px;
            font-family: 'Consolas', monospace; font-size: 1.1rem;
            font-weight: 700; color: var(--safe); text-align: center;
            letter-spacing: 2px; margin-bottom: 25px;
            box-shadow: 0 0 10px rgba(0,255,136,0.1);
        }
        body.light-theme .ip-display { background: #f9fafb; }
        .subsection-label {
            font-size: 0.68rem; font-weight: 800;
            text-transform: uppercase; letter-spacing: 1.5px;
            color: var(--text-muted); margin: 22px 0 12px 0;
            padding-bottom: 6px; border-bottom: 1px solid var(--border-color);
        }
        .input-group input[type=number]::-webkit-inner-spin-button,
        .input-group input[type=number]::-webkit-outer-spin-button { -webkit-appearance: none; margin: 0; }
        .input-group input[type=number] { -moz-appearance: textfield; appearance: textfield; }
        .input-row {
            display: flex; gap: 14px;
        }
        .input-row .input-group { flex: 1; }
        .btn-back {
            background: none; border: none;
            color: var(--text-muted); cursor: pointer;
            display: flex; align-items: center; gap: 6px;
            font-size: 0.8rem; font-weight: 700; letter-spacing: 0.5px;
            transition: all 0.3s; opacity: 0.7; padding: 0;
            font-family: inherit;
        }
        .btn-back:hover { opacity: 1; color: var(--accent); transform: translateX(-4px); }
        .btn-back-wrap { display: flex; justify-content: flex-start; margin-bottom: 8px; }
        input[type="number"]::-webkit-inner-spin-button,
        input[type="number"]::-webkit-outer-spin-button { -webkit-appearance: none; margin: 0; }
        input[type="number"] { -moz-appearance: textfield; }
        .info-chip {
            display: inline-flex; align-items: center; gap: 6px;
            background: rgba(0,255,136,0.07); border: 1px solid rgba(0,255,136,0.2);
            border-radius: 8px; padding: 8px 14px;
            font-size: 0.75rem; font-weight: 700; color: var(--safe);
            text-transform: uppercase; letter-spacing: 0.5px;
            width: 100%; box-sizing: border-box; margin-top: 16px;
        }
        .modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.85); backdrop-filter: blur(10px); display: none; justify-content: center; align-items: center; z-index: 9999; }
        .modal-card { background: var(--card-bg); border: 2px solid var(--border-color); border-radius: 35px; padding: 40px; width: 90%; max-width: 420px; text-align: center; box-shadow: 0 0 30px rgba(0,0,0,0.5); }
        .modal-card h3 { margin: 0 0 10px 0; font-size: 1.5rem; letter-spacing: 2px; font-weight: 900; }
        .modal-card p { color: var(--text-muted); margin-bottom: 0; font-size: 0.9rem; }
        .modal-btns { display: flex; gap: 12px; justify-content: center; margin-top: 25px; }
        .modal-btn { padding: 12px 28px; border-radius: 12px; font-weight: 900; cursor: pointer; border: none; text-transform: uppercase; flex: 1; transition: 0.3s; font-size: 0.85rem; font-family: inherit; }
        .github-link { display: inline-flex; align-items: center; gap: 6px; margin-top: 8px; }
        .github-link:hover i { color: var(--accent) !important; }
        .footer-zenith { width: 100%; background: var(--card-bg); padding: 40px 0; border-top: 1px solid var(--border-color); text-align: center; margin-top: auto; transition: 0.4s; }
        .footer-zenith p { color: var(--text-muted); font-size: 0.95rem; margin: 6px 0; font-weight: 600; }
        .footer-zenith a { color: var(--text-main); text-decoration: none; font-weight: 700; transition: 0.3s; }
        .footer-zenith a:hover { color: var(--accent); }
        @media (max-width: 600px) {
            body { zoom: 1; }
            .back-nav { padding: 15px 5%; grid-template-columns: 1fr 1fr; }
            .nav-center { display: none; }
            .setup-card { padding: 30px 24px; border-radius: 25px; }
            .brand-container h1 { font-size: 2.2rem; }
            .input-row { flex-direction: column; gap: 0; }
        }
    </style>
</head>
<body>

<div class="modal-overlay" id="resetModal">
    <div class="modal-card">
        <i data-lucide="alert-triangle" style="width:50px;height:50px;color:var(--crit);margin-bottom:15px;"></i>
        <h3>RECONFIGURAR</h3>
        <p>Se borrar&aacute;n las credenciales guardadas en la memoria NVS y el ESP32 reiniciar&aacute; en modo AP.</p>
        <div class="modal-btns">
            <button class="modal-btn" style="background:var(--crit);color:#fff;" onclick="ejecutarReset()">S&Iacute;, BORRAR</button>
            <button class="modal-btn" style="background:var(--border-color);color:var(--text-main);" onclick="cerrarModal()">CANCELAR</button>
        </div>
    </div>
</div>

<nav class="back-nav" id="mainNav">
    <div class="nav-left">
        <a href="https://github.com/rubenblascoa/esp32-panelcontrol" target="_blank" class="btn-back" style="color: var(--text-main); opacity: 0.7; display: flex; align-items: center; gap: 8px; transition: 0.3s;" onmouseover="this.style.opacity='1'; this.style.color='var(--accent)'" onmouseout="this.style.opacity='0.7'; this.style.color='var(--text-main)'">
            <i data-lucide="github" style="width:18px;height:18px;"></i> GITHUB
        </a>
    </div>
    <div class="nav-center"></div>
    <div class="nav-right-wrap">
        <button class="theme-btn" onclick="toggleTheme()" id="theme-btn">
            <i data-lucide="sun"></i>
        </button>
    </div>
</nav>

<div class="master-wrap">

    <header class="header-box">
        <div class="header-badge badge-ap" id="statusBadge">MODO AP ACTIVO</div>
        <div class="brand-container">
            <svg xmlns="http://www.w3.org/2000/svg" width="50" height="50" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><path d="M15 2v2M15 20v2M2 9h2M20 9h2M2 15h2M20 15h2M9 2v2M9 20v2"/></svg>
            <h1>ESP32 BLASCO</h1>
        </div>
        <p class="header-subtitle">Configuraci&oacute;n Inicial &middot; Primer Arranque</p>
    </header>

    <div class="steps-wrap" id="stepsWrap">
        <div class="step-item active" id="step0-item"><div class="step-circle">C</div><div class="step-label">Config</div></div>
        <div class="step-line" id="line0"></div>
        <div class="step-item" id="step1-item"><div class="step-circle">1</div><div class="step-label">Red</div></div>
        <div class="step-line" id="line1"></div>
        <div class="step-item" id="step2-item"><div class="step-circle">2</div><div class="step-label">Clave</div></div>
        <div class="step-line" id="line2"></div>
        <div class="step-item" id="step3-item"><div class="step-circle">3</div><div class="step-label">Online</div></div>
    </div>

    <div class="setup-card">

        <!-- ===== SCREEN 0: CONFIGURACIÓN HARDWARE + SEGURIDAD ===== -->
        <div class="screen active" id="screen0">

            <div class="section-label">Pines del hardware</div>

            <div class="subsection-label">NFC (MFRC522)</div>
            <div class="input-row">
                <div class="input-group">
                    <label>RST pin</label>
                    <input type="number" id="nfcRst" placeholder="21" min="0" max="48">
                    <i data-lucide="cpu" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
                </div>
                <div class="input-group">
                    <label>SS pin</label>
                    <input type="number" id="nfcSs" placeholder="5" min="0" max="48">
                    <i data-lucide="cpu" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
                </div>
            </div>

            <div class="subsection-label">Ultrasonidos (HC-SR04)</div>
            <div class="input-row">
                <div class="input-group">
                    <label>TRIG pin</label>
                    <input type="number" id="trigPin" placeholder="15" min="0" max="48">
                    <i data-lucide="cpu" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
                </div>
                <div class="input-group">
                    <label>ECHO pin</label>
                    <input type="number" id="echoPin" placeholder="16" min="0" max="48">
                    <i data-lucide="cpu" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
                </div>
            </div>

            <div class="subsection-label">Temperatura (DHT11)</div>
            <div class="input-group">
                <label>DATA pin</label>
                <input type="number" id="dhtPin" placeholder="4" min="0" max="48">
                <i data-lucide="cpu" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
            </div>

            <hr class="divider">

            <div class="section-label">Seguridad web</div>

            <div class="input-group">
                <label>Usuario</label>
                <input type="text" id="webUser" placeholder="admin">
                <i data-lucide="user" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
            </div>

            <div class="input-group">
                <label>Contrase&ntilde;a</label>
                <input type="text" id="webPass" placeholder="blasco">
                <i data-lucide="lock" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
            </div>

            <hr class="divider">

            <button class="btn-main" id="btnConfigOk" onclick="goStep1()">
                <i data-lucide="arrow-right" style="width:18px;height:18px;"></i>
                CONTINUAR
            </button>

            <div class="info-chip">
                <i data-lucide="info" style="width:14px;height:14px;"></i>
                Los cambios de pines requieren reinicio. Credenciales cifradas en NVS.
            </div>
        </div>

        <div class="screen" id="screen1">
            <div class="section-label">Redes disponibles</div>
            <div class="network-list" id="netList">
                <div style="text-align:center;padding:20px;color:var(--text-muted);font-size:0.85rem;font-weight:600;">No se encontraron redes WiFi. Escaneando...</div>
            </div>

            <div id="manualWrap" style="display:none;">
                <div class="input-group">
                    <label>SSID manual</label>
                    <input type="text" id="manualSsid" placeholder="Nombre de la red..." oninput="onManualInput()">
                    <i data-lucide="wifi" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
                </div>
            </div>

            <div class="manual-toggle" id="manualToggle" onclick="toggleManual()">Introducir SSID manualmente</div>

            <hr class="divider">
            <button class="btn-main" id="btnContinuar1" onclick="goStep2()" disabled>
                <i data-lucide="arrow-right" style="width:18px;height:18px;"></i>
                CONTINUAR
            </button>
            <div class="btn-back-wrap" style="margin-top:14px;">
                <button class="btn-back" onclick="goBackToConfig()">
                    <i data-lucide="arrow-left" style="width:16px;height:16px;"></i>
                    VOLVER A CONFIG
                </button>
            </div>

            <div class="info-chip">
                <i data-lucide="info" style="width:14px;height:14px;"></i>
                Las credenciales se cifran en NVS. Nunca en el c&oacute;digo fuente.
            </div>
        </div>

        <div class="screen" id="screen2">
            <div class="section-label">Red seleccionada</div>
            <div style="background:rgba(0,255,136,0.06);border:1px solid rgba(0,255,136,0.2);border-radius:12px;padding:12px 16px;margin-bottom:22px;display:flex;align-items:center;gap:10px;">
                <i data-lucide="wifi" style="width:18px;height:18px;color:var(--safe);flex-shrink:0;"></i>
                <span style="font-weight:800;font-size:0.95rem;" id="selectedSsidLabel">&mdash;</span>
            </div>

            <div class="input-group">
                <label>Contrase&ntilde;a WiFi</label>
                <input type="password" id="wifiPass" placeholder="&#8226;&#8226;&#8226;&#8226;&#8226;&#8226;&#8226;&#8226;" oninput="checkPass()" autocomplete="off">
                <i data-lucide="lock" style="position:absolute;left:16px;top:50%;margin-top:14px;transform:translateY(-50%);color:var(--text-muted);width:20px;height:20px;"></i>
            </div>

            <button class="btn-main" id="btnGuardar" onclick="goStep3()" disabled>
                <i data-lucide="save" style="width:18px;height:18px;"></i>
                GUARDAR Y CONECTAR
            </button>
            <button class="btn-sec" onclick="goBack()">
                <i data-lucide="arrow-left" style="width:16px;height:16px;"></i>
                VOLVER
            </button>

            <hr class="divider">
            <div class="info-chip">
                <i data-lucide="shield" style="width:14px;height:14px;"></i>
                Cifrado AES-256 en partici&oacute;n NVS del ESP32
            </div>
        </div>

        <div class="screen" id="screen3">
            <div style="text-align:center;margin-bottom:45px;">
                <i data-lucide="refresh-cw" class="spin" style="width:45px;height:45px;color:var(--safe);margin-bottom:15px;"></i>
                <div id="statusTextLive" style="font-weight:900;font-size:1.1rem;text-transform:uppercase;letter-spacing:1px;color:var(--safe); margin-bottom:6px;">Iniciando...</div>
                <div style="color:var(--text-muted);font-size:0.82rem;margin-top:6px;letter-spacing:0.5px;">Por favor, mant&eacute;n el dispositivo encendido</div>
            </div>
            <div class="progress-container" style="margin-bottom:25px;"><div class="progress-bar" id="pbar"></div></div>
            <div class="section-label" style="font-size:0.65rem;margin-bottom:8px;letter-spacing:1px;color:var(--text-muted);">Diagn&oacute;stico del Sistema (Mini-Log)</div>
            <div class="log-box" id="logBox" style="min-height:80px;max-height:110px;padding:12px 16px;font-size:0.78rem;border-radius:14px;margin-bottom:0;"></div>
        </div>

        <div class="screen" id="screen4">
            <div class="success-icon">
                <i data-lucide="check" style="width:30px;height:30px;color:var(--safe);"></i>
            </div>
            <div class="success-title">&iexcl;Conectado!</div>
            <div class="success-sub">
                ZenithMC est&aacute; online y operativo.<br>
                Accede al panel de control en:
            </div>
            <div class="ip-display" id="ipDisplay">192.168.x.x</div>

            <button class="btn-main" onclick="abrirPanel()">
                <i data-lucide="external-link" style="width:18px;height:18px;"></i>
                ABRIR PANEL
            </button>
            <button class="btn-sec btn-crit" onclick="abrirModal()" style="margin-top:12px;">
                <i data-lucide="rotate-ccw" style="width:16px;height:16px;"></i>
                RECONFIGURAR
            </button>
        </div>

    </div>
</div>

<footer class="footer-zenith">
    <p>&copy; 2026 ZenithMC Network. Todos los derechos reservados.</p>
    <p><a href="mailto:soporte@zenithmc.es">soporte@zenithmc.es</a></p>
</footer>

<script>
lucide.createIcons();

window.addEventListener('scroll', function() {
    var nav = document.getElementById('mainNav');
    nav.classList.toggle('scrolled', window.scrollY > 20);
});

function toggleTheme() {
    document.body.classList.toggle('light-theme');
    var isLight = document.body.classList.contains('light-theme');
    document.getElementById('theme-btn').innerHTML = isLight ? '<i data-lucide="moon"></i>' : '<i data-lucide="sun"></i>';
    lucide.createIcons();
}

var selSsid = null;
var manualMode = false;
var polling = false;
var scanInterval = null;
var redesCache = '';

function encryptionLabel(enc) {
    if (enc == 0) return 'OPEN';
    if (enc == 1) return 'WEP';
    if (enc == 2) return 'WPA';
    if (enc == 3) return 'WPA2';
    if (enc == 4) return 'WPA/WPA2';
    if (enc == 5) return 'WPA2-ENT';
    if (enc == 8) return 'WPA3';
    return 'WPA2';
}

function signalBars(rssi) {
    if (rssi >= -50) return 4;
    if (rssi >= -65) return 3;
    if (rssi >= -80) return 2;
    return 1;
}

function scanNetworks() {
    fetch('/api/wifi/scan')
        .then(function(res) {
            if (!res.ok) throw new Error('HTTP ' + res.status);
            return res.json();
        })
        .then(function(data) {
            var fresh = JSON.stringify(data);
            if (fresh !== redesCache || data.length === 0) {
                redesCache = fresh;
                renderRedes(data);
                if (selSsid) reselectNet(selSsid);
            }
        })
        .catch(function() {});
}

function reselectNet(ssid) {
    var items = document.querySelectorAll('.net-item');
    var found = false;
    items.forEach(function(el) {
        var name = el.getAttribute('data-ssid');
        if (name === ssid) { el.classList.add('selected'); found = true; }
        else { el.classList.remove('selected'); }
    });
    document.getElementById('btnContinuar1').disabled = !found;
    if (!found) selSsid = null;
}

function renderRedes(nets) {
    var c = document.getElementById('netList');
    if (!nets || nets.length === 0) {
        c.innerHTML = '<div style="text-align:center;padding:20px;color:var(--text-muted);font-size:0.85rem;font-weight:600;">No se encontraron redes WiFi. Escaneando...</div>';
        return;
    }
    c.innerHTML = nets.map(function(n) {
        var safeSsid = n.ssid.replace(/'/g, "\\'");
        return '<div class="net-item" data-ssid="' + safeSsid + '" onclick="selectNet(\'' + safeSsid + '\', this)">' +
            '<div class="net-left"><i data-lucide="wifi" style="width:18px;height:18px;color:var(--text-muted);flex-shrink:0;"></i>' +
            '<div><div class="net-name">' + n.ssid + '</div>' +
            '<div class="net-sec">' + encryptionLabel(n.encryption) + '</div></div></div>' +
            '<div class="signal-bars">' +
            [4,8,12,16].map(function(h, i) { return '<div class="bar' + (i < signalBars(n.rssi) ? ' on' : '') + '" style="height:' + h + 'px;"></div>'; }).join('') +
            '</div></div>';
    }).join('');
    lucide.createIcons();
}

function selectNet(ssid, el) {
    selSsid = ssid;
    document.querySelectorAll('.net-item').forEach(function(e) { e.classList.remove('selected'); });
    el.classList.add('selected');
    document.getElementById('btnContinuar1').disabled = false;
}

function toggleManual() {
    manualMode = !manualMode;
    document.getElementById('manualWrap').style.display  = manualMode ? 'block' : 'none';
    document.getElementById('netList').style.display     = manualMode ? 'none'  : 'flex';
    document.getElementById('manualToggle').textContent  = manualMode ? 'Volver a la lista' : 'Introducir SSID manualmente';
    document.getElementById('btnContinuar1').disabled    = true;
    selSsid = null;
    lucide.createIcons();
}

function onManualInput() {
    var v = document.getElementById('manualSsid').value.trim();
    selSsid = v.length >= 2 ? v : null;
    document.getElementById('btnContinuar1').disabled = !selSsid;
}

function iniciarScanPeriodico() {
    if (scanInterval) clearInterval(scanInterval);
    scanNetworks(); // primer scan inmediato
    scanInterval = setInterval(scanNetworks, 3000); // refresco cada 3s
}

function detenerScanPeriodico() {
    if (scanInterval) { clearInterval(scanInterval); scanInterval = null; }
}

function setStep(n) {
    var screens = ['screen0','screen1','screen2','screen3','screen4'];
    screens.forEach(function(id, i) { document.getElementById(id).classList.toggle('active', i === n); });
    var items = ['step0-item','step1-item','step2-item','step3-item'];
    var lines = ['line0','line1','line2'];
    items.forEach(function(id, i) {
        var el = document.getElementById(id);
        el.classList.toggle('active', i === n);
        el.classList.toggle('done', i < n);
    });
    lines.forEach(function(id, i) {
        document.getElementById(id).classList.toggle('done', i < n);
    });
    document.getElementById('stepsWrap').style.display = n === 4 ? 'none' : 'flex';
    if (n === 1) iniciarScanPeriodico();
    else detenerScanPeriodico();
}

function goStep1() {
    setStep(1);
}

function goBackToConfig() {
    detenerScanPeriodico();
    document.getElementById('btnContinuar1').disabled = true;
    selSsid = null;
    setStep(0);
}

function goStep2() {
    if (manualMode) selSsid = document.getElementById('manualSsid').value.trim();
    document.getElementById('selectedSsidLabel').textContent = selSsid;
    lucide.createIcons();
    setStep(2);
}

function goBack() {
    document.getElementById('wifiPass').value = '';
    document.getElementById('btnGuardar').disabled = true;
    setStep(1);
}

function checkPass() {
    document.getElementById('btnGuardar').disabled = document.getElementById('wifiPass').value.length < 3;
}

function addLog(logBox, cls, msg) {
    logBox.innerHTML += '<span class="' + cls + '">' + msg + '</span>\n';
    logBox.scrollTop = logBox.scrollHeight;
}

function goStep3() {
    setStep(3);
    iniciarConexion();
}

function iniciarConexion() {
    var log = document.getElementById('logBox');
    var bar = document.getElementById('pbar');
    var statusTxt = document.getElementById('statusTextLive');
    log.innerHTML = '<span class="log-info">[SISTEMA] Iniciando configuraci&oacute;n...</span>';
    bar.style.width = '0%';
    statusTxt.textContent = 'Enviando credenciales...';
    var password = document.getElementById('wifiPass').value;

    addLog(log, 'log-info', '[API]    Enviando credenciales al ESP32...');
    var payload = {ssid: selSsid, password: password};
    var cRst = parseInt(document.getElementById('nfcRst').value);
    if (!isNaN(cRst)) payload.nfcRst = cRst;
    var cSs = parseInt(document.getElementById('nfcSs').value);
    if (!isNaN(cSs)) payload.nfcSs = cSs;
    var cTrig = parseInt(document.getElementById('trigPin').value);
    if (!isNaN(cTrig)) payload.trigPin = cTrig;
    var cEcho = parseInt(document.getElementById('echoPin').value);
    if (!isNaN(cEcho)) payload.echoPin = cEcho;
    var cDht = parseInt(document.getElementById('dhtPin').value);
    if (!isNaN(cDht)) payload.dhtPin = cDht;
    var cUser = document.getElementById('webUser').value;
    if (cUser) payload.webUser = cUser;
    var cPass = document.getElementById('webPass').value;
    if (cPass) payload.webPass = cPass;
    fetch('/api/wifi/configure', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(payload)
    })
    .then(function(res) { return res.json(); })
    .then(function(data) {
        if (data.status !== 'ok') {
            addLog(log, 'log-err', '[ERROR]  ' + (data.message || 'Error al guardar credenciales.'));
            statusTxt.textContent = 'Error';
            return;
        }
        addLog(log, 'log-ok', '[NVS]    Credenciales guardadas en NVS.');
        bar.style.width = '25%';
        statusTxt.textContent = 'Conectando a la red...';
        polling = true;
        pollStatus(log, bar, statusTxt);
    })
    .catch(function() {
        addLog(log, 'log-err', '[ERROR]  No se pudo contactar con el ESP32.');
        statusTxt.textContent = 'Error de comunicaci\u00f3n';
    });
}

function pollStatus(log, bar, statusTxt) {
    if (!polling) return;
    fetch('/api/wifi/status')
    .then(function(res) { return res.json(); })
    .then(function(data) {
        if (data.status === 'connected') {
            polling = false;
            bar.style.width = '100%';
            statusTxt.textContent = 'CONECTADO';
            addLog(log, 'log-ok', '[WiFi]   Conectado exitosamente.');
            addLog(log, 'log-ok', '[IP]     ' + data.ip);
            document.getElementById('ipDisplay').textContent = data.ip;
            setStep(4);
            lucide.createIcons();
            addLog(log, 'log-info', '[SISTEMA] Reiniciando ESP32 en 3 segundos...');
            setTimeout(function() {
                fetch('/api/wifi/reboot', {method: 'POST'}).catch(function(){});
            }, 3000);
            return;
        }
        if (data.status === 'failed') {
            polling = false;
            statusTxt.textContent = 'Conexi\u00f3n fallida';
            addLog(log, 'log-err', '[ERROR]  ' + (data.message || 'No se pudo conectar a la red.'));
            return;
        }
        if (data.status === 'connecting') {
            var pct = data.progress || 30;
            bar.style.width = Math.min(pct, 90) + '%';
            statusTxt.textContent = data.message || 'Conectando...';
        }
        setTimeout(function() { pollStatus(log, bar, statusTxt); }, 1000);
    })
    .catch(function() {
        setTimeout(function() { pollStatus(log, bar, statusTxt); }, 1500);
    });
}

function abrirPanel() {
    window.location.href = '/';
}

function abrirModal()  { document.getElementById('resetModal').style.display = 'flex'; }
function cerrarModal() { document.getElementById('resetModal').style.display = 'none'; }

function ejecutarReset() {
    cerrarModal();
    document.getElementById('wifiPass').value = '';
    selSsid = null;
    manualMode = false;
    document.getElementById('manualWrap').style.display = 'none';
    document.getElementById('netList').style.display    = 'flex';
    document.getElementById('manualToggle').textContent = 'Introducir SSID manualmente';
    document.getElementById('btnContinuar1').disabled   = true;
    document.getElementById('nfcRst').value = '';
    document.getElementById('nfcSs').value = '';
    document.getElementById('trigPin').value = '';
    document.getElementById('echoPin').value = '';
    document.getElementById('dhtPin').value = '';
    document.getElementById('webUser').value = '';
    document.getElementById('webPass').value = '';
    setStep(0);
    lucide.createIcons();
    fetch('/api/wifi/reset', {method: 'POST'}).catch(function() {});
}


</script>
</body>
</html>
)rawliteral";


// ============================================================================
// PANEL WEB 5: CONFIGURACIÓN 
// ============================================================================

const char config_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZenithMC | Configuración del Sistema</title>
    <link rel="icon" type="image/png" href="https://cdn-icons-png.flaticon.com/512/8463/8463850.png">
    <script src="https://unpkg.com/lucide@latest"></script>
    <link href="https://fonts.googleapis.com/css2?family=Playfair+Display:wght@700&family=Inter:wght@400;600;900&display=swap" rel="stylesheet">
    <style>
        :root {
            --bg-color: #05070a; --bg-gradient: radial-gradient(circle at 50% 0%, #161b22 0%, #05070a 80%);
            --card-bg: #0d1117; --border-color: #1f2530; --text-main: #ffffff; --text-muted: #8b949e;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.3); --term-bg: #000000; --term-header: #161a21;
            --accent: #00d4ff; --safe: #00ff88; --warn: #ffcc00; --crit: #ff4b4b;
            --modal-border: #ffffff; --modal-icon: #ffffff; --progress-fill: #ffffff;
        }
        body.light-theme {
            --bg-color: #f3f4f6; --bg-gradient: radial-gradient(circle at 50% 0%, #ffffff 0%, #e5e7eb 100%);
            --card-bg: #ffffff; --border-color: #d1d5db; --text-main: #111827; --text-muted: #6b7280;
            --card-shadow: 0 5px 15px rgba(0,0,0,0.05); --term-bg: #f9fafb; --term-header: #e5e7eb;
            --modal-border: #111827; --modal-icon: #111827; --progress-fill: #111827;
        }
        ::-webkit-scrollbar { width: 8px; height: 8px; }
        ::-webkit-scrollbar-track { background: var(--bg-color); }
        ::-webkit-scrollbar-thumb { background: #30363d; border-radius: 10px; border: 2px solid var(--bg-color); }
        ::-webkit-scrollbar-thumb:hover { background: #484f58; }

        body {
            font-family: 'Inter', system-ui, sans-serif;
            background-color: var(--bg-color); background-image: var(--bg-gradient);
            background-attachment: fixed; color: var(--text-main);
            margin: 0; padding: 0; display: flex; flex-direction: column;
            align-items: center; min-height: 100vh; zoom: 0.9;
            transition: background-color 0.4s, color 0.4s;
        }

        /* ── NAV ── */
        .back-nav {
            position: sticky; top: 0; width: 100%; max-width: 100% !important;
            padding: 30px 5% 20px 5%; display: grid;
            grid-template-columns: 1fr auto 1fr; align-items: center;
            z-index: 1000; transition: all 0.4s ease; box-sizing: border-box; background: transparent;
        }
        .back-nav.scrolled {
            padding: 15px 5%; background: rgba(13,17,23,0.85);
            backdrop-filter: blur(10px); -webkit-backdrop-filter: blur(10px);
            border-bottom: 1px solid var(--border-color); box-shadow: 0 4px 20px rgba(0,0,0,0.4);
        }
        body.light-theme .back-nav.scrolled { background: rgba(255,255,255,0.85); }
        #nav-status {
            opacity: 0; transform: translateY(-10px); transition: all 0.3s ease;
            pointer-events: none; display: flex; align-items: center; justify-content: center;
        }
        .back-nav.scrolled #nav-status { opacity: 1; transform: translateY(4px); }
        .btn-back {
            justify-self: start; display: flex; align-items: center; gap: 10px;
            color: var(--text-main); text-decoration: none; font-weight: 700;
            font-size: 0.95rem; transition: 0.3s; text-transform: uppercase; opacity: 0.7;
        }
        .btn-back:hover { opacity: 1; color: var(--accent); transform: translateX(-4px); }
        .nav-right { justify-self: end; display: flex; align-items: center; gap: 20px; }
        .user-profile {
            display: flex; align-items: center; gap: 10px; color: var(--text-muted);
            font-weight: 800; font-size: 0.75rem; letter-spacing: 1px; text-transform: uppercase; transition: 0.3s;
        }
        .user-profile:hover { color: var(--text-main); }
        .nav-icon-link { display: flex; align-items: center; color: var(--text-muted); opacity: 0.7; transition: 0.3s; }
        .nav-icon-link:hover { opacity: 1; color: var(--accent); }
        .theme-toggle {
            background: transparent; border: none; color: var(--text-main); cursor: pointer;
            opacity: 0.7; transition: 0.3s; display: flex; align-items: center; padding: 0;
        }
        .theme-toggle:hover { opacity: 1; color: var(--accent); transform: scale(1.1) rotate(15deg); }
        .theme-toggle svg { width: 18px; height: 18px; }

        /* ── BADGES ── */
        .header-badge {
            padding: 4px 12px; border-radius: 20px; font-size: 0.75rem; font-weight: 800;
            letter-spacing: 1px; text-transform: uppercase; display: inline-flex; align-items: center; gap: 6px;
        }
        .badge-online  { background: rgba(0,255,136,0.1); color: var(--safe); border: 1px solid rgba(0,255,136,0.3); }
        .badge-online::before  { content:''; width:8px; height:8px; background:var(--safe); border-radius:50%; box-shadow:0 0 10px var(--safe); animation:pulse 2s infinite; }
        .badge-offline { background: rgba(255,75,75,0.1); color: var(--crit); border: 1px solid rgba(255,75,75,0.3); }
        .badge-offline::before { content:''; width:8px; height:8px; background:var(--crit); border-radius:50%; }
        .badge-connecting { background: rgba(255,204,0,0.1); color: var(--warn); border: 1px solid rgba(255,204,0,0.3); }
        .badge-connecting::before { content:''; width:8px; height:8px; background:var(--warn); border-radius:50%; animation:pulse 1s infinite; }
        @keyframes pulse {
            0%  { transform:scale(0.95); box-shadow:0 0 0 0 rgba(0,255,136,0.7); }
            70% { transform:scale(1);    box-shadow:0 0 0 6px rgba(0,255,136,0); }
            100%{ transform:scale(0.95); box-shadow:0 0 0 0 rgba(0,255,136,0); }
        }
        @keyframes spin { from{transform:rotate(0deg)} to{transform:rotate(360deg)} }
        .spin { animation: spin 1.2s linear infinite; }

        /* ── LAYOUT ── */
        .master-wrap { width: 90%; max-width: 1100px; display: flex; flex-direction: column; gap: 35px; padding-bottom: 80px; flex-grow: 1; }
        .header-box { text-align: center; margin-top: 10px; margin-bottom: 10px; display: flex; flex-direction: column; align-items: center; gap: 8px; }
        .brand-container { display: flex; align-items: center; justify-content: center; gap: 20px; }
        .brand-container h1 {
            font-size: clamp(2.2rem,6vw,3.8rem); font-weight: 900; text-transform: uppercase;
            letter-spacing: 2px; margin: 0; color: var(--text-main);
            text-shadow: 0 0 15px rgba(255,255,255,0.15);
        }
        body.light-theme .brand-container h1 { text-shadow: none; }
        .header-subtitle { color: var(--text-muted); font-size: 0.95rem; letter-spacing: 2px; text-transform: uppercase; font-weight: 600; }

        .btn-volver-blanco {
            background: transparent; color: #ffffff; border: 1px solid #ffffff;
            padding: 8px 20px; border-radius: 8px; font-weight: 800; text-decoration: none;
            font-size: 0.85rem; text-transform: uppercase; transition: 0.3s; margin-top: 8px;
            display: inline-flex; align-items: center; gap: 8px;
        }
        .btn-volver-blanco:hover { background: #ffffff; color: #000000; box-shadow: 0 0 15px rgba(255,255,255,0.4); transform: scale(1.05); }
        body.light-theme .btn-volver-blanco { color: #111827; border-color: #111827; }
        body.light-theme .btn-volver-blanco:hover { background: #111827; color: #ffffff; }

        /* ── SECTION BLOCKS ── */
        .section-title {
            font-size: 0.85rem; color: var(--text-muted); text-transform: uppercase;
            letter-spacing: 2px; margin-bottom: 20px; border-bottom: 1px solid var(--border-color);
            padding-bottom: 12px; display: flex; align-items: center; gap: 10px;
        }
        .section-title svg { width: 16px; height: 16px; flex-shrink: 0; }

        .config-card {
            background: var(--card-bg); border: 1px solid var(--border-color);
            border-radius: 24px; padding: 30px; box-shadow: var(--card-shadow);
            transition: border-color 0.3s;
        }
        .config-card:hover { border-color: rgba(255,255,255,0.15); }
        body.light-theme .config-card:hover { border-color: rgba(0,0,0,0.15); }

        /* ── FORM ELEMENTS ── */
        .form-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
        .form-group { display: flex; flex-direction: column; gap: 8px; }
        .form-group.full { grid-column: 1 / -1; }
        .form-label {
            font-size: 0.78rem; font-weight: 800; text-transform: uppercase;
            letter-spacing: 1px; color: var(--text-muted);
        }
        .form-input {
            background: var(--term-bg); border: 1px solid var(--border-color);
            border-radius: 12px; padding: 12px 16px; color: var(--text-main);
            font-family: 'Consolas', monospace; font-size: 0.95rem;
            transition: border-color 0.3s, box-shadow 0.3s; outline: none; width: 100%; box-sizing: border-box;
        }
        .form-input:focus { border-color: var(--accent); box-shadow: 0 0 0 3px rgba(0,212,255,0.1); }
        .form-input::placeholder { color: var(--text-muted); opacity: 0.6; }
        body.light-theme .form-input { background: #f9fafb; color: #111827; }

        .input-with-eye { position: relative; }
        .input-with-eye .form-input { padding-right: 46px; }
        .eye-btn {
            position: absolute; right: 14px; top: 50%; transform: translateY(-50%);
            background: none; border: none; cursor: pointer; color: var(--text-muted);
            display: flex; align-items: center; transition: color 0.2s;
        }
        .eye-btn:hover { color: var(--text-main); }
        .eye-btn svg { width: 16px; height: 16px; }

        .form-hint { font-size: 0.75rem; color: var(--text-muted); margin-top: 2px; }

        /* ── PIN GROUPS ── */
        .pin-groups { display: flex; flex-direction: column; gap: 16px; }
        .pin-group {
            background: var(--term-bg); border: 1px solid var(--border-color);
            border-radius: 16px; padding: 16px 20px;
        }
        .pin-group-header {
            display: flex; align-items: center; gap: 8px;
            font-size: 0.72rem; font-weight: 800; text-transform: uppercase;
            letter-spacing: 0.8px; margin-bottom: 14px;
        }
        .pin-group-header svg { width: 16px; height: 16px; flex-shrink: 0; }
        .pin-group-row {
            display: flex; gap: 12px; flex-wrap: wrap;
        }
        .pin-field {
            flex: 1; min-width: 140px;
            display: flex; flex-direction: column; gap: 4px;
        }
        .pin-field-label {
            font-size: 0.68rem; font-weight: 700; text-transform: uppercase;
            letter-spacing: 0.5px; color: var(--text-muted);
            display: flex; align-items: center; gap: 6px;
        }
        .pin-field-label .cur {
            font-weight: 400; color: var(--text-muted); opacity: 0.6;
        }
        .pin-field-label .cur span { color: var(--text-main); font-family: monospace; }
        .pin-field input {
            background: var(--card-bg); border: 1px solid var(--border-color);
            border-radius: 10px; padding: 10px 14px; color: var(--text-main);
            font-family: 'Consolas', monospace; font-size: 0.9rem;
            transition: border-color 0.2s, box-shadow 0.2s; outline: none; width: 100%;
            box-sizing: border-box; -moz-appearance: textfield;
        }
        .pin-field input::-webkit-inner-spin-button,
        .pin-field input::-webkit-outer-spin-button { -webkit-appearance: none; margin: 0; }
        .pin-field input:focus { border-color: var(--accent); box-shadow: 0 0 0 2px rgba(0,212,255,0.1); }
        body.light-theme .pin-field input { background: #fff; color: #111827; }
        .pin-field input::placeholder { color: var(--text-muted); opacity: 0.4; }

        /* ── BUTTONS ── */
        .btn-primary {
            display: inline-flex; align-items: center; gap: 8px;
            background: var(--text-main); color: var(--bg-color);
            border: none; border-radius: 10px; padding: 10px 22px;
            font-weight: 800; font-size: 0.78rem; text-transform: uppercase;
            letter-spacing: 0.8px; cursor: pointer; transition: 0.3s;
        }
        .btn-primary:hover { opacity: 0.85; box-shadow: 0 0 16px rgba(255,255,255,0.15); }
        body.light-theme .btn-primary:hover { box-shadow: 0 0 16px rgba(0,0,0,0.1); }
        .btn-primary svg { width: 14px; height: 14px; flex-shrink: 0; }

        .btn-sm {
            display: inline-flex; align-items: center; gap: 6px;
            background: transparent; color: var(--text-muted);
            border: 1px solid var(--border-color); border-radius: 8px;
            padding: 7px 16px; font-weight: 700; font-size: 0.72rem;
            text-transform: uppercase; letter-spacing: 0.5px;
            cursor: pointer; transition: 0.3s; white-space: nowrap;
        }
        .btn-sm:hover { border-color: var(--text-main); color: var(--text-main); }
        .btn-sm-danger { color: var(--crit); border-color: rgba(255,75,75,0.3); }
        .btn-sm-danger:hover { background: rgba(255,75,75,0.1); border-color: var(--crit); color: var(--crit); }

        .card-footer { display: flex; justify-content: flex-end; gap: 10px; margin-top: 20px; padding-top: 16px; border-top: 1px solid var(--border-color); flex-wrap: wrap; }

        .action-row { display: flex; flex-direction: column; gap: 12px; }
        .action-item {
            display: flex; align-items: center; justify-content: space-between;
            gap: 16px; padding: 14px 18px;
            background: var(--term-bg); border: 1px solid var(--border-color);
            border-radius: 14px; transition: border-color 0.2s;
        }
        .action-item:hover { border-color: rgba(255,255,255,0.1); }
        body.light-theme .action-item:hover { border-color: rgba(0,0,0,0.1); }
        .action-item-left { flex: 1; min-width: 0; }

        /* ── TOAST ── */
        .toast-container { position: fixed; bottom: 30px; right: 30px; display: flex; flex-direction: column; gap: 10px; z-index: 9998; pointer-events: none; }
        .toast {
            display: flex; align-items: center; gap: 12px;
            padding: 14px 20px; border-radius: 14px; font-weight: 700; font-size: 0.88rem;
            backdrop-filter: blur(10px); border: 1px solid; min-width: 240px;
            transform: translateX(120%); transition: transform 0.4s cubic-bezier(0.34,1.56,0.64,1), opacity 0.3s;
            pointer-events: all; opacity: 0;
        }
        .toast.show { transform: translateX(0); opacity: 1; }
        .toast.hide { transform: translateX(120%); opacity: 0; }
        .toast-ok   { background: rgba(0,255,136,0.12); color: var(--safe); border-color: rgba(0,255,136,0.3); }
        .toast-err  { background: rgba(255,75,75,0.12);  color: var(--crit); border-color: rgba(255,75,75,0.3); }
        .toast-warn { background: rgba(255,204,0,0.12);  color: var(--warn); border-color: rgba(255,204,0,0.3); }
        .toast svg  { width: 18px; height: 18px; flex-shrink: 0; }

        /* ── MODALS ── */
        .modal-overlay {
            position: fixed; top:0; left:0; width:100%; height:100%;
            background: rgba(0,0,0,0.85); backdrop-filter: blur(10px);
            display: none; justify-content: center; align-items: center; z-index: 9999;
        }
        .modal-card {
            background: var(--card-bg); border: 2px solid var(--modal-border);
            border-radius: 30px; padding: 40px; width: 90%; max-width: 420px;
            text-align: center; box-shadow: 0 0 40px rgba(0,0,0,0.6);
        }
        .modal-card h3 { margin: 0 0 10px; font-size: 1.5rem; letter-spacing: 2px; font-weight: 900; }
        .modal-card p  { color: var(--text-muted); margin-bottom: 0; font-size: 0.95rem; }
        .modal-btns { display: flex; gap: 12px; justify-content: center; margin-top: 28px; }
        .modal-btn { padding: 12px 30px; border-radius: 14px; font-weight: 900; cursor: pointer; border: none; text-transform: uppercase; flex: 1; transition: 0.3s; font-size: 0.88rem; }
        .btn-si { background: var(--progress-fill); color: var(--bg-color); }
        .btn-no { background: var(--border-color); color: var(--text-main); }
        .btn-no:hover { filter: brightness(1.3); }

        .progress-container { width: 100%; background: var(--border-color); height: 8px; border-radius: 10px; margin-top: 20px; overflow: hidden; }
        .progress-bar { width: 0%; height: 100%; background: var(--progress-fill); box-shadow: 0 0 15px var(--progress-fill); transition: width 0.3s ease; }

        /* ── WIFI SCAN ── */
        .scan-list { display: flex; flex-direction: column; gap: 8px; margin-top: 16px; max-height: 220px; overflow-y: auto; }
        .scan-item {
            display: flex; align-items: center; justify-content: space-between;
            background: var(--term-bg); border: 1px solid var(--border-color);
            border-radius: 12px; padding: 10px 16px; cursor: pointer; transition: 0.2s;
        }
        .scan-item:hover { border-color: var(--accent); background: rgba(0,212,255,0.05); }
        .scan-ssid { font-weight: 700; font-size: 0.9rem; font-family: monospace; }
        .scan-meta { display: flex; align-items: center; gap: 10px; color: var(--text-muted); font-size: 0.78rem; }
        .signal-bar { display: flex; align-items: flex-end; gap: 2px; height: 14px; }
        .signal-bar span { display: block; width: 3px; background: var(--border-color); border-radius: 2px; }
        .signal-bar span.lit { background: var(--safe); }

        .btn-scan {
            display: inline-flex; align-items: center; gap: 6px;
            background: transparent; color: var(--accent);
            border: 1px solid var(--accent); border-radius: 8px; padding: 6px 14px;
            font-weight: 700; font-size: 0.72rem; text-transform: uppercase; cursor: pointer; transition: 0.3s;
        }
        .btn-scan:hover { background: rgba(0,212,255,0.1); }
        .btn-scan.scanning svg { animation: spin 1s linear infinite; }
        .btn-scan svg { width: 14px; height: 14px; }

        /* ── SYSTEM INFO GRID ── */
        .sys-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 15px; }
        .sys-item {
            background: var(--term-bg); border: 1px solid var(--border-color);
            border-radius: 14px; padding: 16px 20px; display: flex; flex-direction: column; gap: 6px;
        }
        .sys-key { font-size: 0.72rem; font-weight: 800; text-transform: uppercase; letter-spacing: 1px; color: var(--text-muted); }
        .sys-val { font-size: 1rem; font-weight: 900; font-family: monospace; color: var(--text-main); }

        /* ── LOGOUT ── */
        .btn-accion-logout {
            background: transparent; padding: 8px 18px; border-radius: 10px; font-weight: 900;
            font-size: 0.7rem; text-transform: uppercase; transition: 0.3s; display: inline-flex;
            align-items: center; gap: 8px; cursor: pointer; border: 1px solid var(--crit);
            color: var(--crit); text-decoration: none; letter-spacing: 1px;
        }
        .btn-accion-logout:hover { background: var(--crit); color: #fff; box-shadow: 0 0 15px var(--crit); transform: scale(1.05); }
        .btn-accion-logout svg { width: 14px; height: 14px; }

        .footer-zenith {
            width: 100%; background: var(--card-bg); padding: 40px 0;
            border-top: 1px solid var(--border-color); text-align: center; transition: 0.4s;
        }
        .footer-zenith p { color: var(--text-muted); font-size: 0.95rem; margin: 6px 0; }
        .footer-zenith a { color: var(--text-main); text-decoration: none; font-weight: 700; }

        @media (max-width: 900px) {
            .form-grid { grid-template-columns: 1fr; }
            .form-group.full { grid-column: 1; }
            .pin-grid { grid-template-columns: repeat(3, 1fr); }
            .sys-grid { grid-template-columns: repeat(2, 1fr); }
        }
        @media (max-width: 600px) {
            body { zoom: 1; }
            .back-nav { padding: 15px 5%; grid-template-columns: 1fr 1fr; }
            #nav-status { display: none; }
            .brand-container { flex-direction: column; gap: 10px; }
            .pin-grid { grid-template-columns: repeat(2, 1fr); }
            .sys-grid { grid-template-columns: 1fr 1fr; }
            .card-footer { flex-direction: column; }
            .btn-save, .btn-danger, .btn-warn { width: 100%; justify-content: center; }
            .toast-container { bottom: 15px; right: 15px; left: 15px; }
            .toast { min-width: unset; }
        }
    </style>
</head>
<body>

<!-- ═══════════════════════ MODALS ═══════════════════════ -->
<div class="modal-overlay" id="modalReboot">
    <div class="modal-card">
        <i data-lucide="refresh-cw" class="spin" style="width:50px;height:50px;color:var(--modal-icon);margin-bottom:15px;display:block;margin-left:auto;margin-right:auto;"></i>
        <h3>REINICIANDO</h3>
        <p id="rebootText">Aplicando configuración y reiniciando la placa...</p>
        <div class="progress-container"><div class="progress-bar" id="rebootBar"></div></div>
    </div>
</div>

<div class="modal-overlay" id="modalConfirmReset">
    <div class="modal-card">
        <i data-lucide="alert-triangle" style="width:50px;height:50px;color:var(--crit);margin-bottom:15px;display:block;margin-left:auto;margin-right:auto;"></i>
        <h3>¿BORRAR WIFI?</h3>
        <p>Se eliminarán las credenciales WiFi guardadas y la placa entrará en modo punto de acceso (AP). Necesitarás reconectar.</p>
        <div class="modal-btns">
            <button class="modal-btn btn-si" style="background:var(--crit);color:#fff;" onclick="ejecutarResetWifi()">SÍ, BORRAR</button>
            <button class="modal-btn btn-no" onclick="cerrarModal('modalConfirmReset')">CANCELAR</button>
        </div>
    </div>
</div>

<div class="modal-overlay" id="modalConfirmReboot">
    <div class="modal-card">
        <i data-lucide="power" style="width:50px;height:50px;color:var(--modal-icon);margin-bottom:15px;display:block;margin-left:auto;margin-right:auto;"></i>
        <h3>¿REINICIAR?</h3>
        <p>Se perderá la conexión activa temporalmente durante el reinicio.</p>
        <div class="modal-btns">
            <button class="modal-btn btn-si" onclick="ejecutarReboot()">SÍ, REINICIAR</button>
            <button class="modal-btn btn-no" onclick="cerrarModal('modalConfirmReboot')">CANCELAR</button>
        </div>
    </div>
</div>

<!-- ═══════════════════════ NAV ═══════════════════════ -->
<nav class="back-nav" id="main-nav">
    <a href="/" class="btn-back">
        <i data-lucide="arrow-left" style="width:18px;height:18px;"></i> Panel
    </a>
    <div id="nav-status">
        <div class="header-badge badge-connecting" id="status-badge-nav">CONECTANDO...</div>
    </div>
    <div class="nav-right">
        <div class="user-profile">
            <i data-lucide="circle-user" style="width:16px;height:16px;"></i>
            <span id="nav-username">admin</span>
        </div>
        <button class="theme-toggle" onclick="toggleTheme()" title="Cambiar tema">
            <i data-lucide="sun"></i>
        </button>
    </div>
</nav>

<!-- ═══════════════════════ CONTENT ═══════════════════════ -->
<div class="master-wrap">

    <div class="header-box">
        <div class="brand-container">
            <i data-lucide="settings-2" style="width:45px;height:45px;color:var(--text-main);"></i>
            <h1>CONFIGURACIÓN</h1>
        </div>
        <p class="header-subtitle">Panel de Control del Sistema</p>
    </div>

    <!-- ─────────── BLOQUE 1: INFORMACIÓN DEL SISTEMA ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="monitor" style="color:var(--accent);"></i>
            Estado del Sistema
        </div>
        <div class="config-card">
            <div class="sys-grid" id="sysGrid">
                <div class="sys-item"><div class="sys-key">IP Local</div><div class="sys-val" id="si-ip">—</div></div>
                <div class="sys-item"><div class="sys-key">WiFi SSID</div><div class="sys-val" id="si-ssid">—</div></div>
                <div class="sys-item"><div class="sys-key">Señal WiFi</div><div class="sys-val" id="si-rssi">—</div></div>
                <div class="sys-item"><div class="sys-key">Temperatura CPU</div><div class="sys-val" id="si-temp">—</div></div>
                <div class="sys-item"><div class="sys-key">RAM Libre</div><div class="sys-val" id="si-ram">—</div></div>
                <div class="sys-item"><div class="sys-key">Uptime</div><div class="sys-val" id="si-uptime">—</div></div>
            </div>
        </div>
    </section>

    <!-- ─────────── BLOQUE 2: RED WiFi ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="wifi" style="color:var(--accent);"></i>
            Credenciales WiFi
        </div>
        <div class="config-card">
            <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:16px;flex-wrap:wrap;gap:10px;">
                <p style="margin:0;color:var(--text-muted);font-size:0.88rem;">Modifica la red a la que conecta el ESP32. El cambio requiere reinicio.</p>
                <button class="btn-scan" id="btnScan" onclick="escanearRedes()">
                    <i data-lucide="scan-search"></i> ESCANEAR
                </button>
            </div>

            <div class="scan-list" id="scanList" style="display:none;"></div>

            <div class="form-grid" style="margin-top:16px;">
                <div class="form-group">
                    <label class="form-label">SSID (Nombre de red)</label>
                    <input type="text" class="form-input" id="wifiSSID" placeholder="NombreDeTuRed" autocomplete="off">
                </div>
                <div class="form-group">
                    <label class="form-label">Contraseña WiFi</label>
                    <div class="input-with-eye">
                        <input type="password" class="form-input" id="wifiPass" placeholder="••••••••" autocomplete="new-password">
                        <button class="eye-btn" onclick="togglePass('wifiPass',this)" tabindex="-1">
                            <i data-lucide="eye" style="width:16px;height:16px;"></i>
                        </button>
                    </div>
                </div>
            </div>

            <div class="card-footer">
                <button class="btn-primary" onclick="guardarWifi()">
                    <i data-lucide="save"></i> GUARDAR
                </button>
            </div>
        </div>
    </section>

    <!-- ─────────── BLOQUE 3: CREDENCIALES WEB ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="shield" style="color:var(--safe);"></i>
            Credenciales de Acceso Web
        </div>
        <div class="config-card">
            <p style="color:var(--text-muted);font-size:0.88rem;margin:0 0 20px;">
                Usuario y contraseña para el panel web y la sesión Telnet. Los cambios se aplican tras reinicio.
            </p>
            <div class="form-grid">
                <div class="form-group">
                    <label class="form-label">Usuario</label>
                    <input type="text" class="form-input" id="webUser" placeholder="admin" autocomplete="off">
                </div>
                <div class="form-group">
                    <label class="form-label">Contraseña actual</label>
                    <div class="input-with-eye">
                        <input type="password" class="form-input" id="webPassActual" placeholder="Contraseña actual" autocomplete="current-password">
                        <button class="eye-btn" onclick="togglePass('webPassActual',this)" tabindex="-1">
                            <i data-lucide="eye" style="width:16px;height:16px;"></i>
                        </button>
                    </div>
                </div>
                <div class="form-group">
                    <label class="form-label">Nueva contraseña</label>
                    <div class="input-with-eye">
                        <input type="password" class="form-input" id="webPassNueva" placeholder="Nueva contraseña" autocomplete="new-password">
                        <button class="eye-btn" onclick="togglePass('webPassNueva',this)" tabindex="-1">
                            <i data-lucide="eye" style="width:16px;height:16px;"></i>
                        </button>
                    </div>
                </div>
                <div class="form-group">
                    <label class="form-label">Confirmar contraseña</label>
                    <div class="input-with-eye">
                        <input type="password" class="form-input" id="webPassConfirm" placeholder="Repetir nueva contraseña" autocomplete="new-password">
                        <button class="eye-btn" onclick="togglePass('webPassConfirm',this)" tabindex="-1">
                            <i data-lucide="eye" style="width:16px;height:16px;"></i>
                        </button>
                    </div>
                </div>
            </div>
            <div class="card-footer">
                <button class="btn-primary" onclick="guardarCredencialesWeb()">
                    <i data-lucide="save"></i> GUARDAR
                </button>
            </div>
        </div>
    </section>

    <!-- ─────────── BLOQUE 4: PINES DE HARDWARE ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="cpu" style="color:var(--warn);"></i>
            Mapeo de Pines GPIO
        </div>
        <div class="config-card">
            <p style="color:var(--text-muted);font-size:0.88rem;margin:0 0 16px;">
                Asigna los pines físicos del ESP32-S3 a cada periférico. Los valores se guardan en NVS y se aplican tras reinicio.
            </p>

            <div class="pin-groups">

                <div class="pin-group">
                    <div class="pin-group-header" style="color:var(--accent);">
                        <i data-lucide="credit-card"></i> NFC / MFRC522
                    </div>
                    <div class="pin-group-row">
                        <div class="pin-field">
                            <div class="pin-field-label">RST <span class="cur">(actual: <span id="cur-nfcRst">—</span>)</span></div>
                            <input type="number" id="pin-nfcRst" placeholder="21" min="0" max="48">
                        </div>
                        <div class="pin-field">
                            <div class="pin-field-label">CS (SS) <span class="cur">(actual: <span id="cur-nfcSs">—</span>)</span></div>
                            <input type="number" id="pin-nfcSs" placeholder="5" min="0" max="48">
                        </div>
                    </div>
                </div>

                <div class="pin-group">
                    <div class="pin-group-header" style="color:var(--warn);">
                        <i data-lucide="waves"></i> Ultrasonidos HC-SR04
                    </div>
                    <div class="pin-group-row">
                        <div class="pin-field">
                            <div class="pin-field-label">TRIG <span class="cur">(actual: <span id="cur-trigPin">—</span>)</span></div>
                            <input type="number" id="pin-trigPin" placeholder="15" min="0" max="48">
                        </div>
                        <div class="pin-field">
                            <div class="pin-field-label">ECHO <span class="cur">(actual: <span id="cur-echoPin">—</span>)</span></div>
                            <input type="number" id="pin-echoPin" placeholder="16" min="0" max="48">
                        </div>
                    </div>
                </div>

                <div class="pin-group">
                    <div class="pin-group-header" style="color:var(--safe);">
                        <i data-lucide="thermometer"></i> DHT11
                    </div>
                    <div class="pin-group-row">
                        <div class="pin-field">
                            <div class="pin-field-label">DATA <span class="cur">(actual: <span id="cur-dhtPin">—</span>)</span></div>
                            <input type="number" id="pin-dhtPin" placeholder="4" min="0" max="48">
                        </div>
                    </div>
                </div>

            </div>

            <div class="card-footer">
                <button class="btn-primary" onclick="guardarPines()">
                    <i data-lucide="save"></i> GUARDAR
                </button>
            </div>
        </div>
    </section>

    <!-- ─────────── BLOQUE 5: SISTEMA ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="terminal" style="color:var(--crit);"></i>
            Acciones del Sistema
        </div>
        <div class="config-card">
            <div class="action-row">
                <div class="action-item">
                    <div class="action-item-left">
                        <div class="form-label" style="margin:0;">Reinicio suave</div>
                        <p style="color:var(--text-muted);font-size:0.82rem;margin:4px 0 0;">Reinicia el firmware manteniendo las configuraciones en NVS.</p>
                    </div>
                    <button class="btn-sm" onclick="abrirModal('modalConfirmReboot')">
                        <i data-lucide="refresh-cw" style="width:14px;height:14px;"></i> REINICIAR
                    </button>
                </div>
                <div class="action-item">
                    <div class="action-item-left">
                        <div class="form-label" style="margin:0;">Reset de fábrica WiFi</div>
                        <p style="color:var(--text-muted);font-size:0.82rem;margin:4px 0 0;">Borra las credenciales WiFi de la NVS. La placa arrancará en modo AP.</p>
                    </div>
                    <button class="btn-sm btn-sm-danger" onclick="abrirModal('modalConfirmReset')">
                        <i data-lucide="wifi-off" style="width:14px;height:14px;"></i> BORRAR RED
                    </button>
                </div>
            </div>
        </div>
    </section>

</div><!-- /master-wrap -->

<!-- ═══════════════════════ FOOTER ═══════════════════════ -->
<footer class="footer-zenith">
    <p>© 2026 ZenithMC Network. Todos los derechos reservados.</p>
    <p><a href="mailto:soporte@zenithmc.es">soporte@zenithmc.es</a></p>
    <div style="margin-top:20px;">
        <a href="/logout" class="btn-accion-logout">
            <i data-lucide="log-out"></i> CERRAR SESIÓN
        </a>
    </div>
</footer>

<!-- ═══════════════════════ TOASTS ═══════════════════════ -->
<div class="toast-container" id="toastContainer"></div>

<script>
// ── AUTH CHECK ──────────────────────────────────────────────────
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

// ── SCROLL NAV ──────────────────────────────────────────────────
window.addEventListener('scroll', function() {
    const nav = document.getElementById('main-nav');
    if (window.scrollY > 50) nav.classList.add('scrolled');
    else nav.classList.remove('scrolled');
});

// ── THEME ───────────────────────────────────────────────────────
function toggleTheme() {
    document.body.classList.toggle('light-theme');
    const isLight = document.body.classList.contains('light-theme');
    const btn = document.querySelector('.theme-toggle');
    btn.innerHTML = isLight ? '<i data-lucide="moon"></i>' : '<i data-lucide="sun"></i>';
    lucide.createIcons();
    localStorage.setItem('zenith-theme', isLight ? 'light' : 'dark');
}
(function(){
    if (localStorage.getItem('zenith-theme') === 'light') {
        document.body.classList.add('light-theme');
        const btn = document.querySelector('.theme-toggle');
        if (btn) { btn.innerHTML = '<i data-lucide="moon"></i>'; lucide.createIcons(); }
    }
})();

// ── TOAST ───────────────────────────────────────────────────────
function toast(msg, tipo = 'ok') {
    const iconMap = { ok: 'check-circle', err: 'x-circle', warn: 'alert-triangle' };
    const tc = document.getElementById('toastContainer');
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
    }, 3500);
}

// ── MODALS ──────────────────────────────────────────────────────
function abrirModal(id) { document.getElementById(id).style.display = 'flex'; }
function cerrarModal(id) { document.getElementById(id).style.display = 'none'; }

// ── PASSWORD TOGGLE ─────────────────────────────────────────────
function togglePass(inputId, btn) {
    const inp = document.getElementById(inputId);
    const show = inp.type === 'password';
    inp.type = show ? 'text' : 'password';
    btn.innerHTML = show
        ? '<i data-lucide="eye-off" style="width:16px;height:16px;"></i>'
        : '<i data-lucide="eye" style="width:16px;height:16px;"></i>';
    lucide.createIcons();
}

// ── STATUS BADGE ────────────────────────────────────────────────
function setStatusBadge(s) {
    const b = document.getElementById('status-badge-nav');
    if (s === 'online')     { b.className = 'header-badge badge-online';     b.textContent = 'ONLINE'; }
    else if (s === 'offline'){ b.className = 'header-badge badge-offline';   b.textContent = 'OFFLINE'; }
    else                     { b.className = 'header-badge badge-connecting'; b.textContent = 'RECONECTANDO...'; }
}

// ── WEBSOCKET (telemetría básica para el bloque de sistema) ─────
let ws;
function initWebSocket() {
    setStatusBadge('connecting');
    ws = new WebSocket(`ws://${location.hostname}/ws`);
    ws.onopen = () => setStatusBadge('online');
    ws.onclose = () => { setStatusBadge('offline'); setTimeout(initWebSocket, 4000); };
    ws.onmessage = (e) => {
        try {
            const d = JSON.parse(e.data);
            if (d.type === 'telemetry') {
                document.getElementById('si-temp').textContent   = d.temp   !== undefined ? d.temp.toFixed(1) + ' °C' : '—';
                document.getElementById('si-uptime').textContent = d.uptime || '—';
                const ramPct = d.ram !== undefined ? d.ram.toFixed(1) + ' %' : '—';
                document.getElementById('si-ram').textContent = ramPct;
            }
        } catch(_){}
    };
}

// ── FETCH SISTEMA INFO ──────────────────────────────────────────
function cargarInfoSistema() {
    fetch('/api/config/info')
        .then(r => r.json())
        .then(d => {
            document.getElementById('si-ip').textContent     = d.ip     || location.hostname;
            document.getElementById('si-ssid').textContent   = d.ssid   || '—';
            document.getElementById('si-rssi').textContent   = d.rssi   !== undefined ? d.rssi + ' dBm' : '—';
            document.getElementById('nav-username').textContent = d.user || 'admin';

            // Pines actuales
            if (d.pins) {
                document.getElementById('cur-nfcRst').textContent  = d.pins.nfcRst  ?? '—';
                document.getElementById('cur-nfcSs').textContent   = d.pins.nfcSs   ?? '—';
                document.getElementById('cur-trigPin').textContent = d.pins.trigPin ?? '—';
                document.getElementById('cur-echoPin').textContent = d.pins.echoPin ?? '—';
                document.getElementById('cur-dhtPin').textContent  = d.pins.dhtPin  ?? '—';
            }
        })
        .catch(() => {
            // Si no existe el endpoint todavía, mostramos la IP del hostname
            document.getElementById('si-ip').textContent = location.hostname;
        });
}

// ── ESCANEAR REDES ──────────────────────────────────────────────
function escanearRedes() {
    const btn = document.getElementById('btnScan');
    const list = document.getElementById('scanList');
    btn.classList.add('scanning');
    btn.innerHTML = '<i data-lucide="loader"></i> ESCANEANDO...';
    lucide.createIcons();
    list.style.display = 'flex';
    list.innerHTML = '<div style="color:var(--text-muted);font-size:0.85rem;padding:10px;">Buscando redes...</div>';

    fetch('/api/wifi/scan')
        .then(r => r.json())
        .then(redes => {
            list.innerHTML = '';
            if (!redes.length) {
                list.innerHTML = '<div style="color:var(--text-muted);font-size:0.85rem;padding:10px;">No se encontraron redes.</div>';
                return;
            }
            redes.sort((a, b) => b.rssi - a.rssi);
            redes.forEach(red => {
                const bars = rssiToBars(red.rssi);
                const div = document.createElement('div');
                div.className = 'scan-item';
                div.innerHTML = `
                    <span class="scan-ssid">${escapeHtml(red.ssid)}</span>
                    <div class="scan-meta">
                        <span>${red.rssi} dBm</span>
                        ${red.encrypted ? '<i data-lucide="lock" style="width:12px;height:12px;"></i>' : ''}
                        <div class="signal-bar">
                            <span style="height:4px;"  class="${bars>=1?'lit':''}"></span>
                            <span style="height:7px;"  class="${bars>=2?'lit':''}"></span>
                            <span style="height:10px;" class="${bars>=3?'lit':''}"></span>
                            <span style="height:14px;" class="${bars>=4?'lit':''}"></span>
                        </div>
                    </div>`;
                div.addEventListener('click', () => {
                    document.getElementById('wifiSSID').value = red.ssid;
                    document.getElementById('wifiPass').focus();
                    list.style.display = 'none';
                });
                list.appendChild(div);
            });
            lucide.createIcons();
        })
        .catch(() => {
            list.innerHTML = '<div style="color:var(--crit);font-size:0.85rem;padding:10px;">Error al escanear. Comprueba la conexión.</div>';
        })
        .finally(() => {
            btn.classList.remove('scanning');
            btn.innerHTML = '<i data-lucide="scan-search"></i> ESCANEAR';
            lucide.createIcons();
        });
}

function rssiToBars(rssi) {
    if (rssi >= -50) return 4;
    if (rssi >= -65) return 3;
    if (rssi >= -75) return 2;
    return 1;
}
function escapeHtml(t) {
    return t.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');
}

// ── GUARDAR WIFI ────────────────────────────────────────────────
function guardarWifi() {
    const ssid = document.getElementById('wifiSSID').value.trim();
    const pass = document.getElementById('wifiPass').value;
    if (!ssid) { toast('El SSID no puede estar vacío.', 'err'); return; }

    fetch('/api/wifi/configure', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid, password: pass })
    })
    .then(r => r.json())
    .then(d => {
        if (d.status === 'ok') {
            toast('Credenciales guardadas. Reiniciando...', 'ok');
            setTimeout(() => mostrarProgreso('Conectando a ' + ssid + '...', 12000, '/'), 1000);
        } else {
            toast('Error: ' + (d.message || 'desconocido'), 'err');
        }
    })
    .catch(() => toast('No se pudo conectar con el dispositivo.', 'err'));
}

// ── GUARDAR CREDENCIALES WEB ────────────────────────────────────
function guardarCredencialesWeb() {
    const user   = document.getElementById('webUser').value.trim();
    const actual = document.getElementById('webPassActual').value;
    const nueva  = document.getElementById('webPassNueva').value;
    const conf   = document.getElementById('webPassConfirm').value;

    if (!user)   { toast('El usuario no puede estar vacío.', 'err'); return; }
    if (!actual) { toast('Introduce la contraseña actual.', 'err'); return; }
    if (nueva && nueva !== conf) { toast('Las contraseñas nuevas no coinciden.', 'err'); return; }
    if (nueva && nueva.length < 4) { toast('La contraseña debe tener al menos 4 caracteres.', 'err'); return; }

    const payload = { user, currentPass: actual, newPass: nueva || actual };

    fetch('/api/config/webcred', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    })
    .then(r => r.json())
    .then(d => {
        if (d.status === 'ok') {
            toast('Credenciales web actualizadas. Reinicia para aplicar.', 'ok');
            document.getElementById('webPassActual').value = '';
            document.getElementById('webPassNueva').value  = '';
            document.getElementById('webPassConfirm').value = '';
        } else {
            toast('Error: ' + (d.message || 'Contraseña actual incorrecta'), 'err');
        }
    })
    .catch(() => toast('No se pudo conectar con el dispositivo.', 'err'));
}

// ── GUARDAR PINES ───────────────────────────────────────────────
function guardarPines() {
    const vals = {
        nfcRst:  parseInt(document.getElementById('pin-nfcRst').value),
        nfcSs:   parseInt(document.getElementById('pin-nfcSs').value),
        trigPin: parseInt(document.getElementById('pin-trigPin').value),
        echoPin: parseInt(document.getElementById('pin-echoPin').value),
        dhtPin:  parseInt(document.getElementById('pin-dhtPin').value)
    };

    const filled = Object.values(vals).filter(v => !isNaN(v));
    if (!filled.length) { toast('Introduce al menos un pin para guardar.', 'warn'); return; }

    const payload = {};
    Object.entries(vals).forEach(([k, v]) => { if (!isNaN(v)) payload[k] = v; });

    fetch('/api/config/pins', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    })
    .then(r => r.json())
    .then(d => {
        if (d.status === 'ok') {
            toast('Pines guardados en NVS. Reinicia para aplicar.', 'ok');
            cargarInfoSistema();
        } else {
            toast('Error al guardar: ' + (d.message || ''), 'err');
        }
    })
    .catch(() => toast('No se pudo conectar con el dispositivo.', 'err'));
}

// ── REBOOT ──────────────────────────────────────────────────────
function ejecutarReboot() {
    cerrarModal('modalConfirmReboot');
    mostrarProgreso('Reiniciando el sistema...', 8000, '/');
    fetch('/api/wifi/reboot', { method: 'POST' }).catch(()=>{});
}

// ── RESET WIFI ──────────────────────────────────────────────────
function ejecutarResetWifi() {
    cerrarModal('modalConfirmReset');
    mostrarProgreso('Borrando credenciales y reiniciando en modo AP...', 6000, null);
    fetch('/api/wifi/reset', { method: 'POST' }).catch(()=>{});
}

// ── PROGRESO MODAL ──────────────────────────────────────────────
function mostrarProgreso(msg, dur, redirect) {
    const modal = document.getElementById('modalReboot');
    document.getElementById('rebootText').textContent = msg;
    modal.style.display = 'flex';
    const bar = document.getElementById('rebootBar');
    bar.style.width = '0%';
    const start = Date.now();
    const tick = setInterval(() => {
        const pct = Math.min(((Date.now() - start) / dur) * 100, 98);
        bar.style.width = pct + '%';
        if (Date.now() - start >= dur) {
            clearInterval(tick);
            bar.style.width = '100%';
            if (redirect) setTimeout(() => { window.location.href = redirect; }, 500);
            else setTimeout(() => { modal.style.display = 'none'; }, 1000);
        }
    }, 80);
}

// ── INIT ────────────────────────────────────────────────────────
window.addEventListener('load', () => {
    cargarInfoSistema();
    initWebSocket();
});
</script>
</body>
</html>
)rawliteral";