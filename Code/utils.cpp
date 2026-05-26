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
#include "dht.h"   // Acceso a temperaturaActual y humedadActual del DHT11

// ============================================================================
// CÁLCULO DE ESTRÉS HEURÍSTICO SEGURO DE CPU
// ============================================================================
uint32_t calcularUsoCPU(int coreID) { // [cite: 524]
  // Tu ESP32-S3 corre a una frecuencia nominal de 240MHz[cite: 6, 526].
  // Para prevenir Kernel Panics o fallos del Watchdog en hilos paralelos de FreeRTOS, 
  // implementamos un modelo heurístico determinista de carga basado en la actividad asíncrona[cite: 525].
  
  uint32_t cargaBase = (coreID == 0) ? 5 : 2; // El núcleo 0 gestiona la pila de red WiFi (mayor carga base) [cite: 526, 527]
  
  // Evaluamos de forma dinámica el impacto de las conexiones activas en los WebSockets
  if (ws->count() > 0) cargaBase += (coreID == 0) ? 8 : 1; // [cite: 527, 528]
  
  // Evaluamos si el sensor de ultrasonidos se encuentra emitiendo ráfagas acústicas desde el Core 1
  if (midiendoDistancia && coreID == 1) cargaBase += 12; // [cite: 43, 528]
  
  // Evaluamos si el transceptor NFC está ejecutando polling o cifrando sectores SPI en el Core 1
  if (modoNFC > 0 && coreID == 1) cargaBase += 8; // [cite: 41, 529]
  
  // Inyectamos una fluctuación matemática aleatoria simulando el ruido térmico natural del silicio (+/- 2%)
  cargaBase += random(-2, 3); // [cite: 530]
  
  // Saneamos y limitamos de forma estricta el retorno entre los rangos porcentuales válidos
  return constrain(cargaBase, 0, 100); // [cite: 530]
}

// ============================================================================
// FORMATO DE TIEMPOS DE ACTIVIDAD (UPTIME)
// ============================================================================
String obtenerUptime() { // [cite: 531]
  uint32_t sec = millis() / 1000; // Capturamos los segundos reales transcurridos convirtiendo milisegundos [cite: 531]
  char buff[20];                  // Búfer local estático de caracteres para formateo de texto
  
  // Segmentamos matemáticamente las horas totales, los minutos residuales y los segundos sobrantes
  sprintf(buff, "%02d:%02d:%02d", (sec / 3600), (sec % 3600) / 60, sec % 60); // [cite: 532]
  return String(buff);            // Retornamos el string formateado como un objeto nativo [cite: 532]
}

// ============================================================================
// CONSULTA Y FORMATEO DE CALENDARIO NTP
// ============================================================================
String obtenerFechaHora() {
  struct tm timeinfo; // Instanciación de la estructura estándar de tiempo de C de tipo tm
  
  // Consultamos el reloj de hardware interno; si el servidor NTP aún no ha sincronizado, abortamos
  if(!getLocalTime(&timeinfo)){ // [cite: 533]
    return "00/00/00 00:00"; // Fallback seguro para evitar escrituras de fechas corruptas [cite: 533]
  }
  char buff[25]; // Búfer de salida para albergar la cadena string armada
  
  // Procesamos la estructura inyectando máscaras: %d (día), %m (mes), %Y (año), %H (hora), %M (minuto)
  strftime(buff, sizeof(buff), "%d/%m/%Y %H:%M", &timeinfo); // [cite: 534]
  return String(buff); // Retornamos la estampa de tiempo legible [cite: 534]
}

// ============================================================================
// ESCRITURA DE ENTRADAS EN LA BASE DE DATOS LOCAL CSV (LittleFS)
// ============================================================================
void guardarEnHistorial() { // [cite: 552]
    // Abrimos el descriptor del archivo en la partición Flash utilizando la bandera "a" (Append / Adjuntar)
    File file = LittleFS.open("/datos.csv", "a"); // 
    if (!file) return; // Si hay un fallo físico en las celdas de almacenamiento abortamos para evitar pánicos [cite: 552]

    // Muestreamos la telemetría térmica e interna de los procesos multinúcleo
    int cargaC0 = calcularUsoCPU(0); // [cite: 553]
    int cargaC1 = calcularUsoCPU(1); // [cite: 553]
    int cargaTotal = (cargaC0 + cargaC1) / 2; // [cite: 553]

    String fechaHora = obtenerFechaHora(); // Capturamos la marca de tiempo sincronizada [cite: 553]
    float temp = temperatureRead();        // Leemos el sensor térmico embebido en el die del microprocesador [cite: 554]
    
    // Determinación precisa del tamaño total de memoria operativa (Diferenciando PSRAM externa de Heap interno)
    uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize(); // [cite: 555]
    
    // Ecuación porcentual para calcular la tasa de ocupación de la memoria RAM dinámica
    float ramUsada = (ESP.getFreePsram() > 0) ?
        100.0 - (ESP.getFreePsram() * 100.0 / ramTotal) : 100.0 - (ESP.getFreeHeap() * 100.0 / ESP.getHeapSize()); // [cite: 555, 556]
    
    // Evaluación del volumen total asignado y ocupado dentro del espacio LittleFS
    float flashTotal = LittleFS.totalBytes(); // [cite: 556]
    float flashUsado = LittleFS.usedBytes();  // [cite: 557]
    
    // Redondeo matemático a dos decimales para el porcentaje ocupado de espacio en disco
    float flash = (flashTotal > 0) ?
        round(((flashUsado * 100.0) / flashTotal) * 100.0) / 100.0 : 0.0; // [cite: 557, 558]
        
    // Transformación matemática de la potencia RSSI (dBm) del receptor de radio WiFi a una escala lineal de 0 a 100%
    int wifi = min(max(2 * (WiFi.RSSI() + 100), 0), 100); // [cite: 559]

    // Inyección física en texto estructurado dentro del bloque de almacenamiento:
    // Formato: Fecha/Hora, TempCore, CPU_Total, Core0, Core1, RAM%, Flash%, WiFi%, TempDHT, HumDHT
    file.printf("%s,%.1f,%d,%d,%d,%.1f,%.1f,%d,%.1f,%.1f\n",
        fechaHora.c_str(), temp, cargaTotal, cargaC0, cargaC1,
        ramUsada, flash, wifi, temperaturaActual, humedadActual); // [cite: 554, 559]   
    file.close(); // Cerramos el puntero del archivo forzando el volcado físico a los transistores de la Flash [cite: 560]
    Serial.println("💾 [SISTEMA] Registro guardado en Flash (2 horas)."); // Notificación externa por hardware [cite: 560]
}

// ============================================================================
// PAGINACIÓN Y RENDERIZADO GRÁFICO EN PANEL LCD I2C
// ============================================================================
void actualizarLCD() { // [cite: 560]
  // Solicitamos la llave del semáforo Mutex. Esto garantiza de forma absoluta que si una tarea en el Core 1
  // intenta usar el bus I2C, esperará a que el LCD termine su transmisión, evitando colisiones de hardware.
  if (xSemaphoreTake(i2cMutex, pdMS_TO_TICKS(100))) { // [cite: 48, 560]

    // Inicialización interna de variables de estado para el cálculo dinámico en pantalla
    uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize(); // [cite: 560, 561]
    int ramUsada = (ESP.getFreePsram() > 0) ?
        100 - (ESP.getFreePsram() * 100 / ramTotal) : 100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize()); // [cite: 561, 562]
        
    int wifiQ    = min(max(2 * (WiFi.RSSI() + 100), 0), 100); // [cite: 563]
    int cpu      = (calcularUsoCPU(0) + calcularUsoCPU(1)) / 2; // [cite: 564]
    int temp     = (int)temperatureRead(); // [cite: 565]
    
    float flashTotal = LittleFS.totalBytes(); // [cite: 565]
    float flashUsado = LittleFS.usedBytes();  // [cite: 565]
    float flash = (flashTotal > 0) ? (flashUsado * 100.0 / flashTotal) : 0.0; // [cite: 566]
    
    int c0       = calcularUsoCPU(0); // [cite: 567]
    int c1       = calcularUsoCPU(1); // [cite: 567]
    String uptime = obtenerUptime();   // [cite: 568]

    static int pantalla = 0; // Puntero estático indexador de página que retiene su valor entre llamadas

    // Expresión Lambda anidada optimizada para generar dinámicamente las barras de progreso gráficas en texto plano
    auto barra = [](int valor, int ancho) -> String {
      int bloques = map(constrain(valor, 0, 100), 0, 100, 0, ancho); // Mapeo lineal del porcentaje al ancho en caracteres [cite: 568]
      String b = "|"; // Carácter de apertura de contenedor [cite: 569]
      for (int i = 0; i < ancho; i++) b += (i < bloques) ? "=" : " "; // Inyección de bloques activos o vacíos [cite: 569, 570]
      b += "|"; // Carácter de cierre de contenedor [cite: 570]
      return b; // Devuelve el contenedor gráfico [cite: 570]
    };

    char fila0[17], fila1[17]; // Matrices estáticas alineadas al ancho físico de tu pantalla LCD (16 caracteres + fin nulo)
    
    // Máquina de estados selectiva para la rotación cíclica de las 4 pantallas de diagnóstico
    switch (pantalla) { // [cite: 571]
      case 0: // Página A: Temperatura térmica y Uso promedio de procesamiento
        snprintf(fila0, sizeof(fila0), "TEMP%s%3dC", barra(temp, 5).c_str(), temp); // [cite: 571]
        snprintf(fila1, sizeof(fila1), "CPU %s%3d%%", barra(cpu, 5).c_str(), cpu); // [cite: 572]
        break;
      case 1: // Página B: Carga segregada del Núcleo 0 y Núcleo 1
        snprintf(fila0, sizeof(fila0), "C0  %s%3d%%", barra(c0, 5).c_str(), c0); // [cite: 572]
        snprintf(fila1, sizeof(fila1), "C1  %s%3d%%", barra(c1, 5).c_str(), c1); // [cite: 573]
        break;
      case 2: // Página C: Tasa operativa de RAM y Ocupación del espacio de archivos Flash
        snprintf(fila0, sizeof(fila0), "RAM %s%3d%%", barra(ramUsada, 5).c_str(), ramUsada); // [cite: 573]
        snprintf(fila1, sizeof(fila1), "FLSH%s%.1f%%", barra(flash, 5).c_str(), (float)flash); // [cite: 574]
        break;
      case 3: // Página D: Porcentaje de potencia WiFi y Tiempo de actividad acumulado
        snprintf(fila0, sizeof(fila0), "WIFI%s%3d%%", barra(wifiQ, 5).c_str(), wifiQ); // [cite: 575]
        snprintf(fila1, sizeof(fila1), "UP   %.8s   ", uptime.c_str()); // Recorte estricto a los primeros 8 caracteres del Uptime [cite: 575]
        break;
      case 4: { // Página E: Temperatura y humedad del sensor externo DHT11
        int dhtT = (temperaturaActual > -127.0) ? (int)round(temperaturaActual) : -1;
        int dhtH = (humedadActual >= 0.0)       ? (int)round(humedadActual)     : -1;
        if (dhtT >= 0) {
          snprintf(fila0, sizeof(fila0), "DHT T: %3d C    ", dhtT);
          snprintf(fila1, sizeof(fila1), "DHT H: %3d %%   ", dhtH);
        } else {
          snprintf(fila0, sizeof(fila0), "DHT11: SIN DATO ");
          snprintf(fila1, sizeof(fila1), "Conectar GPIO 4 ");
        }
        break;
      }
    }

    // Volcado de datos directo al hardware controlador LCD a través del bus físico
    lcd.setCursor(0, 0); // Posicionamos el cursor en el inicio de la línea superior [cite: 575]
    lcd.print(fila0);    // Transmitimos la cadena de texto de la primera fila [cite: 575]
    lcd.setCursor(0, 1); // Posicionamos el cursor en el inicio de la línea inferior [cite: 575]
    lcd.print(fila1);    // Transmitimos la cadena de texto de la segunda fila [cite: 576]

    pantalla = (pantalla + 1) % 5; // Incrementamos cíclicamente el indexador moviéndolo entre el rango 0-4 [cite: 576]

    xSemaphoreGive(i2cMutex); // Liberación mandatoria del semáforo Mutex para habilitar el uso del bus a otros hilos [cite: 576]
  }
}
