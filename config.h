
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
#include <Adafruit_NeoPixel.h>    // [cite: 4] Gestión del direccionamiento del LED RGB de estado
#include <nvs_flash.h>            // [cite: 4] Librería para formatear la partición oculta del WiFi si es necesario
#include <Arduino.h>              // [cite: 4] Núcleo esencial del entorno Arduino para microcontroladores ESP32
#include <WiFi.h>                 // [cite: 4] Gestión y control de conexiones de redes inalámbricas
#include <ESPAsyncWebServer.h>    // [cite: 4] Motor del servidor web asíncrono y no bloqueante
#include <LittleFS.h>             // [cite: 4] Sistema de archivos interno de la memoria de almacenamiento flash
#include <ArduinoJson.h>          // [cite: 4] Procesamiento, serialización y deserialización de estructuras JSON
#include <ArduinoOTA.h>           // [cite: 5] Actualizaciones seguras de firmware a través de WiFi (Over-The-Air)
#include <SPI.h>                  // [cite: 5] Bus de comunicación por hardware para periféricos rápidos
#include <MFRC522.h>              // [cite: 5] Controlador específico del módulo lector físico NFC/RFID
#include "time.h"                 // [cite: 5] Gestión del reloj interno del sistema y cálculo del Uptime
#include <Wire.h>                 // [cite: 6] Bus de comunicación I2C por hardware
#include <LiquidCrystal_I2C.h>    // [cite: 6] Driver de control de la pantalla de cristal líquido LCD

// ============================================================================
// CONFIGURACIÓN DE PINES (ESP32-S3 N16R8)
// ============================================================================
// Conexión Física de Pantalla: SDA -> GPIO 8 | SCL -> GPIO 9 [cite: 6, 7]
#define LED_PIN         48        // [cite: 9] Pin físico del LED RGB de estado en la placa base
#define NUMPIXELS       1         // [cite: 9] Número total de LEDs NeoPixel integrados en el hardware

#define RST_PIN         21        // [cite: 10] Pin de reinicio de hardware (Reset) del módulo lector MFRC522
#define SS_PIN          5         // [cite: 10] Pin de selección de esclavo SPI (SDA/CS) del módulo MFRC522

#define TRIG_PIN        15        // [cite: 10] Pin digital de salida para el emisor de pulsos ultrasónicos
#define ECHO_PIN        16        // [cite: 10] Pin digital de entrada para el receptor de ecos ultrasónicos

#define DHT_PIN         4         // Pin de datos del sensor DHT11
#define DHT_TYPE        DHT11     // Modelo del sensor de temperatura y humedad

// ============================================================================
// DECLARACIONES EXTERNAS (Para evitar duplicados enlazados por el Linker)
// ============================================================================

// Credenciales de Conexión
extern const char* ssid;          // [cite: 7] Nombre identificador de la red inalámbrica local
extern const char* password;      // [cite: 8] Clave de acceso de la red local cifrada

// Instancias globales de Periféricos
extern LiquidCrystal_I2C lcd;     // [cite: 7] Objeto de control de la pantalla LCD por interfaz I2C
extern Adafruit_NeoPixel pixel;   // [cite: 9] Objeto de control del LED RGB NeoPixel de la placa
// NOTA: extern DHT dht se declara en dht.h (requiere <DHT.h> antes de config.h)

// Instancias de Servidores de Red
extern AsyncWebServer* server;    // [cite: 11] Puntero global al servidor HTTP asíncrono
extern AsyncWebSocket* ws;        // [cite: 11] Puntero al canal WebSocket para comunicación web en tiempo real
extern WiFiServer telnetServer;   // [cite: 12] Instancia del servidor TCP clásico para conexiones por terminal SSH/Putty
extern WiFiClient telnetClient;   // [cite: 13] Objeto de sesión del cliente TCP actualmente activo en el sistema

// Instancias del Hardware de Radiofrecuencia
extern MFRC522 mfrc522;           // [cite: 14] Objeto controlador vinculado a los pines físicos del lector RFID
extern MFRC522::MIFARE_Key key;   // [cite: 15] Estructura de almacenamiento de claves simétricas MIFARE

// Búfers de Sincronización y Flags
extern String entradaWeb;         // [cite: 16] Almacenamiento temporal de comandos de texto de la interfaz web
extern bool hayEntradaWeb;        // [cite: 17] Bandera de estado para la sincronización de hilos entre núcleos

// Límites y Parámetros Operativos
extern const int MAX_WEBSOCKET_CLIENTS; // [cite: 18] Límite de conexiones para prevenir fugas de memoria OOM
extern bool permitirWebLog;       // [cite: 19] Permiso lógico para habilitar la transmisión de logs a la consola web
extern unsigned long tiempoInicioWeb; // [cite: 20] Registrador de tiempo para el control de la sesión web
extern bool sistemaListo;         // [cite: 21] Flag global que indica que los servicios base se han inicializado
extern bool forzarTelemetria;     // [cite: 21] Interruptor global para forzar el envío inmediato de datos JSON

// Variables Globales de Seguridad
extern String tokenSesionActiva;  // [cite: 22] Identificador alfa-numérico único de la cookie de sesión activa

// Búfers y Volcados de Memoria NFC
extern byte bloque0Habitacion[16]; // [cite: 38] Matriz persistente de datos de respaldo NFC configurada por defecto
extern byte bloqueEscaneado[16];   // [cite: 39] Almacenamiento estático temporal de datos leídos físicamente
extern byte bloqueManual[16];      // [cite: 39] Búfer auxiliar para almacenamiento de bloques manuales
extern byte *bloqueAEscribir;      // [cite: 40] Puntero dinámico a la fuente de datos que va a ser clonada

// Máquinas de Estado Operativas
int extern programaActivo;         // [cite: 40] Estado del módulo ejecutor principal (FSM General)
int extern modoNFC;                // [cite: 41] Sub-estado secuencial para la máquina de control RFID
bool extern memoriaLlena;          // [cite: 42] Flag de validación de integridad para indicar datos RFID cargados
bool extern midiendoDistancia;     // [cite: 43] Estado lógico de activación del sensor acústico de distancia
bool extern ledState;              // [cite: 44] Registro de alternancia eléctrica para el parpadeo del LED

// Variables de Sensores Ambientales
extern float temperaturaActual;    // Ultima lectura valida de temperatura en grados Celsius
extern float humedadActual;        // Ultima lectura valida de humedad relativa en porcentaje

// Temporizadores Basados en Tiempo de CPU (millis)
extern unsigned long tiempoUltimoMenuPrincipal; // [cite: 45] Almacenamiento del último instante del menú base
extern unsigned long previousMillisLED;         // [cite: 46] Control de tiempo de parpadeo del NeoPixel
extern unsigned long tiempoUltimaMedicion;      // [cite: 46] Control de muestreo del sensor de distancia
extern unsigned long tiempoUltimoMenuNFC;       // [cite: 47] Almacenamiento del último instante del menú RFID

// Medición real de uso de CPU por núcleo (actualizados en tareas.cpp)
extern volatile unsigned long tiempoProcesamientoCore[2]; // micros de trabajo real en el último ciclo
extern volatile unsigned long tiempoCicloCore[2];         // micros de duración total del último ciclo

// Constantes de Tiempos Críticos
extern const unsigned long intervaloMenu;       // [cite: 47] Límite de inactividad (ms) antes de refrescar el menú
extern const unsigned long intervaloMedicion;   // [cite: 48] Periodo de refresco de la lectura acústica (1Hz)

// Sincronización en Entornos Multitarea (FreeRTOS)
extern SemaphoreHandle_t i2cMutex;

// Autenticación Telnet
extern const char* TELNET_PASSWORD;
extern bool telnetAutenticado;
extern int telnetIntentos;              // [cite: 48] Semáforo de exclusión mutua para la protección del bus I2C
