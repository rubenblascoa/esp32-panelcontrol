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
 * @file main.ino
 * @brief Punto de entrada principal y orquestador del firmware multimódulo ZenithMC.
 */

// ============================================================================
// INCLUSIÓN DE COMPONENTES MODULARES (Capa de abstracción propia)
// ============================================================================
#include "config.h"       // Carga todas las librerías base, macros de pines y variables globales compartidas [cite: 690, 692]
#include "tareas.h"       // Habilita los lazos infinitos asíncronos taskCore0 y taskCore1 de FreeRTOS [cite: 1367]
#include "web_server.h"   // Expone el motor del servidor web y gestores de sockets [cite: 1230]
#include "utils.h"        // Proporciona herramientas de control LCD, base de datos e historial [cite: 1246, 1275]

// ============================================================================
// CONFIGURACIÓN INICIAL DE HARDWARE EN ARRANQUE (RAÍZ DE HARDWARE)
// ============================================================================
void setup() { //
  // 1. Apertura inmediata del puerto serie de depuración física a la velocidad estándar de 115200 baudios
  Serial.begin(115200); //
  delay(2000);          // Ventana de protección eléctrica de 2 segundos para estabilizar voltajes en frío [cite: 1358]

  // 2. Inicialización prioritaria del bus de comunicación I2C (Pines SDA->GPIO 8 | SCL->GPIO 9) [cite: 1358, 1359]
  Wire.begin(8, 9); // Abre los canales físicos de datos
  
  // Creación del semáforo Mutex de FreeRTOS para blindar el acceso compartido al hardware I2C entre núcleos
  i2cMutex = xSemaphoreCreateMutex(); // [cite: 1359]
  
  // Secuencia estricta de doble inicialización del chip controlador del display LCD
  lcd.init(); // Primer disparo de reset eléctrico [cite: 1359]
  lcd.init(); // Segundo disparo forzado para asentar los registros del bus sin usar retrasos delay [cite: 1359]
  lcd.backlight(); // Encendemos los transistores de la retroiluminación de la pantalla LCD [cite: 1359]
  
  // Imprimimos el banner de inicio del sistema operativo Zenith en la primera columna y fila [cite: 1359]
  lcd.setCursor(0, 0); // [cite: 1359]
  lcd.print("ZENITH SYSTEM..."); // [cite: 1359]

  // 3. Volcado de diagnóstico hardware inicial por el puerto serie serial para telemetría física de taller
  Serial.printf("📦 Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024)); // Reporta tamaño físico de la ROM Flash [cite: 1360]
  Serial.printf("🧠 PSRAM: %d MB\n", ESP.getPsramSize() / (1024 * 1024));   // Reporta tamaño neto de la RAM externa [cite: 1360]
  Serial.println("\n\n--- INICIANDO CHIVATO ---"); // Mensaje de control de arranque [cite: 1361]
  Serial.flush(); // Fuerza la salida inmediata de los bytes acumulados por los cables de datos tx/rx [cite: 1361]

  // 4. Reserva dinámica de espacio en la memoria RAM para albergar los objetos servidores asíncronos [cite: 1361]
  Serial.println("[PASO 1] Creando Servidor en RAM..."); // [cite: 1361]
  Serial.flush(); // [cite: 1362]
  server = new AsyncWebServer(80); // Instanciación del servidor HTTP apuntando al puerto por defecto de internet (80) [cite: 1362]
  ws = new AsyncWebSocket("/ws");  // Instanciación del túnel de tiempo real sobre la ruta de sockets /ws [cite: 1362]
  
  // 5. Configuración eléctrica e industrial del mapa de pines generales de entrada y salida (GPIOs) [cite: 1362]
  Serial.println("[PASO 2] Servidor OK. Configurando Pines..."); // [cite: 1362]
  Serial.flush(); //
  pinMode(LED_PIN, OUTPUT);   // Configura el pin del diodo LED direccionable como salida de corriente [cite: 1362]            
  pinMode(TRIG_PIN, OUTPUT); // Configura el pin disparador del ultrasonidos como salida de pulsos [cite: 1362]
  pinMode(ECHO_PIN, INPUT);   // Configura el pin receptor del eco acústico como entrada de impedancia [cite: 1363]

  // 6. Activación del módem interno de radiofrecuencia e inicio de la negociación WiFi [cite: 1363]
  Serial.println("[PASO 3] Pines OK. Encendiendo WiFi..."); // [cite: 1363]
  Serial.flush(); //
  WiFi.mode(WIFI_STA); // Forzamos al procesador de red a trabajar bajo el protocolo de Estación cliente [cite: 1363]
  WiFi.begin(ssid, password); // Disparamos la trama de autenticación con las credenciales cifradas de config.cpp [cite: 1363]
  
  // Sincronización horaria civil configurando los servidores de tiempo atómico internacionales (NTP) para España [cite: 1364]
  configTime(3600, 3600, "pool.ntp.org", "time.nist.gov"); // [cite: 1364]
  
  // Bucle de espera bloqueante controlado para congelar el inicio hasta certificar enlace inalámbrico estable [cite: 1364]
  Serial.println("[PASO 4] Esperando conexion..."); // [cite: 1364]
  Serial.flush(); // [cite: 1364]
  while (WiFi.status() != WL_CONNECTED) { // Mientras el módem reporte que no hay conexión [cite: 1365]
    delay(500); // Pausa controlada de medio segundo entre intentos [cite: 1365]
    Serial.print("."); // Inyecta puntos suspensivos por consola como chivato visual [cite: 1365]                   
    Serial.flush(); // Despacha el byte [cite: 1365]
  }
  
  // Enlace establecido exitosamente [cite: 1365]
  Serial.println("\n[OK] WiFi Conectado!"); // [cite: 1365]
  Serial.print("[IP]: "); // [cite: 1366]
  Serial.println(WiFi.localIP()); // Imprime la dirección IPv4 asignada por el enrutador en tu red de área local [cite: 1366]

  // 7. Arranque de los servicios y escuchas de periféricos rápidos [cite: 1366]
  ArduinoOTA.begin();                     // Inicializa el servicio de escucha inalámbrica para carga de firmwares de red [cite: 1366]
  SPI.begin(18, 13, 11, SS_PIN);         // Inicializa el bus SPI hardware asignando los pines CLK->18, MISO->13 y MOSI->11 [cite: 1366]
  mfrc522.PCD_Init();                     // Inicializa los registros lógicos y la antena inductiva del chip RC522 [cite: 1366]

  // 8. ASIGNACIÓN ASÍNCRONA DE TAREAS A LOS NÚCLEOS FÍSICOS (Planificador FreeRTOS) [cite: 1366]
  Serial.println("[..] Iniciando núcleos de proceso..."); // [cite: 1366]
  
  // Enganchamos la tarea de red taskCore0 en el Núcleo 0 (Core 0), asignándole 8KB de pila y prioridad 1 [cite: 1367]
  xTaskCreatePinnedToCore(taskCore0, "RedTask", 8192, NULL, 1, NULL, 0); // [cite: 1367]
  delay(500); // Margen de gracia crítico de 500ms para permitir al sistema operativo asentar el hilo cero [cite: 1367]
  
  // Enganchamos la tarea de hardware taskCore1 en el Núcleo 1 (Core 1), asignándole 8KB de pila y prioridad 1 [cite: 1367]
  xTaskCreatePinnedToCore(taskCore1, "AppTask", 8192, NULL, 1, NULL, 1); // [cite: 1367]

  // Notificación de culminación exitosa de los servicios de arranque básicos [cite: 1368]
  Serial.println("[EXITO] Todo el sistema esta ONLINE."); // [cite: 1368]

  // Reporte de diagnóstico térmico y de memorias post-arranque en la consola de telemetría de taller [cite: 1368]
  Serial.println("\n--- INICIANDO SECUENCIA DE ARRANQUE ---"); // [cite: 1368]
  Serial.printf("📦 Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024)); // [cite: 1369]
  Serial.printf("🧠 PSRAM: %d MB\n", ESP.getPsramSize() / (1024 * 1024));   // [cite: 1369]
  Serial.println("---------------------------------------"); // [cite: 1369]

  // 9. Configuración cromática inicial del LED indicador RGB integrado [cite: 1369]
  pixel.begin(); // Enciende los controladores internos de la librería [cite: 1369]
  pixel.setBrightness(50); // Atenúa la potencia de salida al 50% para proteger la vista de destellos intensos [cite: 1370]

  delay(100);          // Pausa milimétrica de estabilización eléctrica terminal [cite: 1370]
  sistemaListo = true; // Eleva el interruptor maestro final: Las tareas paralelas de FreeRTOS rompen su barrera [cite: 1370]
}

// ============================================================================
// BUCLE RECURRENTE CLÁSICO DE ARDUINO (Inoperante y Delegado a FreeRTOS)
// ============================================================================
void loop() {    
  // El ciclo repetitivo nativo de Arduino queda completamente anulado.
  // Al invocar vTaskDelete pasando un argumento de tipo NULL, forzamos al programador multinúcleo
  // a auto-destruir el lazo loop() clásico, liberando de forma inmediata todos sus recursos de memoria RAM
  // y delegando el 100% del silicio a los dos hilos asíncronos paralelos distribuidos. [cite: 1371, 1372]
  vTaskDelete(NULL); // [cite: 1372]
}