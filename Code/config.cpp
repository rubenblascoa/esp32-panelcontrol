
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

// Credenciales de Conexión de Red Local
const char* ssid = "NETLLAR_BLASCO"; // [cite: 7] SSID exacto de la red inalámbrica de destino
const char* password = "978830522ro_"; // [cite: 8] Frase de contraseña WPA/WPA2 de la red

// Objetos para Control de Hardware Integrado
LiquidCrystal_I2C lcd(0x27, 16, 2); // [cite: 7] Dirección hexadecimal estándar I2C 0x27, 16 columnas y 2 filas
Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800); // [cite: 9] Configuración básica del bus NeoPixel
// NOTA: DHT dht(...) se define en dht.cpp

// Inicialización de Servidores en Red (Punteros asignados inicialmente a nulo)
AsyncWebServer* server = nullptr; // [cite: 11] Se reservará dinámicamente mediante operador new en el setup
AsyncWebSocket* ws = nullptr;     // [cite: 11] Se reservará dinámicamente junto con el servidor HTTP
WiFiServer telnetServer(23);      // [cite: 12] Vinculación del servidor TCP clásico al puerto estándar Telnet (23)
WiFiClient telnetClient;          // [cite: 13] Instancia vacía preparada para albergar el socket de conexión

// Inicialización del Lector de Radiofrecuencia
MFRC522 mfrc522(SS_PIN, RST_PIN); // [cite: 14] Instanciación pasando los pines de control SPI definidos
MFRC522::MIFARE_Key key;          // [cite: 15] Estructura de claves vacía lista para configurarse en la ejecución

// Estados Lógicos Internos y Controladores de Sincronismo
String entradaWeb = "";           // [cite: 16] Inicialización del búfer web vacío
bool hayEntradaWeb = false;       // [cite: 17] Flag desactivado por defecto (espera comando)
const int MAX_WEBSOCKET_CLIENTS = 2; // [cite: 18] Seguridad contra el desbordamiento crítico de memoria RAM
bool permitirWebLog = false;      // [cite: 19] Comienza desactivado para proteger el búfer inicial
unsigned long tiempoInicioWeb = 0; // [cite: 20] Cronómetro puesto a cero
bool sistemaListo = false;        // [cite: 21] Bloquea hilos de ejecución hasta que setup finalice exitosamente
bool forzarTelemetria = false;    // [cite: 21] Flag inactivo por defecto (se activa al conectar clientes)
String tokenSesionActiva = "";    // [cite: 22] Vacío por defecto (ningún usuario validado al arrancar)

// Datos Estáticos y Dinámicos NFC
byte bloque0Habitacion[16] = {0x60, 0xC9, 0x13, 0x1C, 0xA6, 0x08, 0x04, 0x00, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69}; // [cite: 38] Bloque original extraído de respaldo
byte bloqueEscaneado[16];         // [cite: 39] Espacio en RAM para volcar lecturas de tarjetas físicas
byte bloqueManual[16];            // [cite: 39] Espacio en RAM para operaciones de escritura alternativa
byte *bloqueAEscribir = nullptr;  // [cite: 40] Puntero seguro inicializado apuntando a la dirección nula

// Punteros de Estado Base y Flags de Modulos Hardware
int programaActivo = 0;           // [cite: 40] Comienza en el menú principal por defecto en el arranque
int modoNFC = 0;                  // [cite: 41] Subrutinas NFC inactivas en el inicio del firmware
bool memoriaLlena = false;        // [cite: 42] Indica que el búfer bloqueEscaneado está vacío al iniciar
bool midiendoDistancia = false;   // [cite: 43] El sensor ultrasónico comienza apagado (sin lanzar ráfagas)
bool ledState = LOW;              // [cite: 44] Estado eléctrico inicial bajo (apagado)

// Estado Compartido de Sensores Ambientales
float temperaturaActual = -127.0; // Centinela de "sin lectura": el menú y la LCD lo detectan y muestran "Sin dato"
float humedadActual = -1.0;       // Centinela de "sin lectura": -1.0 indica que el sensor no ha respondido aún

// Registros Temporales de Reloj Interno (Inicializados a Cero)
unsigned long tiempoUltimoMenuPrincipal = 0; // [cite: 45] Instante inicial para refrescos
unsigned long previousMillisLED = 0;         // [cite: 46] Instante inicial para ciclos estroboscópicos
unsigned long tiempoUltimaMedicion = 0;      // [cite: 46] Instante inicial para muestreo del sensor HC-SR04
unsigned long tiempoUltimoMenuNFC = 0;       // [cite: 47] Instante inicial para actualización de pantallas NFC

// Umbrales de Tiempo Constantes
const unsigned long intervaloMenu = 300000;  // [cite: 47] 5 minutos de tiempo muerto antes de redibujar menús
const unsigned long intervaloMedicion = 1000; // [cite: 48] 1000 milisegundos equivalen a lecturas a frecuencia de 1Hz

// Semáforos de Exclusión Mutua para FreeRTOS
SemaphoreHandle_t i2cMutex = nullptr;        // [cite: 48] Puntero inicial nulo para el manejador de exclusión mutua I2C

// Autenticación Telnet
const char* TELNET_PASSWORD = "blasco";      // Contraseña para acceso Telnet
bool telnetAutenticado = false;              // Flag de autenticación Telnet
int telnetIntentos = 0;                      // Contador de intentos fallidos Telnet
