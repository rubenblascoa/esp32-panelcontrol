// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod

/**
 * @file config.h
 * @brief Declaración de variables globales, librerías y pines del sistema.
 *
 * Estado global organizado en structs por responsabilidad:
 *   SesionWeb   — token de sesión HTTP y expiración
 *   SistemaWeb  — flags de runtime del servidor web
 *   Seguridad   — FSM de alertas y umbrales
 *   Tunel       — Cloudflare Worker WebSocket
 *   Telnet      — autenticación y timeout de terminal
 *   Sensores    — lecturas DHT11
 *   NFC         — buffers y estado del lector MFRC522
 */
#pragma once

// ============================================================================
// BIBLIOTECAS
// ============================================================================
//#define HTTPS_ENABLED
#if defined(HTTPS_ENABLED) && !defined(ASYNC_TCP_SSL_ENABLED)
  #define ASYNC_TCP_SSL_ENABLED 1
#endif
#include <Adafruit_NeoPixel.h>
#include <nvs_flash.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <MFRC522.h>
#include "time.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WebSocketsClient.h>

// ============================================================================
// MACROS DE LOGGING
// ============================================================================
#define LOG_I(tag, fmt, ...) Serial.printf("[I][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define LOG_W(tag, fmt, ...) Serial.printf("[W][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define LOG_E(tag, fmt, ...) Serial.printf("[E][%s] " fmt "\n", tag, ##__VA_ARGS__)

// ============================================================================
// PINES FIJOS
// ============================================================================
#define LED_PIN    48
#define NUMPIXELS   1

extern int RST_PIN;
extern int SS_PIN;
extern int TRIG_PIN;
extern int ECHO_PIN;
extern int DHT_PIN;

// ============================================================================
// INSTANCIAS HARDWARE
// ============================================================================
extern LiquidCrystal_I2C lcd;
extern Adafruit_NeoPixel pixel;
#define ServerType AsyncWebServer
extern ServerType*       server;
extern AsyncWebSocket*   ws;
extern WiFiServer        telnetServer;
extern WiFiClient        telnetClient;
extern MFRC522*          mfrc522;
extern MFRC522::MIFARE_Key key;

// ============================================================================
// CREDENCIALES WEB
// ============================================================================
extern String webUser;
extern String webPass;

// ============================================================================
// INFRAESTRUCTURA FREERTOS
// ============================================================================
extern QueueHandle_t     cmdQueue;
extern SemaphoreHandle_t i2cMutex;
extern SemaphoreHandle_t fsmMutex;
extern SemaphoreHandle_t spiMutex;
extern SemaphoreHandle_t cpuMutex;
extern SemaphoreHandle_t segMutex;

// FSM principal
extern volatile int programaActivo;
#define FSM_GET() \
  ([]() -> int { \
    int _v = 0; \
    if (fsmMutex && xSemaphoreTake(fsmMutex, pdMS_TO_TICKS(10)) == pdTRUE) { \
      _v = programaActivo; \
      xSemaphoreGive(fsmMutex); \
    } else { _v = programaActivo; } \
    return _v; \
  }())
#define FSM_SET(val) \
  do { \
    if (fsmMutex && xSemaphoreTake(fsmMutex, pdMS_TO_TICKS(10)) == pdTRUE) { \
      programaActivo = (val); \
      xSemaphoreGive(fsmMutex); \
    } else { programaActivo = (val); } \
  } while(0)

// ============================================================================
// CAPTIVE PORTAL
// ============================================================================
extern bool      modoConfiguracion;
extern DNSServer dnsServer;
extern volatile bool          conexionEnProgreso;
extern volatile bool          conexionExitosa;
extern String                 conexionIP;
extern volatile unsigned long inicioConexion;

// ============================================================================
// TIMERS GENERALES
// ============================================================================
extern volatile unsigned long tiempoProcesamientoCore[2];
extern volatile unsigned long tiempoCicloCore[2];
extern const unsigned long    intervaloMenu;
extern const unsigned long    intervaloMedicion;
extern unsigned long tiempoUltimoMenuPrincipal;
extern unsigned long previousMillisLED;
extern unsigned long tiempoUltimaMedicion;
extern unsigned long tiempoUltimoMenuNFC;
extern volatile bool   midiendoDistancia;
extern bool            ledState;
extern bool            sdDisponible;

// ============================================================================
// STRUCT: SesionWeb — token HTTP y expiración de sesión
// ============================================================================
#define SESSION_TIMEOUT_MS (30UL * 60UL * 1000UL)

struct SesionWeb {
  String        token    = "";   // Token alfanumérico activo (vacío = sin sesión)
  unsigned long inicio   = 0;    // millis() cuando se emitió el token
};
extern SesionWeb sesion;

// ============================================================================
// STRUCT: SistemaWeb — flags de runtime del servidor web
// ============================================================================
struct SistemaWeb {
  bool          listo          = false; // true cuando setup() finaliza
  bool          permitirLog    = false; // habilita transmisión de logs a la consola web
  bool          forzarTelemetria = false; // fuerza push inmediato de telemetría
  bool          pedirMenu      = false; // cliente listo → mandar menú en el próximo ciclo
  unsigned long tiempoInicio   = 0;    // millis() de inicio del servidor
  const int     maxClientes    = 2;    // límite de clientes WebSocket simultáneos
};
extern SistemaWeb sistemaWeb;

// ============================================================================
// STRUCT: Seguridad — FSM de alertas y umbrales
// ============================================================================
// Umbrales CRITICAL
constexpr float SEG_TEMP_CPU_MAX      = 70.0f;
constexpr float SEG_TEMP_DHT_MAX      = 45.0f;
constexpr int   SEG_CPU_MAX           = 95;
constexpr float SEG_RAM_MAX           = 90.0f;
constexpr int   SEG_WIFI_MIN          = -85;
// Umbrales WARNING
constexpr float SEG_WARN_TEMP_CPU_MAX = 60.0f;
constexpr float SEG_WARN_TEMP_DHT_MAX = 40.0f;
constexpr int   SEG_WARN_CPU_MAX      = 80;
constexpr float SEG_WARN_RAM_MAX      = 80.0f;
constexpr int   SEG_WARN_WIFI_MIN     = -80;

struct Seguridad {
  volatile bool     habilitado   = true;  // master toggle desde /config
  volatile bool     modo         = false; // alerta activa
  volatile bool     manual       = false; // activación manual (no auto-recovery)
  volatile uint8_t  nivel        = 0;     // 0=normal 2=warning 3=critical
  String            causa        = "";    // texto corto para logs
  String            medidas      = "";    // JSON array de medidas activas
  String            ultimaAlerta = "-";   // última causa crítica registrada
};
extern Seguridad seguridad;

// ============================================================================
// STRUCT: Tunel — Cloudflare Worker WebSocket saliente
// ============================================================================
struct Tunel {
  WebSocketsClient ws;
  String           host         = "";
  String           path         = "/esp-tunnel";
  String           token        = "";
  bool             habilitado   = false;
  bool             conectado    = false;
  int              wsCount      = 0;     // navegadores conectados vía túnel
  unsigned long    beginSslTs   = 0;     // millis() del último beginSSL (watchdog)
  bool             telnetActivo = false; // sesión Telnet activa vía túnel
};
extern Tunel tunel;

// ============================================================================
// STRUCT: Telnet — autenticación y timeout de terminal
// ============================================================================
struct Telnet {
  volatile bool          autenticado      = false;
  volatile int           intentos         = 0;
  volatile unsigned long ultimaActividad  = 0; // millis() de última actividad (timeout 5min)
};
extern Telnet telnet;

// ============================================================================
// STRUCT: Sensores — lecturas ambientales DHT11
// ============================================================================
struct Sensores {
  volatile float temperatura = -127.0f; // -127 = sin lectura
  volatile float humedad     = -1.0f;   // -1   = sin lectura
};
extern Sensores sensores;

// ============================================================================
// STRUCT: NFC — buffers y estado del lector MFRC522
// ============================================================================
struct NFC {
  byte           bloque0[16]      = {0x60,0xC9,0x13,0x1C,0xA6,0x08,0x04,0x00,
                                     0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69};
  byte           bloqueEscaneado[16] = {};
  byte           bloqueManual[16]    = {};
  byte*          bloqueAEscribir     = nullptr;
  byte           dump[1024]          = {};  // 16 sectores × 4 bloques × 16 bytes
  bool           dumpValido          = false;
  volatile int   modo                = 0;   // sub-estado del reader RFID
  bool           memoriaLlena        = false;
};
extern NFC nfc;
