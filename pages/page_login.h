// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod
//
// This file is auto-extracted from web_pages.cpp.
#pragma once

#include <Arduino.h>

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
        window.lucide = window.lucide || { createIcons: function(){} };
        try { lucide.createIcons(); } catch(e) {}
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

        let gateway = `${window.location.protocol === 'https:' ? 'wss:' : 'ws:'}//${window.location.hostname}/ws`;
        let websocket;

        function initWebSocket() {
            setStatusBadge('connecting');
            websocket = new WebSocket(gateway);
            websocket.onopen = () => setStatusBadge('online');
            websocket.onclose = () => {
                setStatusBadge('offline');
                setTimeout(initWebSocket, 3000);
            };
            websocket.onmessage = (e) => {
                try { JSON.parse(e.data); } catch(_){}
            };
        }

        window.addEventListener('load', () => {
            initWebSocket();
        });

        // ── TOAST ──
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
            }, 4000);
        }
    </script>
<div class="toast-container" id="toastContainer"></div>
</body>
</html>
)rawliteral";
