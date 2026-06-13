// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod
//
// This file is auto-extracted from web_pages.cpp.
#pragma once

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ZenithMC | Recursos ESP32</title>
    <link rel="icon" type="image/png" href="https://cdn-icons-png.flaticon.com/512/8463/8463850.png">
    <script src="https://cdn.jsdelivr.net/npm/chart.js" defer></script>
    <script src="https://unpkg.com/lucide@latest" defer></script>
    
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

        /* =========================================
           BLASCO AI — COPILOT/VSCODE PANEL
           ========================================= */
        .btn-ai-toggle {
            background: transparent; border: none; cursor: pointer;
            display: flex; align-items: center; justify-content: center;
            color: var(--text-main); opacity: 0.7; transition: 0.3s;
            position: relative;
        }
        .btn-ai-toggle svg { width: 18px; height: 18px; }
        .btn-ai-toggle:hover { opacity: 1; color: var(--accent); transform: scale(1.1); }
        .btn-ai-toggle.active { opacity: 1; color: var(--accent); }
        .btn-ai-toggle .ai-dot {
            position: absolute; top: -2px; right: -2px;
            width: 7px; height: 7px; border-radius: 50%;
            background: var(--accent); box-shadow: 0 0 6px var(--accent);
            display: none;
        }
        .btn-ai-toggle.active .ai-dot { display: block; }

        #ai-backdrop {
            display: none; position: fixed; inset: 0;
            background: rgba(0,0,0,0.5);
            z-index: 1100;
        }
        #ai-backdrop.visible { display: block; }

        /* Panel principal */
        #ai-panel {
            position: fixed; top: 0; right: -460px; width: 440px; bottom: 0;
            background: var(--card-bg); border-left: 1px solid var(--border-color);
            z-index: 1200; display: flex; flex-direction: column;
            transition: right 0.3s cubic-bezier(0.4,0,0.2,1);
            font-family: 'Inter', system-ui, sans-serif;
        }
        body.light-theme #ai-panel { background: var(--card-bg); border-left-color: var(--border-color); }
        #ai-panel.open { right: 0; }

        /* Header */
        .ai-panel-header {
            display: flex; align-items: center; gap: 8px;
            padding: 0 12px; border-bottom: 1px solid var(--border-color);
            flex-shrink: 0; background: var(--card-bg);
            height: 35px; min-height: 35px;
        }
        .ai-panel-header .ai-title {
            font-size: 0.7rem; text-transform: uppercase; letter-spacing: 0.08em;
            color: var(--text-muted); font-weight: 400; flex: 1;
        }
        .ai-panel-header .ai-model-tag {
            font-size: 0.62rem; color: var(--text-muted); font-weight: 400;
            background: transparent; padding: 1px 6px; border-radius: 3px;
            border: 1px solid var(--border-color); font-family: 'Consolas', monospace;
        }
        .ai-panel-header .ai-header-actions {
            display: flex; gap: 2px; align-items: center;
        }
        .ai-panel-header button {
            background: none; border: none; cursor: pointer;
            color: var(--text-muted); display: flex; align-items: center;
            padding: 4px 5px; border-radius: 4px; transition: 0.1s;
        }
        .ai-panel-header button:hover { color: var(--text-main); background: rgba(255,255,255,0.06); }
        body.light-theme .ai-panel-header button:hover { color: var(--text-main); background: rgba(0,0,0,0.04); }
        .ai-panel-header button svg { width: 14px; height: 14px; }

        /* Welcome screen */
        #ai-welcome {
            flex: 1; display: flex; flex-direction: column;
            align-items: center; justify-content: center;
            gap: 0; padding: 32px 24px;
            min-height: 0;
        }
        #ai-welcome.hidden { display: none; }
        .ai-welcome-icon {
            width: 52px; height: 52px; margin-bottom: 16px;
            display: flex; align-items: center; justify-content: center;
            color: var(--text-muted);
        }
        .ai-welcome-icon svg { width: 52px; height: 52px; }
        .ai-welcome-title {
            font-size: 1.05rem; font-weight: 500; color: var(--text-main);
            margin-bottom: 8px; text-align: center;
        }
        .ai-welcome-sub {
            font-size: 0.75rem; color: var(--text-muted); text-align: center;
            margin-bottom: 28px; line-height: 1.5;
        }
        .ai-welcome-actions {
            display: flex; gap: 8px; flex-wrap: wrap; justify-content: center;
            margin-bottom: 12px;
        }
        .ai-welcome-btn {
            background: var(--bg-color); border: 1px solid var(--border-color); color: var(--text-main);
            padding: 6px 14px; border-radius: 8px; font-size: 0.73rem;
            cursor: pointer; display: flex; align-items: center; gap: 6px;
            transition: 0.15s; font-family: inherit;
        }
        .ai-welcome-btn:hover { border-color: var(--accent); color: var(--accent); }
        .ai-welcome-btn svg { width: 13px; height: 13px; }
        .ai-welcome-note {
            font-size: 0.65rem; color: var(--text-muted); text-align: center; margin-top: 16px; opacity: 0.6;
        }

        /* Chat area */
        #ai-chat-box {
            flex: 1; overflow-y: auto; padding: 0;
            display: flex; flex-direction: column;
            min-height: 0;
        }
        .ai-chat-box.hidden { display: none; }
        .ai-chat-box::-webkit-scrollbar { width: 6px; }
        .ai-chat-box::-webkit-scrollbar-track { background: transparent; }
        .ai-chat-box::-webkit-scrollbar-thumb { background: var(--border-color); border-radius: 3px; }

        .ai-msg {
            padding: 14px 20px; border-bottom: 1px solid transparent;
            display: flex; flex-direction: column; gap: 6px;
            flex-shrink: 0; margin-bottom: 4px;
        }
        .ai-msg:hover { background: rgba(255,255,255,0.02); }
        body.light-theme .ai-msg:hover { background: rgba(0,0,0,0.02); }
        .ai-msg-header {
            display: flex; align-items: center; gap: 8px;
        }
        .ai-avatar {
            width: 20px; height: 20px; border-radius: 50%; flex-shrink: 0;
            display: flex; align-items: center; justify-content: center;
            font-size: 0.6rem; font-weight: 700;
        }
        .ai-avatar.bot { background: var(--accent); color: #000; }
        .ai-avatar.user { background: var(--border-color); color: var(--text-muted); }
        .ai-avatar svg { width: 11px; height: 11px; }
        .ai-msg-name {
            font-size: 0.72rem; font-weight: 600; color: var(--text-muted);
        }
        .ai-msg-name.bot { color: var(--accent); }

        .ai-copy-btn {
            margin-left: auto; background: none; border: none; cursor: pointer;
            color: var(--text-muted); opacity: 0; padding: 2px; border-radius: 4px;
            display: flex; align-items: center; transition: opacity 0.15s;
        }
        .ai-msg.bot:hover .ai-copy-btn { opacity: 0.6; }
        .ai-copy-btn:hover { opacity: 1 !important; color: var(--accent); background: rgba(255,255,255,0.06); }

        .ai-bubble {
            font-size: 0.82rem; line-height: 1.6; color: var(--text-main);
            padding-left: 28px;
        }
        .ai-bubble pre {
            background: var(--bg-color); padding: 10px 12px; border-radius: 6px;
            font-size: 0.77rem; overflow-x: auto; margin: 8px 0;
            font-family: 'Consolas', monospace; border: 1px solid var(--border-color);
        }
        .ai-bubble code {
            background: rgba(255,255,255,0.06); padding: 1px 5px; border-radius: 3px;
            font-size: 0.78rem; font-family: 'Consolas', monospace; color: var(--accent);
        }
        body.light-theme .ai-bubble code { background: rgba(0,0,0,0.05); color: var(--accent); }
        .ai-msg.user .ai-bubble { color: var(--text-muted); }

        @keyframes aiDotPulse { 0%,60%{opacity:0.2} 30%{opacity:1} }
        .ai-thinking span { animation: aiDotPulse 1.2s infinite; display: inline-flex; gap: 3px; }
        .ai-thinking span em { font-style: normal; animation: aiDotPulse 1.2s infinite; font-size: 1.1rem; }
        .ai-thinking span em:nth-child(2) { animation-delay: 0.2s; }
        .ai-thinking span em:nth-child(3) { animation-delay: 0.4s; }

        /* Telemetry bar */
        .ai-telem-bar {
            padding: 3px 12px; font-size: 0.65rem; color: #000;
            background: var(--accent);
            display: flex; gap: 14px; flex-wrap: wrap; align-items: center;
            flex-shrink: 0; font-family: 'Consolas', monospace;
            height: 22px; min-height: 22px;
        }
        .ai-telem-bar .telem-label {
            font-size: 0.6rem; text-transform: uppercase; letter-spacing: 0.08em;
            color: rgba(0,0,0,0.65); font-weight: 700;
        }
        .ai-telem-item { display: flex; align-items: center; gap: 4px; }
        .ai-telem-item svg { width: 10px; height: 10px; opacity: 0.8; }

        /* Input area */
        .ai-input-area {
            padding: 10px 12px 12px; border-top: 1px solid var(--border-color);
            background: var(--card-bg); flex-shrink: 0;
        }
        .ai-input-box {
            background: var(--bg-color); border: 1px solid var(--border-color);
            border-radius: 8px; padding: 8px 8px 8px 14px;
            display: flex; align-items: flex-end; gap: 6px;
            transition: border-color 0.15s;
        }
        .ai-input-box:focus-within { border-color: var(--accent); }
        .ai-input-box textarea {
            flex: 1; background: transparent; border: none; outline: none;
            color: var(--text-main); font-family: 'Inter', system-ui, sans-serif;
            font-size: 0.82rem; resize: none; min-height: 34px; max-height: 120px;
            padding: 4px 0; line-height: 1.5;
        }
        .ai-input-box textarea::placeholder { color: var(--text-muted); opacity: 0.6; }
        .ai-input-tools { display: flex; align-items: center; gap: 2px; }
        .ai-tool-btn {
            background: none; border: none; cursor: pointer;
            color: var(--text-muted); padding: 5px; border-radius: 4px;
            display: flex; align-items: center; transition: 0.1s;
        }
        .ai-tool-btn:hover { color: var(--accent); background: rgba(0,0,0,0.06); }
        body.light-theme .ai-tool-btn:hover { background: rgba(0,0,0,0.04); }
        .ai-tool-btn svg { width: 15px; height: 15px; }
        .ai-send-btn {
            background: transparent; color: var(--text-muted); border: none; cursor: pointer;
            padding: 5px; border-radius: 4px; font-weight: 400;
            font-size: 0.75rem; display: flex; align-items: center; gap: 4px;
            transition: 0.1s; flex-shrink: 0;
        }
        .ai-send-btn:hover { color: var(--accent); background: rgba(0,0,0,0.06); }
        .ai-send-btn:disabled { opacity: 0.3; cursor: not-allowed; }
        .ai-send-btn svg { width: 16px; height: 16px; }
        .ai-stop-btn {
            background: rgba(255,75,75,0.08); color: var(--crit);
            border: 1px solid rgba(255,75,75,0.3); padding: 4px 8px; border-radius: 4px;
            font-size: 0.68rem; font-weight: 600; cursor: pointer;
            display: none; align-items: center; gap: 4px; transition: 0.12s;
            font-family: 'Consolas', monospace;
        }
        .ai-stop-btn:hover { background: rgba(255,75,75,0.15); }
        .ai-stop-btn svg { width: 11px; height: 11px; }
        .ai-footer-hint {
            font-size: 0.62rem; color: var(--text-muted); margin-top: 6px;
            text-align: center; display: flex; align-items: center;
            justify-content: center; gap: 8px; opacity: 0.6;
        }
        /* File attachment chips */
        #ai-file-preview {
            display: none; flex-wrap: wrap; gap: 6px; padding: 0 0 8px 0;
        }
        .ai-file-chip {
            display: flex; align-items: center; gap: 6px;
            background: var(--bg-color); border: 1px solid var(--border-color);
            border-radius: 4px; padding: 3px 8px; font-size: 0.7rem;
            font-family: 'Consolas', monospace; color: var(--text-main);
        }
        .ai-file-chip button {
            background: none; border: none; cursor: pointer;
            color: var(--text-muted); font-size: 1rem; line-height: 1;
            padding: 0 2px; transition: 0.1s;
        }
        .ai-file-chip button:hover { color: var(--crit); }
        .ai-footer-row {
            display: flex; justify-content: space-between; align-items: center;
            margin-top: 6px;
        }
        .ai-footer-left { display: flex; gap: 4px; align-items: center; }

        /* ── TOAST NOTIFICATIONS ── */
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
            <button class="btn-ai-toggle" id="ai-toggle-btn" onclick="toggleAIPanel()" title="Blasco AI">
                <i data-lucide="bot"></i>
                <div class="ai-dot"></div>
            </button>
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
        <p>© 2026 ZenithMC Network. Todos los derechos reservados.</p>
        <p><a href="mailto:soporte@zenithmc.es">soporte@zenithmc.es</a></p>
        <div style="margin-top: 20px;">
            <a href="/logout" class="btn-accion-logout">
                <i data-lucide="log-out"></i> CERRAR SESIÓN
            </a>
        </div>
    </footer>

<script>

window.lucide = window.lucide || { createIcons: function(){} };

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

try { lucide.createIcons(); } catch(e) { console.warn('Lucide no disponible:', e.message); }

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

    if (typeof chart !== 'undefined' && chart) {
        const gridColor = isLight ? 'rgba(0,0,0,0.05)' : 'rgba(255,255,255,0.05)';
        const textColor = isLight ? '#6b7280' : '#8b949e';
        chart.options.scales.y.grid.color = gridColor;
        chart.options.scales.x.ticks.color = textColor;
        chart.options.scales.y.ticks.color = textColor;
        chart.options.plugins.legend.labels.color = textColor;
        chart.update();
    }
}

let chart = null;

function initChart() {
    try {
        if (typeof Chart === 'undefined') { throw new Error('Chart.js no cargado'); }
        const ctx = document.getElementById('mainChart').getContext('2d');
        chart = new Chart(ctx, {
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
    } catch(e) {
        console.warn('Chart.js no disponible, grafica deshabilitada:', e.message);
        const el = document.getElementById('mainChart');
        if (el) { el.parentNode.innerHTML = '<div style="padding:40px;text-align:center;color:var(--text-muted);">📊 Grafica no disponible (Chart.js no cargo)</div>'; }
    }
}

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

let p = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
let gateway = `${p}//${window.location.hostname}/ws`;
let websocket;
let csvDataRecibido = false;

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
    // Avisar al ESP32 que el cliente está listo para recibir
    websocket.send('__ready__');
}

function onClose(event) {
    csvDataRecibido = false;  // Resetear para que al reconectar se recargue el chart limpio
    setStatusBadge('connecting');
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
        if(d.type === 'csv_data') {
            if (d.text && d.text !== '__TOO_LARGE__') {
                csvDataRecibido = true;
                cargarHistorialGraficaDesdeTexto(d.text);
            }
            return;
        }
        if(d.type === 'security_alert') {
            if (d.habilitado !== false) {
                const lvl = d.nivel || 3;
                const lvlS = {1:'INFO',2:'ADVERTENCIA',3:'CRITICO'}[lvl]||'CRITICO';
                toast('⚠ [' + lvlS + '] ' + (d.causa || 'Parametro critico'), lvl === 3 ? 'err' : 'warn');
            }
            return;
        }
        if(d.type === 'security_ok') {
            // No mostrar toast si el sistema fue desactivado por el usuario — ya tiene feedback en /config
            if (!d.desactivado) toast('✓ Seguridad: parametros normalizados', 'ok');
            return;
        }
    } catch(e) {}
    
    let text = event.data;
    if (text.startsWith('ws_fwd:')) text = text.slice(7);

    const logs = document.getElementById('logs');
    logs.innerHTML += text;
    logs.scrollTop = logs.scrollHeight;
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

// Fallback: si tras 2.5 s de conexión el ESP32 no ha mandado csv_data por WS,
// cargar el historial vía fetch HTTP. En condiciones normales no debería ejecutarse
// porque onWsEvent ya manda el CSV al conectar (LAN y túnel).
function cargarHistorialGrafica() {
    if (csvDataRecibido) return;
    fetch('/datos.csv')
        .then(response => {
            if (!response.ok) throw new Error("Sin datos previos");
            return response.text();
        })
        .then(textoCSV => {
            if (csvDataRecibido) return; // llegó por WS mientras hacíamos el fetch
            cargarHistorialGraficaDesdeTexto(textoCSV);
        })
        .catch(err => console.log("Historial no cargado:", err));
}

function cargarHistorialGraficaDesdeTexto(textoCSV) {
    // Si chart.js aún no ha cargado (CDN lento), reintentar en 300 ms
    if (!chart) { setTimeout(() => cargarHistorialGraficaDesdeTexto(textoCSV), 300); return; }
    // Limpiar datos previos — evita duplicados en reconexiones
    chart.data.labels = [];
    chart.data.datasets.forEach(ds => ds.data = []);
    const filas = textoCSV.split('\n');
    for(let i = 0; i < filas.length; i++) {
        if(filas[i].trim() !== '') {
            const cols = filas[i].split(',');
            while (cols.length < 11) cols.push('');
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
    
    if (chart) {
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

    // Update AI live telemetry bar + store snapshot
    aiLastTelemetry = d;
    let wQ2 = Math.min(Math.max(2*(d.wifi+100),0),100);
    const aitTemp = document.getElementById('ait-temp');
    const aitCpu  = document.getElementById('ait-cpu');
    const aitRam  = document.getElementById('ait-ram');
    const aitWifi = document.getElementById('ait-wifi');
    if(aitTemp) aitTemp.textContent = d.temp + '°C';
    if(aitCpu)  aitCpu.textContent  = d.cpu + '%';
    if(aitRam)  aitRam.textContent  = d.ram + '%';
    if(aitWifi) aitWifi.textContent = wQ2 + '%';
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
    try { initChart(); } catch(e) {}
    try { lucide.createIcons(); } catch(e) {}
    initWebSocket();
    // El ESP32 manda csv_data por WS al conectar — fallback HTTP solo si no llega en 2.5 s
    setTimeout(cargarHistorialGrafica, 2500);
    cargarNombreUsuario();
    setTimeout(initAIPanel, 200);
    // Consultar estado de seguridad al cargar — si ya hay alerta activa
    // mostrarla sin esperar al próximo mensaje WS (puede tardar hasta 3s).
    fetch('/api/security/status').then(r => {
        if (!r.ok) return null;
        const ct = r.headers.get('content-type') || '';
        return ct.includes('application/json') ? r.json() : null;
    }).then(d => {
        if (d && d.modoAlerta) {
            const lvl = d.nivel || 3;
            const lvlS = {1:'INFO',2:'ADVERTENCIA',3:'CRITICO'}[lvl]||'CRITICO';
            toast('⚠ [' + lvlS + '] ' + (d.causa || 'Parametro critico'), lvl === 3 ? 'err' : 'warn');
        }
    }).catch(() => {});
});

// ============================================================================
// BLASCO AI — PANEL OVERLAY
// ============================================================================

let aiPanelOpen = false;
let aiHistorial = [];
let aiAbortCtrl = null;
let aiLastTelemetry = null;
let aiDbData = null;
let aiDbLoaded = false;
let aiAttachedFiles = [];
let aiMsgHistory = [];
let aiMsgHistoryIndex = -1;
let aiReadmeCached = '';
let aiKey = '';

function toggleAIPanel() {
    aiPanelOpen = !aiPanelOpen;
    const panel = document.getElementById('ai-panel');
    const backdrop = document.getElementById('ai-backdrop');
    const btn = document.getElementById('ai-toggle-btn');
    if (aiPanelOpen) {
        panel.classList.add('open');
        backdrop.classList.add('visible');
        btn.classList.add('active');
        document.getElementById('ai-textarea').focus();
        if (!aiReadmeCached) {
            fetch('/.readmeAI').then(r => r.text()).then(t => { aiReadmeCached = t.trim(); }).catch(() => {});
        }
    } else {
        panel.classList.remove('open');
        backdrop.classList.remove('visible');
        btn.classList.remove('active');
    }
}

function initAIPanel() {
    const m = localStorage.getItem('blasco_ai_model') || 'gemini-2.5-flash';
    const tag = document.getElementById('ai-model-tag');
    if (tag) tag.textContent = m;
}

function aiAutoResize(el) {
    el.style.height = 'auto';
    el.style.height = Math.min(el.scrollHeight, 110) + 'px';
}

function aiAppendMsg(role, html, id) {
    const box = document.getElementById('ai-chat-box');
    const welcome = document.getElementById('ai-welcome');
    if (welcome && !welcome.classList.contains('hidden')) {
        welcome.classList.add('hidden');
        box.classList.remove('hidden');
    }
    const div = document.createElement('div');
    div.className = 'ai-msg ' + role;
    if (id) div.id = id;
    const avatarIcon = role === 'bot'
        ? '<i data-lucide="bot" style="width:11px;height:11px;"></i>'
        : '<i data-lucide="user" style="width:11px;height:11px;"></i>';
    const label = role === 'bot' ? 'Blasco AI' : 'Tu';
    const copyBtn = role === 'bot' ? '<button class="ai-copy-btn" onclick="aiCopyMsg(this)" title="Copiar respuesta"><i data-lucide="copy" style="width:11px;height:11px;"></i></button>' : '';
    div.innerHTML =
        '<div class="ai-msg-header">' +
            '<div class="ai-avatar ' + role + '">' + avatarIcon + '</div>' +
            '<span class="ai-msg-name ' + role + '">' + label + '</span>' +
            copyBtn +
        '</div>' +
        '<div class="ai-bubble">' + html + '</div>';
    box.appendChild(div);
    lucide.createIcons();
    box.scrollTop = box.scrollHeight;
    return div;
}

function aiCopyMsg(btn) {
    const bubble = btn.closest('.ai-msg').querySelector('.ai-bubble');
    const text = bubble.textContent || '';
    if (navigator.clipboard && navigator.clipboard.writeText) {
        navigator.clipboard.writeText(text).catch(() => aiCopyFallback(text));
    } else {
        aiCopyFallback(text);
    }
    btn.innerHTML = '<i data-lucide="check" style="width:11px;height:11px;"></i>';
    lucide.createIcons();
    setTimeout(() => { btn.innerHTML = '<i data-lucide="copy" style="width:11px;height:11px;"></i>'; lucide.createIcons(); }, 2000);
}
function aiCopyFallback(text) {
    const ta = document.createElement('textarea');
    ta.value = text; ta.style.position = 'fixed'; ta.style.opacity = '0';
    document.body.appendChild(ta); ta.select();
    try { document.execCommand('copy'); } catch(e) {}
    document.body.removeChild(ta);
}

function aiQuickPrompt(text) {
    const ta = document.getElementById('ai-textarea');
    ta.value = text;
    aiSend();
}

function aiFormatText(raw) {
    return raw
        .replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;')
        .replace(/```(\w*)\n?([\s\S]*?)```/g, '<pre><code>$2</code></pre>')
        .replace(/`([^`]+)`/g, '<code>$1</code>')
        .replace(/\*\*([^*]+)\*\*/g, '<strong>$1</strong>')
        .replace(/\n/g, '<br>');
}

async function aiLoadDB() {
    const btn = document.getElementById('ai-db-btn');
    btn.disabled = true;
    btn.innerHTML = '<i data-lucide="loader" style="width:14px;"></i>';
    lucide.createIcons();
    try {
        const res = await fetch('/datos.csv');
        if (!res.ok) throw new Error('HTTP ' + res.status);
        const text = await res.text();
        const lines = text.trim().split('\n').filter(l => l.trim());
        const last100 = lines.slice(-100);
        aiDbData = last100.join('\n');
        aiDbLoaded = true;
        aiAppendMsg('bot', '📊 <strong>DB cargada:</strong> ' + lines.length + ' registros totales, mostrando últimas ' + last100.length + ' filas al modelo. Puedes preguntar por picos, medias, tendencias...');
    } catch(e) {
        aiAppendMsg('bot', '<span style="color:var(--crit);">❌ Error cargando DB: ' + e.message + '</span>');
    }
    btn.disabled = false;
    btn.innerHTML = '<i data-lucide="database" style="width:14px;"></i>';
    lucide.createIcons();
}

async function aiSend() {
    if (aiAbortCtrl) return;

    const input = document.getElementById('ai-textarea');
    const texto = input.value.trim();

    // Allow sending files even without text
    if (!texto && !aiAttachedFiles.length) return;

    // Check API key from server cache or fetch it
    if (!aiKey) {
        try {
            const resp = await fetch('/api/ai/key');
            if (resp.ok) {
                const data = await resp.json();
                aiKey = data.key || '';
            }
        } catch(e) {}
    }
    if (!aiKey) {
        aiAppendMsg('bot', '⚠️ <strong>API Key no configurada.</strong> Ve a <a href="/config" style="color:var(--accent);text-decoration:underline;">/config</a> y configura tu Gemini API Key para usar Blasco AI.');
        document.getElementById('ai-send-btn').style.display = '';
        return;
    }

    const selectedModel = localStorage.getItem('blasco_ai_model') || 'gemini-2.5-flash';
    input.value = '';
    input.style.height = 'auto';

    // Add to message history for arrow navigation
    aiMsgHistory.push(texto);
    aiMsgHistoryIndex = aiMsgHistory.length;

    // Show user message with file indicators
    let displayText = texto.replace(/</g,'&lt;');
    if (aiAttachedFiles.length) {
        const names = aiAttachedFiles.map(f => f.name).join(', ');
        displayText += '\n\n📎 **' + names + '**';
    }
    aiAppendMsg('user', displayText);
    document.getElementById('ai-send-btn').style.display = 'none';
    document.getElementById('ai-stop-btn').style.display = 'inline-flex';

    // Thinking placeholder
    const botId = 'ai-bot-' + Date.now();
    const botDiv = aiAppendMsg('bot', '<span class="ai-thinking"><span><em>.</em><em>.</em><em>.</em></span></span>', botId);
    const bubbleEl = botDiv.querySelector('.ai-bubble');

    // Build system prompt with full .readmeAI + README context
    const t = aiLastTelemetry;
    let telemStr = t ? 
        'CPU: ' + t.cpu + '%, Core0: ' + t.c0 + '%, Core1: ' + t.c1 + '%, Temp CPU: ' + t.temp + '°C, RAM: ' + t.ram + '%, Flash: ' + t.flash + '%, WiFi: ' + t.wifi + 'dBm, Uptime: ' + t.uptime +
        (t.dhtTemp !== undefined ? ', Temp Amb: ' + t.dhtTemp + '°C, Humedad: ' + t.dhtHum + '%' : '') : 'No disponible (WebSocket no conectado aún)';

    // Load combined .readmeAI + README context from server (cached)
    let readmeaiCtx = aiReadmeCached;
    if (!readmeaiCtx) {
        try {
            const resp = await fetch('/.readmeAI');
            aiReadmeCached = await resp.text();
            readmeaiCtx = aiReadmeCached.trim();
        } catch(e) {
            readmeaiCtx = '';
        }
    }

    let sysPrompt = `Eres Blasco AI, asistente de inteligencia artificial integrado en el dashboard de telemetría en tiempo real del proyecto ESP32 Blasco OS.

Tu creador es Ruben Blasco Armengod (github.com/rubenblascoa). Eres experto en ESP32, Arduino C++, FreeRTOS, sistemas embebidos, NFC/RFID y análisis de telemetría hardware.

Responde siempre en español salvo que el usuario escriba en otro idioma. Sé directo, técnico y preciso.
Cuando el usuario adjunte archivos (imágenes, código, logs, CSV, etc.), léelos atentamente y úsalos como contexto para tu respuesta.

## ═══════════════════════════════════════
## TELEMETRÍA EN TIEMPO REAL (snapshot ESP32)
## ═══════════════════════════════════════
${telemStr}

## ═══════════════════════════════════════
## CONTEXTO COMPLETO DEL PROYECTO (.readmeAI + GitHub README)
## ═══════════════════════════════════════
${readmeaiCtx}
`;

    if (aiDbLoaded && aiDbData) {
        sysPrompt += `
## ═══════════════════════════════════════
## DATOS HISTÓRICOS DB (últimas 100 filas de datos.csv)
## ═══════════════════════════════════════
\`\`\`csv
timestamp,temp_cpu,cpu,core0,core1,ram,flash,wifi,temp_amb,humedad,alerta
${aiDbData}
\`\`\``;
    }

    // Build user message parts with optional file attachments
    const userParts = [{ text: texto }];
    if (aiAttachedFiles.length) {
        for (const file of aiAttachedFiles) {
            if (file.type.startsWith('image/')) {
                try {
                    const b64 = await aiFileToBase64(file);
                    userParts.push({ inline_data: { mime_type: file.type, data: b64.split(',')[1] } });
                } catch(e) {
                    userParts.push({ text: '\n[Error al adjuntar imagen: ' + file.name + ']\n' });
                }
            } else {
                try {
                    const text = await file.text();
                    userParts.push({ text: '\n--- Archivo adjunto: ' + file.name + ' ---\n' + text + '\n--- Fin ---\n' });
                } catch(e) {
                    userParts.push({ text: '\n[Error al leer archivo: ' + file.name + ']\n' });
                }
            }
        }
    }
    aiAttachedFiles = [];
    aiUpdateFilePreview();

    aiHistorial.push({ role: 'user', parts: userParts });
    const contents = aiHistorial.slice(-12);

    aiAbortCtrl = new AbortController();
    let fullReply = '';

    try {
        const url = `https://generativelanguage.googleapis.com/v1beta/models/${selectedModel}:streamGenerateContent?alt=sse&key=${aiKey}`;
        const response = await fetch(url, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                system_instruction: { parts: [{ text: sysPrompt }] },
                contents: contents,
                tools: [{ googleSearch: {} }],
                generationConfig: { temperature: 0.7, maxOutputTokens: 4096 }
            }),
            signal: aiAbortCtrl.signal
        });

        if (!response.ok) {
            const errText = await response.text();
            let errMsg = 'HTTP ' + response.status;
            try { errMsg = JSON.parse(errText).error?.message || errMsg; } catch(e) {}
            throw new Error(errMsg);
        }

        const reader = response.body.getReader();
        const decoder = new TextDecoder();
        let buffer = '';
        let firstChunk = true;

        while (true) {
            const { done, value } = await reader.read();
            if (done) break;
            buffer += decoder.decode(value, { stream: true });
            const lines = buffer.split('\n');
            buffer = lines.pop() || '';
            for (const line of lines) {
                if (!line.startsWith('data: ')) continue;
                const js = line.substring(6).trim();
                if (!js) continue;
                try {
                    const chunk = JSON.parse(js);
                    const parts = chunk.candidates?.[0]?.content?.parts;
                    if (parts) { for (const p of parts) { if (p.text) fullReply += p.text; } }
                } catch(e) {}
            }
            if (firstChunk && fullReply) {
                firstChunk = false;
                bubbleEl.innerHTML = aiFormatText(fullReply);
            } else if (fullReply) {
                bubbleEl.innerHTML = aiFormatText(fullReply);
            }
            document.getElementById('ai-chat-box').scrollTop = 99999;
        }

        bubbleEl.innerHTML = aiFormatText(fullReply || 'Sin respuesta del modelo.');
        aiHistorial.push({ role: 'model', parts: [{ text: fullReply }] });

    } catch(err) {
        if (err.name !== 'AbortError') {
            bubbleEl.innerHTML = '<span style="color:var(--crit);">❌ Error: ' + err.message + '</span>';
        } else {
            if (fullReply) {
                bubbleEl.innerHTML = aiFormatText(fullReply) + '<br><span style="color:var(--text-muted);font-size:0.7rem;">[generación detenida]</span>';
            } else {
                bubbleEl.innerHTML = '<span style="color:var(--text-muted);">[Detenido]</span>';
            }
        }
    }

    aiAbortCtrl = null;
    document.getElementById('ai-stop-btn').style.display = 'none';
    document.getElementById('ai-send-btn').style.display = '';
    lucide.createIcons();
    document.getElementById('ai-chat-box').scrollTop = 99999;
}

function aiStop() {
    if (aiAbortCtrl) { aiAbortCtrl.abort(); aiAbortCtrl = null; }
}

function aiClear() {
    aiHistorial = [];
    aiDbData = null;
    aiDbLoaded = false;
    aiAttachedFiles = [];
    aiUpdateFilePreview();
    const box = document.getElementById('ai-chat-box');
    box.innerHTML = '';
    box.classList.add('hidden');
    const welcome = document.getElementById('ai-welcome');
    if (welcome) welcome.classList.remove('hidden');
    document.getElementById('ai-stop-btn').style.display = 'none';
    document.getElementById('ai-send-btn').style.display = '';
    const dbBtn = document.getElementById('ai-db-btn');
    if (dbBtn) dbBtn.style.color = '';
}

function aiTriggerFileInput() {
    document.getElementById('ai-file-input').click();
}

async function aiHandleFiles() {
    const input = document.getElementById('ai-file-input');
    const files = Array.from(input.files);
    if (!files.length) return;
    for (const file of files) {
        if (file.size > 10 * 1024 * 1024) {
            aiAppendMsg('bot', '<span style="color:var(--warn);">⚠️ El archivo "' + file.name + '" es demasiado grande (máx 10MB).</span>');
            continue;
        }
        aiAttachedFiles.push(file);
    }
    aiUpdateFilePreview();
    input.value = '';
}

function aiUpdateFilePreview() {
    const container = document.getElementById('ai-file-preview');
    if (!container) return;
    if (!aiAttachedFiles.length) {
        container.innerHTML = '';
        container.style.display = 'none';
        return;
    }
    container.style.display = 'flex';
    container.innerHTML = aiAttachedFiles.map(function(f, i) {
        return '<div class="ai-file-chip"><span>📎 ' + f.name + '</span><button onclick="aiRemoveFile(' + i + ')" title="Quitar archivo">&times;</button></div>';
    }).join('');
}

function aiRemoveFile(index) {
    aiAttachedFiles.splice(index, 1);
    aiUpdateFilePreview();
}

function aiFileToBase64(file) {
    return new Promise(function(resolve, reject) {
        const reader = new FileReader();
        reader.onload = function() { resolve(reader.result); };
        reader.onerror = reject;
        reader.readAsDataURL(file);
    });
}

document.addEventListener('keydown', function(e) {
    if (e.key === 'Escape' && aiPanelOpen) toggleAIPanel();
});
</script>

    <!-- BLASCO AI: Contexto README del proyecto -->
    <script type="text/plain" id="blasco-readme">
<div align="center">

<img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="ESP32 Blasco OS Logo" width="200" />

<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:cpu.svg?color=white">
    <img src="https://api.iconify.design/lucide:cpu.svg?color=black" alt="Chip" width="28" align="center" />
  </picture> 
  ESP32 Blasco OS | Multi-Program
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:satellite-dish.svg?color=white">
    <img src="https://api.iconify.design/lucide:satellite-dish.svg?color=black" alt="Antenna" width="28" align="center" />
  </picture>
</h3>

[Español](README.es.md) · **English** · [Français](README.fr.md)

---

**The ultimate multi-program execution environment for low-level engineering.**

[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)

**ESP32 Blasco** is a low-level engineering platform and a multi-program execution environment designed exclusively for the ESP32-S3 microcontroller. It acts as a lightweight "Operating System" remotely accessible via **Telnet (Wi-Fi)** and WebSockets. It allows encapsulating and executing multiple hardware projects on the same board, switching between them through a pure text terminal interface (retro/hacker style), completely **wire-free** and featuring **dynamic runtime hot-configuration**.

[Explore the Code](https://github.com/rubenblascoa/esp32-panelcontrol/tree/main/Code) · [Report a Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Request an Improvement](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

<div align="center">

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Hardware Powered by PCBWay

<br>
<a href="https://www.pcbway.com/">
  <img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="300">
</a>
<br><br>
</div>

> **From prototyping to professional manufacturing:** The development of **ESP32 Blasco OS** demands hardware capable of supporting the asynchronous execution of multiple programs in real time. For the physical deployment of this project, I rely on the manufacturing and assembly (PCBA) services of **[PCBWay](https://www.pcbway.com/)**.

>In an architecture that delegates 100% of the CPU to FreeRTOS threads, the physical design of the motherboard is critical. The integration of PCBWay into this project responds to very specific technical needs of the low-level environment:

>* **Signal Integrity (SPI and I2C):** The NFC cloning module (MFRC522) operates at frequencies where noise is fatal. PCBWay's precise track routing guarantees no electromagnetic interference, preventing data collisions on the bus and ensuring lossless scans.
  
>* **Dissipation for Real Telemetry:** This system extracts stress and temperature data directly from the CPU cores. The quality of the board's copper and fiberglass allows for proper thermal dissipation, vital for keeping the processor stable under continuous load.
  
>* **Assembly Precision (PCBA):** The integration of the ESP32-S3 (N16R8) chip and the soldering of SMD components require exact tolerances so that the final hardware supports the 24/7 execution environment without electrical failures.

>The manufacturing experience has been straightforward and seamless, with smooth management of the Gerber and BOM files thanks to the support of their technical team (with a special mention to Liam for facilitating the sponsorship coordination). If you are looking to manufacture your own schematics with industrial quality, the results meet the standards of hardware engineering.

>**[Explore PCBWay's manufacturing services here](https://www.pcbway.com/)**

---

</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:folder-tree.svg?color=white"><img src="https://api.iconify.design/lucide:folder-tree.svg?color=black" width="26" align="center"></picture> Architecture and Modular Structure

The firmware implements a strict separation of concerns between hardware and software through an asynchronous multi-core decoupling. The purpose and internal execution logic of each system file are detailed below:

### 1. Entry Core and Orchestration
* **`main.ino`**
  * **Purpose:** It is the physical entry point of the ESP32-S3 and the main orchestration file.
  * **Detailed logic:** If the system lacks valid network credentials, it automatically provisions a local **Wi-Fi Access Point (AP)** named `Esp32BlascoOS_Setup` to spin up a Captive Portal. Once connected, it sets up the physical I2C bus, loads NVS settings, hooks up the status LED, initializes the FreeRTOS processing cores (`taskCore0` and `taskCore1` with 16KB of stack allocation each), and purges itself calling `vTaskDelete(NULL)` to grant complete CPU control to the scheduler.

### 2. Configuration and System State
* **`config.h` / `config.cpp`**
  * **Purpose:** Abstract layout definition of global memory structures, thresholds, and runtime pin maps.
  * **Detailed logic:** Declares macros for logging streams (`LOG_I`/`LOG_E`) and binds global state variables via the `extern` keyword to prevent duplication across compiler units. Complex peripherals like the `MFRC522` reader are handled strictly as dynamic pointers (`MFRC522*`), enabling complete runtime remapping of GPIO connections from the Captive Portal or REST endpoints without modifying a single line of source code.

### 3. Multi-Core Scheduler (FreeRTOS)
* **`tareas.h` / `tareas.cpp`**
  * **Purpose:** Controls the hardware-distributed infinite processing loops replacing the default Arduino execution flow.
  * **Detailed logic:**
    * **`taskCore0`:** Manages network interfaces and local storage. It handles raw async WebSocket inputs, executes background OTA flashes, throttles NTP syncing, and triggers a periodic file cronjob every 2 hours to back up historical system state logs.
    * **`taskCore1`:** Dedicated entirely to physical hardware operations. It evaluates input console command queues (`cmdQueue`), refreshes the 5-page diagnostic LCD using safe thread guards, and steps through the running states of the active background sensors.

### 4. Graphical Interface and Frontend
* **`web_pages.h` / `web_pages.cpp`**
  * **Purpose:** Stores the unalterable HTML, CSS, and modern JavaScript structures for the remote administration dashboards.
  * **Detailed logic:** Uses the `PROGMEM` keyword to lock the massive web layouts (Dashboard, DB Viewer, Config Panel, Captive Portal, and Login screens) into the Flash memory space. This prevents the assets from polluting the dynamic RAM Heap, completely removing memory-exhaustion reboots during simultaneous client attachments.

### 5. Routing and Network Security
* **`web_server.h` / `web_server.cpp`**
  * **Purpose:** Handles async network routing and incoming server sockets on Core 0.
  * **Detailed logic:** Provisions a complete REST API engine exposing 19 operational endpoints. Validates request authenticity by verifying the `ZENITH_SESSION` cookie against the dynamic token in RAM. WebSocket inputs are processed using safe memory allocations (`malloc/memcpy`), shielding the system against frame-based heap corruption, and endpoints like `/api/config/pins` handle live JSON data streams to map internal hardware states on the fly.

### 6. Sensor Controllers (Hardware)
* **`nfc.h` / `nfc.cpp`**
  * **Purpose:** Drives proximity reading, block auditing, and cloning cycles over the physical SPI bus.
  * **Detailed logic:** Controls the MFRC522 chip to communicate with MIFARE Classic 1K cards. Validates authorization keys (Key A) before extracting sector blocks. In write mode, it targets rewritable magic cards (CUID/FUID), rewriting Sector 0 and Block 0 to clone UID fingerprints.
* **`ultrasonidos.h` / `ultrasonidos.cpp`**
  * **Purpose:** Captures spatial metric parameters via acoustic reflection using the HC-SR04 transductor.
  * **Detailed logic:** Uses a 100% non-blocking hardware Finite State Machine (FSM). Fires a brief 10µs trigger pulse and binds a low-level interrupt routine (`IRAM_ATTR ecoISR`) to the ECHO pin to calculate sound flight time. This eliminates blocking calls like `pulseIn()`, letting Core 1 multitask freely during acoustic propagation.
* **`dht.h` / `dht.cpp`**
  * **Purpose:** Extracts localized relative humidity and temperature parameters via the DHT11 sensor.
  * **Detailed logic:** Features a custom, dependency-free native 1-Wire protocol implementation. Temporarily isolates the processor from timing anomalies by turning off system interrupts (`noInterrupts()`) during the microsecond-critical bit-banging capture window. Implements an automated 1-second auto-retry mechanic to guarantee data integrity against ambient electromagnetic interference.

### 7. Utilities and Disk Logs
* **`utils.h` / `utils.cpp`**
  * **Purpose:** Backend helper framework for mathematical smoothing, persistent storage, and physical bus routing.
  * **Detailed logic:** Computes actual core stress profiles using an Exponential Moving Average algorithm (EMA 30/70) to screen out momentary spikes. Manages 3 separate persistent NVS spaces (`zenithmc`, `hwconfig`, `webcred`) to protect underlying peripheral mappings from accidental Wi-Fi wipes. Uses an explicit Mutex semaphore (`i2cMutex`) to coordinate shared I2C traffic on the LCD display while rolling across 5 distinct telemetry screens.
* **`sd_card.h` / `sd_card.cpp`** *(Optional)*
  * **Purpose:** Interconnects a physical MicroSD card expansion slot over a dedicated SPI bus.
  * **Detailed logic:** Safeguarded under the preprocessor compilation guard `#ifdef SD_CS_PIN`. When present, it automatically intercepts database logging tasks from internal storage and streams the output directly onto physical disk media.

### 8. Text Layer (CLI Interface)
* **`menus.h` / `menus.cpp`**
  * **Purpose:** Visual framework and action parsing logic for the text-based console interface.
  * **Detailed logic:** Employs the `TerminalHibrida` buffering layout to package complex telemetry arrays (Uptime, Core stress, Heap space) before blasting them onto the network fabric, cutting down TCP packet pollution. Changes the running index `programaActivo` to redirect execution states.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:list.svg?color=white"><img src="https://api.iconify.design/lucide:list.svg?color=black" width="26" align="center"></picture> Main Features

* **100% Wireless Control:** Complete access to the system terminal and diagnostic output through standard Telnet (Port 23) and raw WebSockets.
* **Dynamic Hardware Pin Mapping (Plug & Play):** On its first deployment, the OS launches an automated configuration hotspot (`Esp32BlascoOS_Setup`). Users can safely configure their localized router parameters, assign custom GPIO pins, and set web passwords from any smartphone browser without rebuilding the firmware.
* **EMA-Smoothed Real-Time Telemetry:** Advanced diagnostics detailing:
  * Dynamic RAM/PSRAM boundaries (Total, Allocated, Free).
  * Local File Storage map occupancy (LittleFS / SD Card).
  * True multi-core mathematical processor loads and silicon temperature profiles.
* **NTP Time-Backed Database:** Fully automated background logging routines. Once synced to atomic network clocks, the firmware appends a structured 10-column telemetry line into the system CSV spreadsheet every 2 hours continuously.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:git-compare.svg?color=white"><img src="https://api.iconify.design/lucide:git-compare.svg?color=black" width="26" align="center"></picture> The Problem vs The Solution

Developing and testing multiple hardware projects on a single microcontroller is usually a mess. 

| Without Blasco OS | With Blasco OS |
| :--- | :--- |
| Mandatory USB connection to test | **100% Wireless** via Telnet |
| Flash firmware on every physical pin change | **Runtime Configuration** via Captive Portal |
| Basic monitoring via Serial | **Advanced telemetry** (RAM, Flash, Temp, CPU) |
| Updates via cable | **OTA Support** (Over-The-Air) |
| Coupled projects that break code | **Modular Architecture** (Independent "Drawers") |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:blocks.svg?color=white"><img src="https://api.iconify.design/lucide:blocks.svg?color=black" width="26" align="center"></picture> Active Modules

Currently, the operating system has three main integrated projects:

### 1. NFC Cloning Station Pro (V14)
An advanced RFID auditing and cloning module using **MFRC522** hardware.
* **Deep Reading:** Extracts all information from the card and saves it into the ESP32 RAM.
* **Physical Cloning:** Allows injecting data into Sector 0 of rewritable magic cards (CUID/FUID).

### 2. Ultrasonic Radar (V3)
Physical telemetry module using the **HC-SR04** distance sensor.
* **Asynchronous Execution (ISR):** 100% Non-blocking flow driven by hardware interrupts; the ESP32 suffers zero micro-freezes while the sound bounces.
* **Loop Reading:** Configurable cyclical refresh with thermal fault tolerance ("Out of range").

### 3. Ambient Temperature and Humidity Monitor (V1)
Integrated local climate data acquisition module via **DHT11**.
* **Native 1-Wire Protocol:** Low-level reading (Bit-Banging) that relies on no third-party libraries, optimized with hardware delays and auto-recovery routines against electromagnetic noise.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:hard-drive.svg?color=white"><img src="https://api.iconify.design/lucide:hard-drive.svg?color=black" width="26" align="center"></picture> Required Hardware and Installation

* **Base Board:** ESP32 (S3 N16R8 or similar).
* **NFC Module:** MFRC522 RFID Reader (SPI Bus).
* **Distance Module:** HC-SR04 Sensor.
* **Climate Sensor:** DHT11 Module (with 4.7kΩ pull-up resistor).
* **Storage Module (Optional):** MicroSD Reader (SPI Bus).

> *Note: The system boots with default pins, but all hardware connections (RST, SS, TRIG, ECHO, DHT) can be completely reassigned from the Web Captive Portal without touching the code.*

### Initial Deployment (Via Captive Portal):
1. Flash the compiled source code via USB for the first time using your preferred IDE.
2. The ESP32 will format its internal filesystem (LittleFS) and, upon not detecting valid home credentials, will open an Access Point (AP).
3. Search on your phone or PC for the open Wi-Fi network: **`Esp32BlascoOS_Setup`** and connect to it.
4. A web wizard will open automatically. Follow the steps to enter your local router password, configure your GPIO pins, and define your web admin credentials.
5. Upon clicking Save, the ESP32 will reboot, shut down AP mode, and connect to your home router transparently.
6. Open the Serial console at `115200 baud` or check the LCD screen to discover its newly assigned local IP.
7. Open that IP in your web browser. Enjoy the environment!

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:settings.svg?color=white"><img src="https://api.iconify.design/lucide:settings.svg?color=black" width="26" align="center"></picture> Compilation Configuration (Arduino IDE)

To ensure the project compiles correctly and the Web UI has enough space for the LittleFS database, it is **mandatory** to apply the following settings in the Arduino IDE **Tools** menu. 

*This configuration is optimized for **ESP32-S3 (N16R8)** boards featuring 16MB of Flash and 8MB of PSRAM.*

| Configuration Parameter | Exact Required Value |
| :--- | :--- |
| **Board** | `ESP32S3 Dev Module` |
| **USB CDC On Boot** | `Enabled` |
| **CPU Frequency** | `240MHz (WiFi)` |
| **Core Debug Level** | `None` |
| **USB DFU On Boot** | `Disabled` |
| **Erase All Flash Before Sketch Upload** | `Disabled` |
| **Events Run On** | `Core 1` |
| **Flash Mode** | `QIO 80MHz` |
| **Flash Size** | `16MB (128Mb)` |
| **JTAG Adapter** | `Disabled` |
| **Arduino Runs On** | `Core 1` |
| **USB Firmware MSC On Boot** | `Disabled` |
| **Partition Scheme** | `16M Flash (3MB APP/9.9MB FATFS)` ⚠️ |
| **PSRAM** | `OPI PSRAM` ⚠️ |
| **Upload Mode** | `UART0 / Hardware CDC` |
| **Upload Speed** | `921600` |
| **USB Mode** | `Hardware CDC and JTAG` |
| **Zigbee Mode** | `Disabled` |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:monitor.svg?color=white"><img src="https://api.iconify.design/lucide:monitor.svg?color=black" width="26" align="center"></picture> Interface and Telemetry

Below is the execution environment of the Operating System. Click on the dropdown menus to expand and view the full interface screenshots.

<details>
<summary><b>Control Panel (Telemetry and live execution)</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/db980fe4-600b-43e1-b8bb-b9c366eb26ee" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/eef43556-0c27-49e9-8d07-eda9577c5c75" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/4f032878-b0b0-406b-81f2-786c12560149" />

<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55b18cca-1077-499a-81ed-d8acf6d66cc4" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/6c5cba54-ecff-498f-835e-130887f2bf8b" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/bbca1743-3a4e-4fa7-9f1b-6b4601bce200" />
</details>

<details>
<summary><b>Database Management</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/15c40684-d907-4bca-9387-72960a42e2ad" />

<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/59ef1539-4c79-4654-aa02-3c0c15b51490" />
</details>

<details>
<summary><b>Authentication Login </b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />

<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />
</details>

<details>
<summary><b>Internal Configuration</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/0ef0f0d3-2d07-4788-a5ba-e5ab27a03350" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/fc443c62-634d-437d-b8ac-b34004d1911f" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/25386ba6-4501-4d4b-b53e-b7bc85d0a78c" />


<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/92536347-e43d-4ae2-8d77-b39f28956280" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/f0722d4c-c2fb-45b4-9eb4-78b2052cc72c" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/a1f0496e-58c5-4706-96f9-b061129aa223" />

</details>

<details>
<summary><b>Initial Configuration</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/7baf8092-9362-4781-b994-7e944e0e2b77" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/c21cca77-21ab-4a27-968c-e9bdb81bb691" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/780ecdca-629f-4bed-ac2a-09ff42127f92" />



<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/2d2c4c3b-e5e7-4fae-a8e1-2485f2c790c6" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/8361138e-f22b-478e-94a1-d0e94df2959a" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55d62211-ca00-482a-b7dc-7fac87c9ab99" />

</details>

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:mail.svg?color=white"><img src="https://api.iconify.design/lucide:mail.svg?color=black" width="26" align="center"></picture> Contact & Sponsorship

Developed with passion by **Ruben Blasco Armengod**.

If you are interested in sponsoring the continuous expansion of this project, collaborating on custom hardware motherboard layouts, or embedding new sensors into the multi-tasking core, please reach out through any of the following technical channels:

* **GitHub:** [@rubenblascoa](https://github.com/rubenblascoa)
* **Instagram:** [@rubenblascoa](https://instagram.com/rubenblascoa)
* **Email:** rubenblascoarmengod@gmail.com

    </script>

<!-- BLASCO AI OVERLAY -->
<div id="ai-backdrop" onclick="toggleAIPanel()"></div>

<div id="ai-panel">
    <div class="ai-panel-header">
        <i data-lucide="message-square" style="width:13px;height:13px;color:#858585;flex-shrink:0;"></i>
        <span class="ai-title">Chat</span>
        <span class="ai-model-tag" id="ai-model-tag">gemini-2.5-flash</span>
        <div class="ai-header-actions" style="margin-left:auto;">
            <button onclick="aiLoadDB()" id="ai-db-btn" title="Cargar datos.csv en contexto">
                <i data-lucide="database" style="width:14px;height:14px;"></i>
            </button>
            <a href="/config" title="Configurar API Key" style="display:flex;align-items:center;background:none;border:none;cursor:pointer;color:#858585;padding:4px 5px;border-radius:4px;transition:0.1s;text-decoration:none;" onmouseover="this.style.color='#ccc';this.style.background='rgba(255,255,255,0.08)'" onmouseout="this.style.color='#858585';this.style.background='none'">
                <i data-lucide="settings" style="width:14px;height:14px;"></i>
            </a>
            <button onclick="aiClear()" title="Nueva conversacion">
                <i data-lucide="plus" style="width:14px;height:14px;"></i>
            </button>
            <button onclick="toggleAIPanel()" title="Cerrar panel">
                <i data-lucide="x" style="width:14px;height:14px;"></i>
            </button>
        </div>
    </div>

    <div class="ai-telem-bar" id="ai-telem-bar">
        <span class="telem-label">Live</span>
        <div class="ai-telem-item">
            <i data-lucide="thermometer" style="width:10px;height:10px;opacity:0.8;"></i>
            <span id="ait-temp">--</span>
        </div>
        <div class="ai-telem-item">
            <i data-lucide="cpu" style="width:10px;height:10px;opacity:0.8;"></i>
            <span id="ait-cpu">--</span>
        </div>
        <div class="ai-telem-item">
            <i data-lucide="server" style="width:10px;height:10px;opacity:0.8;"></i>
            <span id="ait-ram">--</span>
        </div>
        <div class="ai-telem-item">
            <i data-lucide="wifi" style="width:10px;height:10px;opacity:0.8;"></i>
            <span id="ait-wifi">--</span>
        </div>
        <div class="ai-telem-item" id="ait-db-status" style="margin-left:auto;"></div>
    </div>

    <div id="ai-welcome">
        <div class="ai-welcome-icon">
            <svg viewBox="0 0 50 50" fill="none" stroke="currentColor" stroke-width="1.2" stroke-linecap="round" stroke-linejoin="round" style="width:52px;height:52px;">
                <circle cx="25" cy="18" r="10"/>
                <circle cx="19" cy="16" r="1" fill="currentColor" stroke="none"/>
                <circle cx="31" cy="16" r="1" fill="currentColor" stroke="none"/>
                <path d="M19 23c1.5 2 3 3 6 3s4.5-1 6-3"/>
                <path d="M8 42c0-9 7.5-15 17-15s17 6 17 15"/>
            </svg>
        </div>
        <div class="ai-welcome-title">Blasco AI</div>
        <div class="ai-welcome-sub">Asistente del ESP32 Blasco OS.<br>Telemetria en vivo inyectada automaticamente.</div>
        <div class="ai-welcome-actions">
            <button class="ai-welcome-btn" onclick="aiQuickPrompt('Estado actual del sistema ESP32')">
                <i data-lucide="activity" style="width:13px;height:13px;"></i>
                Estado del sistema
            </button>
            <button class="ai-welcome-btn" onclick="aiLoadDB()">
                <i data-lucide="database" style="width:13px;height:13px;"></i>
                Cargar historial DB
            </button>
            <button class="ai-welcome-btn" onclick="aiQuickPrompt('Analiza el rendimiento de los cores del ESP32')">
                <i data-lucide="cpu" style="width:13px;height:13px;"></i>
                Analizar rendimiento
            </button>
            <button class="ai-welcome-btn" onclick="aiQuickPrompt('Hay algun problema o anomalia en el sistema?')">
                <i data-lucide="alert-triangle" style="width:13px;height:13px;"></i>
                Detectar anomalias
            </button>
        </div>
        <div class="ai-welcome-note">Revisar la salida de IA antes de aplicar cambios.</div>
    </div>

    <div class="ai-chat-box hidden" id="ai-chat-box"></div>

    <div class="ai-input-area">
        <div id="ai-file-preview" style="display:none;"></div>
        <div class="ai-input-box">
            <textarea id="ai-textarea" placeholder="Pregunta sobre el ESP32..." rows="1"
                oninput="aiAutoResize(this)"
                onkeydown="if(event.key==='Enter'&&!event.shiftKey){event.preventDefault();aiSend();}else if(event.key==='ArrowUp'&&!event.shiftKey){event.preventDefault();aiMsgHistoryIndex=Math.max(0,aiMsgHistoryIndex-1);document.getElementById('ai-textarea').value=aiMsgHistory[aiMsgHistoryIndex]||'';}else if(event.key==='ArrowDown'&&!event.shiftKey){event.preventDefault();aiMsgHistoryIndex=Math.min(aiMsgHistory.length,aiMsgHistoryIndex+1);document.getElementById('ai-textarea').value=aiMsgHistoryIndex<aiMsgHistory.length?aiMsgHistory[aiMsgHistoryIndex]:'';}"></textarea>
            <div class="ai-input-tools">
                <input type="file" id="ai-file-input" accept="image/*,.pdf,.txt,.csv,.js,.py,.cpp,.h,.ino,.md" multiple style="display:none;" onchange="aiHandleFiles()">
                <button class="ai-tool-btn" onclick="aiTriggerFileInput()" title="Adjuntar archivo">
                    <i data-lucide="paperclip" style="width:15px;height:15px;"></i>
                </button>
                <button class="ai-stop-btn" id="ai-stop-btn" onclick="aiStop()" title="Detener">
                    <i data-lucide="square" style="width:11px;height:11px;"></i> Stop
                </button>
                <button class="ai-send-btn" id="ai-send-btn" onclick="aiSend()" title="Enviar (Enter)">
                    <i data-lucide="send" style="width:16px;height:16px;"></i>
                </button>
            </div>
        </div>
        <div class="ai-footer-hint">
            <span>Enter para enviar</span>
            <span style="color:#3a3a3a;">&#xB7;</span>
            <span>Shift+Enter nueva linea</span>
        </div>
    </div>
</div>

<div class="toast-container" id="toastContainer"></div>
</body>
</html>
)rawliteral";
