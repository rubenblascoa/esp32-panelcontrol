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
                <span>admin</span>
            </div>
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
            <div class="section-title">Consola de Control Directa (192.168.18.203)</div>
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

window.addEventListener('load', async () => {
    cargarHistorialGrafica();
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
                <span>admin</span>
            </div>
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

window.addEventListener('load', () => {
    cargarBaseDatos();
    initWebSocket();
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
