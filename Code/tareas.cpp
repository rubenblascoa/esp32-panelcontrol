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
 * @file tareas.cpp
 * @brief Implementación de los lazos infinitos multitarea distribuidos por hardware en el ESP32-S3.
 */
#include "tareas.h"     // Vinculación de prototipos propios
#include "web_server.h" // Habilita la inicialización de los endpoints web 
#include "utils.h"      // Permite el acceso a los diagnósticos de CPU, base de datos y LCD [cite: 592, 601, 589]
#include "menus.h"      // Habilita la inyección de los menús visuales de terminal [cite: 587, 591, 614, 615, 623]
#include "nfc.h"        // Permite invocar las subrutinas de interrogación del bus SPI [cite: 616, 617]
#include "ultrasonidos.h" // Habilita la captura de datos del transductor sónico [cite: 624]
#include "terminal.h"   // Habilita la impresión por el canal dual híbrido [cite: 587, 622, 623, 624, 625]
#include "dht.h"        // Habilita la inicialización y lectura del sensor DHT11 de temperatura y humedad

// ============================================================================
// CORE 0: PILA DE RED, SISTEMA DE ARCHIVOS, CRONJOBS DE BBDD Y LED RGB
// ============================================================================
void taskCore0(void * pvParameters) { // [cite: 576]
  // 1. Inicialización física y montaje del sistema de archivos LittleFS [cite: 576]
  // Configuramos el formateo automático a true para auto-recuperación del chip si está corrupto [cite: 576]
  if (!LittleFS.begin(true, "/spiffs", 10, "ffat")) { // [cite: 576]
      if (!LittleFS.begin(true)) Serial.println("❌ [ERROR] Flash no montada"); // [cite: 576]
      else Serial.println("✅ [OK] Flash montada en Modo Rescate"); // [cite: 577]
  } else {
      Serial.println("✅ [OK] Flash montada correctamente"); // [cite: 577]
  }

  // 2. Filtro de protección OOM y desbordamiento de base de datos local [cite: 578]
  // Si detectamos que el archivo de logs supera de forma crítica los 8 Megabytes, lo purgamos de inmediato [cite: 580]
  if (LittleFS.exists("/datos.csv")) { // [cite: 578]
      File dbFile = LittleFS.open("/datos.csv", "r"); // [cite: 578]
      if (dbFile) { // [cite: 579]
          size_t fileSize = dbFile.size(); // [cite: 579]
          dbFile.close(); // [cite: 579]
          if (fileSize > 8388608) { // [cite: 580]
              LittleFS.remove("/datos.csv"); // [cite: 580]
              Serial.println("🧹 [SISTEMA] Base de datos superó límite. Limpieza ejecutada."); // 
          }
      }
  }

  // 3. Encendido formal de las interfaces del servidor web asíncrono y Telnet [cite: 581, 582]
  iniciarServidorWeb(); // Levanta endpoints HTTP y WebSocket 
  telnetServer.begin(); // Pone a escuchar el puerto TCP 23 [cite: 582]

  // 4. Inicialización del sensor de temperatura y humedad DHT11
  inicializarDHT(); // Configura el pin GPIO4 y realiza la lectura de verificación inicial

  unsigned long lastTelemetryTime = 0; // Temporizador para el ciclo de la telemetría [cite: 582]
  unsigned long tiempoUltimoGuardadoBD = 0; // Temporizador para el cronjob del CSV [cite: 582]
  unsigned long tiempoUltimoDHT = 0;    // Temporizador para el polling periódico del DHT11 (30s)
  bool primerGuardado = true; // Bandera de sincronización inicial para el primer volcado NTP [cite: 583]

  // ==========================================================================
  // LAZO INFINITO ASÍNCRONO DEL NÚCLEO 0
  // ==========================================================================
  uint32_t inicioCiclo0 = micros(); // inicio de medicion de ciclo para CPU real
  for(;;) { // [cite: 584]                      
    // Barrera de seguridad: Mantiene suspendida la tarea hasta que setup() dé luz verde [cite: 584]
    if (!sistemaListo) { // [cite: 584]
        vTaskDelay(100 / portTICK_PERIOD_MS); // Bloqueo de alivio de 100ms [cite: 584]
        continue; // [cite: 585]
    }

    // Medicion de duracion total del ciclo anterior
    uint32_t ahora = micros();
    tiempoCicloCore[0] = ahora - inicioCiclo0;
    inicioCiclo0 = ahora;

    ArduinoOTA.handle(); // Atendemos de forma nativa posibles ráfagas de firmware por red [cite: 585]
    yield();             // Cedemos slots temporales de CPU de manera explícita para alimentar al core [cite: 585]

    // --- 0. POLLING PERIÓDICO DEL SENSOR DHT11 (Cada 30 segundos) ---
    if (millis() - tiempoUltimoDHT >= 3000UL) {
        tiempoUltimoDHT = millis();
        actualizarDHT(); // Lee temperatura y humedad, actualiza temperaturaActual y humedadActual
    }
    
    // --- 1. DETECCIÓN Y ASIGNACIÓN DE SESIONES TELNET TCP ---
    if (telnetServer.hasClient()) { // [cite: 585]
      // Si ya existe una sesión en curso con un cliente Putty/CMD conectado, rechazamos al intruso [cite: 585]
      if (telnetClient && telnetClient.connected()) { // [cite: 585]
          telnetServer.available().stop(); // Fuerza el cierre inmediato del puerto para el segundo socket [cite: 586]
      } else {                      
        // Si el canal está libre, aceptamos de forma nativa la sesión entrante [cite: 586]
        telnetClient = telnetServer.available(); // Asignamos el socket al cliente global [cite: 586]
        Terminal.println("\n[SISTEMA] Conectado TCP."); // [cite: 587]
        mostrarMenuPrincipal();   // Dibuja el panel CLI de forma directa [cite: 587]
      }
    }

    // --- 2. REFRESCO PERIÓDICO DE DIAGNÓSTICOS EN PANTALLA LCD (3 Segundos) ---
    static unsigned long tiempoUltimoLCD = 0; // [cite: 587]
    if (millis() - tiempoUltimoLCD >= 3000) { // [cite: 588]
        tiempoUltimoLCD = millis(); // [cite: 588]
        actualizarLCD(); // Renderiza la siguiente página protegiéndose con Mutex I2C [cite: 589]
    }

    // --- 3. ORQUESTACIÓN DE FLUX WEBSOCKET (Consola Interactiva + Telemetría) ---
    if (ws->count() > 0) { // [cite: 589]
        if (tiempoInicioWeb == 0) tiempoInicioWeb = millis(); // Captura el instante de la primera conexión [cite: 589]
        
        // Timer antibloqueo de 30 segundos: Libera las transmisiones completas tras asentar el buffer inicial [cite: 590]
        if (!permitirWebLog && (millis() - tiempoInicioWeb >= 30000)) { // [cite: 590]
            permitirWebLog = true; // [cite: 590]
            mostrarMenuPrincipal(); // Dibuja la consola del sistema en el navegador [cite: 591]
        }

        // Broadcaster masivo de datos JSON de rendimiento a 0.2Hz (Cada 5 segundos) [cite: 591]
        if (forzarTelemetria || (millis() - lastTelemetryTime >= 3000)) { // [cite: 591]
            forzarTelemetria = false; // [cite: 591]
            lastTelemetryTime = millis(); // [cite: 592]

            int cargaC0 = calcularUsoCPU(0); // [cite: 592]
            int cargaC1 = calcularUsoCPU(1); // [cite: 592]
            int cargaTotal = (cargaC0 + cargaC1) / 2; // [cite: 592]
            
            StaticJsonDocument<320> doc; // Ampliado a 320 para incluir campos DHT11 [cite: 593]
            doc["type"] = "telemetry"; // [cite: 593]
            doc["temp"] = temperatureRead(); // [cite: 593]
            doc["cpu"] = cargaTotal; // [cite: 593]
            doc["c0"] = cargaC0; // [cite: 593]
            doc["c1"] = cargaC1; // [cite: 593]
            
            // Determinamos dinámicamente el tamaño de la memoria operativa a reportar [cite: 594]
            uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize(); // [cite: 594]
            doc["ram"] = (ESP.getFreePsram() > 0) ?
                100 - (ESP.getFreePsram() * 100 / ramTotal) : 100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize()); // [cite: 595]           
            
            // Calculamos el espacio ocupado de LittleFS [cite: 595]
            float flashTotal = LittleFS.totalBytes(); // [cite: 595]
            float flashUsado = LittleFS.usedBytes();  // [cite: 596]
            doc["flash"] = (flashTotal > 0) ?
                round(((flashUsado * 100.0) / flashTotal) * 100.0) / 100.0 : 0.0; // [cite: 597]
            
            doc["wifi"] = WiFi.RSSI(); // Potencia de señal cruda en dBm [cite: 597]      
            doc["uptime"] = obtenerUptime(); // [cite: 597]

            // Campos del sensor DHT11 — -127.0 / -1.0 indican error de lectura
            doc["dhtTemp"] = temperaturaActual;
            doc["dhtHum"]  = humedadActual;
            
            String out; // [cite: 597]
            serializeJson(doc, out); // Compacta el JSON estructurado dentro del string [cite: 598]        
            ws->textAll(out);        // Transmite en paralelo la trama a todos los navegadores abiertos [cite: 598]
            ws->cleanupClients();    // Libera sockets muertos o zombies [cite: 598] 
        }
    } else {
        // Si no quedan navegadores abiertos, cerramos las llaves para no fragmentar el buffer de la terminal [cite: 598]
        tiempoInicioWeb = 0; // [cite: 598]
        permitirWebLog = false; // [cite: 599]
    }

    // --- 4. CRONJOB HISTÓRICO DE BASE DE DATOS (Cada 2 horas) --- [cite: 601]
    if (primerGuardado) { // [cite: 599]
        struct tm timeinfo; // [cite: 599]
        // Obligamos al sistema a esperar a que el año sea superior a 100 (Año > 2000), señal de sincronización NTP exitosa [cite: 600]
        if (getLocalTime(&timeinfo) && timeinfo.tm_year > 100) { // [cite: 600]
            tiempoUltimoGuardadoBD = millis(); // [cite: 600]
            primerGuardado = false; // Desactivamos la bandera de inicio [cite: 601]
            guardarEnHistorial();   // Realiza la primera escritura física en el CSV [cite: 601]
        }
    }
    // Siguientes ciclos recurrentes de guardado autónomo: Cada 2 horas (7,200,000 milisegundos) [cite: 601]
    else if (!primerGuardado && (millis() - tiempoUltimoGuardadoBD >= 7200000UL)) { // [cite: 601]
        tiempoUltimoGuardadoBD = millis(); // [cite: 601]
        guardarEnHistorial(); // Inserta métricas actuales en el almacenamiento local de LittleFS [cite: 602]
    }

    // --- 5. DRIVER DE MONITOREO CROMÁTICO (NeoPixel) --- [cite: 602]
    bool hayEspectadoresLED = (telnetClient && telnetClient.connected()) || (ws->count() > 0); // [cite: 602, 603]
    
    if (hayEspectadoresLED) { // [cite: 603]
      // Si la terminal o la web están activas: Ciclo estroboscópico intermitente Azul a 2Hz (Cada 500ms) [cite: 603]
      if (millis() - previousMillisLED >= 500) { // [cite: 603]
        previousMillisLED = millis(); // [cite: 603]
        ledState = !ledState; // Alterna estado eléctrico virtual [cite: 604]             
        if (ledState) {
            pixel.setPixelColor(0, pixel.Color(0, 0, 70)); // Destello Azul de monitorización activa [cite: 604]
        } else {
            pixel.setPixelColor(0, pixel.Color(0, 0, 0));  // Apagado del pin [cite: 605]
        }
        pixel.show(); // Transmite el tren de pulsos binario al integrado del diodo LED [cite: 606]
      }
    } else {                                      
      // Estado de reposo absoluto: Verde suave de baja intensidad estático [cite: 607]
      pixel.setPixelColor(0, pixel.Color(0, 25, 0)); // [cite: 607]
      pixel.show(); // [cite: 608]
    }
    
    // Medicion del tiempo de trabajo real antes de ceder la CPU
    tiempoProcesamientoCore[0] = micros() - inicioCiclo0;

    // Alivio mandatorio de 1 milisegundo al planificador del sistema operativo.
    // Esto evita que esta tarea monopolice el núcleo, impidiendo que el controlador WiFi colapse (WDT Reset) [cite: 608]
    vTaskDelay(1); // [cite: 608]
  }
}

// ============================================================================
// CORE 1: CAPTURA DE TERMINAL DE TEXTO, POLLED SENSORS FSM Y DISPAROS SÓNICOS
// ============================================================================
void taskCore1(void * pvParameters) { // 
  uint32_t inicioCiclo1 = micros(); // inicio de medicion de ciclo para CPU real
  // BUCLE DE EJECUCIÓN MAESTRO DEL NÚCLEO 1
  for(;;) {
    // Medicion de duracion total del ciclo anterior
    uint32_t ahora = micros();
    tiempoCicloCore[1] = ahora - inicioCiclo1;
    inicioCiclo1 = ahora;

    String entrada = ""; // Inicialización local del contenedor de entrada de datos 
    
    // Multiplexor de captura bidireccional asíncrona de comandos [cite: 610]
    if (telnetClient && telnetClient.connected() && telnetClient.available() > 0) { // Sonda el búfer TCP [cite: 610]
      entrada = telnetClient.readStringUntil('\n'); // Lee la ráfaga hasta el retorno de carro [cite: 610]
      entrada.trim(); // Sanitiza drásticamente [cite: 611]
    } else if (hayEntradaWeb) {           
      // Sincronización de memoria: Captura los strings inyectados por el túnel WebSocket desde el Core 0 [cite: 612]
      entrada = entradaWeb; // Volcado local de la orden [cite: 612]
      hayEntradaWeb = false; // Bajamos la bandera compartida liberando el búfer [cite: 613]
    }

    bool hayEspectadores = (telnetClient && telnetClient.connected()) || (ws->count() > 0); // [cite: 613, 614]
    
    // ========================================================================
    // MÁQUINA DE ESTADOS PRINCIPAL EN COEXISTENCIA PARALELA
    // ========================================================================
    if (programaActivo == 0) { // --- MÓDULO 0: EN REPOSO / MENÚ DE INICIO --- [cite: 614]
      // Si hay una sesión activa conectada y transcurren 5 minutos de inactividad, refrescamos el menú principal [cite: 614]
      if (millis() - tiempoUltimoMenuPrincipal >= intervaloMenu && hayEspectadores) mostrarMenuPrincipal(); // [cite: 614]
      
      // Motor del parser del menú de bienvenida [cite: 615]
      if (entrada.length() > 0) procesarMenuPrincipal(entrada); // Enruta la cadena recibida hacia las funciones de control [cite: 615]
    } 
    else if (programaActivo == 1) { // --- MÓDULO 1: ESTACIÓN DE RADIOFRECUENCIA NFC --- [cite: 615]
      // Refresco de pantalla NFC tras inactividad [cite: 615]
      if (modoNFC == 0 && (millis() - tiempoUltimoMenuNFC >= intervaloMenu) && hayEspectadores) mostrarMenuNFC(); // [cite: 615]
      
      if (entrada.length() > 0) procesarEntradaNFC(entrada); // Parser del submódulo RFID [cite: 616]
      
      // Despacho continuo de subrutinas de hardware mediante polling asíncrono [cite: 616]
      if (modoNFC == 1) modoLecturaNFC();    // Bucle de interrogación criptográfica pasiva SPI [cite: 616]
      if (modoNFC == 4) modoEscrituraNFC();  // Bucle de modulación e inyección masiva SPI [cite: 617]
    } 
    else if (programaActivo == 2) { // --- MÓDULO 2: RADAR ULTRASONIDOS --- [cite: 617]
      entrada.toUpperCase(); // Sanitizamos de forma estricta los strings de comandos sónicos [cite: 618]
      
      // Controladores interactivos de la FSM del sensor acústico [cite: 619]
      if (entrada == "M") { 
          midiendoDistancia = false; programaActivo = 0; mostrarMenuPrincipal(); // Aborto y regreso [cite: 619]
      } 
      else if (entrada == "U") { 
          midiendoDistancia = false; mostrarMenuUltrasonidos(); // Volver a pintar las opciones de ultrasonidos [cite: 620]
      }             
      else if (entrada == "1") { 
          midiendoDistancia = true; // Arranca el disparo intermitente [cite: 621]
          Terminal.println("\n[+] RADAR ACTIVADO..."); // [cite: 622]
      } 
      else if (entrada == "2") { 
          midiendoDistancia = false; // Apaga los transductores [cite: 622]
          Terminal.println("\n[-] Radar detenido."); mostrarMenuUltrasonidos(); // [cite: 623]
      } 
      
      // Despachador de muestreo de hardware no bloqueante a una frecuencia fija de 1Hz (Cada 1000ms) [cite: 623]
      if (midiendoDistancia && (millis() - tiempoUltimaMedicion >= intervaloMedicion)) { // [cite: 623]
        tiempoUltimaMedicion = millis(); // [cite: 623]
        long d = medirDistanciaFisica();  // Disparo físico del pin TRIG e interceptación eléctrica de ECHO [cite: 624]
        
        // Formateo del diagnóstico según los umbrales de la ráfaga devuelta [cite: 624]
        if (d == -1) Terminal.println(" -> Distancia: FUERA DE RANGO"); // [cite: 624]
        else { Terminal.print(" -> Distancia: "); Terminal.print(d); Terminal.println(" cm"); } // [cite: 625]
      }
    }
    else if (programaActivo == 3) { // --- MÓDULO 3: SENSOR AMBIENTAL DHT11 ---
      if (entrada.length() > 0) procesarEntradaDHT(entrada);
    }
    
    // Medicion del tiempo de trabajo real antes de ceder la CPU
    tiempoProcesamientoCore[1] = micros() - inicioCiclo1;

    // Rendición obligatoria de 1 tick completo al planificador multinúcleo de FreeRTOS.
    // Esto es una medida de ingeniería crítica: calma de forma instantánea al Watchdog (WDT) del Core 1,
    // garantizando la estabilidad absoluta del silicio y previniendo Kernel Panics o reinicios bruscos.
    vTaskDelay(1 / portTICK_PERIOD_MS); // [cite: 625, 626]
  }
}