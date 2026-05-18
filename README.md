<div align="center">

<img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="ESP32 Blasco OS Logo" width="320" />

<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:cpu.svg?color=white">
    <img src="https://api.iconify.design/lucide:cpu.svg?color=black" alt="Chip" width="32" align="center" />
  </picture> 
  ESP32 Blasco OS | Multi-Program
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:satellite-dish.svg?color=white">
    <img src="https://api.iconify.design/lucide:satellite-dish.svg?color=black" alt="Antenna" width="32" align="center" />
  </picture>
</h1>

**El entorno de ejecución multiprograma definitivo para ingeniería a bajo nivel.**

[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/network/members)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)

**ESP32 Blasco** es una plataforma de ingeniería a bajo nivel y un entorno de ejecución multiprograma diseñado exclusivamente para el microcontrolador ESP32. Actúa como un pequeño "Sistema Operativo" accesible de forma remota a través de **Telnet (Wi-Fi)**. Permite encapsular y ejecutar múltiples proyectos de hardware en la misma placa, intercambiando entre ellos mediante una interfaz de terminal de texto puro (estilo retro/hacker), sin necesidad de usar cables ni volver a flashear el firmware.

[Explorar el código](https://github.com/rubenblascoa/esp32-panelcontrol) · [Reportar un Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Solicitar una Mejora](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Patrocinado por
<a href="https://www.pcbway.es/"><img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="280"></a>

> **Hardware impulsado por PCBWay:** Para el desarrollo y despliegue del ESP32 Blasco OS, contar con un hardware robusto es indispensable. He confiado en PCBWay para la fabricación de mis placas debido a la excelente calidad de sus PCBs y la precisión en el ensamblaje (PCBA). En un entorno donde las conexiones SPI/I2C de alta frecuencia y la telemetría en tiempo real no pueden fallar, la fiabilidad de sus componentes ha sido clave para garantizar la estabilidad del sistema. Su plataforma es intuitiva, el servicio de atención al cliente es rápido y los tiempos de envío son inmejorables. Totalmente recomendado para cualquier ingeniero o maker que busque dar el salto a un hardware de calidad profesional.

---
</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:list.svg?color=white"><img src="https://api.iconify.design/lucide:list.svg?color=black" width="26" align="center"></picture> Características Principales

* **Control 100% Inalámbrico:** Acceso completo a la interfaz de usuario mediante cualquier cliente Telnet (Puerto 23) a través de la red Wi-Fi local.
* **Actualizaciones OTA (Over-The-Air):** Soporte integrado para inyectar nuevo código de forma remota sin conexión USB.
* **Telemetría Avanzada en Tiempo Real:** Monitorización del sistema que incluye:
  * Consumo de RAM (Total, Usada, Libre).
  * Estado de la Memoria Flash (Almacenamiento).
  * Temperatura de los núcleos del silicio (`Temp Core`).
  * Frecuencia del Procesador (`Vel. CPU`).
  * Tiempo de Actividad Ininterrumpida (`Uptime`).
* **Arquitectura Modular ("Cajones"):** El código separa el menú maestro de los subprogramas, permitiendo añadir nuevos sensores o proyectos sin romper el código de los demás.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:git-compare.svg?color=white"><img src="https://api.iconify.design/lucide:git-compare.svg?color=black" width="26" align="center"></picture> El Problema vs La Solución

Desarrollar y probar múltiples proyectos de hardware en un solo microcontrolador suele ser un caos: cables USB constantes, flasheos interminables y código espagueti al intentar juntar todo. 

| Sin Blasco OS | Con Blasco OS |
| :--- | :--- |
| Conexión USB obligatoria para probar | **100% Inalámbrico** vía Telnet |
| Flashear el firmware en cada cambio | **Intercambio en caliente** entre módulos |
| Monitorización básica por Serial | **Telemetría avanzada** (RAM, Flash, Temp, CPU) |
| Actualizaciones por cable | **Soporte OTA** (Over-The-Air) |
| Proyectos acoplados que rompen el código | **Arquitectura Modular** ("Cajones" independientes) |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:blocks.svg?color=white"><img src="https://api.iconify.design/lucide:blocks.svg?color=black" width="26" align="center"></picture> Módulos Activos

Actualmente, el sistema operativo cuenta con tres proyectos principales integrados:

### 1. Estación de Clonación NFC Pro (V14)
Un módulo avanzado de auditoría y clonación RFID utilizando el hardware **MFRC522**.
* **Lectura Profunda:** Extrae toda la información de la tarjeta (incluyendo el Bloque 0 / Sector de máxima seguridad) y la guarda en la memoria RAM del ESP32.
* **Clonación Física:** Permite inyectar datos en el Sector 0 de tarjetas regrabables (*Magic Cards / Gen2 / CUID*).
* **Multi-Origen:** Puedes elegir clonar la tarjeta que tienes guardada en la RAM, inyectar una llave maestra pre-programada (Hardcoded) o dictar un código hexadecimal de 32 caracteres manualmente desde el teclado.

### 2. Radar Ultrasónico (V3)
Módulo de telemetría física utilizando el sensor de distancia **HC-SR04**.
* **Ejecución Asíncrona:** Flujo no-bloqueante; el ESP32 sigue atendiendo las peticiones de red y el servidor web mientras realiza el muestreo de distancia.
* **Lectura en Bucle:** Refresco cíclico configurable con lógica de tolerancia a fallos térmicos y filtrado de rebotes erróneos (Timeout > 400cm / "Fuera de rango").

### 3. Monitor de Temperatura y Humedad Ambiental (V1)
Módulo de adquisición de datos climáticos locales integrado en el ecosistema.
* **Muestreo de Precisión:** Telemetría continua de humedad relativa y temperatura ambiente.
* **Gestión de Datos:** Sincronización automática de lecturas históricas para su almacenamiento estructurado dentro del sistema de archivos interno de la base de datos.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:hard-drive.svg?color=white"><img src="https://api.iconify.design/lucide:hard-drive.svg?color=black" width="26" align="center"></picture> Hardware Requerido e Instalación

* **Placa Base:** ESP32 (S3 N16R8 o similar).
* **Módulo NFC:** Lector RFID MFRC522 (Conectado por bus SPI: SDA->D5, RST->D21).
* **Módulo Distancia:** Sensor HC-SR04 (TRIG->D12, ECHO->D14).

### Puesta en Marcha:
1. Configura tus credenciales Wi-Fi (`ssid` y `password`) en el código fuente.
2. Flashea el código por USB la primera vez mediante Arduino IDE.
3. Abre la consola Serial a `115200 baudios` para descubrir la IP local asignada.
4. Abre el navegador y busca dicha IP local.
5. ¡Disfruta del entorno!

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:settings.svg?color=white"><img src="https://api.iconify.design/lucide:settings.svg?color=black" width="26" align="center"></picture> Configuración de Compilación (Arduino IDE)

Para que el proyecto compile correctamente y el panel web tenga espacio suficiente para la base de datos (LittleFS), es **obligatorio** usar la siguiente configuración en el menú **Herramientas (Tools)** de Arduino IDE. 

> **Acceso al sistema:** El usuario y contraseña establecidos por defecto son **`admin` / `blasco`**. Dicha combinación es modificable en el archivo .ino principal.

*Esta configuración está optimizada para placas **ESP32-S3 (N16R8)** con 16MB de Flash y 8MB de PSRAM.*

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

> **IMPORTANTE:** > * **Partition Scheme:** Es vital seleccionar la partición `9.9MB FATFS`. Esto reserva casi 10MB de memoria exclusivamente para el sistema de archivos (LittleFS) donde se alojan el historial de telemetría y los archivos `.csv`.
> * **PSRAM:** Asegúrate de marcar `OPI PSRAM` para desbloquear los 8MB de memoria RAM externa de la placa, necesarios para manejar los buffers del servidor web asíncrono sin cuelgues.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:monitor.svg?color=white"><img src="https://api.iconify.design/lucide:monitor.svg?color=black" width="26" align="center"></picture> Interfaz y Telemetría

A continuación se muestra el entorno de ejecución del Sistema Operativo. Haz clic en los menús desplegables para ver las capturas de pantalla de la interfaz completa.

<details>
<summary><b>Panel Control (Telemetría y ejecución en vivo)</b></summary>
<br>

<p><i>Versión Oscura</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/db980fe4-600b-43e1-b8bb-b9c366eb26ee" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/eef43556-0c27-49e9-8d07-eda9577c5c75" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/4f032878-b0b0-406b-81f2-786c12560149" />

<p><i>Versión Blanca</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55b18cca-1077-499a-81ed-d8acf6d66cc4" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/6c5cba54-ecff-498f-835e-130887f2bf8b" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/bbca1743-3a4e-4fa7-9f1b-6b4601bce200" />
</details>

<details>
<summary><b>Base de Datos</b></summary>
<br>

<p><i>Versión Oscura</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/15c40684-d907-4bca-9387-72960a42e2ad" />

<p><i>Versión Blanca</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/59ef1539-4c79-4654-aa02-3c0c15b51490" />
</details>

<details>
<summary><b>Login</b></summary>
<br>

<p><i>Versión Oscura</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />

<p><i>Versión Blanca</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />
</details>

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:user.svg?color=white"><img src="https://api.iconify.design/lucide:user.svg?color=black" width="26" align="center"></picture> Contacta conmigo

Desarrollado por **Ruben Blasco Armengod**.

Si tienes alguna duda, sugerencia o propuesta de colaboración, no dudes en ponerte en contacto conmigo:
* **GitHub:** [rubenblascoa](https://github.com/rubenblascoa)
* **Email:** rubenblascoarmengod@gmail.com

También puedes abrir un *issue* en este repositorio y te responderé lo antes posible.
