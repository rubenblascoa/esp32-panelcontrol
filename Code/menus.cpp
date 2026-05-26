
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
#include "terminal.h" // Habilita el acceso al objeto interactivo dual Terminal [cite: 37]
#include "utils.h"    // Requerido para invocar la función obtenerUptime() de diagnóstico [cite: 531, 638]
#include "dht.h"      // Permite mostrar la lectura del termohigrómetro en el menú principal

// ============================================================================
// MENÚ DE BIENVENIDA Y REPORTE ESTADÍSTICO DE HARDWARE
// ============================================================================
void mostrarMenuPrincipal() { // [cite: 626]

  // Activamos el modo de retención en el búfer web. Bloqueamos el flujo inmediato byte a byte
  // para concatenar todo el menú y lanzarlo en un único paquete TCP, protegiendo la red.
  Terminal.iniciarBloque(); // 👈 Retenemos la transmisión automática hacia el WebSocket [cite: 26, 627]
  
  // Dibujamos las cabeceras visuales del software Zenith System
  Terminal.println("\n=================================================="); // [cite: 627]
  Terminal.println("                ESP32 BLASCO ARMENGOD             "); // [cite: 628]
  Terminal.println("=================================================="); // [cite: 629]
  Terminal.println("\n SELECCIONA UN PROGRAMA:"); // [cite: 629]
  Terminal.println(" [1] ESTACION DE CLONACION NFC (V14)"); // [cite: 629]
  Terminal.println(" [2] PROYECTO ULTRASONIDOS (V3)"); // [cite: 629]
  Terminal.println(" [3] LEER TEMPERATURA Y HUMEDAD (DHT11)"); // Módulo termohigrómetro
  Terminal.println("\n--------------------------------------------------"); // [cite: 629]
  Terminal.println("\n TELEMETRIA DEL SISTEMA:"); // [cite: 630]
  
  // LOGICA MATEMÁTICA DE DETERMINACIÓN DE MEMORIAS OPERATIVAS
  // Calculamos los bloques totales de RAM mapeando la PSRAM física disponible si existe, o el Heap interno
  float ramTotal = (ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize()) / 1024.0; // [cite: 630]
  // Determinamos los bloques libres residuales en los controladores dinámicos
  float ramLibre = (ESP.getFreePsram() > 0 ? ESP.getFreePsram() : ESP.getFreeHeap()) / 1024.0; // [cite: 631]
  float ramUsada = ramTotal - ramLibre; // Obtenemos el volumen real neto de RAM en uso por los núcleos [cite: 631]
  
  // Consultamos el tamaño total asignado y el consumo físico actual en el chip de almacenamiento Flash
  float flashTotal = LittleFS.totalBytes() / 1024.0; // [cite: 632]
  float flashUsada = LittleFS.usedBytes() / 1024.0;  // [cite: 632]
  
  // IMPRESIÓN DETALLADA DEL ESTADO DEL ALMACENAMIENTO EN DISCO
  Terminal.print(" - Almacenamiento: "); // [cite: 633]
  if(flashTotal > 0) { // Evaluamos si el sistema de archivos local está correctamente montado
      // Imprimimos los Kilobytes ocupados frente a la capacidad de la partición
      Terminal.print(flashUsada, 0); Terminal.print(" KB / "); Terminal.print(flashTotal, 0); Terminal.println(" KB"); // [cite: 634]
  } else {
      Terminal.println("0 KB / 0 KB (No montado)"); // Mensaje protector en caso de fallo de montaje [cite: 635]
  }
  
  // TRANSMISIÓN DE LOGS DE RENDIMIENTO CORE EN TIEMPO REAL
  Terminal.print(" - RAM Usada: "); Terminal.print(ramUsada, 0); Terminal.print(" KB / "); Terminal.print(ramTotal, 0); Terminal.println(" KB"); // [cite: 636]
  Terminal.print(" - Temp Core: "); Terminal.print(temperatureRead(), 1); Terminal.println(" C"); // Lectura de silicio interna [cite: 637]
  // Lectura del termohigrómetro externo DHT11
  if (temperaturaActual > -127.0) {
    Terminal.print(" - Temp DHT11: "); Terminal.print(temperaturaActual, 1); Terminal.println(" C");
    Terminal.print(" - Humedad   : "); Terminal.print(humedadActual, 1);    Terminal.println(" %");
  } else {
    Terminal.println(" - DHT11: Sin lectura valida");
  }
  Terminal.print(" - Vel. CPU : "); Terminal.print(ESP.getCpuFreqMHz()); Terminal.println(" MHz"); // Frecuencia del oscilador de reloj [cite: 637]
  
  // Mapeamos el contador de tiempo de actividad ininterrumpida formateado desde utils
  Terminal.print(" - Tiempo Activo : "); Terminal.println(obtenerUptime()); // [cite: 638]
  Terminal.print(" - Direccion IP : "); Terminal.println(WiFi.localIP()); // Muestra la IP asignada por el DHCP local [cite: 639]

  Terminal.println("\n=================================================="); // [cite: 639]
  Terminal.println("               Ruben Blasco Armengod              "); // [cite: 639]
  Terminal.println("=================================================="); // [cite: 640]

  // Liberamos las compuertas lógicas y disparamos la ráfaga masiva de caracteres hacia la web
  Terminal.enviarBloque(); // [cite: 640]
  
  tiempoUltimoMenuPrincipal = millis(); // Reseteamos el cronómetro de inactividad para evitar repintados automáticos prematuros [cite: 640]
}

// ============================================================================
// PARSER Y ENRUTADOR DEL MENÚ PRINCIPAL
// ============================================================================
void procesarMenuPrincipal(String entrada) { // [cite: 640]
  if (entrada == "1") { // Opción 1: Transición de la FSM hacia el entorno de clonación por proximidad
      programaActivo = 1; // Fijamos el puntero ejecutor en modo NFC [cite: 40, 641]
      modoNFC = 0;        // Inicializamos el sub-estado del lector RFID en modo reposo [cite: 41, 641]
      mostrarMenuNFC();   // Despachamos de forma inmediata la interfaz del transceptor [cite: 641]
  } 
  else if (entrada == "2") { // Opción 2: Transición de la FSM hacia el entorno sónico
      programaActivo = 2;       // Fijamos el puntero ejecutor en modo Ultrasonidos [cite: 40, 641]
      midiendoDistancia = false; // El radar ultrasónico comienza apagado por defecto hasta orden expresa [cite: 43, 642]
      mostrarMenuUltrasonidos(); // Despachamos el menú del sensor de distancia [cite: 642]
  }
  else if (entrada == "3") { // Opción 3: Menú del termohigrómetro DHT11
      programaActivo = 3; // Fijamos el puntero ejecutor en modo DHT11
      mostrarMenuDHT();   // Despachamos la interfaz del sensor ambiental
  }
}

// ============================================================================
// INTERFAZ DE CONTROL PARA LA ESTACIÓN DE TRANSMISIÓN RFID
// ============================================================================
void mostrarMenuNFC() { // [cite: 642]

  Terminal.iniciarBloque(); // 👈 Abrimos retención para unificar la pantalla NFC [cite: 642]

  Terminal.println("\n**************************************************"); // [cite: 642]
  Terminal.println("          ESTACION DE CLONACION PRO V14           "); // [cite: 643]
  Terminal.println("**************************************************"); // [cite: 644]
  Terminal.println(" [1] LEER Y GUARDAR: Radiografia completa + Memoria"); // [cite: 644]
  Terminal.println(" [2] CLONAR: Elige que datos quieres copiar."); // [cite: 644]
  Terminal.println(" [N] VOLVER AL MENU NFC (A esta pantalla)"); // [cite: 645]
  Terminal.println(" [M] SALIR AL MENU PRINCIPAL (ESP32 Blasco)"); // [cite: 645]
  Terminal.println("--------------------------------------------------"); // [cite: 645]
  
  // GESTIÓN VISUAL DEL ESTADO DE LOS REGISTROS NFC EN RAM
  if (memoriaLlena) { 
    Terminal.println(" >> MEMORIA: Tarjeta guardada en RAM [OK]"); // Los buffers globales contienen datos íntegros clonables [cite: 646]
  } else { 
    Terminal.println(" >> MEMORIA: Vacia (Usa la opcion 1)"); // El búfer dinámico se encuentra en estado huérfano [cite: 647]
  }
  
  Terminal.println("**************************************************"); // [cite: 647]
  Terminal.enviarBloque(); // Emitimos el bloque completo hacia los clientes conectados [cite: 648]

  tiempoUltimoMenuNFC = millis(); // Capturamos la marca de tiempo para el control de inactividad de la interfaz [cite: 648]
}

// ============================================================================
// PARSER Y ENRUTADOR DE SUB-ESTADOS DEL ENTORNO NFC
// ============================================================================
void procesarEntradaNFC(String entrada) { // [cite: 648]
  entrada.toUpperCase(); // Convertimos drásticamente a mayúsculas para sanitizar entradas accidentales [cite: 648]
  
  if (entrada == "M") { // Comando de aborto: Regreso seguro a la raíz del firmware
      programaActivo = 0; // Conmutamos la máquina de estados principal a reposo [cite: 40, 648]
      mostrarMenuPrincipal(); // Dibuja la pantalla de bienvenida con su telemetría [cite: 648]
      return; // Interrumpe la función de forma inmediata [cite: 649]
  } 
  
  if (modoNFC == 0 && entrada == "1") { // Solicitud de escaneo de credenciales físicas original
      modoNFC = 1; // Cambiamos el sub-estado a modo escucha activa (interrogación en bus SPI) [cite: 41, 649]
      Terminal.println("\n>>> ACERCA TARJETA ORIGINAL..."); // Notificación instructiva por terminal [cite: 649]
  } 
  else if (modoNFC == 0 && entrada == "2") { // Solicitud de volcado e inyección criptográfica
      if(!memoriaLlena) { // Validación estricta contra errores de buffer vacío
          Terminal.println("[!] ERROR: Memoria vacia"); // Bloqueo preventivo para no corromper tags [cite: 650]
          return; // Aborta la operación [cite: 651]
      } 
      // Asignación física del puntero dinámico de escritura hacia la dirección de la matriz leída
      bloqueAEscribir = bloqueEscaneado; // [cite: 39, 651]
      modoNFC = 4; // Cambiamos el sub-estado a modo inyección masiva (escritura en bus SPI) [cite: 41, 651]
      Terminal.println("\n>>> ACERCA TARJETA DESTINO..."); // Instructivo visual para aproximar el tag virgen [cite: 652]
  }
}

// ============================================================================
// INTERFAZ DE CONTROL PARA EL SENSOR ULTRASONIDOS HC-SR04
// ============================================================================
void mostrarMenuUltrasonidos() { // [cite: 663]

  Terminal.iniciarBloque(); // 👈 Retenemos para unificar el bloque sónico [cite: 663]

  Terminal.println("\n=================================================="); // [cite: 663]
  Terminal.println("             PROYECTO 2: ULTRASONIDOS             "); // [cite: 663]
  Terminal.println("=================================================="); // [cite: 664]
  Terminal.println(" [1] INICIAR radar de distancia en tiempo real"); // [cite: 664]
  Terminal.println(" [2] DETENER radar de distancia"); // [cite: 664]
  Terminal.println(" [U] VOLVER AL MENU ULTRASONIDOS (A esta pantalla)"); // [cite: 665]
  Terminal.println(" [M] VOLVER AL MENU PRINCIPAL (ESP32 Blasco)"); // [cite: 665]
  Terminal.println("=================================================="); // [cite: 665]

  Terminal.enviarBloque(); // Despachamos la pantalla acústica [cite: 665]
  tiempoUltimoMenuPrincipal = millis(); // Capturamos marca de tiempo de sincronización [cite: 666]
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

  if (temperaturaActual > -127.0) {
    Terminal.print(" >> Ultima lectura — Temp: "); Terminal.print(temperaturaActual, 1); Terminal.print(" C");
    Terminal.print("  Hum: "); Terminal.print(humedadActual, 1); Terminal.println(" %");
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
      programaActivo = 0; // Conmutamos la máquina de estados principal a reposo
      mostrarMenuPrincipal(); // Dibuja la pantalla de bienvenida
      return;
  }

  if (entrada == "1") { // Lectura fresca del sensor
      actualizarDHT();
      Terminal.iniciarBloque();
      if (temperaturaActual > -127.0) {
          Terminal.println("\n------------------ DHT11 ------------------");
          Terminal.print(" Temperatura : "); Terminal.print(temperaturaActual, 1); Terminal.println(" C");
          Terminal.print(" Humedad     : "); Terminal.print(humedadActual, 1);    Terminal.println(" %");
          Terminal.println("---------------------------------------------");
      } else {
          Terminal.println("\n [!] Error: Sensor no responde o datos invalidos.");
      }
      Terminal.enviarBloque();
      mostrarMenuDHT(); // Volvemos a mostrar el menú
  }
}
