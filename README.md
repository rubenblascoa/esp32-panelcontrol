# 🚀 ESP32 Blasco OS | Multi-Program 

**ESP32 Blasco** es una plataforma de ingeniería a bajo nivel y un entorno de ejecución multiprograma diseñado exclusivamente para el microcontrolador ESP32. 

Actúa como un pequeño "Sistema Operativo" accesible de forma remota a través de **Telnet (Wi-Fi)**. Permite encapsular y ejecutar múltiples proyectos de hardware en la misma placa, intercambiando entre ellos mediante una interfaz de terminal de texto puro (estilo retro/hacker), sin necesidad de usar cables ni volver a flashear el firmware.

👨‍💻 **Desarrollado por:** Ruben Blasco Armengod

---

## ⚙️ Características Principales

* 📡 **Control 100% Inalámbrico:** Acceso completo a la interfaz de usuario mediante cualquier cliente Telnet (Puerto 23) a través de la red Wi-Fi local.
* 🔄 **Actualizaciones OTA (Over-The-Air):** Soporte integrado para inyectar nuevo código de forma remota sin conexión USB.
* 📊 **Telemetría Avanzada en Tiempo Real:** Monitorización del sistema que incluye:
  * Consumo de RAM (Total, Usada, Libre).
  * Estado de la Memoria Flash (Almacenamiento).
  * Temperatura de los núcleos del silicio (`Temp Core`).
  * Frecuencia del Procesador (`Vel. CPU`).
  * Tiempo de Actividad Ininterrumpida (`Uptime`).
* 🧱 **Arquitectura Modular ("Cajones"):** El código separa el menú maestro de los subprogramas, permitiendo añadir nuevos sensores o proyectos sin romper el código de los demás.

---

## 🧰 Módulos Activos

Actualmente, el sistema operativo cuenta con dos proyectos principales integrados:

### 1. 🛡️ Estación de Clonación NFC Pro (V14)
Un módulo avanzado de auditoría y clonación RFID utilizando el hardware **MFRC522**.
* **Lectura Profunda:** Extrae toda la información de la tarjeta (incluyendo el Bloque 0 / Sector de máxima seguridad) y la guarda en la memoria RAM del ESP32.
* **Clonación Física:** Permite inyectar datos en el Sector 0 de tarjetas regrabables (*Magic Cards / Gen2 / CUID*).
* **Multi-Origen:** Puedes elegir clonar la tarjeta que tienes guardada en la RAM, inyectar una llave maestra pre-programada (Hardcoded) o dictar un código hexadecimal de 32 caracteres manualmente desde el teclado.

### 2. 📡 Radar Ultrasónico (V3)
Módulo de telemetría física utilizando el sensor de distancia **HC-SR04**.
* Ejecución no-bloqueante (el ESP32 sigue atendiendo la red mientras mide).
* Lectura en bucle con refresco de 1 segundo.
* Detección de errores térmicos y de rebote (Timeout > 400cm / "Fuera de rango").

---

## 🔌 Hardware Requerido e Instalación

* **Placa Base:** ESP32 (WROOM-32 o similar).
* **Módulo NFC:** Lector RFID MFRC522 (Conectado por bus SPI: SDA->D5, RST->D21).
* **Módulo Distancia:** Sensor HC-SR04 (TRIG->D12, ECHO->D14).

### Puesta en Marcha:
1. Configura tus credenciales Wi-Fi (`ssid` y `password`) en el código fuente.
2. Flashea el código por USB la primera vez mediante Arduino IDE.
3. Abre la consola Serial a `115200 baudios` para descubrir la IP local asignada.
4. Abre el navegador y busca dicha ip local.
5. ¡Disfruta del entorno!.

---
### ⚙️ Configuración de Compilación (Arduino IDE)

Para que el proyecto compile correctamente y el panel web tenga espacio suficiente para la base de datos (LittleFS), es **obligatorio** usar la siguiente configuración en el menú **Herramientas (Tools)** de Arduino IDE. 

Además, el usuario y contraseña establecidos son **admin/blasco**. Dicha combinación es modificable en el archivo arduino.

Esta configuración está optimizada para placas **ESP32-S3 (N16R8)** con 16MB de Flash y 8MB de PSRAM.

| Parámetro de Configuración | Valor Exacto Requerido |
| :--- | :--- |
| **Placa (Board)** | `ESP32S3 Dev Module` |
| **USB CDC On Boot** | `Enabled` |
| **CPU Frequency** | `240MHz (WiFi)` |
| **Core Debug Level** | `None` |
| **USB DFU On Boot** | `Disabled` |
| **Erase All Flash Before Sketch Upload** | `Disabled` |
| **Events Run On** | `Core 1` |
| **Flash Mode** | `DIO 80MHz` |
| **Flash Size** | `16MB (128Mb)` |
| **JTAG Adapter** | `Disabled` |
| **Arduino Runs On** | `Core 1` |
| **USB Firmware MSC On Boot** | `Disabled` |
| **Partition Scheme** | `16M Flash (3MB APP/9.9MB FATFS)` ⚠️ |
| **PSRAM** | `OPI PSRAM` ⚠️ |
| **Upload Mode** | `UART0 / Hardware CDC` |
| **Upload Speed** | `921600` |
| **USB Mode** | `Hardware CDC and JTAG` |
| **Zigbee Mode** | `Disabled` |

> ⚠️ **IMPORTANTE:** > * **Partition Scheme:** Es vital seleccionar la partición `9.9MB FATFS`. Esto reserva casi 10MB de memoria exclusivamente para el sistema de archivos (LittleFS) donde se alojan el historial de telemetría y los archivos `.csv`.
> * **PSRAM:** Asegúrate de marcar `OPI PSRAM` para desbloquear los 8MB de memoria RAM externa de la placa, necesarios para manejar los buffers del servidor web asíncrono sin cuelgues.



---
## 📸 Interfaz y Telemetría Panel Control

A continuación se muestra el entorno de ejecución de el panel de control del Sistema Operativo:

<p><i>Panel de telemetría y ejecución en vivo: Versión Oscura</i></p>
<img width="1911" height="638" alt="image" src="https://github.com/user-attachments/assets/db980fe4-600b-43e1-b8bb-b9c366eb26ee" />
<img width="1917" height="541" alt="image" src="https://github.com/user-attachments/assets/eef43556-0c27-49e9-8d07-eda9577c5c75" />
<img width="1910" height="905" alt="image" src="https://github.com/user-attachments/assets/4f032878-b0b0-406b-81f2-786c12560149" />


<br><br><br>
<p><i>Panel de telemetría y ejecución en vivo: Versión Blanca</i></p>
<img width="1909" height="628" alt="image" src="https://github.com/user-attachments/assets/55b18cca-1077-499a-81ed-d8acf6d66cc4" />
<img width="1890" height="551" alt="image" src="https://github.com/user-attachments/assets/6c5cba54-ecff-498f-835e-130887f2bf8b" />
<img width="1858" height="910" alt="image" src="https://github.com/user-attachments/assets/bbca1743-3a4e-4fa7-9f1b-6b4601bce200" />

---

## 📸 Interfaz y Telemetría Base de Datos

A continuación se muestra el entorno de ejecución de las bases de datos del Sistema Operativo:

<p><i>Panel de telemetría y ejecución en vivo: Versión Oscura</i></p>
<img width="1906" height="907" alt="image" src="https://github.com/user-attachments/assets/9eb98e35-184f-4a02-a418-f3e88d0f6121" />


<br><br><br>
<p><i>Panel de telemetría y ejecución en vivo: Versión Blanca</i></p>
<img width="1900" height="902" alt="image" src="https://github.com/user-attachments/assets/2304afdb-c12e-40c0-8ad8-01674a1acfa1" />

---
## 📸 Interfaz y Telemetría Login

A continuación se muestra el entorno de ejecución de el login del Sistema Operativo:

<p><i>Panel de telemetría y ejecución en vivo: Versión Oscura</i></p>
<img width="1914" height="904" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />


<br><br><br>
<p><i>Panel de telemetría y ejecución en vivo: Versión Blanca</i></p>
<img width="1906" height="910" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />







