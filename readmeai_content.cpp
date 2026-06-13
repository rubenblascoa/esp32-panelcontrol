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
 * @file readmeai_content.cpp
 * @brief Contenido combinado .readmeAI + GitHub README incrustado en Flash (PROGMEM).
 *        Servido en /.readmeAI como contexto único de Blasco AI.
 */
#include "readmeai_content.h"

// ============================================================================
// CONTENIDO COMBINADO .readmeAI + GitHub README
// ============================================================================
const char readmeai_content[] PROGMEM = R"rawliteral(
## ══════════════════════════════════════════════════════════
## .readmeAI — AI-to-AI Project Context
## ══════════════════════════════════════════════════════════

# .readmeAI

---

## ⚙️ AI PROTOCOL

### Token efficiency rules
- STRUCTURE MAP y SYMBOL INDEX llenos: NO escanear filesystem.
- Nunca leer un archivo sin razón concreta. Usar SYMBOL INDEX para ubicar símbolos exactos.
- No re-inferir lo ya registrado aquí. Confiar en stack, arquitectura y convenciones.

### Session start
1. Leer este archivo completo (único full-read obligatorio).
2. Cargar CURRENT SESSION STATE. Continuar sin pedir recap.

### Session end (silencioso, sin prompt al usuario)
Actualizar: SESSION STATE, DECISIONS LOG, CHANGE LOG, STRUCTURE MAP, SYMBOL INDEX, PROGRESS, KNOWN ISSUES, DEPENDENCIES, AI NOTES.
Zero-dependency check: ¿puede una IA fría continuar solo con este archivo?

### Core rules
- **Zero dependency** — Este archivo solo debe bastar para cualquier IA.
- **Trust order** — Código > este archivo > memoria.
- **Append, never delete** — Logs son append-only.
- **Quality gate** — Antes de entregar código, ejecutar checklist de CODE QUALITY.
- **Scope creep guard** — Request fuera de scope → Backlog, flag al usuario.

---

## 🧭 PROJECT CONTEXT

### Purpose
Firmware unificado para ESP32-S3 con múltiples módulos independientes (NFC, radar ultrasónico, web dashboard, CLI terminal) seleccionables en runtime sin re-flashear.

### Goals
1. Módulos runtime sin re-flashear
2. Control remoto web
3. Telemetría en tiempo real + logging histórico CSV
4. Clonación NFC MIFARE Classic 1K
5. Radar ultrasónico HC-SR04
6. Sensor DHT11 (temp/humedad)
7. LCD 16x2 con métricas rotativas (5 páginas)
8. OTA firmware updates
9. Terminal híbrida Telnet + WebSocket
10. Blasco AI — chat Gemini embebido en dashboard (streaming SSE, adjuntos, telemetría en contexto)

### Non-goals
App móvil, cloud sync, multi-usuario, otros tipos NFC, audio/video.

### Key constraints
| Constraint | Detail |
|-----------|--------|
| MCU | ESP32-S3 N16R8 (16MB Flash, 8MB PSRAM, 240MHz dual-core) |
| Pinout | NeoPixel=48. Defaults: NFC RST=21/SS=5, TRIG=15/ECHO=16, SDA=8/SCL=9, MOSI=11/MISO=13/CLK=18, DHT=4. **Todos configurables en NVS en runtime.** |
| Display | LCD 16x2 I2C (0x27), 5 páginas rotativas |
| NFC | MFRC522 + MIFARE Classic 1K únicamente |
| WebSocket | Máx 2 clientes simultáneos |
| CSV | Auto-delete a 8MB |
| Telnet | Sesión única |
| Stack | 16KB por tarea FreeRTOS |
| CPU load | EMA-smoothed 30/70, recalculado cada 1s por core |
| AI panel | Gemini API client-side fetch. API key en NVS (`/api/ai/key`). Soporta gemini-2.5-flash, 2.0-flash, 1.5-flash/pro, 2.0-flash-lite. Streaming SSE. |
| Cloudflare Tunnel | Worker + Durable Object TunnelSession. ESP32↔Worker (`/esp-tunnel` WS), Browser↔Worker (`/ws` WS). Worker proxia HTTP a ESP32 via WS con **HMAC-SHA256** (x-zenith-sig, 30s window). |
| Console format | JSON `{"type":"ws_fwd","data":"..."}` por túnel; texto raw en LAN. |
| CSV push on connect | Últimas 500 filas vía WS al conectar browser por túnel. |
| HTTPS | Opcional con #ifdef HTTPS_ENABLED, certs EC-256 autofirmados en certs.h |
| Module registry | Modulo struct en modules.h — dispatch por bucle en vez de if/else if FSM |
| Hardware recovery | fallosNFC/DHT/Ultrasonidos — auto-reinit tras umbral con cooldown |

### Business rules (resumen)
- NFC cloning solo funciona con tarjetas magic (CUID/FUID).
- Diccionario 20 claves NFC por sector; clave exitosa guardada en `sectorKeyExitosas[16]`.
- Menú auto-refresh tras 5 min inactividad (solo si hay clientes conectados).
- CSV logging requiere NTP sync (año > 2000).
- Log web silenciado 30s tras connect WS (evita flood).
- Telnet: sesión única, auth con `webPass`, timeout 5 min.
- OTA protegido con `webPass`.
- FreeRTOS: 2 tareas a prioridad 1. Core 0: vTaskDelay(10), Core 1: vTaskDelay(1).
- I2C protegido por `i2cMutex`.
- DHT11: polling cada 3s en Core 1, deshabilita interrupciones ~5ms.
- HC-SR04: ISR-based, FSM 3 estados, mínimo 200ms entre triggers.
- CSV cron: NTP, hora par :00 con anti-duplicate.
- LCD: 5 páginas rotativas cada 3s.
- GPIO configurables en runtime vía NVS/web.
- HMAC: proxy túnel firmado con HMAC-SHA256 (x-zenith-sig, ventana 30s).
- Módulos: struct Modulo con punteros a función; dispatch por bucle en taskCore1.
- Recuperación HW: contador fallos por sensor; reinit automático tras 3 fallos (cooldown 30s).
- CSV col 11: ultimaAlertaCritica — se escribe cuando seguridadNivel >= 3.

---

## 📋 PROJECT IDENTITY

| Field | Value |
|-------|-------|
| Name | ESP32 Blasco OS |
| Version | 0.3.0 |
| Phase | development |
| Type | Embedded firmware (ESP32-S3 N16R8) |
| Primary language | C++ (Arduino framework) |
| Secondary languages | HTML, CSS, JavaScript |
| Repository | github.com/rubenblacoa/esp32-panelcontrol |
| Last updated | 2026-06-13 |

---

## 🛠 TECH STACK

| Layer | Technology |
|-------|-----------|
| Runtime | ESP32-S3 @ 240MHz, FreeRTOS dual-core |
| Framework | Arduino for ESP32 |
| Web server | ESPAsyncWebServer (Async HTTP + WebSocket) |
| Filesystem | LittleFS (/datos.csv) |
| Serialization | ArduinoJson |
| NFC/RFID | MFRC522, MIFARE Classic 1K, SPI |
| Display | LiquidCrystal_I2C 16x2, I2C 0x27 |
| LED | Adafruit_NeoPixel, 1x RGB, GPIO 48 |
| Ultrasonic | HC-SR04, ISR-based, TRIG=15, ECHO=16 |
| Temp/Humidity | DHT11, DATA=GPIO4, 1-Wire nativo (sin librería) |
| OTA | ArduinoOTA |
| Persistence | Preferences NVS (4 namespaces: zenithmc, hwconfig, webcred, aikey) |
| Storage | SD Card SPI, opcional (#ifdef SD_CS_PIN) |
| Tunnel | Cloudflare Workers + Durable Object TunnelSession |
| WS Client | WebSockets (Markus Sattler), cfWsClient → Worker |
| HMAC | mbedTLS `mbedtls_md_hmac_*` — firma/verificación proxy túnel |

---

## 🏗 STRUCTURE MAP

```
project-root/
├── .readmeAI
├── main.ino               ← setup() + loop() → vTaskDelete, FreeRTOS dual-core init
├── config.h / config.cpp  ← includes, pin defines, extern globals
├── menus.h / menus.cpp    ← menús principal, NFC, ultrasonidos, DHT
├── modules.h              ← Modulo struct para registro de módulos (NUEVO)
├── nfc.h / nfc.cpp        ← MIFARE read/write + Modulo instance + hardware reinit
├── tareas.h / tareas.cpp  ← Core0 (network/OTA/WS/CSV/HMAC proxy) + Core1 (LCD/DHT/module dispatch)
├── terminal.h / terminal.cpp ← TerminalHibrida: Telnet + WebSocket dual-path
├── ultrasonidos.h/.cpp    ← HC-SR04 ISR-based + Modulo instance + hardware reinit
├── dht.h / dht.cpp        ← DHT11 1-Wire + Modulo instance + hardware reinit
├── utils.h / utils.cpp    ← CPU EMA, uptime, NTP, CSV logging, LCD 5-page, NVS, HMAC-SHA256
├── web_server.h/.cpp      ← 22 endpoints HTTP, WS handler, auth, CSV pagination, certs.h include
├── web_pages.h/.cpp       ← 6 páginas PROGMEM via pages/page_*.h (NUEVO split)
├── pages/                 ← 6 page_*.h extraídos de web_pages.cpp (NUEVO)
│   ├── page_index.h       ← Dashboard + terminal (1867 líneas)
│   ├── page_db.h          ← DB viewer con paginación (628 líneas)
│   ├── page_login.h       ← Login (270 líneas)
│   ├── page_wifi.h        ← Captive portal (846 líneas)
│   ├── page_config.h      ← Config panel (1547 líneas)
│   └── page_landing.h     ← Landing pública (975 líneas)
├── certs.h                ← HTTPS certs EC-256 PEM (#ifdef HTTPS_ENABLED, NUEVO)
├── sd_card.h / sd_card.cpp ← SD init, CSV append/delete, last-500-lines leerCSV_SD()
├── worker.js              ← Cloudflare Worker: HMAC signing + DO TunnelSession
├── wrangler.toml          ← Worker deploy config
├── package.json           ← Node deps (wrangler)
├── readmeai_content.h     ← extern PROGMEM declaration
├── readmeai_content.cpp   ← Este archivo
└── test/                  ← Tests unitarios (NUEVO)
    └── test_utils.cpp     ← Unity Test: HMAC-SHA256, CSV snprintf
```

**Arquitectura:** Monolítico FreeRTOS dual-core. Core 0: I/O, red, display, LED. Core 1: input, FSM, sensores. Web como capa UI remota. Estructura Arduino plana (sin subdirectorios).

---

## 🔍 SYMBOL INDEX

### Entry points
| Symbol | Location | Description |
|--------|----------|-------------|
| `setup()` | main.ino:42 | NVS config load, Serial, I2C, LCD, WiFi, NTP, OTA, SPI, MFRC522, FreeRTOS tasks 16KB |
| `loop()` | main.ino:186 | `vTaskDelete(NULL)` — delega a FreeRTOS |
| `taskCore0()` | tareas.cpp:40 | Core 0: LittleFS, OTA, Telnet, WS telemetry 3s, CSV cron, tunnel proxy + **HMAC verification** |
| `taskCore1()` | tareas.cpp:199 | Core 1: LCD 3s/5 páginas, DHT11 3s, NeoPixel, input, **module dispatch via Modulo array**, **hardware error recovery** |
| `TunnelSession.fetch()` | worker.js:58 | Worker entry: `/esp-tunnel` (ESP32 WS), `/ws` (browser WS), HTTP proxy |

### Core functions
| Symbol | Location | Description |
|--------|----------|-------------|
| `mostrarMenuPrincipal()` | menus.cpp:36 | Print main menu + telemetry banner |
| `procesarMenuPrincipal(String)` | menus.cpp:102 | Parse input (1=NFC, 2=Ultrasonidos, 3=DHT) |
| `mostrarMenuNFC()` | menus.cpp:122 | Print NFC menu |
| `procesarEntradaNFC(String)` | menus.cpp:151 | Parse NFC input (1=read+save, 2=clone, M=exit) |
| `mostrarMenuUltrasonidos()` | menus.cpp:179 | Print ultrasonic menu |
| `mostrarMenuDHT()` | menus.cpp:199 | Print DHT11 menu |
| `procesarEntradaDHT(String)` | menus.cpp:224 | Parse DHT input (1=read, M=exit) |
| `modoLecturaNFC()` | nfc.cpp:93 | Poll card, 20 keys/sector, auth, read 64 blocks → dumpTarjeta[1024] |
| `modoEscrituraNFC()` | nfc.cpp:168 | Poll target, auth stored key, write 64 blocks from dumpTarjeta |
| `autenticarSector(byte)` | nfc.cpp:43 | Prueba clave sector anterior primero, luego dict 20; guarda en sectorKeyExitosas[] |
| `medirDistanciaFisica()` | ultrasonidos.cpp:50 | ISR FSM: -1 waiting, -2 timeout, distancia cm |
| `ecoISR()` | ultrasonidos.cpp:41 | **IRAM_ATTR** ISR CHANGE en ECHO_PIN |
| `inicializarDHT()` | dht.cpp:88 | Init bus DHT11 en Core 1 |
| `leerTemperatura()` | dht.cpp:103 | Read °C nativo, -127.0 on error |
| `leerHumedad()` | dht.cpp:115 | Read % nativo, -1.0 on error |
| `actualizarDHT()` | dht.cpp:127 | Read temp+hum, 3 reintentos 500ms, actualiza globals |
| `calcularUsoCPU(int)` | utils.cpp:34 | CPU load/core via micros(), EMA 30/70, cada 1s |
| `obtenerUptime()` | utils.cpp:68 | millis() → "HH:MM:SS" |
| `obtenerFechaHora()` | utils.cpp:80 | NTP → "DD/MM/YYYY HH:MM" |
| `hmacSha256(const String&, const String&)` | utils.cpp:270 | HMAC-SHA256 via mbedTLS → hex string 64 chars |
| `reinicializarNFC()` | nfc.cpp:230 | SPI.end + delay + SPI.begin + PCD_Init (fallosNFC >= 3) |
| `reinicializarUltrasonidos()` | ultrasonidos.cpp:270 | detachInterrupt + pin reset + attachInterrupt (fallosUltrasonidos >= 3) |
| `reinicializarDHT()` | dht.cpp:140 | pin mode reset + inicializarDHT (fallosDHT >= 3) |
| `guardarEnHistorial()` | utils.cpp:97 | Append CSV 11 columnas a /datos.csv |
| `guardarConfigHardware(...)` | utils.cpp:192 | NFC RST/SS, TRIG/ECHO, DHT → NVS `hwconfig` |
| `cargarConfigHardware()` | utils.cpp:205 | NVS → RST_PIN, SS_PIN, TRIG_PIN, ECHO_PIN, DHT_PIN |
| `guardarCredencialesWeb(...)` | utils.cpp:220 | user/pass → NVS `webcred` |
| `cargarCredencialesWeb()` | utils.cpp:230 | NVS → webUser, webPass |
| `guardarClaveAI(String)` | utils.cpp:241 | Gemini API Key → NVS `aikey` |
| `cargarClaveAI()` | utils.cpp:250 | NVS `aikey` → String |
| `actualizarLCD()` | utils.cpp:245 | 5 páginas LCD vía i2cMutex |
| `iniciarServidorWeb()` | web_server.cpp:89 | 22 endpoints + WS, TZ, start |
| `estaLogueado(...)` | web_server.cpp:40 | Valida ZENITH_SESSION cookie |
| `onWsEvent(...)` | web_server.cpp:53 | WS connect/disconnect/data, máx 2 clientes |
| `_flushBuffer(const String&)` | terminal.cpp:38 | Flush buffer a LAN WS + tunnel WS en JSON |
| `enviarRespuestaProxy(...)` | tareas.cpp:81 | HTTP chunked via tunnel WS |
| `procesarProxyTunnel()` | tareas.cpp:177 | Handle tunnel HTTP: landing, login, dashboard, config, db, API, TCP fallback + **HMAC verification** |
| `Modulo` | modules.h:1 | Struct: nombre, entrada(), loop(), icono |
| `moduloNFC` | nfc.cpp:240 | Modulo instance NFC |
| `moduloUltrasonidos` | ultrasonidos.cpp:280 | Modulo instance ultrasonidos |
| `moduloDHT` | dht.cpp:150 | Modulo instance DHT |

### Key class
| Symbol | Location | Description |
|--------|----------|-------------|
| `TerminalHibrida : public Print` | terminal.h:35 | Singleton Telnet+WS, buffered, anti-OOM, race-safe. Instancia: `Terminal` |

### Key constants / globals
| Symbol | Default | Location |
|--------|---------|----------|
| `LED_PIN` | 48 | config.h:59 |
| `RST_PIN` | 21 (NVS) | config.cpp:64 |
| `SS_PIN` | 5 (NVS) | config.cpp:65 |
| `TRIG_PIN` | 15 (NVS) | config.cpp:66 |
| `ECHO_PIN` | 16 (NVS) | config.cpp:67 |
| `DHT_PIN` | 4 (NVS) | config.cpp:68 |
| `webUser` / `webPass` | admin / blasco (NVS) | config.cpp:60-61 |
| `programaActivo` | 0-3 (FSM) | config.cpp:86 |
| `MAX_WEBSOCKET_CLIENTS` | 2 | config.cpp:72 |
| `temperaturaActual` / `humedadActual` | -127.0 / -1.0 sentinel | config.cpp:93-94 |
| `dumpTarjeta[1024]` | — | config.cpp:84 |
| `cmdQueue` | QueueHandle_t, 10 items | config.cpp:71 |
| `TELEMETRY_INTERVAL` | 3000ms | tareas.cpp:106 |
| `DHT_POLL_INTERVAL` | 3000ms (Core 1) | tareas.cpp:194 |
| `LCD_INTERVAL` | 3000ms | tareas.cpp:188 |

### Sistema de seguridad — globals y umbrales
| Symbol | Default | Location | Description |
|--------|---------|----------|-------------|
| `seguridadHabilitado` | true | config.cpp:137 | Master toggle — desactivar desde /config inhibe toda evaluacion |
| `modoSeguridad` | false | config.cpp:138 | true = alerta activa (cualquier nivel) |
| `seguridadManual` | false | config.cpp:139 | true = activado por usuario; inhibe auto-recuperacion |
| `alertaEnviada` | false | tareas.cpp (static) | Guard broadcast — evita spam WS por alerta repetida |
| `causaSeguridad` | "" | config.cpp:140 | Texto del parametro que disparo la alerta, ej. "TEMP_CPU:85.3C (max 80C)" |
| `seguridadNivel` | 0 | config.cpp:141 | 0=normal, 1=info, 2=warning, 3=critical |
| `seguridadMedidas` | "" | config.cpp:142 | JSON array de medidas activas, ej. ["FSM detenida","NFC detenido"] |
| `SEG_TEMP_CPU_MAX` | 80.0C | config.h:234 | Umbral CRITICAL temperatura die ESP32 |
| `SEG_TEMP_DHT_MAX` | 35.0C | config.h:235 | Umbral CRITICAL temperatura ambiente DHT11 |
| `SEG_CPU_MAX` | 90% | config.h:236 | Umbral CRITICAL carga CPU |
| `SEG_RAM_MAX` | 90.0% | config.h:237 | Umbral CRITICAL uso RAM |
| `SEG_WIFI_MIN` | -85dBm | config.h:238 | Umbral CRITICAL señal WiFi |
| `SEG_WARN_TEMP_CPU_MAX` | 60.0C | config.h:241 | Umbral WARNING temperatura die |
| `SEG_WARN_TEMP_DHT_MAX` | 40.0C | config.h:242 | Umbral WARNING temperatura ambiente |
| `SEG_WARN_CPU_MAX` | 80% | config.h:243 | Umbral WARNING carga CPU |
| `SEG_WARN_RAM_MAX` | 80.0% | config.h:244 | Umbral WARNING uso RAM |
| `SEG_WARN_WIFI_MIN` | -80dBm | config.h:245 | Umbral WARNING señal WiFi |

---

## 🛡 SISTEMA DE SEGURIDAD

> Descripcion detallada del subsistema de supervision y proteccion autonoma del ESP32.

### Proposito
Detectar parametros del sistema fuera de rango, activar medidas de proteccion automaticamente, informar al usuario por todos los canales disponibles (LCD, consola, WebSocket, dashboard web), y recuperarse solo cuando los parametros se normalizan. Disenado para sobrevivir uso intensivo simultaneo de todos los modulos (NFC + ultrasonidos + SD + telemetria WiFi) que puede llevar la temperatura de idle ~47C hasta el umbral critico.

### Niveles de prioridad
| Nivel | Valor | Nombre | Efecto |
|-------|-------|--------|--------|
| Normal | 0 | — | Sin alerta. Operacion completa. |
| Info | 1 | INFO | Alerta leve. Sin medidas. Solo notificacion. |
| Warning | 2 | ADVERTENCIA | Parametro en zona de aviso. Sin medidas activas. |
| Critical | 3 | CRITICO | Medidas activas. FSM detenida. DHT pausado. Consola bloqueada. |

### Parametros monitorizados y umbrales
| Parametro | WARNING | CRITICAL | Fuente |
|-----------|---------|----------|--------|
| Temperatura die CPU | >=60C | >=80C | `temperatureRead()` |
| Temperatura ambiente | >=40C | >=35C | DHT11 `temperaturaActual` |
| Carga CPU media | >=80% | >=90% | `calcularUsoCPU(0+1)/2` |
| Uso RAM | >=80% | >=90% | `ESP.getFreePsram()` / `getHeapSize()` |
| Señal WiFi RSSI | <=-80dBm | <=-85dBm | `WiFi.RSSI()` |

Evaluacion cada 3s en taskCore0 (bloque estatico). Boot delay de 30s para evitar falsas alarmas por spike de arranque (CPU al 100% durante init).

### Flujo de activacion (CRITICAL)
1. `causaDetectada` no vacia + `!modoSeguridad` → activa alerta
2. `modoSeguridad=true`, `causaSeguridad=causaDetectada`, `seguridadNivel=3`
3. Medidas automaticas: detiene ultrasonidos (`midiendoDistancia=false`), devuelve FSM a 0 (`FSM_SET(0)`), detiene NFC (`modoNFC=0`). Guarda estado previo en `fsmGuardado` para restaurar al salir.
4. Broadcast WS: `{"type":"security_alert","nivel":3,"causa":"...","medidas":[...]}`
5. LCD muestra " S. SEGURIDAD: " / " ⚠ Critico ⚠ " (simbolo custom char 0, posiciones 2 y 12)
6. Consola bloqueada: cualquier input imprime banner de alerta con separadores === y vacia `entrada`
7. DHT pausado: `taskDHT` no llama `actualizarDHT()` mientras `seguridadNivel>=3`. El timer no avanza → mide inmediatamente al recuperarse.

### Auto-recuperacion
Cada 3s se reevaluan los parametros. Si `causaDetectada` vuelve a estar vacia y la alerta no es manual → `modoSeguridad=false`. En la siguiente iteracion se detecta la transicion de salida (`transicionSalida=true`):
- `alertaEnviada=false`, `seguridadNivel=0`, `seguridadMedidas=""`
- FSM restaurada a `fsmGuardado` si estaba en 0
- DHT retoma medicion en siguiente ciclo de 100ms
- LCD retoma rotacion normal de 5 paginas
- Consola desbloqueada
- Broadcast WS: `{"type":"security_ok","hora":"..."}`

### Activacion manual
Desde /config el usuario puede activar/desactivar manualmente. Con `seguridadManual=true` la auto-recuperacion queda inhibida — el sistema permanece en CRITICAL hasta que el usuario lo desactive explicitamente desde /config.

### Comportamiento LCD en CRITICAL
- Fila 0: " S. SEGURIDAD:  " (fija)
- Fila 1: "  ⚠ Critico ⚠   " (simbolo custom HD44780 char 0 en posiciones 2 y 12)
- Volcado via `lcd.write()` caracter a caracter (16 bytes exactos) — no usa `lcd.print()` que corta en NUL
- Sin parpadeo. Al salir de CRITICAL: rotacion de 5 paginas reanuda desde la pagina en curso.

### Comportamiento consola en CRITICAL
Cualquier comando recibido (Telnet o WebSocket) durante CRITICAL es descartado y se imprime:
```
==================================================
           ALERTA DE SEGURIDAD
==================================================

  Prioridad: CRITICA
  Parametro: <causaSeguridad>
  Medidas: <seguridadMedidas>
  El sistema operara en modo reducido.

==================================================
  Consola bloqueada. Resuelve la alerta en /config.
==================================================
```

### Racionalizacion del diseño
El ESP32-S3 en idle con WiFi activo opera a ~47C. Uso simultaneo de NFC (SPI bloqueante), ultrasonidos (ISR continua), SD (LittleFS write), telemetria WebSocket y DHT (noInterrupts 5ms cada 3s) puede llevar el die a 80C+. Al detener los modulos no esenciales la carga baja lo suficiente para que el die se enfrie y el sistema se recupere autonomamente sin intervencion humana.

---

## 📐 CONVENTIONS

- Funciones en español descriptivo: `mostrarMenuPrincipal`, `guardarEnHistorial`
- Constants: SCREAMING_SNAKE_CASE
- Comentarios: **siempre en español**, explican WHY no WHAT
- Sin clases excepto `TerminalHibrida`; estilo procedural/free-function
- Estado global en config.h/config.cpp (patrón extern)
- PROGMEM para strings HTML grandes
- FreeRTOS: vTaskDelay(10) Core 0, vTaskDelay(1) Core 1
- I2C protegido por i2cMutex
- Usar String (Arduino), no std::string

---

## ✅ CODE QUALITY checklist
- Una función = una responsabilidad. Si necesitas "y", divide.
- Sin funciones >30 líneas, sin nesting >3 niveles.
- Sin lógica duplicada, sin valores hardcoded, sin variables no usadas.
- Errores manejados. Sin fallos silenciosos.
- Nuevos archivos en la ubicación correcta según STRUCTURE MAP.

---

## 🔌 API ENDPOINTS

| Method | Path | Auth | Notes |
|--------|------|------|-------|
| GET | `/` | No | Landing pública |
| GET | `/dashboard` | Cookie | Dashboard principal |
| GET | `/db` | Cookie | DB viewer |
| GET | `/config` | Cookie | Config: pines + credenciales |
| GET | `/setup` | No | Captive portal (modoConfiguracion=true) |
| POST | `/login` | No | user/pass → cookie ZENITH_SESSION |
| GET | `/logout` | No | Clear cookie |
| GET | `/datos.csv` | Cookie | Stream CSV |
| GET | `/delete-db` | Cookie | Eliminar CSV |
| GET | `/api/system/info` | Cookie | JSON: chip, ip, flash, psram, heap, uptime |
| GET | `/api/csv` | Cookie | CSV paginado con ?page=N&size=M, X-Total-Lines header |
| GET | `/api/config/info` | Cookie | JSON: ip, ssid, rssi, user, pins |
| POST | `/api/config/webcred` | Cookie | Cambiar user/pass web |
| POST | `/api/config/pins` | Cookie | Guardar GPIO mapping NVS |
| POST | `/api/wifi/reboot` | No | Reboot ESP32 |
| GET | `/api/wifi/scan` | No | WiFi scan results |
| POST | `/api/wifi/configure` | No | Guardar WiFi + config + creds NVS |
| GET | `/api/wifi/status` | No | Estado conexión WiFi |
| POST | `/api/wifi/reset` | No | Clear NVS + reboot |
| POST | `/import-csv` | Cookie | Import/reemplazar CSV |
| GET | `/.readmeAI` | No | PROGMEM: .readmeAI + README + PCB |
| GET | `/api/ai/key` | Cookie | Gemini API Key desde NVS |
| POST | `/api/ai/key` | Cookie | Guardar Gemini API Key NVS |
| WS | `/ws` (LAN) | — | Bidireccional, máx 2 clientes |
| WS | `/esp-tunnel` (Worker) | Token | ESP32 ↔ Worker DO |
| WS | `/ws` (Worker) | — | Browser ↔ Worker, relay ws_fwd |
| HTTP | `/*` (Worker) | Cookie | Proxy HTTP → ESP32 via WS, timeout 8s |

### CSV schema (11 columnas)
`timestamp, temp_chip, cpu%, core0%, core1%, ram%, flash%, wifi_signal, dht_temp, dht_hum, ultima_alerta`
Col 10 = `ultimaAlertaCritica` — se escribe cuando seguridadNivel >= 3, "-" si no hay alerta.
Cron: hora par :00, anti-duplicate, verificado cada 30s.

---

## 🔐 SECURITY

- Auth: cookie ZENITH_SESSION, single user, token en `tokenSesionActiva` (RAM).
- Usuario plano: admin, acceso total.
- Datos sensibles en NVS sin cifrado: WiFi pass, web pass, Gemini key.
- HTTPS opcional con #ifdef HTTPS_ENABLED (certs EC-256 autofirmados).
- Tunnel proxy autenticado con HMAC-SHA256 (x-zenith-sig, ventana 30s).
- Sin sanitización de inputs. Sin watchdog.
- OTA protegido con webPass. Telnet auth con webPass.
- API endpoints protegidos con `esNavegadorDirecto()`: redirige a `/` si Accept contiene text/html.

---

## 🐛 KNOWN ISSUES

| ID | Status | Description |
|----|--------|-------------|
| DHT-001 | open | DHT11 falla intermitente en ESP32-S3 @ 240MHz |
| NFC-001 | open | NFC polling bloqueante en Core 1 |
| SEC-003 | fixed | Sistema de seguridad en /config: initWebSocket sin __ready__, security_alert sin check habilitado, seguridadRefreshStatus con error toast en vez de catch silencioso, seguridadRenderStatus sin null-safety en btnText/querySelector, falta toast alerta en carga |
| CSV-003 | open | CSV cronjob edge case medianoche |

**Tech debt:** ~30 globals en config.h/cpp, no watchdog, Wire.begin() llamado 2 veces, web_pages.cpp reducido a ~30 líneas con split pages/.

---

## 📦 DEPENDENCIES

Arduino core ESP32, ESPAsyncWebServer, MFRC522, LiquidCrystal_I2C, Adafruit_NeoPixel, ArduinoJson, Preferences (NVS built-in), DHT11 nativo (sin librería externa), WebSockets (Sattler) para tunnel.

---

## ✅ PROGRESS

### Completado reciente
- MEJ-001 a MEJ-010 (2026-06-13): 10 mejoras de `mejoras_esp32_blascos_os.html`:
  Watchdog LittleFS, JSON snprintf, paginación CSV, frontend paginación, HTTPS certs, HMAC túnel, módulos struct, recuperación HW, tests Unity, split pages.
- Bugfixes: } huérfana web_server.cpp, certs.h EC-256 reales, CSV 500 líneas WS, LOG_W, static order, operator+.
- SEC-003: Sistema de seguridad en /config corregido.
- AI-013: readmeai_content.h/cpp con .readmeAI + README + PCB como PROGMEM.
- WEB-005: API bloqueada desde navegador.
- UI-006: Config page responsive móvil.

### Backlog
NFC-001 non-blocking, DET-001 auto-detect módulos, MQTT-001, GRF-001 charts DHT, DHT-003 soporte DHT22.

---

## 🎯 CURRENT SESSION STATE

**Objetivo:** Implementar 10 mejoras estructurales desde `mejoras_esp32_blascos_os.html`.
**Ultimo paso:** Las 10 mejoras implementadas. .readmeAI y readmeai_content.cpp actualizados.
**Siguiente:** Compilar en Arduino IDE y verificar funcionamiento.
**Blockers:** Sin CLI build system (Arduino IDE). Compilación manual requerida.

---

## 📚 DECISIONS LOG (recientes)

| Date | Decision | Rationale |
|------|---------|-----------|
| 2026-06-13 | snprintf + stack buffer 800B para JSON pequeños en vez de ArduinoJson | Sin platformio.ini, no hay gestor de paquetes. snprintf evita heap fragmentation. |
| 2026-06-13 | reserve() + += para JSON grande (proxy body >2KB) | Body no cabe en stack. reserve() pre-asigna y evita realloc múltiples. |
| 2026-06-13 | CSV paginación newest-first con X-Total-Lines | Frontend necesita totalRows para calcular páginas. Newest-first porque usuario ve lo último primero. |
| 2026-06-13 | HMAC payload = timestamp + method + path + body | Protege contra replay (timestamp), método y path son parte del mensaje autenticado. |
| 2026-06-13 | Módulos con struct punteros a función en vez de FSM integer | Agregar/eliminar módulos sin tocar taskCore1. |
| 2026-06-13 | CSV WebSocket limitado a últimas 500 líneas | >8MB no cabe en mensaje WS. 500 líneas es suficiente para gráfica+tabla. |
| 2026-06-13 | web_pages.cpp partido en pages/*.h en vez de LittleFS | No requiere cambiar filesystem ni setup. PROGMEM intacto. |
| 2026-06-08 | API bloqueada desde navegador con `esNavegadorDirecto()` | fetch() manda `Accept: */*`; navegador manda `text/html`. Diferencia limpia sin romper frontend. |
| 2026-06-08 | Config page responsive: breakpoints 900/600/480px | Página ilegible en móvil. Solo ajusta layout. |
| 2026-06-08 | Contexto Blasco AI unificado en único PROGMEM string `/.readmeAI` | Endpoint no existía → fetch fallaba → AI nunca recibía contexto. |
| 2026-06-07 | Landing pública en `/`, dashboard protegido en `/dashboard` | Acceso público con enlace al dashboard. |
| 2026-06-11 | seguridadRefreshStatus con catch silencioso (sin error toast) | Dashboard y /db ya tragan errores silenciosamente; /config era la única página que mostraba error toast, confundiendo al usuario |
| 2026-06-11 | seguridadRenderStatus con null-safety en todas las ramas (txt, sub, btnText, querySelector i) | TypeError en btnText.textContent si el DOM no estaba listo, propagándose al catch de seguridadRefreshStatus |

---

## 🗒 AI NOTES

- Al modificar .readmeAI, pcb_context.md o README: regenerar readmeai_content.cpp.
- Blasco AI API key en NVS, nunca en localStorage.
- lucide usa `<svg>` no `<i>` en botones copy.
- LCD custom char 0 (triangulo alerta): registrado en setup() con lcd.createChar(0, charAlerta) despues de lcd.init(). SIEMPRE usar lcd.write() para fila1 en modo CRITICAL — lcd.print() corta en char 0. Para el resto de paginas usar el bucle write() con sustitucion NUL->espacio para evitar que bytes residuales del snprintf aparezcan como triangulo.
- Sistema de seguridad: idle ~47C, umbral CRITICAL 80C, margen ~33C. Al parar FSM+NFC+ultrasonidos la carga baja suficiente para recuperacion autonoma en uso intensivo simultaneo. seguridadManual inhibe auto-recuperacion — requiere desactivacion manual en /config.
- HMAC-SHA256: mbedtls_md_hmac_starts() necesita key como (const unsigned char*)key.c_str()
- El operador + de String se evalua izquierda-a-derecha. Con mezcla String+char*+int, usar parentesis o String() explicito.
- CSV paginacion newest-first: requiere leer archivo completo para contar lineas. Lento para >8MB pero inevitable sin indice.
- Certificados OpenSSL EC-256: formato PEM. ESPmbedTLS acepta PEM directo. Key debe ser PKCS#8 (BEGIN PRIVATE KEY), no SEC1 (BEGIN EC PRIVATE KEY).
- Al dividir web_pages.cpp en pages/*.h, raw-strings (R"()") no se pueden dividir entre archivos. Cada pagina queda en su propio .h.
- UTF-8 de archivos .h extraidos se corrompio por PowerShell (Set-Content encoding por defecto). Forzar -Encoding UTF8.

---

## ══════════════════════════════════════════════════════════
## GitHub README — Official Project README
## ══════════════════════════════════════════════════════════

ESP32 Blasco OS | Multi-Program

**The ultimate multi-program execution environment for low-level engineering.**

ESP32 Blasco is a low-level engineering platform and a multi-program execution environment designed exclusively for the ESP32-S3 microcontroller. It acts as a lightweight "Operating System" remotely accessible via Telnet (Wi-Fi) and WebSockets. It allows encapsulating and executing multiple hardware projects on the same board, switching between them through a pure text terminal interface (retro/hacker style), completely wire-free and featuring dynamic runtime hot-configuration.

### Hardware Powered by PCBWay

The development of ESP32 Blasco OS demands hardware capable of supporting the asynchronous execution of multiple programs in real time. For the physical deployment of this project, manufacturing and assembly (PCBA) services of PCBWay are used.

### Architecture and Modular Structure

1. **Entry Core and Orchestration** (main.ino) — Physical entry point and main orchestration.
2. **Configuration and System State** (config.h/cpp) — Global memory structures, thresholds, runtime pin maps.
3. **Multi-Core Scheduler** (tareas.h/cpp) — Hardware-distributed infinite processing loops (FreeRTOS).
4. **Graphical Interface and Frontend** (web_pages.h/cpp) — HTML, CSS, and JS for web dashboards.
5. **Routing and Network Security** (web_server.h/cpp) — Async network routing and REST API (19 endpoints).
6. **Sensor Controllers** — NFC (nfc.h/cpp), Ultrasonic (ultrasonidos.h/cpp), DHT11 (dht.h/cpp).
7. **Utilities and Disk Logs** (utils.h/cpp, sd_card.h/cpp) — EMA smoothing, NVS storage, CSV logging.
8. **Text Layer** (menus.h/cpp) — CLI visual framework and action parsing.

### Main Features

- 100% Wireless Control via Telnet (Port 23) and WebSockets.
- Dynamic Hardware Pin Mapping (Plug & Play) via Captive Portal.
- EMA-Smoothed Real-Time Telemetry (RAM, Flash, CPU, temperature).
- NTP Time-Backed Database (10-column CSV every 2 hours).

### The Problem vs The Solution

| Without Blasco OS | With Blasco OS |
| :--- | :--- |
| Mandatory USB connection to test | 100% Wireless via Telnet |
| Flash firmware on every physical pin change | Runtime Configuration via Captive Portal |
| Basic monitoring via Serial | Advanced telemetry (RAM, Flash, Temp, CPU) |
| Updates via cable | OTA Support (Over-The-Air) |
| Coupled projects that break code | Modular Architecture (Independent Drawers) |

### Active Modules

1. **NFC Cloning Station Pro (V14)** — MFRC522-based RFID auditing and cloning.
2. **Ultrasonic Radar (V3)** — HC-SR04 non-blocking distance measurement.
3. **Ambient Temperature and Humidity Monitor (V1)** — DHT11 native 1-Wire module.

### Required Hardware and Installation

- Base Board: ESP32-S3 N16R8
- NFC Module: MFRC522 RFID Reader (SPI Bus)
- Distance Module: HC-SR04 Sensor
- Climate Sensor: DHT11 Module (with 4.7kΩ pull-up resistor)
- Storage Module (Optional): MicroSD Reader (SPI Bus)

Initial deployment via Captive Portal:
1. Flash via USB first time.
2. ESP32 opens AP "Esp32BlascoOS_Setup".
3. Connect to AP, web wizard opens automatically.
4. Enter WiFi credentials, configure GPIO pins.
5. ESP32 reboots, connects to home router.
6. Open IP in browser — enjoy!

### Compilation Configuration (Arduino IDE)

Board: ESP32S3 Dev Module, USB CDC On Boot: Enabled, CPU Frequency: 240MHz, Flash Mode: QIO 80MHz, Flash Size: 16MB, Partition Scheme: 16M Flash (3MB APP/9.9MB FATFS), PSRAM: OPI PSRAM, Upload Speed: 921600.

### Contact & Sponsorship

Developed with passion by Ruben Blasco Armengod.
- GitHub: @rubenblascoa
- Instagram: @rubenblascoa
- Email: rubenblascoarmengod@gmail.com

## ══════════════════════════════════════════════════════════
## PCB CONTEXT — Hardware Context (PCB1)
## ══════════════════════════════════════════════════════════

## PLACA
- **Nombre**: ESP32 Blasco OS — PCB1 | **Diseñador**: Rubén Blasco Armengod
- **Fabricante**: PCBWay (T-L6W1088877A) | **Herramienta**: EasyEDA
- **Esquemático**: V1.0, creado 2026-05-07, actualizado 2026-06-02
- **Fabricadas**: 5 unidades | **Dimensiones**: 200×100mm, doble capa, negro, ENIG
- **Coste 5u**: $156.26 (componentes $47.17 + ensamblado $29 + PCB $80.09)

## MCU / ALIMENTACIÓN / PROGRAMACIÓN
- MCU: ESP32-S3-WROOM-1U-N16R8. Flash 16MB, PSRAM 8MB, LX7 dual-core 240MHz. WiFi 2.4GHz, BLE 5.0, antena externa U.FL. Coste unitario: $4.023.
- Entrada: USB-C 5V. Regulador: AMS1117-3.3 LDO 1A. Protección: fusible 2A + diodos Schottky B5819W.
- USB-UART: CH340C SOP-16 con DTR/RTS auto-reset. ESD: USBLC6-4SC6Y. Transistores SS8050 para EN/IO0.

## CONECTORES
| Ref | Tipo | Señales |
|-----|------|---------|
| H1 NFC | 2.54-1×8P | 3V3, RST, GND, IRQ, MISO, MOSI, SCK, SDA — MFRC522 |
| H2 PANTALLA | 2.54-1×4P | GND, 5V, SDA(8), SCL(9) — LCD 16×2 I2C |
| H3 ULTRASONIDO | 2.54-1×4P | GND, TRG(15), ECHO(16), 5V — HC-SR04 |
| H4 UNIVERSAL | 2.54-1×10P | 5V, GND×2, 3V3, GPIOs 6,7,15,18,19,20 |
| H5 CLIMA | 2.54-1×3P | 3V3, DATA(4), GND — DHT11 |
| CN1/CN2 | XH2.54-4P | 5V, GND, SDA, SCL — I2C externo |

## OTROS COMPONENTES
- MicroSD: TF-01A SMD, modo SPI, #ifdef SD_CS_PIN.
- LED-RGB: WS2812B-5050RGB (NeoPixel GPIO 48, brillo 50/255).
- LED-TX: verde, LED-RX: azul (CH340C), LED-POWER: rojo (3V3).
- BOTON-RESET (EN), BOTON-BOOT (GPIO0), SWITCH ON/OFF ESP32.

## GPIO MAP
| GPIO | Función | Conector | NVS |
|------|---------|----------|-----|
| 0 | BOOT | BOTON-BOOT | No |
| 4 | DHT11 DATA | H5 | ✅ dhtPin |
| 5 | NFC SS/CS | H1 | ✅ nfcSs |
| 8 | I2C SDA | H2+CN | No |
| 9 | I2C SCL | H2+CN | No |
| 11 | SPI MOSI | H1 | No |
| 13 | SPI MISO | H1 | No |
| 15 | TRIG HC-SR04 | H3 | ✅ trigPin |
| 16 | ECHO HC-SR04 | H3 | ✅ echoPin |
| 18 | SPI CLK | H1 | No |
| 21 | NFC RST | H1 | ✅ nfcRst |
| 36 | RXD0 CH340C | USB | No |
| 37 | TXD0 CH340C | USB | No |
| 42 | IRQ NFC | H1 | No |
| 48 | NeoPixel DIN | LED-RGB | No |
| EN | Reset | BOTON-RESET | No |

## BUSES
- SPI: MOSI=11, MISO=13, CLK=18, CS=5, RST=21 — MFRC522 dedicado
- I2C: SDA=8, SCL=9 — LCD (0x27), extensible CN1/CN2, protegido i2cMutex
- 1-Wire: DATA=4 — DHT11 bit-banging nativo
- UART: TX=37, RX=36 — CH340C → USB
- NeoPixel: DIN=48 — WS2812B
- HC-SR04: TRIG=15, ECHO=16 — ISR no bloqueante
)rawliteral";