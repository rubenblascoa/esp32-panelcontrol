
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
 * @file config.cpp
 * @brief Definición e inicialización de variables globales del sistema.
 */
#include "config.h" // Vinculamos las firmas del archivo de cabecera

// ============================================================================
// INICIALIZACIÓN COPIA EN RAM DE VARIABLES GLOBALES
// ============================================================================

// Modo de Configuración Inicial (AP + Captive Portal)
bool modoConfiguracion = false;    // false = modo normal STA; true = AP de setup
DNSServer dnsServer;               // Captive portal DNS

// Estado de conexión asíncrona WiFi (config mode)
volatile bool conexionEnProgreso = false;
volatile bool conexionExitosa = false;
String conexionIP = "";
volatile unsigned long inicioConexion = 0;

// Objetos para Control de Hardware Integrado
LiquidCrystal_I2C lcd(0x27, 16, 2); //  Dirección hexadecimal estándar I2C 0x27, 16 columnas y 2 filas
Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800); //  Configuración básica del bus NeoPixel
// NOTA: DHT dht(...) se define en dht.cpp

// Inicialización de Servidores en Red (Punteros asignados inicialmente a nulo)
AsyncWebServer* server = nullptr; //  Se reservará dinámicamente mediante operador new en el setup
AsyncWebSocket* ws = nullptr;     //  Se reservará dinámicamente junto con el servidor HTTP
WiFiServer telnetServer(23);      //  Vinculación del servidor TCP clásico al puerto estándar Telnet (23)
WiFiClient telnetClient;          //  Instancia vacía preparada para albergar el socket de conexión

// Inicialización del Lector de Radiofrecuencia
MFRC522* mfrc522 = nullptr;        // Se inicializa en setup() con los pines cargados desde NVS
MFRC522::MIFARE_Key key;          //  Estructura de claves vacía lista para configurarse en la ejecución

// Credenciales web dinámicas (cargadas desde NVS)
String webUser = "admin";
String webPass = "blasco";

// Variables runtime de configuración de pines (cargadas desde NVS)
int RST_PIN  = 21;
int SS_PIN   = 5;
int TRIG_PIN = 15;
int ECHO_PIN = 16;
int DHT_PIN  = 4;

// Cola de comandos FreeRTOS para comunicación entre cores (thread-safe)
QueueHandle_t cmdQueue = nullptr; // Se crea en setup() con xQueueCreate
const int MAX_WEBSOCKET_CLIENTS = 2; //  Seguridad contra el desbordamiento crítico de memoria RAM
bool permitirWebLog = false;      //  Comienza desactivado para proteger el búfer inicial
unsigned long tiempoInicioWeb = 0; //  Cronómetro puesto a cero
bool sistemaListo = false;        //  Bloquea hilos de ejecución hasta que setup finalice exitosamente
bool forzarTelemetria = false;    //  Flag inactivo por defecto (se activa al conectar clientes)
String tokenSesionActiva = "";    //  Vacío por defecto (ningún usuario validado al arrancar)

// Datos Estáticos y Dinámicos NFC
byte bloque0Habitacion[16] = {0x60, 0xC9, 0x13, 0x1C, 0xA6, 0x08, 0x04, 0x00, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69}; //  Bloque original extraído de respaldo
byte bloqueEscaneado[16];         //  Espacio en RAM para volcar lecturas de tarjetas físicas
byte bloqueManual[16];            //  Espacio en RAM para operaciones de escritura alternativa
byte *bloqueAEscribir = nullptr;  //  Puntero seguro inicializado apuntando a la dirección nula
byte dumpTarjeta[1024];           //  Buffer para dump completo (16 sectores x 4 bloques x 16 bytes)
bool dumpValido = false;          //  Flag de integridad del dump completo

// Punteros de Estado Base y Flags de Modulos Hardware
volatile int programaActivo = 0;  //  Comienza en el menú principal por defecto en el arranque
int modoNFC = 0;                  //  Subrutinas NFC inactivas en el inicio del firmware
bool memoriaLlena = false;        //  Indica que el búfer bloqueEscaneado está vacío al iniciar
volatile bool midiendoDistancia = false; //  El sensor ultrasónico comienza apagado (sin lanzar ráfagas)
bool ledState = LOW;              //  Estado eléctrico inicial bajo (apagado)

// Estado Compartido de Sensores Ambientales
volatile float temperaturaActual = -127.0; // Centinela de "sin lectura": el menú y la LCD lo detectan y muestran "Sin dato"
volatile float humedadActual = -1.0;       // Centinela de "sin lectura": -1.0 indica que el sensor no ha respondido aún

// Registros Temporales de Reloj Interno (Inicializados a Cero)
unsigned long tiempoUltimoMenuPrincipal = 0; //  Instante inicial para refrescos
unsigned long previousMillisLED = 0;         //  Instante inicial para ciclos estroboscópicos
unsigned long tiempoUltimaMedicion = 0;      //  Instante inicial para muestreo del sensor HC-SR04
unsigned long tiempoUltimoMenuNFC = 0;       //  Instante inicial para actualización de pantallas NFC

// Variables de medición real de CPU por núcleo
volatile unsigned long tiempoProcesamientoCore[2] = {0, 0}; // micros de trabajo
volatile unsigned long tiempoCicloCore[2] = {10000, 10000}; // micros de ciclo (~10ms por defecto)

// Umbrales de Tiempo Constantes
const unsigned long intervaloMenu = 300000;  //  5 minutos de tiempo muerto antes de redibujar menús
const unsigned long intervaloMedicion = 1000; //  1000 milisegundos equivalen a lecturas a frecuencia de 1Hz

// Semáforos de Exclusión Mutua para FreeRTOS
SemaphoreHandle_t i2cMutex = nullptr;        //  Puntero inicial nulo para el manejador de exclusión mutua I2C
// [FIX] Mutex para bus SPI compartido (MFRC522 + SD Card)
SemaphoreHandle_t spiMutex = nullptr;

// Autenticación Telnet
const char* TELNET_PASSWORD = "blasco";      // Contraseña para acceso Telnet
volatile bool telnetAutenticado = false;              // Flag de autenticación Telnet
volatile int telnetIntentos = 0;                      // Contador de intentos fallidos Telnet
volatile unsigned long tiempoUltimaActividadTelnet = 0; // Timestamp de última actividad Telnet

// Tarjeta SD
bool sdDisponible = false;                   // Flag de disponibilidad de la tarjeta SD
