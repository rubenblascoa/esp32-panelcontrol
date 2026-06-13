// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod

/**
 * @file config.cpp
 * @brief Definición e inicialización de variables globales del sistema.
 */
#include "config.h"

// ── Pines (runtime, cargados desde NVS en setup) ────────────────────────────
int RST_PIN  = 21;
int SS_PIN   = 5;
int TRIG_PIN = 15;
int ECHO_PIN = 16;
int DHT_PIN  = 4;

// ── Hardware ─────────────────────────────────────────────────────────────────
LiquidCrystal_I2C  lcd(0x27, 16, 2);
Adafruit_NeoPixel  pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
ServerType*        server  = nullptr;
AsyncWebSocket*    ws      = nullptr;
WiFiServer         telnetServer(23);
WiFiClient         telnetClient;
MFRC522*           mfrc522 = nullptr;
MFRC522::MIFARE_Key key;

// ── Credenciales web ─────────────────────────────────────────────────────────
String webUser = "admin";
String webPass = "";

// ── Infraestructura FreeRTOS ─────────────────────────────────────────────────
QueueHandle_t     cmdQueue = nullptr;
SemaphoreHandle_t i2cMutex = nullptr;
SemaphoreHandle_t fsmMutex = nullptr;
SemaphoreHandle_t spiMutex = nullptr;
SemaphoreHandle_t cpuMutex = nullptr;
SemaphoreHandle_t segMutex = nullptr;

volatile int programaActivo = 0;

// ── Captive portal ───────────────────────────────────────────────────────────
bool                  modoConfiguracion   = false;
DNSServer             dnsServer;
volatile bool         conexionEnProgreso  = false;
volatile bool         conexionExitosa     = false;
String                conexionIP          = "";
volatile unsigned long inicioConexion     = 0;

// ── Timers generales ─────────────────────────────────────────────────────────
volatile unsigned long tiempoProcesamientoCore[2] = {0, 0};
volatile unsigned long tiempoCicloCore[2]         = {10000, 10000};
const unsigned long    intervaloMenu               = 300000;
const unsigned long    intervaloMedicion           = 1000;
unsigned long tiempoUltimoMenuPrincipal = 0;
unsigned long previousMillisLED         = 0;
unsigned long tiempoUltimaMedicion      = 0;
unsigned long tiempoUltimoMenuNFC       = 0;
volatile bool midiendoDistancia = false;
bool          ledState          = LOW;
bool          sdDisponible      = false;

// ── Structs de estado ────────────────────────────────────────────────────────
SesionWeb  sesion;
SistemaWeb sistemaWeb;
Seguridad  seguridad;
Tunel      tunel;
Telnet     telnet;
Sensores   sensores;
NFC        nfc;
