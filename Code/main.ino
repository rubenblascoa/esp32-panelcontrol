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
#include "config.h"      // Carga todas las librerías base, macros de pines y variables globales compartidas 
#include "tareas.h"      // Habilita los lazos infinitos asíncronos taskCore0 y taskCore1 de FreeRTOS 
#include "web_server.h"  // Expone el motor del servidor web y gestores de sockets 
#include "utils.h"       // Proporciona herramientas de control LCD, base de datos e historial 
#include "sd_card.h"     // Proporciona funciones para la tarjeta SD
#include <Preferences.h>


// ============================================================================
// CONFIGURACIÓN INICIAL DE HARDWARE EN ARRANQUE (RAÍZ DE HARDWARE)
// ============================================================================
void setup() {  
  // 1. Apertura inmediata del puerto serie de depuración física a la velocidad estándar de 115200 baudios
  Serial.begin(115200);  //
  delay(2000);           // Ventana de protección eléctrica de 2 segundos para estabilizar voltajes en frío 

  // Cargar configuración de hardware y credenciales web desde NVS
  cargarConfigHardware();
  cargarCredencialesWeb();
  Serial.printf("[CONFIG] Pines: NFC RST=%d CS=%d | HC-SR04 TRIG=%d ECHO=%d | DHT=%d\n",
    RST_PIN, SS_PIN, TRIG_PIN, ECHO_PIN, DHT_PIN);

  // 2. Inicialización prioritaria del bus de comunicación I2C (Pines SDA->GPIO 8 | SCL->GPIO 9) 
  Wire.begin(8, 9);  // Abre los canales físicos de datos

  // Creación de semáforos Mutex de FreeRTOS para blindar accesos compartidos entre núcleos
  i2cMutex = xSemaphoreCreateMutex();
  // [FIX] Mutex para el bus SPI compartido entre MFRC522 y SD Card.
  // Sin este mutex, operaciones simultáneas desde distintos cores corrompen las transacciones SPI.
  spiMutex = xSemaphoreCreateMutex();

  // [FIX] Delay de estabilización del bus I2C antes de init (reemplaza la doble
  // inicialización anterior, que dejaba registros del HD44780 en estados intermedios
  // en algunos chips). 50 ms es suficiente para que el bus I2C estabilice la tensión.
  delay(50);
  lcd.init();
  lcd.backlight();  // Encendemos los transistores de la retroiluminación de la pantalla LCD

  
  // Imprimimos el banner de inicio del sistema operativo Zenith en la primera columna y fila 
  lcd.setCursor(0, 0);            // 
  lcd.print("ZENITH SYSTEM...");  // 


  // 3. Volcado de diagnóstico hardware inicial por el puerto serie serial para telemetría física de taller
  Serial.printf("📦 Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));  // Reporta tamaño físico de la ROM Flash 
  Serial.printf("🧠 PSRAM: %d MB\n", ESP.getPsramSize() / (1024 * 1024));   // Reporta tamaño neto de la RAM externa 
  Serial.println("\n\n--- INICIANDO CHIVATO ---");                            // Mensaje de control de arranque 
  Serial.flush();                                                             // Fuerza la salida inmediata de los bytes acumulados por los cables de datos tx/rx 

  // 3b. Creación de la cola de comandos FreeRTOS para comunicación entre cores
  cmdQueue = xQueueCreate(10, sizeof(char*));
  
  // 4. Reserva dinámica de espacio en la memoria RAM para albergar los objetos servidores asíncronos 
  Serial.println("[PASO 1] Creando Servidor en RAM...");  // 
  Serial.flush();                                         // 
  server = new AsyncWebServer(80);                        // Instanciación del servidor HTTP apuntando al puerto por defecto de internet (80) 
  ws = new AsyncWebSocket("/ws");                         // Instanciación del túnel de tiempo real sobre la ruta de sockets /ws 

  // 5. Configuración eléctrica e industrial del mapa de pines generales de entrada y salida (GPIOs) 
  Serial.println("[PASO 2] Servidor OK. Configurando Pines...");  // 
  Serial.flush();                                                 //
  pinMode(LED_PIN, OUTPUT);                                       // Configura el pin del diodo LED direccionable como salida de corriente 
  pinMode(TRIG_PIN, OUTPUT);                                      // Configura el pin disparador del ultrasonidos como salida de pulsos 
  pinMode(ECHO_PIN, INPUT);                                       // Configura el pin receptor del eco acústico como entrada de impedancia 

  // 6. Activación del módem e inicio WiFi con NVS (credenciales guardadas o AP) 
  Serial.println("[PASO 3] Pines OK. Cargando credenciales WiFi...");
  Serial.flush();

  String credSSID, credPass;
  bool credencialesOK = cargarCredenciales(credSSID, credPass);

  if (credencialesOK && credSSID.length() > 0) {
    Serial.printf("[WiFi] Credenciales encontradas: %s. Conectando en modo STA...\n", credSSID.c_str());
    // Conectar en modo STA puro: sin AP activo no hay interferencia de canal ni confusion de DHCP
    WiFi.mode(WIFI_STA);
    WiFi.begin(credSSID.c_str(), credPass.c_str());
    configTime(3600, 0, "pool.ntp.org", "time.nist.gov");

    Serial.println("[PASO 4] Esperando conexion (max 20s)...");
    Serial.flush();
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 40) {  // 40 x 500ms = 20 segundos max
      delay(500);
      Serial.print(".");
      Serial.flush();
      intentos++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[OK] WiFi Conectado!");
      Serial.print("[IP]: ");
      Serial.println(WiFi.localIP());
      modoConfiguracion = false;
    } else if (setupCompletado()) {
      Serial.println("\n[WiFi] No conectado a los 20s, pero ya configurado antes. AP activo solo para acceso local.");
      modoConfiguracion = false;
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP("Esp32BlascoOS_Setup");
      dnsServer.start(53, "*", WiFi.softAPIP());
    } else {
      Serial.println("\n[WARN] No se pudo conectar. Iniciando modo AP para reconfigurar...");
      modoConfiguracion = true;
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP("Esp32BlascoOS_Setup");
      dnsServer.start(53, "*", WiFi.softAPIP());
      Serial.println("[AP] Esp32BlascoOS_Setup activo en 192.168.4.1");
    }
  } else {
    Serial.println("[WiFi] No hay credenciales. Iniciando modo AP de configuracion...");
    modoConfiguracion = true;
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("Esp32BlascoOS_Setup");
    dnsServer.start(53, "*", WiFi.softAPIP());
    Serial.println("[AP] Esp32BlascoOS_Setup activo en 192.168.4.1");
  }

  // 7. Arranque de los servicios y escuchas de periféricos rápidos 
  if (webPass.length() > 0) {
      ArduinoOTA.setPassword(webPass.c_str());
  }
  ArduinoOTA.begin();

  SPI.begin(18, 13, 11, SS_PIN);
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);
  mfrc522->PCD_Init(); 

  // 8. INICIALIZACIÓN DE LA TARJETA SD (deshabilitada por defecto; descomentar pines en config.h) 
  #ifdef SD_CS_PIN
  Serial.println("[..] Inicializando tarjeta SD...");
  inicializarSD();
  #endif

  // 10. ASIGNACIÓN ASÍNCRONA DE TAREAS A LOS NÚCLEOS FÍSICOS (Planificador FreeRTOS) 
  Serial.println("[..] Iniciando núcleos de proceso...");  // 

  // Enganchamos la tarea de red taskCore0 en el Núcleo 0 (Core 0), asignándole 8KB de pila y prioridad 1 
  xTaskCreatePinnedToCore(taskCore0, "RedTask", 16384, NULL, 1, NULL, 0);
  delay(500);

  xTaskCreatePinnedToCore(taskCore1, "AppTask", 16384, NULL, 1, NULL, 1);
  delay(100);

  xTaskCreatePinnedToCore(taskDHT, "DHTTask", 4096, NULL, 0, NULL, 1);

  // Notificación de culminación exitosa de los servicios de arranque básicos 
  Serial.println("[EXITO] Todo el sistema esta ONLINE.");  // 

  // Reporte de diagnóstico térmico y de memorias post-arranque en la consola de telemetría de taller 
  Serial.println("\n--- INICIANDO SECUENCIA DE ARRANQUE ---");                // 
  Serial.printf("📦 Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));  // 
  Serial.printf("🧠 PSRAM: %d MB\n", ESP.getPsramSize() / (1024 * 1024));   // 
  Serial.println("---------------------------------------");                  // 

  // 11. Configuración cromática inicial del LED indicador RGB integrado 
  pixel.begin();            // Enciende los controladores internos de la librería 
  pixel.setBrightness(50);  // Atenúa la potencia de salida al 50% para proteger la vista de destellos intensos 

  delay(100);           // Pausa milimétrica de estabilización eléctrica terminal 
  sistemaListo = true;  // Eleva el interruptor maestro final: Las tareas paralelas de FreeRTOS rompen su barrera 
}

// ============================================================================
// BUCLE RECURRENTE CLÁSICO DE ARDUINO (Inoperante y Delegado a FreeRTOS)
// ============================================================================
void loop() {
  // El ciclo nativo de Arduino queda completamente delegado a FreeRTOS.
  // [FIX] Se usa vTaskDelay(portMAX_DELAY) en lugar de vTaskDelete(NULL):
  // la tarea existe pero duerme indefinidamente sin consumir CPU, evitando
  // comportamientos indefinidos si Arduino reimplementa su scheduler interno.
  vTaskDelay(portMAX_DELAY);
}