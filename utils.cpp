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
 * @file utils.cpp
 * @brief Implementación de las herramientas de diagnóstico de rendimiento, persistencia local y telemetría LCD.
 */
#include "utils.h" // Vinculación formal con su archivo de cabecera de definición
#include "dht.h"   // Acceso a sensores.temperatura y sensores.humedad del DHT11
#include "sd_card.h" // Acceso a funciones de tarjeta SD
#include "esp_task_wdt.h"
#include "mbedtls/md.h"

// ============================================================================
// CÁLCULO REAL DE USO DE CPU POR NÚCLEO
// ============================================================================
uint32_t calcularUsoCPU(int coreID) { // 
  // Mide el porcentaje real de CPU usado en cada núcleo comparando el tiempo
  // de trabajo efectivo frente al tiempo total del ciclo de la tarea principal.
  // Los valores son actualizados por tareas.cpp en cada iteración del bucle.
  static uint32_t suavizado[2] = {5, 2};
  static unsigned long ultimoMs[2] = {0, 0};
  
  unsigned long ahora = millis();
  if (ahora - ultimoMs[coreID] < 1000) {
    return constrain(suavizado[coreID], 0, 100);
  }
  ultimoMs[coreID] = ahora;
  
  // [FIX B3] Leer par trabajo/ciclo bajo mutex para evitar data race entre núcleos.
  // Sin mutex, Core 1 puede actualizar tiempoProcesamientoCore[0] entre que Core 0
  // lee trabajo y ciclo, produciendo divisiones con operandos inconsistentes.
  uint32_t trabajo = 0, ciclo = 0;
  if (cpuMutex && xSemaphoreTake(cpuMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
    trabajo = (uint32_t)tiempoProcesamientoCore[coreID];
    ciclo   = (uint32_t)tiempoCicloCore[coreID];
    xSemaphoreGive(cpuMutex);
  } else {
    trabajo = (uint32_t)tiempoProcesamientoCore[coreID];
    ciclo   = (uint32_t)tiempoCicloCore[coreID];
  }

  uint32_t raw = 0;
  if (ciclo > 0 && trabajo <= ciclo) {
    raw = trabajo * 100 / ciclo;
  } else if (ciclo > 0 && trabajo > ciclo) {
    raw = 100;
  }
  
  suavizado[coreID] = constrain(
    (raw * 3 + suavizado[coreID] * 7) / 10,
    0, 100
  );
  
  return suavizado[coreID];
}

// ============================================================================
// FORMATO DE TIEMPOS DE ACTIVIDAD (UPTIME)
// ============================================================================
String obtenerUptime() { // 
  uint32_t sec = millis() / 1000; // Capturamos los segundos reales transcurridos convirtiendo milisegundos 
  char buff[20];                  // Búfer local estático de caracteres para formateo de texto
  
  // Segmentamos matemáticamente las horas totales, los minutos residuales y los segundos sobrantes
  sprintf(buff, "%02d:%02d:%02d", (sec / 3600), (sec % 3600) / 60, sec % 60); // 
  return String(buff);            // Retornamos el string formateado como un objeto nativo 
}

// ============================================================================
// CONSULTA Y FORMATEO DE CALENDARIO NTP
// ============================================================================
String obtenerFechaHora() {
  struct tm timeinfo; // Instanciación de la estructura estándar de tiempo de C de tipo tm
  
  // Consultamos el reloj de hardware interno; si el servidor NTP aún no ha sincronizado, abortamos
  if(!getLocalTime(&timeinfo)){ // 
    return "00/00/00 00:00"; // Fallback seguro para evitar escrituras de fechas corruptas 
  }
  char buff[25]; // Búfer de salida para albergar la cadena string armada
  
  // Procesamos la estructura inyectando máscaras: %d (día), %m (mes), %Y (año), %H (hora), %M (minuto)
  strftime(buff, sizeof(buff), "%d/%m/%Y %H:%M", &timeinfo); // 
  return String(buff); // Retornamos la estampa de tiempo legible 
}

// ============================================================================
// ESCRITURA DE ENTRADAS EN LA BASE DE DATOS LOCAL CSV (SD / LittleFS)
// ============================================================================
void guardarEnHistorial() { // 
    // Muestreamos la telemetría térmica e interna de los procesos multinúcleo
    int cargaC0 = calcularUsoCPU(0); // 
    int cargaC1 = calcularUsoCPU(1); // 
    int cargaTotal = (cargaC0 + cargaC1) / 2; // 

    String fechaHora = obtenerFechaHora(); // Capturamos la marca de tiempo sincronizada 
    float temp = temperatureRead();        // Leemos el sensor térmico embebido en el die del microprocesador 
    
    // Determinación precisa del tamaño total de memoria operativa (Diferenciando PSRAM externa de Heap interno)
    uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize(); // 
    
    // Ecuación porcentual para calcular la tasa de ocupación de la memoria RAM dinámica
    float ramUsada = (ESP.getFreePsram() > 0) ?
        100.0 - (ESP.getFreePsram() * 100.0 / ramTotal) : 100.0 - (ESP.getFreeHeap() * 100.0 / ESP.getHeapSize()); // 
    
    // Evaluación del volumen total asignado y ocupado dentro del espacio LittleFS
    float flashTotal = LittleFS.totalBytes(); // 
    float flashUsado = LittleFS.usedBytes();  // 
    
    // Redondeo matemático a dos decimales para el porcentaje ocupado de espacio en disco
    float flash = (flashTotal > 0) ?
        round(((flashUsado * 100.0) / flashTotal) * 100.0) / 100.0 : 0.0; // 
        
    // Transformación matemática de la potencia RSSI (dBm) del receptor de radio WiFi a una escala lineal de 0 a 100%
    int wifi = min(max(2 * (WiFi.RSSI() + 100), 0), 100); // 

    // [FIX B6] Truncar seguridad.ultimaAlerta antes del snprintf.
    // Sin truncar, una alerta larga (>~200 chars) puede superar los 320 bytes
    // del buffer y corromper la pila de Core 0.
    char alerta_trunc[180];
    snprintf(alerta_trunc, sizeof(alerta_trunc), "%s", seguridad.ultimaAlerta.c_str());

    // Construcción de la línea CSV completa en memoria (11 columnas)
    char linea[320];
    snprintf(linea, sizeof(linea), "%s,%.1f,%d,%d,%d,%.1f,%.1f,%d,%.1f,%.1f,%s\n",
        fechaHora.c_str(), temp, cargaTotal, cargaC0, cargaC1,
        ramUsada, flash, wifi, sensores.temperatura, sensores.humedad, alerta_trunc);
    seguridad.ultimaAlerta = "-";

    // Volcado físico: SD si está disponible, sino LittleFS
    if (sdDisponible) {
        esp_task_wdt_reset();
        guardarLineaSD(String(linea));
        esp_task_wdt_reset();
    } else {
        File file = LittleFS.open("/datos.csv", "a");
        if (!file) return;
        esp_task_wdt_reset();
        file.print(linea);
        esp_task_wdt_reset();
        file.close();
    }
    Serial.println("💾 [SISTEMA] Registro guardado (2 horas).");
}

// ============================================================================
// FUNCIONES DE PERSISTENCIA NVS (Credenciales WiFi)
// ============================================================================
bool guardarCredenciales(const char* ssid, const char* pass) {
    Preferences prefs;
    prefs.begin("zenithmc", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", pass);
    prefs.end();
    Serial.printf("[NVS] Credenciales guardadas: SSID=%s\n", ssid);
    return true;
}

bool cargarCredenciales(String& ssid, String& pass) {
    Preferences prefs;
    prefs.begin("zenithmc", true);
    ssid = prefs.getString("ssid", "");
    pass = prefs.getString("password", "");
    prefs.end();
    bool ok = (ssid.length() > 0 && pass.length() > 0);
    if (ok) Serial.printf("[NVS] Credenciales cargadas: SSID=%s\n", ssid.c_str());
    return ok;
}

bool borrarCredenciales() {
    Preferences prefs;
    prefs.begin("zenithmc", false);
    prefs.clear();
    prefs.end();
    Serial.println("[NVS] Credenciales borradas.");
    return true;
}

bool guardarSetupCompletado() {
    Preferences prefs;
    prefs.begin("zenithmc", false);
    prefs.putBool("setupDone", true);
    prefs.end();
    Serial.println("[NVS] Setup marcado como completado.");
    return true;
}

bool setupCompletado() {
    Preferences prefs;
    prefs.begin("zenithmc", true);
    bool done = prefs.getBool("setupDone", false);
    prefs.end();
    return done;
}

bool guardarConfigHardware(int nfcRst, int nfcSs, int trigPin, int echoPin, int dhtPin) {
    Preferences prefs;
    prefs.begin("hwconfig", false);
    prefs.putInt("nfcRst", nfcRst);
    prefs.putInt("nfcSs", nfcSs);
    prefs.putInt("trigPin", trigPin);
    prefs.putInt("echoPin", echoPin);
    prefs.putInt("dhtPin", dhtPin);
    prefs.end();
    Serial.println("[NVS] Configuracion de hardware guardada.");
    return true;
}

bool cargarConfigHardware() {
    Preferences prefs;
    prefs.begin("hwconfig", true);
    if (!prefs.isKey("nfcRst")) { prefs.end(); return false; }
    RST_PIN  = prefs.getInt("nfcRst", RST_PIN);
    SS_PIN   = prefs.getInt("nfcSs", SS_PIN);
    TRIG_PIN = prefs.getInt("trigPin", TRIG_PIN);
    ECHO_PIN = prefs.getInt("echoPin", ECHO_PIN);
    DHT_PIN  = prefs.getInt("dhtPin", DHT_PIN);
    prefs.end();
    Serial.printf("[NVS] Config hardware cargada: RST=%d CS=%d TRIG=%d ECHO=%d DHT=%d\n",
        RST_PIN, SS_PIN, TRIG_PIN, ECHO_PIN, DHT_PIN);
    return true;
}

bool guardarCredencialesWeb(String user, String pass) {
    Preferences prefs;
    prefs.begin("webcred", false);
    prefs.putString("user", user);
    prefs.putString("pass", pass);
    prefs.end();
    Serial.println("[NVS] Credenciales web guardadas.");
    return true;
}

bool cargarCredencialesWeb() {
    Preferences prefs;
    prefs.begin("webcred", true);
    if (!prefs.isKey("user")) { prefs.end(); return false; }
    webUser = prefs.getString("user", "admin");
    webPass = prefs.getString("pass", "blasco");
    prefs.end();
    Serial.println("[NVS] Credenciales web cargadas.");
    return true;
}

bool guardarClaveAI(String key) {
    Preferences prefs;
    prefs.begin("aikey", false);
    prefs.putString("key", key);
    prefs.end();
    Serial.println("[NVS] Clave AI guardada.");
    return true;
}

String cargarClaveAI() {
    Preferences prefs;
    prefs.begin("aikey", true);
    String key = prefs.getString("key", "");
    prefs.end();
    return key;
}

bool guardarConfigCFWorker(String host, String path, String token, bool habilitado) {
    Preferences prefs;
    prefs.begin("cfworker", false);
    prefs.putString("host",       host);
    prefs.putString("path",       path);
    prefs.putString("token",      token);
    prefs.putBool("habilitado",   habilitado);
    prefs.end();
    Serial.println("[NVS] Configuracion Cloudflare Worker guardada.");
    return true;
}

bool cargarConfigCFWorker(String& host, String& path, String& token, bool& habilitado) {
    Preferences prefs;
    prefs.begin("cfworker", true);
    host       = prefs.getString("host",  "");
    path       = prefs.getString("path",  "/esp-tunnel");
    token      = prefs.getString("token", "");
    habilitado = prefs.getBool("habilitado", false);
    prefs.end();
    return host.length() > 0;
}


// ============================================================================
// PAGINACIÓN Y RENDERIZADO GRÁFICO EN PANEL LCD I2C
// ============================================================================
void actualizarLCD() { // 
  // Solicitamos la llave del semáforo Mutex. Esto garantiza de forma absoluta que si una tarea en el Core 1
  // intenta usar el bus I2C, esperará a que el LCD termine su transmisión, evitando colisiones de hardware.
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100))) { // 

    // totalBytes() es constante (tamaño de partición); se cachea una vez para evitar
    // llamadas redundantes a la API de LittleFS en cada rotación de pantalla.
    static float flashTotalCached = 0.0f;
    if (flashTotalCached == 0.0f) flashTotalCached = (float)LittleFS.totalBytes();

    // Inicialización interna de variables de estado para el cálculo dinámico en pantalla
    uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize(); // 
    int ramUsada = (ESP.getFreePsram() > 0) ?
        100 - (ESP.getFreePsram() * 100 / ramTotal) : 100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize()); // 
        
    int wifiQ    = min(max(2 * (WiFi.RSSI() + 100), 0), 100); // 
    int c0       = calcularUsoCPU(0); // 
    int c1       = calcularUsoCPU(1); // 
    int cpu      = (c0 + c1) / 2; // 
    int temp     = (int)temperatureRead(); // 
    
    float flashUsado = LittleFS.usedBytes();  // 
    float flash = (flashTotalCached > 0) ? (flashUsado * 100.0 / flashTotalCached) : 0.0; // 
    String uptime = obtenerUptime();   // 

    static int pantalla = 0; // Puntero estático indexador de página que retiene su valor entre llamadas

    // Expresión Lambda anidada optimizada para generar dinámicamente las barras de progreso gráficas en texto plano
    auto barra = [](int valor, int ancho) -> String {
      int bloques = map(constrain(valor, 0, 100), 0, 100, 0, ancho); // Mapeo lineal del porcentaje al ancho en caracteres 
      String b = "|"; // Carácter de apertura de contenedor 
      for (int i = 0; i < ancho; i++) b += (i < bloques) ? "=" : " "; // Inyección de bloques activos o vacíos 
      b += "|"; // Carácter de cierre de contenedor 
      return b; // Devuelve el contenedor gráfico 
    };

    // Cache local de volátiles para evitar TOCTOU entre la preparación y el volcado
    bool alertaActiva = seguridad.modo && seguridad.nivel >= 3;

    char fila0[17], fila1[17];
    memset(fila0, ' ', 16); fila0[16] = '\0';
    memset(fila1, ' ', 16); fila1[16] = '\0';
    
    // ── Override: pantalla de alerta si nivel crítico activo ──
    if (alertaActiva) {
      snprintf(fila0, sizeof(fila0), " S. SEGURIDAD:  ");
      // fila1: "  ⚠ Critico ⚠   " — no usar (char)0 en el array
      // porque el volcado lcd.write() lo filtra como \0 → espacio.
      // Se escribe directamente con lcd.write(0) más abajo.
      memset(fila1, ' ', 16); fila1[16] = '\0';
      fila1[4]  = 'C'; fila1[5]  = 'r'; fila1[6]  = 'i';
      fila1[7]  = 't'; fila1[8]  = 'i'; fila1[9]  = 'c'; fila1[10] = 'o';
    } else {
      // Máquina de estados selectiva para la rotación cíclica de las 5 pantallas de diagnóstico
      switch (pantalla) { // 
      case 0: // Página A: Temperatura térmica y Uso promedio de procesamiento
        snprintf(fila0, sizeof(fila0), "TEMP%s%3dC", barra(temp, 5).c_str(), temp); // 
        snprintf(fila1, sizeof(fila1), "CPU %s%3d%%", barra(cpu, 5).c_str(), cpu); // 
        break;
      case 1: // Página B: Carga segregada del Núcleo 0 y Núcleo 1
        snprintf(fila0, sizeof(fila0), "C0  %s%3d%%", barra(c0, 5).c_str(), c0); // 
        snprintf(fila1, sizeof(fila1), "C1  %s%3d%%", barra(c1, 5).c_str(), c1); // 
        break;
      case 2: // Página C: Tasa operativa de RAM y Ocupación del espacio de archivos Flash
        snprintf(fila0, sizeof(fila0), "RAM %s%3d%%", barra(ramUsada, 5).c_str(), ramUsada); // 
        snprintf(fila1, sizeof(fila1), "FLSH%s%.1f%%", barra(flash, 5).c_str(), (float)flash); // 
        break;
      case 3: // Página D: Porcentaje de potencia WiFi y Tiempo de actividad acumulado
        snprintf(fila0, sizeof(fila0), "WIFI%s%3d%%", barra(wifiQ, 5).c_str(), wifiQ); // 
        snprintf(fila1, sizeof(fila1), "UP   %.8s   ", uptime.c_str()); // Recorte estricto a los primeros 8 caracteres del Uptime 
        break;
      case 4: { // Página E: Temperatura y humedad del sensor externo DHT11
        int dhtT = (sensores.temperatura > -127.0) ? (int)round(sensores.temperatura) : -1;
        int dhtH = (sensores.humedad >= 0.0)       ? (int)round(sensores.humedad)     : -1;
        if (dhtT >= 0) {
          snprintf(fila0, sizeof(fila0), "DHT T: %3d C    ", dhtT);
          snprintf(fila1, sizeof(fila1), "DHT H: %3d %%   ", dhtH);
        } else {
          snprintf(fila0, sizeof(fila0), "DHT11: SIN DATO ");
          snprintf(fila1, sizeof(fila1), "Conectar GPIO 4 ");
        }
        break;
      }
      } // fin switch
    } // fin else (rotación normal)

    // Volcado de datos directo al hardware controlador LCD a través del bus físico
    lcd.setCursor(0, 0);
    for (int i = 0; i < 16; i++) lcd.write(fila0[i] == '\0' ? ' ' : (uint8_t)fila0[i]);
    lcd.setCursor(0, 1);
    if (alertaActiva) {
      // Fila 1 con triángulos en posiciones 2 y 12
      for (int i = 0; i < 16; i++) {
        if (i == 2 || i == 12) lcd.write((uint8_t)0);
        else lcd.write(fila1[i] == '\0' ? ' ' : (uint8_t)fila1[i]);
      }
    } else {
      for (int i = 0; i < 16; i++) lcd.write(fila1[i] == '\0' ? ' ' : (uint8_t)fila1[i]);
    }

    pantalla = (pantalla + 1) % 5; // Incrementamos cíclicamente el indexador moviéndolo entre el rango 0-4 

    xSemaphoreGive(i2cMutex); // Liberación mandatoria del semáforo Mutex para habilitar el uso del bus a otros hilos 
  }
}

String hmacSha256(const String& key, const String& payload) {
  if (key.length() == 0 || payload.length() == 0) return "";
  uint8_t result[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  // [FIX U5] Verificar que el algoritmo SHA256 está disponible en esta build de mbedTLS.
  // mbedtls_md_info_from_type() devuelve NULL si el algoritmo no está compilado.
  const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
  if (md_info == NULL) {
    mbedtls_md_free(&ctx);
    return "";
  }
  if (mbedtls_md_setup(&ctx, md_info, 1)) {
    mbedtls_md_free(&ctx);
    return "";
  }
  if (mbedtls_md_hmac_starts(&ctx, (const uint8_t*)key.c_str(), key.length()) ||
      mbedtls_md_hmac_update(&ctx, (const uint8_t*)payload.c_str(), payload.length()) ||
      mbedtls_md_hmac_finish(&ctx, result)) {
    mbedtls_md_free(&ctx);
    return "";
  }
  mbedtls_md_free(&ctx);
  char hex[65];
  for (int i = 0; i < 32; i++) sprintf(hex + i * 2, "%02x", result[i]);
  hex[64] = 0;
  return String(hex);
}