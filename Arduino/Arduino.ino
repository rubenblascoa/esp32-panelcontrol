MIT License

Copyright (c) 2026 Ruben Blasco Armengod

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

---
// ============================================================================
// CARGAR BIBLIOTECAS
// ============================================================================
#include <Arduino.h>              // Núcleo del entorno Arduino para ESP32
#include <WiFi.h>                 // Gestión de conexiones de red inalámbricas
#include <ESPAsyncWebServer.h>    // Motor del servidor web asíncrono (no bloqueante)
#include <LittleFS.h>             // Sistema de archivos interno de la memoria flash
#include <ArduinoJson.h>          // Serialización y deserialización de datos estructurados (Telemetría)
#include <ArduinoOTA.h>           // Actualizaciones de firmware a través de WiFi (Over-The-Air)
#include <SPI.h>                  // Bus de comunicación hardware (necesario para RFID)
#include <MFRC522.h>              // Controlador del módulo lector NFC/RFID
#include "time.h"                 // Gestión del reloj y cálculo de tiempos (Uptime)

// ============================================================================
// 1. CONFIGURACIÓN Y PINES
// ============================================================================
const char* ssid = "";     // Credenciales de la red local
const char* password = "";

#define LED_PIN   2               // Pin del LED de estado de la placa base

#define RST_PIN   21              // Pin de reinicio (Reset) del módulo MFRC522
#define SS_PIN    5               // Pin de selección de esclavo (SDA/CS) del módulo MFRC522

#define TRIG_PIN  25              // Pin emisor de pulsos ultrasónicos
#define ECHO_PIN  33              // Pin receptor de rebotes ultrasónicos

// ============================================================================
// 2. OBJETOS Y VARIABLES GLOBALES
// ============================================================================
AsyncWebServer server(80);        // Instancia del servidor HTTP en el puerto por defecto
AsyncWebSocket ws("/ws");         // Instancia del túnel de comunicación bidireccional en tiempo real

WiFiServer telnetServer(23);      // Instancia del servidor TCP clásico para conexiones por terminal (Putty)
WiFiClient telnetClient;          // Objeto que representa la sesión del cliente TCP activo

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instancia del controlador de radiofrecuencia vinculada a sus pines
MFRC522::MIFARE_Key key;          // Estructura de memoria para almacenar las claves de autenticación MIFARE

String entradaWeb = "";           // Búfer temporal para comandos de texto recibidos vía web
bool hayEntradaWeb = false;       // Señal (flag) de sincronización entre hilos para procesar comandos web

const int MAX_WEBSOCKET_CLIENTS = 2; // Protección contra desbordamiento de memoria (OOM) limitando conexiones concurrentes

// ----------------------------------------------------------------------------
// CLASE TERMINAL HÍBRIDA: Redirecciona el flujo de impresión (Print)
// ----------------------------------------------------------------------------
class TerminalHibrida : public Print { 
public:
  size_t write(uint8_t c) override {   
    if (telnetClient && telnetClient.connected()) telnetClient.write(c); // Emisión hacia el socket TCP
    if (ws.count() > 0) { char buf[2] = { (char)c, '\0' }; ws.textAll(buf); } // Emisión empaquetada hacia el túnel web
    return 1;
  }
  size_t write(const uint8_t *buffer, size_t size) override { 
    if (telnetClient && telnetClient.connected()) telnetClient.write(buffer, size); 
    if (ws.count() > 0) {              
        String str = "";               
        for(size_t i=0; i<size; i++) str += (char)buffer[i];
        ws.textAll(str);
    }
    return size;
  }
};
TerminalHibrida Terminal;         // Objeto global instanciado para reemplazar los Serial.print convencionales

// ----------------------------------------------------------------------------
// VARIABLES DE LÓGICA DE ESTADO (Máquinas de estado de los módulos)
// ----------------------------------------------------------------------------
byte bloque0Habitacion[16] = {0x60, 0xC9, 0x13, 0x1C, 0xA6, 0x08, 0x04, 0x00, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69}; // Matriz de datos de respaldo
byte bloqueEscaneado[16];         // Matriz temporal para la extracción de datos físicos
byte bloqueManual[16];            
byte *bloqueAEscribir;            // Puntero de asignación dinámica hacia la fuente de datos a clonar
int programaActivo = 0;           // Puntero de estado principal (Máquina de estados finitos)
int modoNFC = 0;                  // Puntero de estado secundario (Subrutinas del módulo RFID)
bool memoriaLlena = false;        // Validación de integridad de datos extraídos
bool midiendoDistancia = false;   // Control del ciclo de ejecución del módulo acústico
bool ledState = LOW;              // Registro de la alternancia del estado eléctrico del LED
unsigned long tiempoUltimoMenuPrincipal = 0; // Registros temporales para gestión de eventos no bloqueantes (millis)
unsigned long previousMillisLED = 0;
unsigned long tiempoUltimaMedicion = 0;
unsigned long tiempoUltimoMenuNFC = 0;
const unsigned long intervaloMenu = 30000;   // Umbral de inactividad (30s) para repintado automático de interfaz
const unsigned long intervaloMedicion = 1000;// Frecuencia de muestreo (1Hz) del sensor de distancia

// ============================================================================
// PANEL WEB INCRUSTADO EN MEMORIA FLASH (PROGMEM)
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
        <a href="https://github.com/rubenblascoa" target="_blank" class="btn-back" style="color: var(--text-main); opacity: 0.7; display: flex; align-items: center; gap: 8px; transition: 0.3s;" onmouseover="this.style.opacity='1'; this.style.color='var(--accent)'" onmouseout="this.style.opacity='0.7'; this.style.color='var(--text-main)'">
			      <div style="width: 22px; height: 22px; background-color: currentColor; -webkit-mask: url('https://icones.pro/wp-content/uploads/2021/06/icone-github-grise.png') center/contain no-repeat; mask: url('https://icones.pro/wp-content/uploads/2021/06/icone-github-grise.png') center/contain no-repeat;"></div> GITHUB
		    </a>
        
        <div id="nav-status">
            <div class="header-badge badge-offline" id="status-badge-nav">Conectando...</div>
        </div>

        <button class="theme-toggle" id="theme-btn" onclick="toggleTheme()" title="Cambiar Tema"><i data-lucide="sun"></i></button>
    </nav>

    <div class="master-wrap">
        <div class="header-box">
            <div class="header-badge badge-offline" id="status-badge">Conectando...</div>
            
            <div class="brand-container">
                <img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="Chip ESP32">
                <h1>ESP32 BLASCO</h1>
            </div>
            
            <p class="header-subtitle">Panel de Control Multimódulo</p>
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
            </div>
        </section>

        <section>
            <div class="section-title">Historial de Rendimiento</div>
            <div class="chart-box"><div class="chart-h"><canvas id="mainChart"></canvas></div></div>
        </section>

        <section>
            <div class="section-title">Consola de Control Directa (192.168.18.192)</div>
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
    </footer>

<script>
lucide.createIcons();

// LOGICA DE LA BARRA ADHESIVA AL HACER SCROLL
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
            { label: 'WiFi', data: [], borderColor: '#00ff88', tension: 0.3, pointRadius: 0 }
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
                        return 'Hora exacta: ' + tooltipItems[0].label;
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
    if(t==='wifi') return v>70 ? '#00ff88' : (v>40 ? '#ffcc00' : '#ff4b4b'); 
    return v<60 ? '#00ff88' : (v<85 ? '#ffcc00' : '#ff4b4b'); 
}

function setStatusBadge(status) {
    const badge = document.getElementById('status-badge');
    const badgeNav = document.getElementById('status-badge-nav');
    
    const update = (el) => {
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

let gateway = `ws://192.168.18.192/ws`;
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

function actualizarDashboard(d) {
    let wifiQ = Math.min(Math.max(2 * (d.wifi + 100), 0), 100);
    
    const map = { 'v-t': d.temp+'°', 'v-c':d.cpu+'%', 'v-c0':d.c0+'%', 'v-c1':d.c1+'%', 'v-r':d.ram+'%', 'v-f':d.flash+'%', 'v-w':wifiQ+'%', 'v-u':d.uptime };
    for(let id in map) { let el = document.getElementById(id); if(el) el.innerText = map[id]; }

    const barValues = { 't':d.temp, 'c':d.cpu, 'c0':d.c0, 'c1':d.c1, 'r':d.ram, 'f':d.flash, 'w':wifiQ };
    for(let key in barValues) {
        let val = barValues[key];
        let type = key === 't' ? 'temp' : (key === 'w' ? 'wifi' : 'perc');
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

	const now = new Date().toLocaleTimeString([], {hour: '2-digit', minute:'2-digit', second:'2-digit'});
    
    // Histórico ampliado a 2000 puntos (aprox. 3 horas de datos)
    if(chart.data.labels.length > 2000) { 
        chart.data.labels.shift(); 
        chart.data.datasets.forEach(s => s.data.shift()); 
    }
    
    chart.data.labels.push(now);
    chart.data.datasets[0].data.push(d.temp);
    chart.data.datasets[1].data.push(d.cpu);
    chart.data.datasets[2].data.push(d.c0);
    chart.data.datasets[3].data.push(d.c1);
    chart.data.datasets[4].data.push(d.ram);
    chart.data.datasets[5].data.push(d.flash);
    chart.data.datasets[6].data.push(wifiQ);
    chart.update('none');
	
	}

window.addEventListener('load', initWebSocket);

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
</script>
</body>
</html>

)rawliteral";

// ============================================================================
// 3. FUNCIONES DE APOYO Y UTILIDAD
// ============================================================================
String obtenerUptime() {          // Cálculo del ciclo de actividad ininterrumpida del microcontrolador
  uint32_t sec = millis() / 1000; 
  char buff[20];                  
  sprintf(buff, "%02d:%02d:%02d", (sec/3600), (sec%3600)/60, sec%60); 
  return String(buff);            
}

// Declaraciones anticipadas (Forward declarations) para resolución del compilador
void mostrarMenuPrincipal();      
void procesarMenuPrincipal(String entrada);
void mostrarMenuNFC();
void procesarEntradaNFC(String entrada);
void modoLecturaNFC();
void modoEscrituraNFC();          
void mostrarMenuUltrasonidos();   
long medirDistanciaFisica();

// ============================================================================
// 4. EVENTOS Y FUNCIONES WEB
// ============================================================================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) { 
  if (type == WS_EVT_CONNECT) {   // Manejador de negociación inicial (Handshake)
    if (ws.count() > MAX_WEBSOCKET_CLIENTS) { // Control de concurrencia y protección de recursos
        client->close();          
        return;
    }
    Terminal.println("\n[SISTEMA] Dispositivo conectado al SO Blasco mediante Web-Telnet.");
    mostrarMenuPrincipal();
  } else if (type == WS_EVT_DATA) { // Manejador de recepción de tramas de datos
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) { // Validación de trama de texto única y completa
      data[len] = 0;              // Terminación nula obligatoria para conversión segura a cadena (C-string)
      entradaWeb = String((char*)data); 
      entradaWeb.trim();          // Saneamiento de la cadena entrante
      if (entradaWeb == "reboot") ESP.restart(); // Interrupción por software para reinicio en caliente
      else hayEntradaWeb = true;  // Elevación de bandera para delegar el procesamiento al Núcleo 1
    }
  }
}

void iniciarServidorWeb() {       // Configuración de enrutamiento y asignación de manejadores del servidor HTTP
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/html", index_html); // Transmisión directa desde la memoria de programa (Flash) para máxima velocidad
  });
  
  server.on("/esp32", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/html", index_html);       
  });
  
  server.serveStatic("/assets", LittleFS, "/assets"); // Exposición de directorio de recursos estáticos

  ws.onEvent(onWsEvent);          // Vinculación del bus de eventos del WebSocket
  server.addHandler(&ws);         // Integración del protocolo WS en la pila HTTP
  server.begin();                 // Inicialización del servicio de escucha en red
}

// ============================================================================
// 5. TAREAS MULTINÚCLEO (FreeRTOS)
// ============================================================================
void taskCore0(void * pvParameters) { // HILO DE EJECUCIÓN (CORE 0): Gestión de Red, Protocolos y Telemetría
  if(!LittleFS.begin(true)) {     // Inicialización del sistema de archivos con formato automático en caso de fallo
    Serial.println("LittleFS Fail");
    vTaskDelete(NULL);            // Destrucción segura del hilo en caso de error crítico de hardware
  }

  iniciarServidorWeb();
  telnetServer.begin();

  unsigned long lastTelemetryTime = 0;
  for(;;) {                       // Ciclo infinito del despachador de red
    ArduinoOTA.handle();          // Sondeo activo de peticiones de actualización de firmware
    ws.cleanupClients();          // Recolección de basura (Garbage collection) de sockets inactivos
    
    if (telnetServer.hasClient()) { // Administración de sesiones TCP entrantes
      if (telnetClient && telnetClient.connected()) telnetServer.available().stop(); // Rechazo de conexiones concurrentes no autorizadas
      else {                      
        telnetClient = telnetServer.available();
        Terminal.println("\n[SISTEMA] Conectado al SO Blasco mediante TCP.");
        mostrarMenuPrincipal();   
      }
    }

    if (ws.count() > 0) {         // Generación de métricas condicionada a la presencia de oyentes (Ahorro de ciclos de CPU)
        if (millis() - lastTelemetryTime >= 5000) { // Temporizador no bloqueante (5Hz)
            lastTelemetryTime = millis(); 
            StaticJsonDocument<256> doc; // Asignación de memoria estática en la pila (Stack) para máxima eficiencia
            doc["type"] = "telemetry";
            doc["temp"] = temperatureRead(); // Lectura del sensor de diodo térmico interno del silicio
            doc["cpu"] = random(8, 15);      // Valor de carga sintético
            doc["c0"] = 10;               
            doc["c1"] = (programaActivo == 0) ? 2 : 75; // Carga sintética dependiente del estado lógico
            doc["ram"] = (ESP.getFreeHeap() * 100) / ESP.getHeapSize(); // Cálculo de presión de memoria dinámica
            doc["flash"] = (LittleFS.usedBytes() * 100) / LittleFS.totalBytes(); // Cálculo de ocupación de almacenamiento no volátil
            doc["wifi"] = WiFi.RSSI();       // Nivel de intensidad de señal recibida (Radiofrecuencia)
            doc["uptime"] = obtenerUptime();
            
            String out;
            serializeJson(doc, out);         // Compilación del documento JSON en una cadena serializada
            ws.textAll(out);                 // Difusión (Broadcast) de la trama a todos los clientes del WebSocket
        }
    }

    if ((telnetClient && telnetClient.connected()) || ws.count() > 0) { // Señalización visual de actividad de red
      if (millis() - previousMillisLED >= 500) { 
        previousMillisLED = millis();
        ledState = !ledState;             
        digitalWrite(LED_PIN, ledState);  // Alternancia del estado del pin GPIO
      }
    } else {                              
      digitalWrite(LED_PIN, HIGH);        // Estado de reposo definido para el indicador
    }
    
    vTaskDelay(20 / portTICK_PERIOD_MS);  // Rendición de control al planificador (Scheduler) para prevenir inanición de otras tareas del SO
  }
}

void taskCore1(void * pvParameters) {     // HILO DE EJECUCIÓN (CORE 1): Lógica de Aplicación y Control de Hardware I/O
  for(;;) {                               // Ciclo infinito del procesador de eventos de hardware
    String entrada = "";
    
    if (telnetClient && telnetClient.connected() && telnetClient.available() > 0) { // Sondeo del búfer del socket TCP
      entrada = telnetClient.readStringUntil('\n'); // Extracción de comando delimitado por salto de línea
      entrada.trim();                     
    } else if (hayEntradaWeb) {           // Sincronización de variables modificadas por el Núcleo 0
      entrada = entradaWeb;
      hayEntradaWeb = false;              // Restablecimiento de la bandera de estado compartido
    }

    bool hayEspectadores = (telnetClient && telnetClient.connected()) || (ws.count() > 0);
    
    if (programaActivo == 0) {            
      if (millis() - tiempoUltimoMenuPrincipal >= intervaloMenu && hayEspectadores) mostrarMenuPrincipal(); 
      if (entrada.length() > 0) procesarMenuPrincipal(entrada); // Enrutamiento de la entrada hacia el motor de decisiones
    } else if (programaActivo == 1) {     
      if (modoNFC == 0 && (millis() - tiempoUltimoMenuNFC >= intervaloMenu) && hayEspectadores) mostrarMenuNFC(); 
      if (entrada.length() > 0) procesarEntradaNFC(entrada); 
      if (modoNFC == 1) modoLecturaNFC();   // Llamada continua a la subrutina de interrogación del bus SPI
      if (modoNFC == 4) modoEscrituraNFC(); // Llamada continua a la subrutina de modulación del bus SPI
    } else if (programaActivo == 2) {     
      entrada.toUpperCase();
      if (entrada == "M") { midiendoDistancia = false; programaActivo = 0; mostrarMenuPrincipal(); } 
      else if (entrada == "U") { midiendoDistancia = false; mostrarMenuUltrasonidos(); }             
      else if (entrada == "1") { midiendoDistancia = true; Terminal.println("\n[+] RADAR ACTIVADO..."); } 
      else if (entrada == "2") { midiendoDistancia = false; Terminal.println("\n[-] Radar detenido."); mostrarMenuUltrasonidos(); } 
      
      if (midiendoDistancia && (millis() - tiempoUltimaMedicion >= intervaloMedicion)) { 
        tiempoUltimaMedicion = millis();
        long d = medirDistanciaFisica();  // Llamada a la función bloqueante de medición de hardware
        if (d == -1) Terminal.println(" -> Distancia: FUERA DE RANGO");
        else { Terminal.print(" -> Distancia: "); Terminal.print(d); Terminal.println(" cm"); } 
      }
    }
    
    vTaskDelay(1 / portTICK_PERIOD_MS);   // Rendición crítica de 1 tick al planificador para alimentar al Watchdog (WDT) y evitar Kernel Panic
  }
}

// ============================================================================
// 6. FUNCIONES DE LÓGICA DE APLICACIÓN
// ============================================================================
void mostrarMenuPrincipal() {             
  Terminal.println("\n=================================================="); 
  Terminal.println("                ESP32 BLASCO ARMENGOD             ");
  Terminal.println("=================================================="); 
  Terminal.println(" [1] ESTACION DE CLONACION NFC (V14)");
  Terminal.println(" [2] PROYECTO ULTRASONIDOS (V3)");                    
  Terminal.println("--------------------------------------------------"); 
  Terminal.println(" Escribe el numero y pulsa ENVIAR.");                 
  tiempoUltimoMenuPrincipal = millis();
}
void procesarMenuPrincipal(String entrada) { 
  if (entrada == "1") { programaActivo = 1; modoNFC = 0; mostrarMenuNFC(); } // Transición de estado en la máquina principal
  else if (entrada == "2") { programaActivo = 2; midiendoDistancia = false; mostrarMenuUltrasonidos(); } 
}
void mostrarMenuNFC() {                   
  Terminal.println("\n**************************************************"); 
  Terminal.println("          ESTACION DE CLONACION PRO V14           ");
  Terminal.println("**************************************************"); 
  Terminal.println(" [1] LEER Y GUARDAR");                                  
  Terminal.println(" [2] CLONAR");
  Terminal.println(" [M] SALIR AL MENU PRINCIPAL");                         
  Terminal.println("**************************************************"); 
  tiempoUltimoMenuNFC = millis();
}
void procesarEntradaNFC(String entrada) { 
  entrada.toUpperCase();
  if (entrada == "M") { programaActivo = 0; mostrarMenuPrincipal(); return; } 
  if (modoNFC == 0 && entrada == "1") { modoNFC = 1; Terminal.println("\n>>> ACERCA TARJETA ORIGINAL..."); } // Transición al modo de interrogación pasiva
  else if (modoNFC == 0 && entrada == "2") { 
    if(!memoriaLlena) { Terminal.println("[!] ERROR: Memoria vacia"); return; } // Validación de condiciones previas operativas
    bloqueAEscribir = bloqueEscaneado; modoNFC = 4; Terminal.println("\n>>> ACERCA TARJETA DESTINO..."); // Transición al modo de escritura activa con asignación de puntero
  }
}
void modoLecturaNFC() {                   
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return; // Salida rápida si no se detecta campo de proximidad (PICC)
  byte buffer[18];
  byte size = sizeof(buffer); 
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522.uid)); // Proceso de autenticación criptográfica del sector base
  if (status == MFRC522::STATUS_OK) {     
    status = mfrc522.MIFARE_Read(0, buffer, &size); // Ejecución del comando de lectura de bloque físico
    if (status == MFRC522::STATUS_OK) {   
      for (byte j = 0; j < 16; j++) bloqueEscaneado[j] = buffer[j]; // Copia de memoria segura del búfer hacia la matriz estática
      memoriaLlena = true; Terminal.println("\n[EXITO] Bloque 0 guardado.");
    }
  }
  mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1(); // Secuencia de terminación de comunicación de bajo nivel
  delay(2000); modoNFC = 0; mostrarMenuNFC(); // Retardo bloqueante local para prevenir lecturas fantasmas repetitivas
}
void modoEscrituraNFC() {                 
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) return; 
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522.uid)); 
  if (status == MFRC522::STATUS_OK) {     
    status = mfrc522.MIFARE_Write(0, bloqueAEscribir, 16); // Ejecución del comando de modulación de escritura de bloque físico
    if (status == MFRC522::STATUS_OK) Terminal.println("\n[EXITO] Tarjeta Clonada.");
    else Terminal.println("\n[ERROR] Tarjeta no grabable.");
  }
  mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1();
  delay(3000); modoNFC = 0; mostrarMenuNFC();
}
void mostrarMenuUltrasonidos() {          
  Terminal.println("\n=================================================="); 
  Terminal.println("             PROYECTO 2: ULTRASONIDOS             ");
  Terminal.println("=================================================="); 
  Terminal.println(" [1] INICIAR RADAR | [2] DETENER | [M] SALIR");       
  Terminal.println("=================================================="); 
}
long medirDistanciaFisica() {             // Ejecución de la secuencia de temporización precisa del transductor
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);   // Estabilización de la línea a potencial cero
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10); // Generación del pulso de excitación del cristal piezoeléctrico
  digitalWrite(TRIG_PIN, LOW);                         
  long duracion = pulseIn(ECHO_PIN, HIGH, 30000);      // Medición bloqueante del ancho del pulso de retorno (Timeout: 30ms)
  if (duracion == 0) return -1;                        // Detección de condición de pérdida de eco (Out of bounds)
  return duracion * 0.034 / 2;                         // Conversión escalar de microsegundos a espacio euclidiano (Velocidad del sonido / 2)
}

// ============================================================================
// 7. SETUP ARDUINO PRINCIPAL (Punto de entrada de hardware)
// ============================================================================
void setup() {                            
  Serial.begin(115200);                   // Inicialización del controlador UART por hardware
  pinMode(LED_PIN, OUTPUT);               // Asignación de multiplexión de pines GPIO
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);             // Inicialización de la pila TCP/IP y del radio de banda base
  while (WiFi.status() != WL_CONNECTED) { // Lazo de bloqueo a la espera de la asignación de DHCP
    delay(500);
    Serial.print(".");                    
  }
  
  ArduinoOTA.begin();                     // Habilitación del servicio de escucha de puerto UDP para flasheo remoto
  SPI.begin();                            // Activación y enrutamiento del bus maestro SPI periférico
  mfrc522.PCD_Init();                     // Secuencia de encendido y configuración de registros internos del IC RFID
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF; // Carga de la estructura del sector con la clave de fabricante FFFFFFFFFFFF

  // DESPLIEGUE DEL SISTEMA OPERATIVO EN TIEMPO REAL (RTOS)
  xTaskCreatePinnedToCore(taskCore0, "RedTask", 10000, NULL, 1, NULL, 0); // Asignación del hilo de red al procesador de protocolo (PRO_CPU)
  xTaskCreatePinnedToCore(taskCore1, "AppTask", 10000, NULL, 1, NULL, 1); // Asignación del hilo de aplicación al procesador de aplicación (APP_CPU)
}

void loop() {                             
  // El ciclo de ejecución maestro nativo queda inoperante. 
  // El control ha sido delegado completamente al planificador de tareas de FreeRTOS.
}
