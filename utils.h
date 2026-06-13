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
 * @file utils.h
 * @brief Declaración de funciones de diagnóstico de CPU, tiempos NTP, LittleFS y control del LCD.
 */
#pragma once // Directiva que impide al preprocesador duplicar cabeceras en el proceso de compilación

#include "config.h" // Inclusión mandatoria para interactuar con tipos de datos de Arduino y objetos globales

// ============================================================================
// DECLARACIÓN DE FUNCIONES DE UTILIDAD Y APOYO SISTEMA
// ============================================================================

/**
 * @brief Calcula el porcentaje real de uso de CPU de un núcleo comparando tiempo de trabajo vs tiempo de ciclo.
 * @param coreID Identificador numérico del núcleo a evaluar (0 o 1).
 * @return uint32_t Porcentaje entero de uso real de CPU (0 a 100%).
 */
uint32_t calcularUsoCPU(int coreID); // 

/**
 * @brief Construye una cadena formateada con las horas, minutos y segundos transcurridos desde el arranque.
 * @return String Cadena de texto con formato de reloj "HH:MM:SS".
 */
String obtenerUptime(); // 

/**
 * @brief Consulta el chip de tiempo interno para formatear la fecha y hora obtenida por el servidor NTP.
 * @return String Cadena de texto con formato de calendario estándar "DD/MM/YYYY HH:MM".
 */
String obtenerFechaHora(); // 

/**
 * @brief Abre el volumen LittleFS en modo adjuntar y añade una línea CSV con las métricas actuales.
 */
void guardarEnHistorial(); // 

/**
 * @brief Orquesta la paginación de datos del sistema en la pantalla LCD I2C protegiendo el bus mediante Mutex.
 */
void actualizarLCD(); // 

// ============================================================================
// FUNCIONES DE PERSISTENCIA NVS (Credenciales WiFi)
// ============================================================================

/**
 * @brief Guarda SSID y contraseña WiFi en la partición NVS.
 * @param ssid Nombre de la red.
 * @param pass Contraseña de la red.
 * @return true si se guardó correctamente.
 */
bool guardarCredenciales(const char* ssid, const char* pass);

/**
 * @brief Carga SSID y contraseña WiFi desde NVS.
 * @param ssid String de salida con el SSID.
 * @param pass String de salida con la contraseña.
 * @return true si existían credenciales guardadas.
 */
bool cargarCredenciales(String& ssid, String& pass);

/**
 * @brief Borra las credenciales WiFi de la NVS.
 * @return true si se borraron correctamente.
 */
bool borrarCredenciales();

/**
 * @brief Guarda la configuracion de pines hardware en NVS.
 */
bool guardarConfigHardware(int nfcRst, int nfcSs, int trigPin, int echoPin, int dhtPin);

/**
 * @brief Carga la configuracion de pines hardware desde NVS y actualiza las variables globales.
 * @return true si existia configuracion guardada.
 */
bool cargarConfigHardware();

/**
 * @brief Guarda usuario y contraseña web en NVS.
 */
bool guardarCredencialesWeb(String user, String pass);

/**
 * @brief Carga las credenciales web desde NVS y actualiza webUser/webPass.
 * @return true si existian credenciales guardadas.
 */
bool cargarCredencialesWeb();

/**
 * @brief Guarda la API Key de Gemini AI en NVS.
 */
bool guardarClaveAI(String key);

/**
 * @brief Carga la API Key de Gemini AI desde NVS.
 * @return String con la clave, o cadena vacía si no existe.
 */
String cargarClaveAI();

/**
 * @brief Guarda un flag en NVS indicando que el setup inicial se completó.
 */
bool guardarSetupCompletado();

/**
 * @brief Verifica si el setup inicial ya se completó.
 * @return true si ya se configuró el WiFi al menos una vez.
 */
bool setupCompletado();

/**
 * @brief Escapa una cadena para su uso seguro dentro de un string JSON.
 * @param s Cadena de entrada con caracteres potencialmente peligrosos.
 * @return String lista para incrustar entre comillas dobles JSON.
 */
String jsonStr(const String& s);

/**
 * @brief Calcula HMAC-SHA256 de un payload usando una clave secreta (mbedTLS).
 * @param key  Clave secreta compartida.
 * @param payload Mensaje a firmar.
 * @return String con el hash hexadecimal en minúsculas (64 chars), o cadena vacía si falla.
 */
String hmacSha256(const String& key, const String& payload);

/**
 * @brief Guarda la configuración del túnel Cloudflare Worker en NVS.
 * @param host  Hostname del Worker (ej: "mi-worker.workers.dev")
 * @param path  Path del endpoint WebSocket (ej: "/esp-tunnel")
 * @param token Token secreto compartido entre el Worker y el ESP32
 * @return true siempre (no falla silenciosamente)
 */
bool guardarConfigCFWorker(String host, String path, String token, bool habilitado = false);

/**
 * @brief Carga la configuración del túnel Cloudflare Worker desde NVS.
 * @param host       String de salida con el hostname del Worker
 * @param path       String de salida con el path del endpoint
 * @param token      String de salida con el token secreto
 * @param habilitado bool de salida con el flag de habilitación
 * @return true si existía configuración guardada (host no vacío)
 */
bool cargarConfigCFWorker(String& host, String& path, String& token, bool& habilitado);