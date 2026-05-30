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
#include "utils.h"      // Permite el acceso a los diagnósticos de CPU, base de datos y LCD 
#include "menus.h"      // Habilita la inyección de los menús visuales de terminal 
#include "nfc.h"        // Permite invocar las subrutinas de interrogación del bus SPI 
#include "ultrasonidos.h" // Habilita la captura de datos del transductor sónico 
#include "terminal.h"   // Habilita la impresión por el canal dual híbrido 
#include "dht.h"        // Habilita la inicialización y lectura del sensor DHT11 de temperatura y humedad
#include "sd_card.h"    // Habilita la comprobación de tamaño del CSV en SD

// ============================================================================
// CORE 0: PILA DE RED, WiFi, HTTP, DNS, TELNET, WEBSOCKET, BBDD
// ============================================================================
void taskCore0(void * pvParameters) {
  if (!LittleFS.begin(true, "/spiffs", 10, "ffat")) {
      if (!LittleFS.begin(true)) Serial.println("❌ [ERROR] Flash no montada");
      else Serial.println("✅ [OK] Flash montada en Modo Rescate");
  } else {
      Serial.println("✅ [OK] Flash montada correctamente");
  }
  if (sdDisponible) {
      if (tamanoCSV_SD() > 8388608) {
          eliminarCSV_SD();
          Serial.println("🧹 [SISTEMA] Base de datos en SD superó límite. Limpieza ejecutada.");
      }
  } else if (LittleFS.exists("/datos.csv")) {
      File dbFile = LittleFS.open("/datos.csv", "r");
      if (dbFile) {
          size_t fileSize = dbFile.size();
          dbFile.close();
          if (fileSize > 8388608) {
              LittleFS.remove("/datos.csv");
              Serial.println("🧹 [SISTEMA] Base de datos superó límite. Limpieza ejecutada.");
          }
      }
  }

  telnetServer.begin();
  iniciarServidorWeb();

  unsigned long lastTelemetryTime = 0;
  unsigned long lastNtpCheck = 0;
  int ultimaHoraGuardada = -1;
  bool primerGuardado = true;

  uint32_t inicioCiclo0 = micros();
  for(;;) {
    if (!sistemaListo) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        continue;
    }

    uint32_t ahora = micros();
    tiempoCicloCore[0] = ahora - inicioCiclo0;
    inicioCiclo0 = ahora;

    ArduinoOTA.handle();
    dnsServer.processNextRequest();
    yield();

    // --- TELNET ---
    if (telnetServer.hasClient()) {
      if (telnetClient && telnetClient.connected()) {
          telnetServer.available().stop();
      } else {
        telnetClient = telnetServer.available();
        // [FIX] Resetear estado de autenticacion al conectar un nuevo cliente
        telnetAutenticado = false;
        telnetIntentos = 0;
        tiempoUltimaActividadTelnet = millis();
        // Pedir contraseña antes de mostrar el menu
        telnetClient.println("\r\n[ZENITH OS] Autenticacion requerida.");
        telnetClient.print("Contrasena: ");
      }
    }

    // [FIX] Gestión de autenticación Telnet: procesar contraseña antes de permitir comandos
    if (telnetClient && telnetClient.connected() && !telnetAutenticado && telnetClient.available() > 0) {
      String passIntento = telnetClient.readStringUntil('\n');
      passIntento.trim();
      if (passIntento == webPass) {
        telnetAutenticado = true;
        telnetIntentos = 0;
        Terminal.println("\n[SISTEMA] Autenticacion correcta. Conectado al SO Blasco.");
        mostrarMenuPrincipal();
      } else {
        telnetIntentos++;
        if (telnetIntentos >= 3) {
          telnetClient.println("[SISTEMA] Demasiados intentos fallidos. Conexion cerrada.");
          telnetClient.stop();
          telnetAutenticado = false;
          telnetIntentos = 0;
        } else {
          telnetClient.print("[SISTEMA] Contrasena incorrecta. Intento ");
          telnetClient.print(telnetIntentos);
          telnetClient.println("/3. Contrasena: ");
        }
      }
    }

    // --- TELNET TIMEOUT (5 min inactividad) ---
    if (telnetClient && telnetClient.connected() && millis() - tiempoUltimaActividadTelnet > 300000) {
        telnetClient.println("\r\n[SISTEMA] Conexion cerrada por inactividad (5 min).");
        telnetClient.stop();
        telnetAutenticado = false;
        telnetIntentos = 0;
    }

    // --- WEBSOCKET TELEMETRÍA ---
    if (ws->count() > 0) {
        if (tiempoInicioWeb == 0) tiempoInicioWeb = millis();
        if (!permitirWebLog && (millis() - tiempoInicioWeb >= 30000)) {
            permitirWebLog = true;
            mostrarMenuPrincipal();
        }
        if (forzarTelemetria || (millis() - lastTelemetryTime >= 3000)) {
            forzarTelemetria = false;
            lastTelemetryTime = millis();

            int cargaC0 = calcularUsoCPU(0);
            int cargaC1 = calcularUsoCPU(1);
            int cargaTotal = (cargaC0 + cargaC1) / 2;

            StaticJsonDocument<320> doc;
            doc["type"] = "telemetry";
            doc["temp"] = temperatureRead();
            doc["cpu"] = cargaTotal;
            doc["c0"] = cargaC0;
            doc["c1"] = cargaC1;

            uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize();
            doc["ram"] = (ESP.getFreePsram() > 0) ?
                100 - (ESP.getFreePsram() * 100 / ramTotal) : 100 - (ESP.getFreeHeap() * 100 / ESP.getHeapSize());

            float flashTotal = LittleFS.totalBytes();
            float flashUsado = LittleFS.usedBytes();
            doc["flash"] = (flashTotal > 0) ?
                round(((flashUsado * 100.0) / flashTotal) * 100.0) / 100.0 : 0.0;

            doc["wifi"] = WiFi.RSSI();
            doc["uptime"] = obtenerUptime();
            doc["dhtTemp"] = temperaturaActual;
            doc["dhtHum"]  = humedadActual;

            String out;
            serializeJson(doc, out);
            ws->textAll(out);
            ws->cleanupClients();
        }
    } else {
        tiempoInicioWeb = 0;
        permitirWebLog = false;
    }

    // --- CRONJOB CSV (Cada 2 horas usando hora NTP, verificado cada 30s) ---
    unsigned long ahoraMillis = millis();
    if (ahoraMillis - lastNtpCheck >= 30000) {
        lastNtpCheck = ahoraMillis;

        if (primerGuardado) {
            guardarEnHistorial();
            primerGuardado = false;
        }

        struct tm timeinfo;
        if (getLocalTime(&timeinfo) && timeinfo.tm_year > 100) {
            if (timeinfo.tm_hour % 2 == 0 && timeinfo.tm_min == 0 && timeinfo.tm_hour != ultimaHoraGuardada) {
                guardarEnHistorial();
                ultimaHoraGuardada = timeinfo.tm_hour;
            }
        }
    }

    tiempoProcesamientoCore[0] = micros() - inicioCiclo0;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ============================================================================
// CORE 1: LCD, DHT11, NEOPIXEL, TERMINAL INPUT, SENSORES FSM
// ============================================================================
void taskCore1(void * pvParameters) {
  inicializarDHT();

  unsigned long tiempoUltimoLCD = 0;
  unsigned long tiempoUltimoDHT = 0;
  unsigned long tiempoUltimoLED = 0;

  uint32_t inicioCiclo1 = micros();
  for(;;) {
    uint32_t ahora = micros();
    tiempoCicloCore[1] = ahora - inicioCiclo1;
    inicioCiclo1 = ahora;

    // --- LCD (cada 3s) ---
    if (millis() - tiempoUltimoLCD >= 3000) {
        tiempoUltimoLCD = millis();
        actualizarLCD();
    }

    // --- DHT11 (cada 3s) ---
    if (millis() - tiempoUltimoDHT >= 3000UL) {
        tiempoUltimoDHT = millis();
        actualizarDHT();
    }

    // --- NEOPIXEL ---
    bool hayEspectadoresLED = (telnetClient && telnetClient.connected() && telnetAutenticado) || (ws->count() > 0);
    if (hayEspectadoresLED) {
      if (millis() - tiempoUltimoLED >= 500) {
        tiempoUltimoLED = millis();
        ledState = !ledState;
        if (ledState) {
            pixel.setPixelColor(0, pixel.Color(0, 0, 70));
        } else {
            pixel.setPixelColor(0, pixel.Color(0, 0, 0));
        }
        pixel.show();
      }
    } else {
      pixel.setPixelColor(0, pixel.Color(0, 25, 0));
      pixel.show();
    }

    // --- ENTRADA DE TEXTO (Telnet / WebSocket) ---
    // [FIX] Solo procesar entrada Telnet si el cliente esta autenticado
    String entrada = "";
    if (telnetClient && telnetClient.connected() && telnetAutenticado && telnetClient.available() > 0) {
      entrada = telnetClient.readStringUntil('\n');
      entrada.trim();
      tiempoUltimaActividadTelnet = millis();
    } else {
      char* cmdPtr = NULL;
      if (xQueueReceive(cmdQueue, &cmdPtr, 0) == pdTRUE && cmdPtr) {
          entrada = String(cmdPtr);
          entrada.trim();
          free(cmdPtr);
      }
    }

    bool hayEspectadores = (telnetClient && telnetClient.connected() && telnetAutenticado) || (ws->count() > 0);

    // --- FSM ---
    if (programaActivo == 0) {
      if (millis() - tiempoUltimoMenuPrincipal >= intervaloMenu && hayEspectadores) mostrarMenuPrincipal();
      if (entrada.length() > 0) procesarMenuPrincipal(entrada);
    }
    else if (programaActivo == 1) {
      if (modoNFC == 0 && (millis() - tiempoUltimoMenuNFC >= intervaloMenu) && hayEspectadores) mostrarMenuNFC();
      if (entrada.length() > 0) procesarEntradaNFC(entrada);
      if (modoNFC == 1) modoLecturaNFC();
      if (modoNFC == 4) modoEscrituraNFC();
    }
    else if (programaActivo == 2) {
      entrada.toUpperCase();
      if (entrada == "M") {
          midiendoDistancia = false; programaActivo = 0; mostrarMenuPrincipal();
      }
      else if (entrada == "U") {
          midiendoDistancia = false; mostrarMenuUltrasonidos();
      }
      else if (entrada == "1") {
          midiendoDistancia = true;
          Terminal.println("\n[+] RADAR ACTIVADO...");
      }
      else if (entrada == "2") {
          midiendoDistancia = false;
          Terminal.println("\n[-] Radar detenido."); mostrarMenuUltrasonidos();
      }
      if (midiendoDistancia) {
        long d = medirDistanciaFisica();
        if (d >= 0) {
          Terminal.print(" -> Distancia: "); Terminal.print(d); Terminal.println(" cm");
        } else if (d == -2) {
          Terminal.println(" -> Distancia: FUERA DE RANGO");
        }
      }
    }
    else if (programaActivo == 3) {
      if (entrada.length() > 0) procesarEntradaDHT(entrada);
    }

    tiempoProcesamientoCore[1] = micros() - inicioCiclo1;
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}