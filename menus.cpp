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
 * @file menus.cpp
 * @brief Implementación de las pantallas de texto y motores de enrutamiento de comandos CLI.
 */
#include "menus.h"    // Vinculación formal con su cabecera de prototipos
#include "terminal.h" // Habilita el acceso al objeto interactivo dual Terminal 
#include "utils.h"    // Requerido para invocar la función obtenerUptime() de diagnóstico 
#include "dht.h"      // Permite mostrar la lectura del termohigrómetro en el menú principal

// ============================================================================
// MENÚ DE BIENVENIDA Y REPORTE ESTADÍSTICO DE HARDWARE
// ============================================================================
void mostrarMenuPrincipal() { // 

  // Activamos el modo de retención en el búfer web. Bloqueamos el flujo inmediato byte a byte
  // para concatenar todo el menú y lanzarlo en un único paquete TCP, protegiendo la red.
  Terminal.iniciarBloque(); // 👈 Retenemos la transmisión automática hacia el WebSocket 
  
  // Dibujamos las cabeceras visuales del software Zenith System
  Terminal.println("\n=================================================="); // 
  Terminal.println("                ESP32 BLASCO ARMENGOD             "); // 
  Terminal.println("=================================================="); // 
  Terminal.println("\n SELECCIONA UN PROGRAMA:"); // 
  Terminal.println(" [1] ESTACION DE CLONACION NFC (V14)"); // 
  Terminal.println(" [2] PROYECTO ULTRASONIDOS (V3)"); // 
  Terminal.println(" [3] LEER TEMPERATURA Y HUMEDAD (DHT11)"); // Módulo termohigrómetro
  Terminal.println(" [4] REINICIAR PLACA");
  Terminal.println("\n--------------------------------------------------"); // 
  Terminal.println("\n TELEMETRIA DEL SISTEMA:"); // 
  
  // LOGICA MATEMÁTICA DE DETERMINACIÓN DE MEMORIAS OPERATIVAS
  // Calculamos los bloques totales de RAM mapeando la PSRAM física disponible si existe, o el Heap interno
  float ramTotal = (ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize()) / 1024.0; // 
  // Determinamos los bloques libres residuales en los controladores dinámicos
  float ramLibre = (ESP.getFreePsram() > 0 ? ESP.getFreePsram() : ESP.getFreeHeap()) / 1024.0; // 
  float ramUsada = ramTotal - ramLibre; // Obtenemos el volumen real neto de RAM en uso por los núcleos 
  
  // Consultamos el tamaño total asignado y el consumo físico actual en el chip de almacenamiento Flash
  float flashTotal = LittleFS.totalBytes() / 1024.0; // 
  float flashUsada = LittleFS.usedBytes() / 1024.0;  // 
  
  // IMPRESIÓN DETALLADA DEL ESTADO DEL ALMACENAMIENTO EN DISCO
  Terminal.print(" - Almacenamiento: "); // 
  if(flashTotal > 0) { // Evaluamos si el sistema de archivos local está correctamente montado
      // Imprimimos los Kilobytes ocupados frente a la capacidad de la partición
      Terminal.print(flashUsada, 0); Terminal.print(" KB / "); Terminal.print(flashTotal, 0); Terminal.println(" KB"); // 
  } else {
      Terminal.println("0 KB / 0 KB (No montado)"); // Mensaje protector en caso de fallo de montaje 
  }
  
  // TRANSMISIÓN DE LOGS DE RENDIMIENTO CORE EN TIEMPO REAL
  Terminal.print(" - RAM Usada: "); Terminal.print(ramUsada, 0); Terminal.print(" KB / "); Terminal.print(ramTotal, 0); Terminal.println(" KB"); // 
  Terminal.print(" - Temp Core: "); Terminal.print(temperatureRead(), 1); Terminal.println(" C"); // Lectura de silicio interna 
  // Lectura del termohigrómetro externo DHT11
  if (sensores.temperatura > -127.0) {
    Terminal.print(" - Temp DHT11: "); Terminal.print(sensores.temperatura, 1); Terminal.println(" C");
    Terminal.print(" - Humedad   : "); Terminal.print(sensores.humedad, 1);    Terminal.println(" %");
  } else {
    Terminal.println(" - DHT11: Sin lectura valida");
  }
  Terminal.print(" - Vel. CPU : "); Terminal.print(ESP.getCpuFreqMHz()); Terminal.println(" MHz"); // Frecuencia del oscilador de reloj 
  
  // Mapeamos el contador de tiempo de actividad ininterrumpida formateado desde utils
  Terminal.print(" - Tiempo Activo : "); Terminal.println(obtenerUptime()); // 
  Terminal.print(" - Direccion IP : "); Terminal.println(WiFi.localIP()); // Muestra la IP asignada por el DHCP local 

  Terminal.println("\n=================================================="); // 
  Terminal.println("               Ruben Blasco Armengod              "); // 
  Terminal.println("=================================================="); // 

  // Liberamos las compuertas lógicas y disparamos la ráfaga masiva de caracteres hacia la web
  Terminal.enviarBloque(); // 
  
  tiempoUltimoMenuPrincipal = millis(); // Reseteamos el cronómetro de inactividad para evitar repintados automáticos prematuros 
}

// ============================================================================
// PARSER Y ENRUTADOR DEL MENÚ PRINCIPAL
// ============================================================================
void procesarMenuPrincipal(String entrada) { // 
  if (entrada == "1") { // Opción 1: Transición de la FSM hacia el entorno de clonación por proximidad
      FSM_SET(1); // Fijamos el puntero ejecutor en modo NFC 
      nfc.modo = 0;        // Inicializamos el sub-estado del lector RFID en modo reposo 
      mostrarMenuNFC();   // Despachamos de forma inmediata la interfaz del transceptor 
  } 
  else if (entrada == "2") { // Opción 2: Transición de la FSM hacia el entorno sónico
      FSM_SET(2);       // Fijamos el puntero ejecutor en modo Ultrasonidos 
      midiendoDistancia = false; // El radar ultrasónico comienza apagado por defecto hasta orden expresa 
      mostrarMenuUltrasonidos(); // Despachamos el menú del sensor de distancia 
  }
  else if (entrada == "3") { // Opción 3: Menú del termohigrómetro DHT11
      FSM_SET(3); // Fijamos el puntero ejecutor en modo DHT11
      mostrarMenuDHT();   // Despachamos la interfaz del sensor ambiental
  }
  else if (entrada == "4") { // Opción 4: Confirmación de reinicio
      FSM_SET(4);
      Terminal.iniciarBloque();
      Terminal.println("\n==================================================");
      Terminal.println("                 REINICIAR PLACA                 ");
      Terminal.println("==================================================");
      Terminal.println(" Esta accion reiniciara el ESP32.");
      Terminal.println(" La conexion se perdera y se reconectara sola.");
      Terminal.println("");
      Terminal.println(" ¿Confirmas el reinicio? [S] Si  [N] No");
      Terminal.println("==================================================");
      Terminal.enviarBloque();
  }
}

// ============================================================================
// INTERFAZ DE CONTROL PARA LA ESTACIÓN DE TRANSMISIÓN RFID
// ============================================================================
void mostrarMenuNFC() { // 

  Terminal.iniciarBloque(); // 👈 Abrimos retención para unificar la pantalla NFC 

  Terminal.println("\n**************************************************"); // 
  Terminal.println("          ESTACION DE CLONACION PRO V14           "); // 
  Terminal.println("**************************************************"); // 
  Terminal.println(" [1] LEER Y GUARDAR: Dump completo (64 bloques)"); // 
  Terminal.println(" [2] CLONAR: Escribe dump completo en tarjeta destino."); // 
  Terminal.println(" [N] VOLVER AL MENU NFC (A esta pantalla)"); // 
  Terminal.println(" [M] SALIR AL MENU PRINCIPAL (ESP32 Blasco)"); // 
  Terminal.println("--------------------------------------------------"); // 
  
  // GESTIÓN VISUAL DEL ESTADO DE LOS REGISTROS NFC EN RAM
  if (nfc.memoriaLlena) { 
    Terminal.println(" >> MEMORIA: Tarjeta guardada en RAM [OK]"); // Los buffers globales contienen datos íntegros clonables 
  } else { 
    Terminal.println(" >> MEMORIA: Vacia (Usa la opcion 1)"); // El búfer dinámico se encuentra en estado huérfano 
  }
  
  Terminal.println("**************************************************"); // 
  Terminal.enviarBloque(); // Emitimos el bloque completo hacia los clientes conectados 

  tiempoUltimoMenuNFC = millis(); // Capturamos la marca de tiempo para el control de inactividad de la interfaz 
}

// ============================================================================
// PARSER Y ENRUTADOR DE SUB-ESTADOS DEL ENTORNO NFC
// ============================================================================
void procesarEntradaNFC(String entrada) { // 
  entrada.toUpperCase(); // Convertimos drásticamente a mayúsculas para sanitizar entradas accidentales 
  
  if (entrada == "M") { // Comando de aborto: Regreso seguro a la raíz del firmware
      FSM_SET(0); // Conmutamos la máquina de estados principal a reposo 
      mostrarMenuPrincipal(); // Dibuja la pantalla de bienvenida con su telemetría 
      return; // Interrumpe la función de forma inmediata 
  } 
  
  if (nfc.modo == 0 && entrada == "1") { // Solicitud de escaneo de credenciales físicas original
      nfc.modo = 1; // Cambiamos el sub-estado a modo escucha activa (interrogación en bus SPI) 
      Terminal.println("\n>>> ACERCA TARJETA ORIGINAL..."); // Notificación instructiva por terminal 
  } 
  else if (nfc.modo == 0 && entrada == "2") { // Solicitud de volcado e inyección criptográfica
      if(!nfc.memoriaLlena) { // Validación estricta contra errores de buffer vacío
          Terminal.println("[!] ERROR: Memoria vacia"); // Bloqueo preventivo para no corromper tags 
          return; // Aborta la operación 
      } 
      // Asignación física del puntero dinámico de escritura hacia la dirección de la matriz leída
      nfc.bloqueAEscribir = nfc.bloqueEscaneado; // 
      nfc.modo = 4; // Cambiamos el sub-estado a modo inyección masiva (escritura en bus SPI) 
      Terminal.println("\n>>> ACERCA TARJETA DESTINO..."); // Instructivo visual para aproximar el tag virgen 
  }
}

// ============================================================================
// INTERFAZ DE CONTROL PARA EL SENSOR ULTRASONIDOS HC-SR04
// ============================================================================
void mostrarMenuUltrasonidos() { // 

  Terminal.iniciarBloque(); // 👈 Retenemos para unificar el bloque sónico 

  Terminal.println("\n=================================================="); // 
  Terminal.println("             PROYECTO 2: ULTRASONIDOS             "); // 
  Terminal.println("=================================================="); // 
  Terminal.println(" [1] INICIAR radar de distancia en tiempo real"); // 
  Terminal.println(" [2] DETENER radar de distancia"); // 
  Terminal.println(" [U] VOLVER AL MENU ULTRASONIDOS (A esta pantalla)"); // 
  Terminal.println(" [M] VOLVER AL MENU PRINCIPAL (ESP32 Blasco)"); // 
  Terminal.println("=================================================="); // 

  Terminal.enviarBloque(); // Despachamos la pantalla acústica 
  tiempoUltimoMenuPrincipal = millis(); // Capturamos marca de tiempo de sincronización 
}

// ============================================================================
// INTERFAZ DE CONTROL PARA EL SENSOR DHT11
// ============================================================================
void mostrarMenuDHT() {

  Terminal.iniciarBloque(); // Abrimos retención para unificar la pantalla DHT

  Terminal.println("\n**************************************************");
  Terminal.println("          SENSOR DHT11 — TEMPERATURA/HUMEDAD       ");
  Terminal.println("**************************************************");
  Terminal.println(" [1] LEER temperatura y humedad ahora");
  Terminal.println(" [M] SALIR AL MENU PRINCIPAL (ESP32 Blasco)");
  Terminal.println("--------------------------------------------------");

  if (sensores.temperatura > -127.0) {
    Terminal.print(" >> Ultima lectura — Temp: "); Terminal.print(sensores.temperatura, 1); Terminal.print(" C");
    Terminal.print("  Hum: "); Terminal.print(sensores.humedad, 1); Terminal.println(" %");
  } else {
    Terminal.println(" >> Ultima lectura: Sin datos (esperando lectura)");
  }

  Terminal.println("**************************************************");
  Terminal.enviarBloque(); // Emitimos el bloque completo hacia los clientes conectados
}

// ============================================================================
// PARSER DE SUB-ESTADOS DEL ENTORNO DHT11
// ============================================================================
void procesarEntradaDHT(String entrada) {
  entrada.toUpperCase(); // Sanitizamos a mayúsculas

  if (entrada == "M") { // Comando de aborto: Regreso seguro a la raíz del firmware
      FSM_SET(0); // Conmutamos la máquina de estados principal a reposo
      mostrarMenuPrincipal(); // Dibuja la pantalla de bienvenida
      return;
  }

  if (entrada == "1") { // Lectura fresca del sensor
      actualizarDHT();
      Terminal.iniciarBloque();
      if (sensores.temperatura > -127.0) {
          Terminal.println("\n------------------ DHT11 ------------------");
          Terminal.print(" Temperatura : "); Terminal.print(sensores.temperatura, 1); Terminal.println(" C");
          Terminal.print(" Humedad     : "); Terminal.print(sensores.humedad, 1);    Terminal.println(" %");
          Terminal.println("---------------------------------------------");
      } else {
          Terminal.println("\n [!] Error: Sensor no responde o datos invalidos.");
      }
      Terminal.enviarBloque();
      mostrarMenuDHT(); // Volvemos a mostrar el menú
  }
}