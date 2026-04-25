// MIT License

// Copyright (c) 2026 Ruben Blasco Armengod

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// ============================================================================
// CARGAR BIBLIOTECAS
// ============================================================================
#include <Adafruit_NeoPixel.h>
#include <nvs_flash.h>    // Librería para formatear la partición oculta del WiFi
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
const char* ssid = "NETLLAR_BLASCO";     // Credenciales de la red local
const char* password = "978830522ro_";

#define LED_PIN   48               // Pin del LED de estado de la placa base
#define NUMPIXELS       1  // Solo hay un LED en la placa
Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define RST_PIN   21              // Pin de reinicio (Reset) del módulo MFRC522
#define SS_PIN    5               // Pin de selección de esclavo (SDA/CS) del módulo MFRC522

#define TRIG_PIN  15              // Pin emisor de pulsos ultrasónicos
#define ECHO_PIN  16              // Pin receptor de rebotes ultrasónicos

// ============================================================================
// 2. OBJETOS Y VARIABLES GLOBALES
// ============================================================================
AsyncWebServer* server;        // Instancia del servidor HTTP en el puerto por defecto
AsyncWebSocket* ws;         // Instancia del túnel de comunicación bidireccional en tiempo real

WiFiServer telnetServer(23);      // Instancia del servidor TCP clásico para conexiones por terminal (Putty)
WiFiClient telnetClient;          // Objeto que representa la sesión del cliente TCP activo

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instancia del controlador de radiofrecuencia vinculada a sus pines
MFRC522::MIFARE_Key key;          // Estructura de memoria para almacenar las claves de autenticación MIFARE

String entradaWeb = "";           // Búfer temporal para comandos de texto recibidos vía web
bool hayEntradaWeb = false;       // Señal (flag) de sincronización entre hilos para procesar comandos web

const int MAX_WEBSOCKET_CLIENTS = 2; // Protección contra desbordamiento de memoria (OOM) limitando conexiones concurrentes

bool permitirWebLog = false;         // Llave que deja pasar el texto a la web
unsigned long tiempoInicioWeb = 0;   // Cronómetro de los 30 segundos
bool sistemaListo = false;

// Variable global para almacenar el token de sesión activa
String tokenSesionActiva = ""; 

// Función para verificar si el usuario tiene la cookie de sesión correcta
bool estaLogueado(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    // Comprobamos si el token coincide y no está vacío
    if (tokenSesionActiva != "" && cookie.indexOf("ZENITH_SESSION=" + tokenSesionActiva) != -1) {
      return true;
    }
  }
  return false;
}

// ----------------------------------------------------------------------------
// CLASE TERMINAL HÍBRIDA (Con Agrupación en Bloque Antibloqueo)
// ----------------------------------------------------------------------------
class TerminalHibrida : public Print { 
  String bufferWeb = ""; 
  bool modoBloque = false; // Llave maestra para agrupar mensajes
public:
  void iniciarBloque() { modoBloque = true; bufferWeb = ""; } // Cierra el grifo temporalmente
  void enviarBloque() {  // Abre el grifo y lanza la bomba de texto
    if (permitirWebLog && ws->count() > 0 && bufferWeb.length() > 0) {
      ws->textAll(bufferWeb); 
    }
    bufferWeb = ""; 
    modoBloque = false; 
  }

  size_t write(uint8_t c) override {   
    if (telnetClient && telnetClient.connected()) telnetClient.write(c); // El Telnet (CMD) siempre es en directo
    
    if (permitirWebLog && ws->count() > 0) { 
        bufferWeb += (char)c;
        // Solo envía línea por línea si NO estamos en Modo Bloque
        if (!modoBloque && c == '\n') { 
            ws->textAll(bufferWeb);
            bufferWeb = ""; 
        }
    }
    return 1;
  }
  
  size_t write(const uint8_t *buffer, size_t size) override { 
    if (telnetClient && telnetClient.connected()) telnetClient.write(buffer, size);
    
    if (permitirWebLog && ws->count() > 0) {
        for(size_t i = 0; i < size; i++) {
            bufferWeb += (char)buffer[i];
            if (!modoBloque && buffer[i] == '\n') {
                ws->textAll(bufferWeb);
                bufferWeb = "";
            }
        }
        if (!modoBloque && bufferWeb.length() > 200) {
            ws->textAll(bufferWeb);
            bufferWeb = "";
        }
    }
    return size;
  }
};
TerminalHibrida Terminal;

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
const unsigned long intervaloMenu = 60000;   // Umbral de inactividad (30s) para repintado automático de interfaz
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
        width: 18px; /* Reducido de 22px */
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
            border: 1px solid var(--crit); /* Color rojo de tu paleta */
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
    
    // Si venimos del login exitoso, marcamos esta pestaña como autorizada
    if (params.get('auth') === 'true') {
        sessionStorage.setItem('zenith_active', '1');
        // Limpiamos la URL para que no se vea el ?auth=true
        window.history.replaceState({}, document.title, window.location.pathname);
    }

    // Si no hay marca en sessionStorage, significa que la pestaña se acaba de abrir/reabrir
    // Forzamos el logout para limpiar cookies persistentes del navegador
    if (!sessionStorage.getItem('zenith_active')) {
        window.location.href = '/logout';
    }
})();

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
    if(t==='wifi') return v>70 ? '#00ff88' : (v>40 ? '#ffcc00' : '#ff4b4b'); 
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

// -------------------------------------------------------------
// LÓGICA DE CARGA DE HISTORIAL (LittleFS)
// -------------------------------------------------------------
function cargarHistorialGrafica() {
    fetch('/datos.csv')
        .then(response => {
            if (!response.ok) throw new Error("Sin datos previos");
            return response.text();
        })
        .then(textoCSV => {
            const filas = textoCSV.split('\n');
            
            // Leemos de más antiguo a más nuevo
            for(let i = 0; i < filas.length; i++) {
                if(filas[i].trim() !== '') {
                    const cols = filas[i].split(',');
                    if(cols.length < 8) continue; // Filtro de seguridad
                    
                    chart.data.labels.push(cols[0]); // Uptime
                    chart.data.datasets[0].data.push(parseFloat(cols[1])); // Temp
                    chart.data.datasets[1].data.push(parseInt(cols[2]));   // CPU
                    chart.data.datasets[2].data.push(parseInt(cols[3]));   // C0
                    chart.data.datasets[3].data.push(parseInt(cols[4]));   // C1
                    chart.data.datasets[4].data.push(parseFloat(cols[5])); // RAM
                    chart.data.datasets[5].data.push(parseFloat(cols[6])); // Flash
                    chart.data.datasets[6].data.push(parseInt(cols[7]));   // WiFi
                }
            }
            chart.update();
        })
        .catch(err => console.log("Historial no cargado:", err));
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
    
    // Límite de seguridad en RAM (2000 puntos guardados en la gráfica)
    if(chart.data.labels.length > 2000) { 
        chart.data.labels.shift();
        chart.data.datasets.forEach(s => s.data.shift()); 
    }
    
    // Dibuja el nuevo punto sincronizado con Uptime
    chart.data.labels.push(d.uptime);
    chart.data.datasets[0].data.push(d.temp);
    chart.data.datasets[1].data.push(d.cpu);
    chart.data.datasets[2].data.push(d.c0);
    chart.data.datasets[3].data.push(d.c1);
    chart.data.datasets[4].data.push(d.ram);
    chart.data.datasets[5].data.push(d.flash);
    chart.data.datasets[6].data.push(wifiQ);
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

// ARRANQUE DOBLE: Cargar historial de Flash y enganchar WebSocket en vivo
window.addEventListener('load', async () => {
    // 1. Los círculos ya cargan por defecto al abrir el HTML.

    // 2. Cargamos el historial de la gráfica tras un breve respiro (500ms)
    setTimeout(() => {
        cargarHistorialGrafica(); 
    }, 500);

    // 3. Iniciamos el WebSocket (Telnet y Telemetría en vivo) al final (1.5 segundos)
    // Esto asegura que la gráfica antigua ya esté pintada antes de recibir datos nuevos.
    setTimeout(() => {
        initWebSocket();
    }, 1500);
});
</script>
</body>
</html>

)rawliteral";

// ============================================================================
// PANEL WEB 2: BASE DE DATOS INCRUSTADA EN FLASH
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
        th { padding: 15px 20px; color: var(--text-muted); font-weight: 800; font-size: 0.8rem; text-transform: uppercase; letter-spacing: 1px; border-bottom: 1px solid var(--border-color); }
        td { padding: 12px 20px; border-bottom: 1px solid var(--border-color); color: var(--text-main); font-family: 'Consolas', monospace; font-size: 0.95rem; font-weight: 600; }
        tr:last-child td { border-bottom: none; }
        tr:hover td { background-color: var(--term-bg); }

        .d-crit { color: var(--crit); text-shadow: 0 0 10px rgba(255,75,75,0.3); }
        .d-warn { color: var(--warn); }
        .d-ok { color: var(--safe); }
        .d-id { color: var(--text-muted); font-size: 0.8rem; }

        /* PAGINACIÓN Y BOTONES ACCIÓN */
        .pagination-wrapper { display: flex; justify-content: space-between; align-items: center; margin-top: 15px; padding: 0 5px; }
        .pagination { display: inline-flex; align-items: center; background: var(--card-bg); border: 1px solid var(--border-color); border-radius: 8px; overflow: hidden; box-shadow: var(--card-shadow); }
        .pagination button { background: transparent; border: none; border-right: 1px solid var(--border-color); color: var(--text-main); padding: 10px 16px; font-size: 0.9rem; cursor: pointer; display: inline-flex; align-items: center; gap: 5px; transition: 0.3s; font-family: 'Inter', sans-serif; font-weight: 600; }
        .pagination button:last-child { border-right: none; }
        .pagination button:hover:not(:disabled) { background: rgba(255,255,255,0.05); }
        body.light-theme .pagination button:hover:not(:disabled) { background: rgba(0,0,0,0.05); }
        
        /* Botón de página activa en BLANCO */
        .pagination button.active { background: #ffffff !important; color: #000000 !important; font-weight: 900; box-shadow: 0 0 10px rgba(255, 255, 255, 0.3); }
        
        .pagination button:disabled { color: var(--text-muted); cursor: not-allowed; opacity: 0.5; }
        .pagination svg { width: 16px; height: 16px; }

        /* Estilo para botones de acción pequeños */
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
            border: 1px solid var(--crit); /* Color rojo de tu paleta */
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

        /* ESTILOS PARA EL PERFIL DE USUARIO EN TOPBAR */
        .nav-right {
            justify-self: end;
            display: flex;
            align-items: center;
            gap: 20px;
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

        .user-profile img {
            width: 18px; /* Reducido de 22px */
            height: 18px;
            border-radius: 50%;
            filter: grayscale(1) opacity(0.7);
            border: 1px solid var(--border-color);
        }


        /* Estilos para que el modal funcione correctamente */
        .modal-overlay { 
            position: fixed; 
            top: 0; left: 0; 
            width: 100%; height: 100%; 
            background: rgba(0,0,0,0.85); 
            backdrop-filter: blur(10px); 
            display: none; /* Esto hace que el menú desaparezca hasta que pulses borrar */
            justify-content: center; 
            align-items: center; 
            z-index: 9999; 
        }

        .modal-card { 
            background: var(--card-bg); 
            border: 2px solid var(--border-color); 
            border-radius: 30px; 
            padding: 40px; 
            width: 90%; 
            max-width: 400px; 
            text-align: center; 
            box-shadow: 0 10px 30px rgba(0,0,0,0.5); 
        }

        .modal-btns { 
            display: flex; 
            gap: 15px; 
            justify-content: center; 
            margin-top: 25px; 
        }

        .modal-btn { 
            padding: 12px 25px; 
            border-radius: 12px; 
            font-weight: 800; 
            cursor: pointer; 
            border: none; 
            text-transform: uppercase; 
            flex: 1; 
            transition: 0.3s;
        }

        @media (max-width: 600px) { 
            body { zoom: 1; }
            .back-nav { padding: 15px 5%; grid-template-columns: 1fr 1fr; } 
            #nav-status { display: none; } 
            th, td { padding: 10px 15px; font-size: 0.85rem; }
            .pagination-wrapper { flex-direction: column; gap: 15px; justify-content: center; }
        }
    </style>
</head>

</div>

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
                                <th><i data-lucide="clock" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>Uptime</th>
                                <th><i data-lucide="thermometer" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>Temp (°C)</th>
                                <th><i data-lucide="cpu" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>CPU (%)</th>
                                <th><i data-lucide="activity" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>Core 0</th>
                                <th><i data-lucide="activity" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>Core 1</th>
                                <th><i data-lucide="database" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>RAM (%)</th>
                                <th><i data-lucide="hard-drive" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>Flash (%)</th>
                                <th><i data-lucide="wifi" style="width: 14px; margin-right: 5px; vertical-align: middle;"></i>WiFi (%)</th>
                            </tr>
                        </thead>
                        <tbody id="tablaCuerpo"></tbody>
                    </table>
                </div>
            </div>
            
        <div class="pagination-wrapper" style="display: flex; justify-content: space-between; align-items: center; margin-top: 15px;">
            <div style="display: flex; gap: 8px;">
                <a href="/datos.csv" download="historial.csv" class="btn-accion btn-exportar" style="border-color: var(--safe); color: var(--safe);">
                    <i data-lucide="download"></i> EXPORTAR
                </a>
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

// --- GUARDIA DE SESIÓN DE PESTAÑA ---
(function() {
    const params = new URLSearchParams(window.location.search);
    
    // Si venimos del login exitoso, marcamos esta pestaña como autorizada
    if (params.get('auth') === 'true') {
        sessionStorage.setItem('zenith_active', '1');
        // Limpiamos la URL para que no se vea el ?auth=true
        window.history.replaceState({}, document.title, window.location.pathname);
    }

    // Si no hay marca en sessionStorage, significa que la pestaña se acaba de abrir/reabrir
    // Forzamos el logout para limpiar cookies persistentes del navegador
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
                    if(cols.length < 8) continue; 
                    allDataRows.push({ id: id, cols: cols });
                    id--;
                }
            }
            if(allDataRows.length === 0) throw new Error("Sin datos");
            renderPage(1);
        })
        .catch(error => {
            document.getElementById('tablaCuerpo').innerHTML = `<tr><td colspan="9" style="text-align:center; padding: 30px; color: var(--text-main);"><i data-lucide="alert-circle" style="width: 24px; height: 24px; vertical-align: middle; margin-right: 10px;"></i>Sin datos registrados (Esperando al primer guardado)</td></tr>`;
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

    pageData.forEach(row => {
        const cols = row.cols;
        const fechaHora = cols[0]; // <--- Ahora contiene "DD/MM/YYYY HH:MM"
        const temp = parseFloat(cols[1]);
        const cpu = parseInt(cols[2]);
        const c0 = parseInt(cols[3]);
        const c1 = parseInt(cols[4]);
        const ram = parseFloat(cols[5]);
        const flash = parseFloat(cols[6]);
        const wifi = parseInt(cols[7]);

        let tempClass = temp >= 65 ? 'd-crit' : (temp >= 45 ? 'd-warn' : 'd-ok');
        let cpuClass = cpu >= 85 ? 'd-crit' : (cpu >= 60 ? 'd-warn' : 'd-ok');
        let ramClass = ram >= 80 ? 'd-crit' : (ram >= 50 ? 'd-warn' : 'd-ok');
        let wifiClass = wifi <= 40 ? 'd-crit' : (wifi <= 70 ? 'd-warn' : 'd-ok');

        tbody.innerHTML += `<tr>
            <td class="d-id">#${String(row.id).padStart(4, '0')}</td>
            <td>${fechaHora}</td>
            <td class="${tempClass}">${temp.toFixed(1)}</td>
            <td class="${cpuClass}">${cpu}</td>
            <td>${c0}</td>
            <td>${c1}</td>
            <td class="${ramClass}">${ram.toFixed(1)}</td>
            <td>${flash.toFixed(1)}</td>
            <td class="${wifiClass}">${wifi}</td>
        </tr>`;
    });

    renderPagination(totalPages, page);
}

// Función que se llama al pulsar el botón "BORRAR TODO"
function confirmarBorradoDB() {
    document.getElementById('confirmModalDB').style.display = 'flex';
}

// Cierra el modal sin hacer nada
function cerrarModalDB() {
    document.getElementById('confirmModalDB').style.display = 'none';
}

// Ejecuta la petición al ESP32 si el usuario confirma
function ejecutarBorradoFinal() {
    cerrarModalDB();
    fetch('/delete-db')
        .then(res => {
            if (res.ok) {
                // Recarga la página para mostrar la tabla vacía
                location.reload(); 
            }
        })
        .catch(err => console.error("Error al borrar:", err));
}

// LÓGICA REFINADA EXACTA: [Anterior] X Y Z [Siguiente]
function renderPagination(totalPages, currentPage) {
    const container = document.getElementById('pagination-container');
    if (totalPages <= 1) {
        container.innerHTML = '';
        return;
    }

    let html = '<div class="pagination">';
    
    // Botón Anterior
    html += `<button ${currentPage === 1 ? 'disabled' : ''} onclick="renderPage(${currentPage - 1})"><i data-lucide="chevron-left"></i> Anterior</button>`;

    // Calculamos la ventana dinámica de 3 números
    let startPage = currentPage - 1;
    let endPage = currentPage + 1;

    if (currentPage === 1) {
        startPage = 1;
        endPage = Math.min(totalPages, 3);
    } else if (currentPage === totalPages) {
        startPage = Math.max(1, totalPages - 2);
        endPage = totalPages;
    }

    // Dibujamos EXCLUSIVAMENTE esos 3 números
    for (let i = startPage; i <= endPage; i++) {
        html += `<button class="${i === currentPage ? 'active' : ''}" onclick="renderPage(${i})">${i}</button>`;
    }

    // Botón Siguiente
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
// PANEL WEB 3: LOGIN
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

        /* VARIABLES PARA TEMA CLARO */
        body.light-theme {
            --bg-color: #f3f4f6; --bg-gradient: radial-gradient(circle at 50% 0%, #ffffff 0%, #e5e7eb 100%);
            --card-bg: #ffffff; --border-color: #d1d5db; --text-main: #111827; --text-muted: #6b7280;
        }

        /* LAYOUT ESTRUCTURAL */
        html, body { height: 100%; margin: 0; padding: 0; overflow-x: hidden; }
        body { 
            font-family: 'Inter', sans-serif; background-color: var(--bg-color); background-image: var(--bg-gradient); 
            background-attachment: fixed; color: var(--text-main); display: flex; flex-direction: column; zoom: 0.9;
            transition: background-color 0.4s, color 0.4s;
        }

        /* TOP BAR */
        .back-nav { 
            position: sticky; top: 0; width: 100%; padding: 30px 5% 20px 5%; 
            display: grid; grid-template-columns: 1fr auto 1fr; 
            align-items: center; z-index: 1000; transition: all 0.4s ease; box-sizing: border-box; background: transparent; 
        }
        .back-nav.scrolled {
            padding: 15px 5%; background: rgba(13, 17, 23, 0.85); backdrop-filter: blur(10px);
            border-bottom: 1px solid var(--border-color);
        }
        body.light-theme .back-nav.scrolled { background: rgba(255, 255, 255, 0.85); }

        /* POSICIÓN DE ELEMENTOS EN TOP BAR */
        .nav-terminal { justify-self: start; display: flex; align-items: center; gap: 8px; color: var(--text-muted); font-weight: 800; font-size: 0.8rem; letter-spacing: 1px; }
        .nav-status { justify-self: center; opacity: 0; transform: translateY(-10px); transition: 0.3s ease; }
        .back-nav.scrolled .nav-status { opacity: 1; transform: translateY(0); }
        .nav-theme { justify-self: end; }

        /* BADGES */
        .header-badge { padding: 4px 12px; border-radius: 20px; font-size: 0.75rem; font-weight: 800; text-transform: uppercase; display: inline-flex; align-items: center; gap: 6px; }
        .badge-online { background: rgba(0, 255, 136, 0.1); color: var(--safe); border: 1px solid rgba(0, 255, 136, 0.3); }
        .badge-online::before { content: ''; width: 8px; height: 8px; background: var(--safe); border-radius: 50%; box-shadow: 0 0 10px var(--safe); animation: pulse 2s infinite;}
        .badge-offline { background: rgba(255, 75, 75, 0.1); color: var(--crit); border: 1px solid rgba(255, 75, 75, 0.3); }
        .badge-offline::before { content: ''; width: 8px; height: 8px; background: var(--crit); border-radius: 50%; box-shadow: 0 0 5px var(--crit); }
        .badge-connecting { background: rgba(255, 204, 0, 0.1); color: var(--warn); border: 1px solid rgba(255, 204, 0, 0.3); }
        .badge-connecting::before { content: ''; width: 8px; height: 8px; background: var(--warn); border-radius: 50%; box-shadow: 0 0 10px var(--warn); animation: pulse 1s infinite; }

        @keyframes pulse { 0% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0, 255, 136, 0.7); } 70% { transform: scale(1); box-shadow: 0 0 0 6px rgba(0, 255, 136, 0); } 100% { transform: scale(0.95); box-shadow: 0 0 0 0 rgba(0, 255, 136, 0); } }

        /* CUERPO CENTRAL */
        .master-wrap { 
            flex: 1; display: flex; flex-direction: column; align-items: center; justify-content: flex-start; 
            padding-top: 8vh; width: 95%; max-width: 1400px; align-self: center;
        }
        
        .header-box { text-align: center; margin-bottom: 30px; }
        .brand-container { display: flex; align-items: center; justify-content: center; gap: 15px; margin: 10px 0; }
        h1 { font-size: 3rem; font-weight: 900; letter-spacing: -1px; margin: 0; }
        .header-subtitle { color: var(--text-muted); font-size: 0.85rem; letter-spacing: 4px; text-transform: uppercase; font-weight: 600; }

        /* CARD DE LOGIN */
        .login-card { 
            background: var(--card-bg); border: 1px solid var(--border-color); border-radius: 35px; 
            padding: 50px 40px; width: 100%; max-width: 400px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); 
        }
        
        /* INPUTS */
        .input-group { text-align: left; margin-bottom: 25px; position: relative; }
        .input-group svg { position: absolute; left: 16px; top: 38px; color: var(--text-muted); width: 20px; height: 20px; transition: 0.3s; }
        label { display: block; margin-bottom: 10px; font-weight: 800; font-size: 0.75rem; text-transform: uppercase; color: var(--text-muted); padding-left: 5px; }
        input { 
            width: 100%; background: #000; border: 1px solid var(--border-color); border-radius: 12px; 
            padding: 14px 15px 14px 48px; color: #fff; font-family: 'Inter'; transition: 0.3s; box-sizing: border-box; font-size: 1rem;
        }
        body.light-theme input { background: #fff; color: #000; }
        
        /* SELECCIÓN DE CAMPOS EN BLANCO */
        input:focus { border-color: var(--text-main); outline: none; box-shadow: 0 0 15px rgba(255, 255, 255, 0.15); }
        input:focus + svg, .input-group svg:focus-within { color: var(--text-main); }
        
        /* BOTÓN TODO EN BLANCO */
        .btn-login { 
            width: 100%; background: transparent; color: var(--text-main); border: 2px solid var(--text-main); 
            padding: 16px; border-radius: 15px; font-weight: 900; font-size: 1rem; cursor: pointer; 
            text-transform: uppercase; transition: 0.3s; margin-top: 10px;
        }
        .btn-login:hover { background: var(--text-main); color: #000; box-shadow: 0 0 20px rgba(255, 255, 255, 0.4); transform: scale(1.02); }
        body.light-theme .btn-login:hover { color: #fff; }

        /* FOOTER (Copiado de las otras páginas) */
        .footer-zenith { width: 100%; background: var(--card-bg); padding: 40px 0; border-top: 1px solid var(--border-color); text-align: center; margin-top: auto; transition: 0.4s; }
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
        
        // Manejo de scroll para la Top Bar
        window.addEventListener('scroll', () => {
            const nav = document.getElementById('mainNav');
            if (window.scrollY > 20) {
                nav.classList.add('scrolled');
            } else {
                nav.classList.remove('scrolled');
            }
        });

        // TEMA FUNCIONAL
        function toggleTheme() {
            document.body.classList.toggle('light-theme');
            const isLight = document.body.classList.contains('light-theme');
            document.getElementById('theme-btn').innerHTML = isLight ? '<i data-lucide="moon"></i>' : '<i data-lucide="sun"></i>';
            lucide.createIcons();
        }

        // FUNCIÓN DE STATUS 
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

        // WEBSOCKET PARA SINCRONIZAR STATUS REAL
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
// 3. FUNCIONES DE APOYO Y UTILIDAD
// ============================================================================
String obtenerUptime() {          // Cálculo del ciclo de actividad ininterrumpida del microcontrolador
  uint32_t sec = millis() / 1000; 
  char buff[20];                  
  sprintf(buff, "%02d:%02d:%02d", (sec/3600), (sec%3600)/60, sec%60); 
  return String(buff);            
}

String obtenerFechaHora() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "00/00/00 00:00"; // Si falla el NTP, devuelve esto
  }
  char buff[25];
  // Formato: Día/Mes/Año Hora:Minuto
  strftime(buff, sizeof(buff), "%d/%m/%Y %H:%M", &timeinfo);
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
    if (ws->count() > MAX_WEBSOCKET_CLIENTS) { // Control de concurrencia y protección de recursos
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

void iniciarServidorWeb() {       
  
  // 1. RUTA DE LOGIN (FORMULARIO)
  server->on("/login-page", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/html", login_html); 
  });

  // 2. PROCESO DE LOGIN (CON REDIRECCIÓN ESPECIAL)
  server->on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
      String u = "", p = "";
      if(request->hasParam("user", true)) u = request->getParam("user", true)->value();
      if(request->hasParam("password", true)) p = request->getParam("password", true)->value();

      if(u == "admin" && p == "blasco") {
          tokenSesionActiva = String(random(100000, 999999)) + String(millis());
          AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
          
          // Redirigimos con un parámetro especial '?auth=true'
          response->addHeader("Location", "/?auth=true");
          response->addHeader("Set-Cookie", "ZENITH_SESSION=" + tokenSesionActiva + "; Path=/; HttpOnly; SameSite=Strict"); 
          request->send(response);
      } else {
          request->redirect("/login-page?error=1"); 
      }
  });

  // 3. RUTA DE LOGOUT (LIMPIEZA TOTAL)
  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      tokenSesionActiva = "";
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
      response->addHeader("Location", "/login-page");
      response->addHeader("Set-Cookie", "ZENITH_SESSION=deleted; Path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT"); 
      request->send(response);
  });

  // 4. RUTAS PROTEGIDAS
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; }
    request->send_P(200, "text/html", index_html); 
  });

  server->on("/db", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; }
    request->send_P(200, "text/html", db_html); 
  });

  // WebSocket y arranque
  ws->onEvent(onWsEvent);         
  server->addHandler(ws);         
  server->begin();
}

void guardarEnHistorial() {
    File file = LittleFS.open("/datos.csv", "a");
    if (!file) return;

    String fechaHora = obtenerFechaHora();

    String uptime = obtenerFechaHora();
    float temp = temperatureRead();
    int cpu = random(8, 15);
    int c0 = 10;
    int c1 = (programaActivo == 0) ? 2 : 75;
    
    uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize();
    float ramUsada = (ESP.getFreePsram() > 0) ? 100.0 - (ESP.getFreePsram() * 100.0 / ramTotal) : 100.0 - (ESP.getFreeHeap() * 100.0 / ESP.getHeapSize());
    
    float flashTotal = LittleFS.totalBytes();
    float flashUsado = LittleFS.usedBytes();
    float flash = (flashTotal > 0) ? round(((flashUsado * 100.0) / flashTotal) * 100.0) / 100.0 : 0.0;
    int wifi = min(max(2 * (WiFi.RSSI() + 100), 0), 100);

    file.printf("%s,%.1f,%d,%d,%d,%.1f,%.1f,%d\n", fechaHora.c_str(), temp, cpu, c0, c1, ramUsada, flash, wifi);    
    file.close();
}

// ============================================================================
// 5. TAREAS MULTINÚCLEO (FreeRTOS)
// ============================================================================
void taskCore0(void * pvParameters) { 
  // 1. Inicialización de archivos (Solo ocurre una vez al arrancar el hilo)
  if (!LittleFS.begin(true, "/spiffs", 10, "ffat")) { 
      if (!LittleFS.begin(true)) Serial.println("❌ [ERROR] Flash no montada");
      else Serial.println("✅ [OK] Flash montada en Modo Rescate");
  } else {
      Serial.println("✅ [OK] Flash montada correctamente");
  }

  // ====================================================================
  // SISTEMA DE AUTO-LIMPIEZA INTELIGENTE (Límite: 5 MB)
  // ====================================================================
  if (LittleFS.exists("/datos.csv")) {
      File dbFile = LittleFS.open("/datos.csv", "r");
      if (dbFile) {
          size_t fileSize = dbFile.size();
          dbFile.close(); // Cerramos el archivo antes de decidir si lo borramos

          // 5 MB en bytes = 5 * 1024 * 1024 = 5242880 bytes
          if (fileSize > 8388608) {
              LittleFS.remove("/datos.csv");
              Serial.println("🧹 [SISTEMA] Base de datos superó los 5MB. Limpieza automática ejecutada.");
          } else {
              Serial.printf("📊 [SISTEMA] Tamaño actual de la BD: %d KB\n", fileSize / 1024);
          }
      }
  }
  // ====================================================================


  iniciarServidorWeb();
  telnetServer.begin(); 

  unsigned long lastTelemetryTime = 0;

  for(;;) {                       
    // 🛡️ ESCUDO ANTI-CRASH: Si el setup() no ha terminado, esperamos fuera
    if (!sistemaListo) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        continue; 
    }

    // Mantenimiento de red y OTA
    ArduinoOTA.handle();          
    yield(); // Cede el paso al stack de WiFi para evitar micro-cortes
    
    // --- 1. GESTIÓN DEL TELNET ---
    if (telnetServer.hasClient()) { 
      if (telnetClient && telnetClient.connected()) {
          telnetServer.available().stop(); 
      } else {                      
        telnetClient = telnetServer.available();
        Terminal.println("\n[SISTEMA] Conectado al SO Blasco mediante TCP.");
        mostrarMenuPrincipal();   
      }
    }

    // --- 2. GESTIÓN DE LA PÁGINA WEB ---
    if (ws->count() > 0) { 
        if (tiempoInicioWeb == 0) tiempoInicioWeb = millis(); 
        
        // Abrir consola tras 30 segundos
        if (!permitirWebLog && (millis() - tiempoInicioWeb >= 30000)) {
            permitirWebLog = true; 
            mostrarMenuPrincipal(); 
        }

        // Envío de telemetría cada 5 segundos
        if (millis() - lastTelemetryTime >= 5000) { 
            lastTelemetryTime = millis(); 
            StaticJsonDocument<256> doc; 
            doc["type"] = "telemetry";
            doc["temp"] = temperatureRead(); 
            doc["cpu"] = random(8, 15);      
            doc["c0"] = 10;               
            doc["c1"] = (programaActivo == 0) ? 2 : 75; 
            
            uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize();
            doc["ram"] = (ESP.getFreePsram() > 0) ? 100 - (ESP.getFreePsram() * 100 / ramTotal) : 100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize());            
            
            float flashTotal = LittleFS.totalBytes();
            float flashUsado = LittleFS.usedBytes();
            doc["flash"] = (flashTotal > 0) ? round(((flashUsado * 100.0) / flashTotal) * 100.0) / 100.0 : 0.0;
            
            doc["wifi"] = WiFi.RSSI();       
            doc["uptime"] = obtenerUptime();
            
            String out;
            serializeJson(doc, out);         
            ws->textAll(out);   
            ws->cleanupClients();             
        }
    } else {
        tiempoInicioWeb = 0;
        permitirWebLog = false;
    }

    // --- 3. LÓGICA DEL LED RGB (Pin 48) ---
    // Comprobamos el estado del WiFi de forma segura
    wl_status_t wifiStatus = WiFi.status();

    if (wifiStatus != WL_CONNECTED) {
        // Si no hay WiFi, LED Apagado (puedes poner pixel.Color(10, 0, 0) para Rojo si quieres)
        pixel.setPixelColor(0, pixel.Color(0, 0, 0)); 
    } 
    else {
        // WiFi OK -> ¿Hay alguien conectado?
        bool alguienConectado = (telnetClient && telnetClient.connected()) || (ws->count() > 0);

        if (alguienConectado) { 
            // ACTIVIDAD -> AZUL PARPADEANTE
            if (millis() - previousMillisLED >= 500) { 
                previousMillisLED = millis();
                ledState = !ledState;             
                pixel.setPixelColor(0, ledState ? pixel.Color(0, 0, 10) : pixel.Color(0, 0, 0));
            }
        } else {                                      
            // REPOSO -> VERDE FIJO
            pixel.setPixelColor(0, pixel.Color(0, 10, 0)); 
            ledState = false; 
        }
    }
    pixel.show(); 


    // --- 4. Guardar Historial Base de Datos ---
    static unsigned long tiempoUltimoGuardadoBD = 0;
    if (millis() - tiempoUltimoGuardadoBD >= 1800000) { // Guarda cada 1 minuto (60000 ms)
        tiempoUltimoGuardadoBD = millis();
        guardarEnHistorial();
    }    
    
    vTaskDelay(1); // Mantiene el Watchdog feliz y el WiFi fluido
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

    bool hayEspectadores = (telnetClient && telnetClient.connected()) || (ws->count() > 0);
    
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

  Terminal.iniciarBloque(); // 👈 1. RETENEMOS LA WEB
  
  Terminal.println("\n==================================================");
  Terminal.println("                ESP32 BLASCO ARMENGOD             ");
  Terminal.println("==================================================");
  Terminal.println("\n SELECCIONA UN PROGRAMA:");
  Terminal.println(" [1] ESTACION DE CLONACION NFC (V14)");
  Terminal.println(" [2] PROYECTO ULTRASONIDOS (V3)");
  Terminal.println("\n--------------------------------------------------");
  Terminal.println("\n TELEMETRIA DEL SISTEMA:");
  
  // LÓGICA DE TELEMETRÍA
  float ramTotal = (ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize()) / 1024.0;
  float ramLibre = (ESP.getFreePsram() > 0 ? ESP.getFreePsram() : ESP.getFreeHeap()) / 1024.0;
  float ramUsada = ramTotal - ramLibre;

  float flashTotal = LittleFS.totalBytes() / 1024.0;
  float flashUsada = LittleFS.usedBytes() / 1024.0;

  // IMPRESIÓN DE TELEMETRÍA EN PANTALLA
  Terminal.print(" - Almacenamiento: "); 
  if(flashTotal > 0) {
      Terminal.print(flashUsada, 0); Terminal.print(" KB / "); Terminal.print(flashTotal, 0); Terminal.println(" KB");
  } else {
      Terminal.println("0 KB / 0 KB (No montado)");
  }
  
  Terminal.print(" - RAM Usada: "); Terminal.print(ramUsada, 0); Terminal.print(" KB / "); Terminal.print(ramTotal, 0); Terminal.println(" KB");
  Terminal.print(" - Temp Core: "); Terminal.print(temperatureRead(), 1); Terminal.println(" C");
  Terminal.print(" - Vel. CPU : "); Terminal.print(ESP.getCpuFreqMHz()); Terminal.println(" MHz");
  
  // AQUI ESTA EL ARREGLO DEL UPTIME:
  Terminal.print(" - Tiempo Activo : "); Terminal.println(obtenerUptime());
  Terminal.print(" - Direccion IP : "); Terminal.println(WiFi.localIP());

  Terminal.println("\n==================================================");
  Terminal.println("               Ruben Blasco Armengod              ");
  Terminal.println("==================================================");

  Terminal.enviarBloque();
  
  tiempoUltimoMenuPrincipal = millis(); 
}

void procesarMenuPrincipal(String entrada) { 
  if (entrada == "1") { programaActivo = 1; modoNFC = 0; mostrarMenuNFC(); } // Transición de estado en la máquina principal
  else if (entrada == "2") { programaActivo = 2; midiendoDistancia = false; mostrarMenuUltrasonidos(); } 
}

void mostrarMenuNFC() { 

  Terminal.iniciarBloque(); // 👈 1. RETENEMOS LA WEB

  Terminal.println("\n**************************************************");
  Terminal.println("          ESTACION DE CLONACION PRO V14           ");
  Terminal.println("**************************************************");
  Terminal.println(" [1] LEER Y GUARDAR: Radiografia completa + Memoria");
  Terminal.println(" [2] CLONAR: Elige que datos quieres copiar.");
  Terminal.println(" [N] VOLVER AL MENU NFC (A esta pantalla)");
  Terminal.println(" [M] SALIR AL MENU PRINCIPAL (ESP32 Blasco)");
  Terminal.println("--------------------------------------------------");
  
  // AQUI ESTA EL ARREGLO DE LA MEMORIA:
  if (memoriaLlena) { 
    Terminal.println(" >> MEMORIA: Tarjeta guardada en RAM [OK]");
  } else { 
    Terminal.println(" >> MEMORIA: Vacia (Usa la opcion 1)");
  }
  
  Terminal.println("**************************************************");

  Terminal.enviarBloque();

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

  Terminal.iniciarBloque(); // 👈 1. RETENEMOS LA WEB

  Terminal.println("\n==================================================");
  Terminal.println("             PROYECTO 2: ULTRASONIDOS             ");
  Terminal.println("==================================================");
  Terminal.println(" [1] INICIAR radar de distancia en tiempo real");
  Terminal.println(" [2] DETENER radar de distancia");
  Terminal.println(" [U] VOLVER AL MENU ULTRASONIDOS (A esta pantalla)"); 
  Terminal.println(" [M] VOLVER AL MENU PRINCIPAL (ESP32 Blasco)");
  Terminal.println("==================================================");

  Terminal.enviarBloque();

  tiempoUltimoMenuPrincipal = millis(); 
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
  Serial.begin(115200);
  delay(2000);

  Serial.printf("📦 Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("🧠 PSRAM: %d MB\n", ESP.getPsramSize() / (1024 * 1024)); 

  Serial.println("\n\n--- INICIANDO CHIVATO ---");
  Serial.flush(); // 👈 Obligamos a que el texto salga por el cable ya mismo

  Serial.println("[PASO 1] Creando Servidor en RAM...");
  Serial.flush();
  server = new AsyncWebServer(80);
  ws = new AsyncWebSocket("/ws");
  
  Serial.println("[PASO 2] Servidor OK. Configurando Pines...");
  Serial.flush();
  pinMode(LED_PIN, OUTPUT);               
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("[PASO 3] Pines OK. Encendiendo WiFi...");
  Serial.flush();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);  

  // Configurar servidor de tiempo (NTP) para España
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");           
  
  Serial.println("[PASO 4] Esperando conexion...");
  Serial.flush();
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print(".");                    
    Serial.flush();
  }
  
  Serial.println("\n[OK] WiFi Conectado!");
  Serial.print("[IP]: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin();                     
  SPI.begin(18, 13, 11, SS_PIN);
  mfrc522.PCD_Init();

  // Lanzamos las tareas con un pequeño retraso entre ellas
  Serial.println("[..] Iniciando núcleos de proceso...");
  xTaskCreatePinnedToCore(taskCore0, "RedTask", 8192, NULL, 1, NULL, 0);
  delay(500); 
  xTaskCreatePinnedToCore(taskCore1, "AppTask", 8192, NULL, 1, NULL, 1);

  Serial.println("[EXITO] Todo el sistema esta ONLINE.");

  Serial.println("\n--- INICIANDO SECUENCIA DE ARRANQUE ---");
  Serial.printf("📦 Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("🧠 PSRAM: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
  Serial.println("---------------------------------------");

  pixel.begin(); // Inicializa el LED RGB
  pixel.setBrightness(50); // Brillo al 50% para no quedarte ciego

  delay(100);
  sistemaListo = true;
}

void loop() {    
  // El ciclo de ejecución maestro nativo queda inoperante. 
  // El control ha sido delegado completamente al planificador de tareas de FreeRTOS.
  vTaskDelete(NULL);
}
