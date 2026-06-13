// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod
//
// This file is auto-extracted from web_pages.cpp.
#pragma once

#include <Arduino.h>

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
            display: flex; align-items: center; gap: 10px;
            padding: 14px 20px; border-radius: 12px;
            font-size: 0.82rem; font-weight: 600;
            border: 1px solid; backdrop-filter: blur(10px);
            min-width: 280px; max-width: 420px;
            transform: translateX(120%); opacity: 0;
            transition: transform 0.4s cubic-bezier(0.22,1,0.36,1), opacity 0.4s ease;
            box-shadow: 0 8px 32px rgba(0,0,0,0.4);
            pointer-events: auto;
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
            .sys-grid { grid-template-columns: repeat(2, 1fr); }
            .config-card { padding: 24px; }
        }
        @media (max-width: 600px) {
            body { zoom: 1; }
            .back-nav { padding: 15px 5%; grid-template-columns: 1fr 1fr; }
            #nav-status { display: none; }
            .brand-container { flex-direction: column; gap: 10px; }
            .sys-grid { grid-template-columns: 1fr 1fr; }
            .card-footer { flex-direction: column; }
            .card-footer .btn-primary { width: 100%; justify-content: center; }
            .toast-container { bottom: 15px; right: 15px; left: 15px; }
            .toast { min-width: unset; }
            .config-card { padding: 18px; border-radius: 18px; }
            .master-wrap { width: 95%; }
            .pin-group { padding: 12px 14px; }
            .pin-group-row { flex-direction: column; }
            .pin-field { min-width: 100%; }
            .action-item { flex-direction: column; align-items: stretch; }
            .action-item .btn-sm { align-self: flex-end; }
            .modal-card { padding: 24px; margin: 0 10px; }
            .scan-item { flex-direction: column; align-items: flex-start; gap: 6px; }
            .user-profile span { display: none; }
        }
        @media (max-width: 480px) {
            .sys-grid { grid-template-columns: 1fr; }
            .btn-back span { display: none; }
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
    <a href="/dashboard" class="btn-back">
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

    <!-- ─────────── BLOQUE 2: SISTEMA DE SEGURIDAD ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="shield" style="color:var(--safe);"></i>
            Sistema de Seguridad
        </div>
        <div class="config-card">
            <div style="display:flex;flex-direction:column;gap:20px;">

                <!-- Toggle habilitado -->
                <div style="background:var(--term-bg);border:1px solid var(--border-color);border-radius:14px;padding:14px 18px;display:flex;align-items:center;gap:14px;">
                    <div style="display:flex;flex-direction:column;gap:3px;flex:1;">
                        <div style="font-size:0.78rem;font-weight:700;color:var(--text-main);">Monitorización de seguridad</div>
                        <div style="font-size:0.72rem;color:var(--text-muted);">Supervisa temperatura CPU, temperatura ambiente, uso de CPU, RAM y señal WiFi. Ante una anomalía desactiva módulos no críticos y notifica en todas las páginas.</div>
                    </div>
                    <label style="position:relative;display:inline-block;width:42px;height:24px;flex-shrink:0;">
                        <input type="checkbox" id="seguridad-enabled" onchange="seguridadToggle()" checked style="opacity:0;width:0;height:0;">
                        <span id="seguridad-toggle-track" style="position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:var(--safe);border-radius:24px;transition:0.3s;">
                            <span id="seguridad-toggle-thumb" style="position:absolute;content:'';height:18px;width:18px;left:3px;top:3px;background:#fff;border-radius:50%;transition:0.3s;transform:translateX(18px);"></span>
                        </span>
                    </label>
                </div>

                <!-- Estado actual -->
                <div style="background:var(--term-bg);border:1px solid var(--border-color);border-radius:14px;padding:14px 18px;display:flex;align-items:center;gap:12px;">
                    <div id="seguridad-status-dot" style="width:9px;height:9px;border-radius:50%;background:var(--text-muted);flex-shrink:0;"></div>
                    <div style="flex:1;min-width:0;">
                        <div style="font-size:0.78rem;font-weight:700;color:var(--text-main);" id="seguridad-status-text">Cargando...</div>
                        <div style="font-size:0.7rem;color:var(--text-muted);font-family:monospace;word-break:break-all;" id="seguridad-status-causa"></div>
                        <div style="font-size:0.65rem;color:var(--text-muted);font-family:monospace;margin-top:3px;" id="seguridad-status-medidas"></div>
                    </div>
                    <button class="btn-sm" style="margin-left:auto;" onclick="seguridadRefreshStatus()" id="seguridad-refresh-btn">
                        <i data-lucide="refresh-cw" style="width:13px;height:13px;"></i> Actualizar
                    </button>
                </div>

                <!-- Botón activación manual -->
                <button class="btn-primary" id="seguridad-manual-btn" onclick="seguridadToggleManual()" style="display:none;">
                    <i data-lucide="alert-triangle" style="width:14px;height:14px;"></i> <span id="seguridad-manual-btn-text">Activar seguridad manualmente</span>
                </button>

            </div>
        </div>
    </section>

    <!-- ─────────── BLOQUE 3: RED WiFi ─────────── -->
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

    <!-- ─────────── BLOQUE 4: CREDENCIALES WEB ─────────── -->
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

    <!-- ─────────── BLOQUE 5: PINES DE HARDWARE ─────────── -->
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

    <!-- ─────────── BLOQUE 6: BLASCO AI ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="bot" style="color:var(--accent);"></i>
            Blasco AI
        </div>
        <div class="config-card">
            <div style="display:flex;flex-direction:column;gap:20px;">

                <!-- API Key -->
                <div class="form-group">
                    <label class="form-label">
                        <i data-lucide="key" style="width:14px;height:14px;"></i>
                        Gemini API Key
                    </label>
                    <div class="input-with-eye">
                        <input type="password" class="form-input" id="ai-cfg-key"
                            placeholder="AIzaSy..."
                            oninput="aiCfgDirty()">
                        <button class="eye-btn" onclick="toggleAiKeyVisibility()" title="Mostrar/Ocultar">
                            <i data-lucide="eye" id="ai-eye-icon"></i>
                        </button>
                    </div>
                    <p class="form-hint">Obten tu clave en <a href="https://aistudio.google.com/app/apikey" target="_blank" style="color:var(--accent);">aistudio.google.com</a>. Se guarda en la memoria del ESP32 y no se pierde al cerrar el navegador.</p>
                </div>

                <!-- Modelo -->
                <div class="form-group">
                    <label class="form-label">
                        <i data-lucide="cpu" style="width:14px;height:14px;"></i>
                        Modelo Gemini
                    </label>
                    <select class="form-input" id="ai-cfg-model" onchange="aiCfgDirty()">
                        <option value="gemini-2.5-flash">gemini-2.5-flash — Nueva generación, rápido y potente (Recomendado)</option>
                        <option value="gemini-2.0-flash">gemini-2.0-flash — Anterior generación, equilibrado</option>
                        <option value="gemini-1.5-flash">gemini-1.5-flash — Estable</option>
                        <option value="gemini-1.5-pro">gemini-1.5-pro — Pro, más lento</option>
                        <option value="gemini-2.0-flash-lite">gemini-2.0-flash-lite — Ligero, mínimo</option>
                    </select>
                    <p class="form-hint">El modelo seleccionado se usa en el panel de chat del dashboard.</p>
                </div>

                <!-- Estado actual -->
                <div style="background:var(--term-bg);border:1px solid var(--border-color);border-radius:14px;padding:14px 18px;display:flex;align-items:center;gap:12px;">
                    <div id="ai-cfg-status-dot" style="width:9px;height:9px;border-radius:50%;background:var(--text-muted);flex-shrink:0;"></div>
                    <div>
                        <div style="font-size:0.78rem;font-weight:700;color:var(--text-main);" id="ai-cfg-status-text">Sin configurar</div>
                        <div style="font-size:0.7rem;color:var(--text-muted);" id="ai-cfg-status-model"></div>
                    </div>
                    <button class="btn-sm" style="margin-left:auto;" onclick="aiCfgTest()" id="ai-cfg-test-btn">
                        <i data-lucide="zap" style="width:13px;height:13px;"></i> Probar conexión
                    </button>
                </div>

                <!-- Guardar -->
                <div style="display:flex;justify-content:flex-end;">
                    <button class="btn-primary" id="ai-cfg-save-btn" onclick="aiCfgSave()">
                        <i data-lucide="save" style="width:14px;height:14px;"></i> GUARDAR CONFIGURACIÓN IA
                    </button>
                </div>
            </div>
        </div>
    </section>

    <!-- ─────────── BLOQUE 7: TÚNEL CLOUDFLARE WORKER ─────────── -->
    <section>
        <div class="section-title">
            <i data-lucide="globe" style="color:var(--accent));"></i>
            Túnel Remoto (Cloudflare Worker)
        </div>
        <div class="config-card">
            <div style="display:flex;flex-direction:column;gap:20px;">

                <!-- Worker Host -->
                <div class="form-group">
                    <label class="form-label">
                        <i data-lucide="server" style="width:14px;height:14px;"></i>
                        Host del Worker
                    </label>
                    <input type="text" class="form-input" id="tunnel-host"
                        placeholder="mi-worker.workers.dev"
                        oninput="tunnelCfgDirty()">
                    <p class="form-hint">Dominio del Cloudflare Worker que actúa de relay. Puede ser <code style="color:var(--accent);font-family:monospace;">*.workers.dev</code> (gratis) o un dominio propio.</p>
                </div>

                <!-- Worker Path -->
                <div class="form-group">
                    <label class="form-label">
                        <i data-lucide="route" style="width:14px;height:14px;"></i>
                        Path del endpoint WebSocket
                    </label>
                    <input type="text" class="form-input" id="tunnel-path"
                        placeholder="/esp-tunnel"
                        oninput="tunnelCfgDirty()">
                    <p class="form-hint">Ruta del Worker que acepta la conexión WebSocket del ESP32. Por defecto <code style="color:var(--accent);font-family:monospace;">/esp-tunnel</code>.</p>
                </div>

                <!-- Token secreto -->
                <div class="form-group">
                    <label class="form-label">
                        <i data-lucide="key-round" style="width:14px;height:14px;"></i>
                        Token secreto compartido
                    </label>
                    <div class="input-with-eye">
                        <input type="password" class="form-input" id="tunnel-token"
                            placeholder="token-secreto-largo"
                            oninput="tunnelCfgDirty()">
                        <button class="eye-btn" onclick="toggleTunnelTokenVisibility()" title="Mostrar/Ocultar">
                            <i data-lucide="eye" id="tunnel-eye-icon"></i>
                        </button>
                    </div>
                    <p class="form-hint">Cadena secreta que el ESP32 envía al Worker al conectar para autenticarse. Debe coincidir con la variable de entorno <code style="color:var(--accent);font-family:monospace;">ESP_TOKEN</code> del Worker.</p>
                </div>

                <!-- Toggle habilitado -->
                <div style="background:var(--term-bg);border:1px solid var(--border-color);border-radius:14px;padding:14px 18px;display:flex;align-items:center;gap:14px;">
                    <div style="display:flex;flex-direction:column;gap:3px;flex:1;">
                        <div style="font-size:0.78rem;font-weight:700;color:var(--text-main);">Activar túnel al arrancar</div>
                        <div style="font-size:0.72rem;color:var(--text-muted);">El ESP32 abrirá un WebSocket saliente al Worker tras conectar al WiFi. Sin port forwarding ni VPS.</div>
                    </div>
                    <label style="position:relative;display:inline-block;width:42px;height:24px;flex-shrink:0;">
                        <input type="checkbox" id="tunnel-enabled" onchange="tunnelCfgDirty()" style="opacity:0;width:0;height:0;">
                        <span id="tunnel-toggle-track" style="position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background:var(--border-color);border-radius:24px;transition:0.3s;">
                            <span id="tunnel-toggle-thumb" style="position:absolute;content:'';height:18px;width:18px;left:3px;top:3px;background:var(--text-muted);border-radius:50%;transition:0.3s;"></span>
                        </span>
                    </label>
                </div>

                <!-- Estado actual -->
                <div style="background:var(--term-bg);border:1px solid var(--border-color);border-radius:14px;padding:14px 18px;display:flex;align-items:center;gap:12px;">
                    <div id="tunnel-status-dot" style="width:9px;height:9px;border-radius:50%;background:var(--text-muted);flex-shrink:0;"></div>
                    <div style="flex:1;min-width:0;">
                        <div style="font-size:0.78rem;font-weight:700;color:var(--text-main);" id="tunnel-status-text">Sin configurar</div>
                        <div style="font-size:0.7rem;color:var(--text-muted);font-family:monospace;word-break:break-all;" id="tunnel-status-host"></div>
                    </div>
                    <button class="btn-sm" style="margin-left:auto;" onclick="tunnelRefreshStatus()" id="tunnel-refresh-btn">
                        <i data-lucide="refresh-cw" style="width:13px;height:13px;"></i> Actualizar
                    </button>
                </div>

                <!-- Guardar -->
                <div style="display:flex;justify-content:flex-end;gap:8px;flex-wrap:wrap;">
                    <button class="btn-primary" id="tunnel-save-btn" onclick="tunnelCfgSave()">
                        <i data-lucide="save" style="width:14px;height:14px;"></i> GUARDAR CONFIGURACIÓN TÚNEL
                    </button>
                </div>
            </div>
        </div>
    </section>

    <!-- ─────────── BLOQUE 8: ACCIONES DEL SISTEMA ─────────── -->
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
    if (!tc) return;
    const el = document.createElement('div');
    el.className = `toast toast-${tipo}`;
    el.innerHTML = `<i data-lucide="${iconMap[tipo]}" style="width:18px;height:18px;flex-shrink:0;"></i><span>${msg}</span>`;
    tc.appendChild(el);
    try { lucide.createIcons(); } catch(e) {}
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
    ws = new WebSocket(`${window.location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.hostname}/ws`);
    ws.onopen = () => {
        setStatusBadge('online');
        ws.send('__ready__');
    };
    ws.onclose = () => { setStatusBadge('offline'); setTimeout(initWebSocket, 4000); };
    ws.onmessage = (e) => {
        try {
            const d = JSON.parse(e.data);
            if (d.type === 'telemetry') {
                document.getElementById('si-temp').textContent   = d.temp   !== undefined ? d.temp.toFixed(1) + ' °C' : '—';
                document.getElementById('si-uptime').textContent = d.uptime || '—';
                const ramPct = d.ram !== undefined ? d.ram.toFixed(1) + ' %' : '—';
                document.getElementById('si-ram').textContent = ramPct;
            } else if (d.type === 'security_alert') {
                if (d.habilitado !== false) {
                    const lvl = d.nivel || 3;
                    const lvlS = {1:'INFO',2:'ADVERTENCIA',3:'CRITICO'}[lvl]||'CRITICO';
                    toast('⚠ [' + lvlS + '] ' + (d.causa || 'Parametro critico'), lvl === 3 ? 'err' : 'warn');
                }
                _seguridadManual = d.manual || false;
                _seguridadAlerta = true;
                _seguridadNivel = d.nivel || 3;
                _seguridadMedidas = d.medidas || [];
                seguridadRenderStatus(true, true, _seguridadManual, d.causa || '');
            } else if (d.type === 'security_ok') {
                if (!d.desactivado) toast('✓ Seguridad: parametros normalizados', 'ok');
                _seguridadManual = false;
                _seguridadAlerta = false;
                _seguridadNivel = 0;
                _seguridadMedidas = [];
                seguridadRenderStatus(true, false, false, '');
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
            redes = redes.filter(r => r.ssid && r.ssid.trim().length > 0);
            redes.sort((a, b) => b.rssi - a.rssi);
            if (!redes.length) {
                list.innerHTML = '<div style="color:var(--text-muted);font-size:0.85rem;padding:10px;">No se encontraron redes.</div>';
                return;
            }
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
    aiCfgInit();
    tunnelCfgInit();
    seguridadRefreshStatus();
});

// ── BLASCO AI CONFIG ─────────────────────────────────────────────
function aiCfgInit() {
    fetch('/api/ai/key').then(r => r.json()).then(data => {
        aiKey = data.key || '';
        const keyEl = document.getElementById('ai-cfg-key');
        if (keyEl) keyEl.value = aiKey;
        aiCfgRefreshStatus();
    }).catch(() => { aiCfgRefreshStatus(); });
    const model = localStorage.getItem('blasco_ai_model') || 'gemini-2.5-flash';
    const modelEl = document.getElementById('ai-cfg-model');
    if (modelEl) modelEl.value = model;
}

function aiCfgDirty() {
    const btn = document.getElementById('ai-cfg-save-btn');
    if (btn) btn.style.outline = '2px solid var(--accent)';
}

function toggleAiKeyVisibility() {
    const inp  = document.getElementById('ai-cfg-key');
    const icon = document.getElementById('ai-eye-icon');
    if (!inp) return;
    const isHidden = inp.type === 'password';
    inp.type = isHidden ? 'text' : 'password';
    if (icon) { icon.dataset.lucide = isHidden ? 'eye-off' : 'eye'; lucide.createIcons(); }
}

function aiCfgRefreshStatus() {
    const dot   = document.getElementById('ai-cfg-status-dot');
    const txt   = document.getElementById('ai-cfg-status-text');
    const mdl   = document.getElementById('ai-cfg-status-model');
    if (!dot) return;
    if (aiKey) {
        dot.style.background    = 'var(--safe)';
        dot.style.boxShadow     = '0 0 8px var(--safe)';
        txt.textContent         = 'API Key configurada';
        mdl.textContent         = 'Modelo: ' + (document.getElementById('ai-cfg-model')?.value || 'gemini-2.5-flash');
    } else {
        dot.style.background    = 'var(--text-muted)';
        dot.style.boxShadow     = 'none';
        txt.textContent         = 'Sin configurar — introduce tu API Key';
        mdl.textContent         = '';
    }
}

function aiCfgSave() {
    const key   = (document.getElementById('ai-cfg-key')?.value   || '').trim();
    const model =  document.getElementById('ai-cfg-model')?.value  || 'gemini-2.5-flash';
    aiKey = key;
    localStorage.setItem('blasco_ai_model', model);
    fetch('/api/ai/key', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ key: key }) }).catch(() => {});
    const btn = document.getElementById('ai-cfg-save-btn');
    if (btn) btn.style.outline = 'none';
    aiCfgRefreshStatus();
    toast('Configuración de Blasco AI guardada', 'ok');
}

async function aiCfgTest() {
    const key   = (document.getElementById('ai-cfg-key')?.value || aiKey || '').trim();
    const model =  document.getElementById('ai-cfg-model')?.value || localStorage.getItem('blasco_ai_model') || 'gemini-2.5-flash';
    const btn   = document.getElementById('ai-cfg-test-btn');
    const dot   = document.getElementById('ai-cfg-status-dot');
    const txt   = document.getElementById('ai-cfg-status-text');
    const mdl   = document.getElementById('ai-cfg-status-model');

    if (!key) { toast('Introduce una API Key primero', 'warn'); return; }

    if (btn) btn.disabled = true;
    dot.style.background = 'var(--warn)';
    dot.style.boxShadow  = '0 0 8px var(--warn)';
    txt.textContent = 'Probando conexión...';
    mdl.textContent = '';

    try {
        const res = await fetch(
            `https://generativelanguage.googleapis.com/v1beta/models/${model}:generateContent?key=${key}`,
            {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    contents: [{ role: 'user', parts: [{ text: 'Responde solo con la palabra: OK' }] }],
                    generationConfig: { maxOutputTokens: 10 }
                })
            }
        );
        const data = await res.json();
        if (!res.ok) throw new Error(data.error?.message || 'HTTP ' + res.status);
        const reply = data.candidates?.[0]?.content?.parts?.[0]?.text || '?';
        dot.style.background = 'var(--safe)';
        dot.style.boxShadow  = '0 0 8px var(--safe)';
        txt.textContent = '✓ Conexión OK — respuesta: ' + reply.trim();
        mdl.textContent = 'Modelo: ' + model;
        toast('Blasco AI conectado correctamente', 'ok');
    } catch(err) {
        dot.style.background = 'var(--crit)';
        dot.style.boxShadow  = '0 0 8px var(--crit)';
        txt.textContent = '✗ Error: ' + err.message;
        mdl.textContent = '';
        toast('Error: ' + err.message, 'err');
    }
    if (btn) btn.disabled = false;
}

// ── TÚNEL CLOUDFLARE WORKER ──────────────────────────────────────
function tunnelCfgInit() {
    fetch('/api/tunnel/config').then(r => r.json()).then(data => {
        const host = document.getElementById('tunnel-host');
        const path = document.getElementById('tunnel-path');
        const tok  = document.getElementById('tunnel-token');
        const cb   = document.getElementById('tunnel-enabled');
        if (host) host.value = data.host || '';
        if (path) path.value = data.path || '/esp-tunnel';
        if (tok)  tok.value  = data.token || '';
        if (cb) {
            cb.checked = data.habilitado || false;
            tunnelSyncToggleUI(cb.checked);
        }
        tunnelRenderStatus(data.habilitado, data.conectado, data.host || '');
    }).catch(() => {});
}

function tunnelCfgDirty() {
    const btn = document.getElementById('tunnel-save-btn');
    if (btn) btn.style.outline = '2px solid var(--accent)';
    const cb = document.getElementById('tunnel-enabled');
    if (cb) tunnelSyncToggleUI(cb.checked);
}

function tunnelSyncToggleUI(active) {
    const track = document.getElementById('tunnel-toggle-track');
    const thumb = document.getElementById('tunnel-toggle-thumb');
    if (!track || !thumb) return;
    track.style.background = active ? 'var(--accent)' : 'var(--border-color)';
    thumb.style.background  = active ? '#fff' : 'var(--text-muted)';
    thumb.style.transform   = active ? 'translateX(18px)' : 'translateX(0)';
}

function toggleTunnelTokenVisibility() {
    const inp  = document.getElementById('tunnel-token');
    const icon = document.getElementById('tunnel-eye-icon');
    if (!inp) return;
    const isHidden = inp.type === 'password';
    inp.type = isHidden ? 'text' : 'password';
    if (icon) { icon.dataset.lucide = isHidden ? 'eye-off' : 'eye'; lucide.createIcons(); }
}

function tunnelRenderStatus(habilitado, conectado, host) {
    const dot = document.getElementById('tunnel-status-dot');
    const txt = document.getElementById('tunnel-status-text');
    const sub = document.getElementById('tunnel-status-host');
    if (!dot) return;
    if (!habilitado) {
        dot.style.background = 'var(--text-muted)';
        dot.style.boxShadow  = 'none';
        txt.textContent = 'Desactivado';
        sub.textContent = '';
    } else if (conectado) {
        dot.style.background = 'var(--safe)';
        dot.style.boxShadow  = '0 0 8px var(--safe)';
        txt.textContent = 'Túnel activo — acceso remoto disponible';
        sub.textContent = 'Worker: ' + host;
    } else if (habilitado && !conectado) {
        dot.style.background = 'var(--warn)';
        dot.style.boxShadow  = '0 0 8px var(--warn)';
        txt.textContent = 'Configurado — conectando al Worker...';
        sub.textContent = host ? 'Worker: ' + host : 'Guarda y reinicia para aplicar.';
    } else {
        dot.style.background = 'var(--text-muted)';
        dot.style.boxShadow  = 'none';
        txt.textContent = 'Sin configurar — introduce el host del Worker';
        sub.textContent = '';
    }
}

async function tunnelRefreshStatus() {
    const btn = document.getElementById('tunnel-refresh-btn');
    if (btn) btn.disabled = true;
    try {
        const res  = await fetch('/api/tunnel/status');
        const data = await res.json();
        tunnelRenderStatus(data.habilitado, data.conectado, data.host || '');
    } catch(e) {
        toast('Error al obtener estado del túnel', 'err');
    }
    if (btn) btn.disabled = false;
}

function tunnelCfgSave() {
    const host       = (document.getElementById('tunnel-host')?.value    || '').trim();
    const path       = (document.getElementById('tunnel-path')?.value    || '/esp-tunnel').trim();
    const token      = (document.getElementById('tunnel-token')?.value   || '').trim();
    const habilitado =  document.getElementById('tunnel-enabled')?.checked || false;
    if (habilitado && !host) { toast('Introduce el host del Worker antes de activar', 'warn'); return; }
    fetch('/api/tunnel/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ host, path, token, habilitado })
    })
    .then(r => r.json())
    .then(() => {
        const btn = document.getElementById('tunnel-save-btn');
        if (btn) btn.style.outline = 'none';
        tunnelRenderStatus(habilitado, false, host);
        toast('Configuraci\u00f3n del t\u00fanel guardada. Reinicia para aplicar.', 'ok');
    })
    .catch(() => toast('Error al guardar', 'err'));
}

// ── SISTEMA DE SEGURIDAD ─────────────────────────────────────────
function seguridadSyncToggleUI(active) {
    const track = document.getElementById('seguridad-toggle-track');
    const thumb = document.getElementById('seguridad-toggle-thumb');
    if (!track || !thumb) return;
    track.style.background = active ? 'var(--safe)' : 'var(--border-color)';
    thumb.style.background  = active ? '#fff' : 'var(--text-muted)';
    thumb.style.transform   = active ? 'translateX(18px)' : 'translateX(0)';
}

function seguridadRenderStatus(habilitado, modoAlerta, manual, causa) {
    _seguridadAlerta = modoAlerta;
    const dot = document.getElementById('seguridad-status-dot');
    const txt = document.getElementById('seguridad-status-text');
    const sub = document.getElementById('seguridad-status-causa');
    const med = document.getElementById('seguridad-status-medidas');
    const btn = document.getElementById('seguridad-manual-btn');
    const btnText = document.getElementById('seguridad-manual-btn-text');
    if (!dot) return;
    if (!habilitado) {
        dot.style.background = 'var(--text-muted)';
        dot.style.boxShadow  = 'none';
        if (txt) txt.textContent = 'Desactivado';
        if (sub) { sub.textContent = ''; sub.style.display = 'none'; }
        if (med) { med.textContent = ''; med.style.display = 'none'; }
        if (btn) btn.style.display = 'none';
    } else if (modoAlerta) {
        const colores = {1:'#ffaa00',2:'#ff8800',3:'#ff3333'};
        const lvlN = _seguridadNivel || 3;
        dot.style.background = colores[lvlN] || '#ff3333';
        dot.style.boxShadow  = '0 0 12px ' + (colores[lvlN] || '#ff3333');
        const nivelTxt = {1:'INFORMACION',2:'ADVERTENCIA',3:'CRITICO'}[lvlN]||'CRITICO';
        if (txt) txt.textContent = (manual ? '⚠ ALERTA MANUAL' : '⚠ ALERTA [' + nivelTxt + ']');
        if (sub) { sub.textContent = causa || 'Causa desconocida'; sub.style.display = ''; }
        if (med) {
            const arr = Array.isArray(_seguridadMedidas) ? _seguridadMedidas : [];
            med.textContent = arr.length ? 'Medidas: ' + arr.join(', ') : '';
            med.style.display = arr.length ? '' : 'none';
        }
        if (btn) {
            btn.style.display = 'flex';
            if (btnText) btnText.textContent = 'Desactivar seguridad';
            const i = btn.querySelector('i');
            if (i) i.setAttribute('data-lucide', 'shield-off');
        }
    } else {
        dot.style.background = 'var(--safe)';
        dot.style.boxShadow  = '0 0 8px var(--safe)';
        if (txt) txt.textContent = 'Monitorizando — todo normal';
        if (sub) sub.textContent = '';
        if (med) { med.textContent = ''; med.style.display = 'none'; }
        if (btn) {
            btn.style.display = 'flex';
            if (btnText) btnText.textContent = 'Activar seguridad manualmente';
            const i = btn.querySelector('i');
            if (i) i.setAttribute('data-lucide', 'alert-triangle');
        }
    }
    if (btn && typeof lucide !== 'undefined') lucide.createIcons();
}

async function seguridadRefreshStatus() {
    const btn = document.getElementById('seguridad-refresh-btn');
    if (btn) btn.disabled = true;
    try {
        const res = await fetch('/api/security/status');
        if (res.status === 401) { window.location.href = '/login-page'; return; }
        const ct = res.headers.get('content-type') || '';
        if (!res.ok || !ct.includes('application/json')) { window.location.href = '/login-page'; return; }
        const data = await res.json();
        _seguridadManual = data.manual || false;
        seguridadSyncToggleUI(data.habilitado);
        const cb = document.getElementById('seguridad-enabled');
        if (cb) cb.checked = data.habilitado;
        _seguridadNivel = data.modoAlerta ? (data.nivel || 3) : 0;
        const rawMedidas = data.modoAlerta ? (data.medidas || []) : [];
        _seguridadMedidas = Array.isArray(rawMedidas) ? rawMedidas : [];
        seguridadRenderStatus(data.habilitado, data.modoAlerta, _seguridadManual, data.causa || '');
        if (data.modoAlerta) {
            const lvl = data.nivel || 3;
            const lvlS = {1:'INFO',2:'ADVERTENCIA',3:'CRITICO'}[lvl]||'CRITICO';
            toast('⚠ [' + lvlS + '] ' + (data.causa || 'Parametro critico'), lvl === 3 ? 'err' : 'warn');
        }
    } catch(_) {}
    if (btn) btn.disabled = false;
}

async function seguridadToggle() {
    const habilitado = document.getElementById('seguridad-enabled').checked;
    seguridadSyncToggleUI(habilitado);
    try {
        const res = await fetch('/api/security/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ habilitado })
        });
        if (res.status === 401) { window.location.href = '/login-page'; return; }
        const ct = res.headers.get('content-type') || '';
        if (!ct.includes('application/json')) { window.location.href = '/login-page'; return; }
        const data = await res.json();
        if (data.status === 'ok') {
            toast('Sistema de seguridad ' + (habilitado ? 'ACTIVADO' : 'DESACTIVADO'), 'ok');
            seguridadRefreshStatus();
        }
    } catch(e) {
        toast('Error al cambiar estado de seguridad', 'err');
        document.getElementById('seguridad-enabled').checked = !habilitado;
        seguridadSyncToggleUI(!habilitado);
    }
}

var _seguridadManual = false;
var _seguridadAlerta = false;
var _seguridadNivel = 0;
var _seguridadMedidas = [];

async function seguridadToggleManual() {
    const btn = document.getElementById('seguridad-manual-btn');
    if (btn) btn.disabled = true;
    // [FIX] El estado a invertir es _seguridadManual, no _seguridadAlerta.
    // Usar _seguridadAlerta causaba que una alerta automática activa
    // desincronizara el toggle: el botón mostraría "Desactivar" pero
    // enviaría activar:false aunque la seguridad manual estuviera inactiva.
    const newState = !_seguridadManual;
    try {
        const res = await fetch('/api/security/manual', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ activar: newState })
        });
        if (res.status === 401) { window.location.href = '/login-page'; return; }
        const ct = res.headers.get('content-type') || '';
        if (!ct.includes('application/json')) { window.location.href = '/login-page'; return; }
        const data = await res.json();
        if (data.status === 'ok') {
            toast('Seguridad manual ' + (newState ? 'ACTIVADA' : 'DESACTIVADA'), newState ? 'warn' : 'ok');
            seguridadRefreshStatus();
        }
    } catch(e) {
        toast('Error al cambiar seguridad manual', 'err');
    }
    if (btn) btn.disabled = false;
}
</script>
</body>
</html>
)rawliteral";
