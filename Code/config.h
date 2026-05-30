
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
 * @file config.h
 * @brief Declaración de variables globales, librerías y pines del sistema.
 */
#pragma once // Evita duplicidades si el archivo es incluido múltiples veces en la compilación

// ============================================================================
// CARGAR BIBLIOTECAS
// ============================================================================
#include <Adafruit_NeoPixel.h>    //  Gestión del direccionamiento del LED RGB de estado
#include <nvs_flash.h>            //  Librería para formatear la partición oculta del WiFi si es necesario
#include <Arduino.h>              //  Núcleo esencial del entorno Arduino para microcontroladores ESP32
#include <WiFi.h>                 //  Gestión y control de conexiones de redes inalámbricas
#include <ESPAsyncWebServer.h>    //  Motor del servidor web asíncrono y no bloqueante
#include <LittleFS.h>             //  Sistema de archivos interno de la memoria de almacenamiento flash
#include <ArduinoJson.h>          //  Procesamiento, serialización y deserialización de estructuras JSON
#include <ArduinoOTA.h>           //  Actualizaciones seguras de firmware a través de WiFi (Over-The-Air)
#include <SPI.h>                  //  Bus de comunicación por hardware para periféricos rápidos
#include <MFRC522.h>              //  Controlador específico del módulo lector físico NFC/RFID
#include "time.h"                 //  Gestión del reloj interno del sistema y cálculo del Uptime
#include <Wire.h>                 //  Bus de comunicación I2C por hardware
#include <LiquidCrystal_I2C.h>    //  Driver de control de la pantalla de cristal líquido LCD
#include <DNSServer.h>            // Captive portal: intercepta tráfico DNS y redirige al AP
#include <Preferences.h>          // Almacenamiento persistente en NVS

// ============================================================================
// MACROS DE LOGGING
// ============================================================================
#define LOG_I(tag, fmt, ...) Serial.printf("[I][%s] " fmt "\n", tag, ##__VA_ARGS__)
#define LOG_E(tag, fmt, ...) Serial.printf("[E][%s] " fmt "\n", tag, ##__VA_ARGS__)

// ============================================================================
// CONFIGURACIÓN DE PINES (ESP32-S3 N16R8)
// ============================================================================
// Conexión Física de Pantalla: SDA -> GPIO 8 | SCL -> GPIO 9 
#define LED_PIN         48        //  Pin físico del LED RGB de estado en la placa base
#define NUMPIXELS       1         //  Número total de LEDs NeoPixel integrados en el hardware

extern int RST_PIN;               // Pin de reinicio de hardware (Reset) del módulo lector MFRC522
extern int SS_PIN;                // Pin de selección de esclavo SPI (SDA/CS) del módulo MFRC522

extern int TRIG_PIN;              // Pin digital de salida para el emisor de pulsos ultrasónicos
extern int ECHO_PIN;              // Pin digital de entrada para el receptor de ecos ultrasónicos

extern int DHT_PIN;               // Pin de datos del sensor DHT11


// ============================================================================
// CONFIGURACIÓN DE PINES PARA TARJETA SD (MicroSD en modo SPI)
// ============================================================================
// Descomentar y ajustar según el cableado de la PCB:
// #define SD_CS_PIN    41   // Chip Select
// #define SD_CLK_PIN   40   // Clock
// #define SD_MOSI_PIN  11   // Master Out Slave In
// #define SD_MISO_PIN  13   // Master In Slave Out


// ============================================================================
// DECLARACIONES EXTERNAS (Para evitar duplicados enlazados por el Linker)
// ============================================================================

// Modo de Configuración Inicial (AP + Captive Portal)
extern bool modoConfiguracion;    // true = AP abierto, sin credenciales guardadas
extern DNSServer dnsServer;       // Servidor DNS para captive portal

// Estado de conexión asíncrona WiFi (config mode)
extern volatile bool conexionEnProgreso;
extern volatile bool conexionExitosa;
extern String conexionIP;
extern volatile unsigned long inicioConexion;

// Instancias globales de Periféricos
extern LiquidCrystal_I2C lcd;     //  Objeto de control de la pantalla LCD por interfaz I2C
extern Adafruit_NeoPixel pixel;   //  Objeto de control del LED RGB NeoPixel de la placa
// NOTA: extern DHT dht se declara en dht.h (requiere <DHT.h> antes de config.h)

// Instancias de Servidores de Red
extern AsyncWebServer* server;    //  Puntero global al servidor HTTP asíncrono
extern AsyncWebSocket* ws;        //  Puntero al canal WebSocket para comunicación web en tiempo real
extern WiFiServer telnetServer;   //  Instancia del servidor TCP clásico para conexiones por terminal SSH/Putty
extern WiFiClient telnetClient;   //  Objeto de sesión del cliente TCP actualmente activo en el sistema

// Instancias del Hardware de Radiofrecuencia
extern MFRC522* mfrc522;           //  Puntero al controlador del lector RFID
extern MFRC522::MIFARE_Key key;   //  Estructura de almacenamiento de claves simétricas MIFARE

// Credenciales web dinámicas
extern String webUser;
extern String webPass;

// Cola de comandos FreeRTOS para comunicación entre cores (Core 0 WebSocket → Core 1 tareas)
extern QueueHandle_t cmdQueue;    // Reemplaza entradaWeb/hayEntradaWeb — thread-safe

// Límites y Parámetros Operativos
extern const int MAX_WEBSOCKET_CLIENTS; //  Límite de conexiones para prevenir fugas de memoria OOM
extern bool permitirWebLog;       //  Permiso lógico para habilitar la transmisión de logs a la consola web
extern unsigned long tiempoInicioWeb; //  Registrador de tiempo para el control de la sesión web
extern bool sistemaListo;         //  Flag global que indica que los servicios base se han inicializado
extern bool forzarTelemetria;     //  Interruptor global para forzar el envío inmediato de datos JSON

// Variables Globales de Seguridad
extern String tokenSesionActiva;  //  Identificador alfa-numérico único de la cookie de sesión activa

// Búfers y Volcados de Memoria NFC
extern byte bloque0Habitacion[16]; //  Matriz persistente de datos de respaldo NFC configurada por defecto
extern byte bloqueEscaneado[16];   //  Almacenamiento estático temporal de datos leídos físicamente
extern byte bloqueManual[16];      //  Búfer auxiliar para almacenamiento de bloques manuales
extern byte *bloqueAEscribir;      //  Puntero dinámico a la fuente de datos que va a ser clonada
extern byte dumpTarjeta[1024];     //  Buffer para dump completo (16 sectores x 4 bloques x 16 bytes)
extern bool dumpValido;            //  Flag de integridad del dump completo

// Máquinas de Estado Operativas
int extern programaActivo;         //  Estado del módulo ejecutor principal (FSM General)
int extern modoNFC;                //  Sub-estado secuencial para la máquina de control RFID
bool extern memoriaLlena;          //  Flag de validación de integridad para indicar datos RFID cargados
bool extern midiendoDistancia;     //  Estado lógico de activación del sensor acústico de distancia
bool extern ledState;              //  Registro de alternancia eléctrica para el parpadeo del LED

// Variables de Sensores Ambientales
extern float temperaturaActual;    // Ultima lectura valida de temperatura en grados Celsius
extern float humedadActual;        // Ultima lectura valida de humedad relativa en porcentaje

// Temporizadores Basados en Tiempo de CPU (millis)
extern unsigned long tiempoUltimoMenuPrincipal; //  Almacenamiento del último instante del menú base
extern unsigned long previousMillisLED;         //  Control de tiempo de parpadeo del NeoPixel
extern unsigned long tiempoUltimaMedicion;      //  Control de muestreo del sensor de distancia
extern unsigned long tiempoUltimoMenuNFC;       //  Almacenamiento del último instante del menú RFID

// Medición real de uso de CPU por núcleo (actualizados en tareas.cpp)
extern volatile unsigned long tiempoProcesamientoCore[2]; // micros de trabajo real en el último ciclo
extern volatile unsigned long tiempoCicloCore[2];         // micros de duración total del último ciclo

// Constantes de Tiempos Críticos
extern const unsigned long intervaloMenu;       //  Límite de inactividad (ms) antes de refrescar el menú
extern const unsigned long intervaloMedicion;   //  Periodo de refresco de la lectura acústica (1Hz)

// Sincronización en Entornos Multitarea (FreeRTOS)
extern SemaphoreHandle_t i2cMutex;
// [FIX] Mutex para el bus SPI compartido entre MFRC522 y SD Card.
// Sin este mutex, operaciones simultáneas desde Core 0 (SD) y Core 1 (NFC)
// corrompen las transacciones SPI produciendo lecturas fantasma silenciosas.
extern SemaphoreHandle_t spiMutex;

// Flag de disponibilidad de la tarjeta SD
extern bool sdDisponible;                        

// Autenticación Telnet
extern const char* TELNET_PASSWORD;
extern bool telnetAutenticado;
extern int telnetIntentos;
extern unsigned long tiempoUltimaActividadTelnet; // Timestamp de última actividad Telnet
