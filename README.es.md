<div align="center">

<img src="https://cdn-icons-png.flaticon.com/512/8463/8463850.png" alt="ESP32 Blasco OS Logo" width="200" />

<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:cpu.svg?color=white">
    <img src="https://api.iconify.design/lucide:cpu.svg?color=black" alt="Chip" width="28" align="center" />
  </picture> 
  ESP32 Blasco OS | Multi-Program
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:satellite-dish.svg?color=white">
    <img src="https://api.iconify.design/lucide:satellite-dish.svg?color=black" alt="Antenna" width="28" align="center" />
  </picture>
</h3>

**Español** · [English](README.md) · [Français](README.fr.md)

---

**El entorno de ejecución multiprograma definitivo para ingeniería a bajo nivel.**

[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)
![Views](https://komarev.com/ghpvc/?username=rubenblascoa&repo=esp32-panelcontrol&label=Views&icon=0&color=121011&style=flat-square)

**ESP32 Blasco** es una plataforma de ingeniería a bajo nivel y un entorno de ejecución multiprograma diseñado exclusivamente para el microcontrolador ESP32-S3. Actúa como un pequeño "Sistema Operativo" accesible de forma remota a través de **Telnet (Wi-Fi)** y WebSockets. Permite encapsular y ejecutar múltiples proyectos de hardware en la misma placa, intercambiando entre ellos mediante una interfaz de terminal de texto puro (estilo retro/hacker), completamente **sin cables** y con **configuración dinámica en caliente**.

[Explorar el Código](https://github.com/rubenblascoa/esp32-panelcontrol/tree/main/Code) · [Reportar un Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Solicitar una Mejora](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

<div align="center">

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Hardware Impulsado por PCBWay

<br>
<a href="https://www.pcbway.es/">
  <img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="300">
</a>
<br><br>
</div>

> **De la creación de prototipos a la fabricación profesional:** El desarrollo de **ESP32 Blasco OS** exige un hardware capaz de soportar la ejecución asíncrona de múltiples programas en tiempo real. Para el despliegue físico de este proyecto, confío en los servicios de fabricación y ensamblaje (PCBA) de **[PCBWay](https://www.pcbway.es/)**.

> En una arquitectura que delega el 100% de la CPU a los hilos de FreeRTOS, el diseño físico de la placa base es crítico. La integración de PCBWay en este proyecto responde a necesidades técnicas muy específicas del entorno de bajo nivel:

>* **Integridad de Señal (SPI e I2C):** El módulo de clonación NFC (MFRC522) opera a frecuencias donde el ruido es fatal. El enrutamiento preciso de pistas de PCBWay garantiza la ausencia de interferencias electromagnéticas, previniendo colisiones de datos en el bus y asegurando escaneos sin pérdidas.
  
>* **Disipación para Telemetría Real:** Este sistema extrae datos de estrés y temperatura directamente de los núcleos de la CPU. La calidad del cobre y la fibra de vidrio de la placa permite una correcta disipación térmica, vital para mantener el procesador estable bajo carga continua.
  
>* **Precisión de Ensamblaje (PCBA):** La integración del chip ESP32-S3 (N16R8) y la soldadura de componentes SMD requieren tolerancias exactas para que el hardware final soporte el entorno de ejecución 24/7 sin fallos eléctricos.

> La experiencia de fabricación ha sido directa y sin contratiempos, con una gestión fluida de los archivos Gerber y BOM gracias al soporte de su equipo técnico (con una mención especial a Liam por facilitar la coordinación del patrocinio). Si buscas fabricar tus propios esquemas con calidad industrial, los resultados cumplen con los estándares de la ingeniería de hardware.

>**[Explora los servicios de fabricación de PCBWay aquí](https://www.pcbway.es/)**

---

</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:folder-tree.svg?color=white"><img src="https://api.iconify.design/lucide:folder-tree.svg?color=black" width="26" align="center"></picture> Arquitectura y Estructura Modular

El firmware implementa una separación estricta de responsabilidades por hardware y software mediante un desacoplamiento multinúcleo asíncrono. A continuación se detalla el propósito y la lógica de ejecución interna de cada archivo del sistema:

### 1. Núcleo de Entrada y Orquestación
* **`main.ino`**
  * **Propósito:** Es el punto de entrada físico del ESP32-S3 y el archivo de orquestación principal.
  * **Lógica detallada:** Si el sistema carece de credenciales de red válidas, aprovisiona automáticamente un **Punto de Acceso (AP) Wi-Fi** local llamado `Esp32BlascoOS_Setup` para levantar un Captive Portal. Una vez conectado, configura el bus I2C físico, carga los ajustes NVS, conecta el LED de estado, inicializa los núcleos de procesamiento de FreeRTOS (`taskCore0` y `taskCore1` con 16KB de pila cada uno) y se purga a sí mismo llamando a `vTaskDelete(NULL)` para otorgar el control total de la CPU al planificador.

### 2. Configuración y Estado del Sistema
* **`config.h` / `config.cpp`**
  * **Propósito:** Definición abstracta del diseño de las estructuras de memoria global, umbrales y mapas de pines en tiempo de ejecución.
  * **Lógica detallada:** Declara macros para flujos de registro (`LOG_I`/`LOG_E`) y vincula las variables de estado global a través de la palabra clave `extern` para prevenir duplicidades entre unidades de compilación. Los periféricos complejos como el lector `MFRC522` se manejan estrictamente como punteros dinámicos (`MFRC522*`), lo que permite remapear completamente las conexiones GPIO en tiempo de ejecución desde el Captive Portal o los endpoints REST sin modificar una sola línea de código fuente.

### 3. Planificador Multinúcleo (FreeRTOS)
* **`tareas.h` / `tareas.cpp`**
  * **Propósito:** Controla los lazos infinitos de procesamiento distribuidos por hardware que reemplazan el flujo de ejecución predeterminado de Arduino.
  * **Lógica detallada:**
    * **`taskCore0`:** Gestiona las interfaces de red y el almacenamiento local. Maneja las entradas asíncronas crudas del WebSocket, ejecuta flasheos OTA en segundo plano, regula la sincronización NTP y dispara un cronjob de archivos periódico cada 2 horas para respaldar los registros del estado histórico del sistema.
    * **`taskCore1`:** Dedicado por completo a las operaciones físicas de hardware. Evalúa las colas de comandos de la consola de entrada (`cmdQueue`), refresca el LCD de diagnóstico de 5 páginas usando guardas seguras de hilos, y avanza a través de los estados de ejecución de los sensores activos en segundo plano.

### 4. Interfaz Gráfica y Frontend
* **`web_pages.h` / `web_pages.cpp`**
  * **Propósito:** Almacena las estructuras inalterables de HTML, CSS y JavaScript moderno para los paneles de administración remota.
  * **Lógica detallada:** Usa la palabra clave `PROGMEM` para bloquear los diseños web masivos (Dashboard, Visor de BD, Panel de Configuración, Captive Portal y pantallas de Login) dentro del espacio de la memoria Flash. Esto evita que los recursos contaminen el Heap de la RAM dinámica, eliminando por completo los reinicios por agotamiento de memoria durante conexiones simultáneas de clientes.

### 5. Enrutamiento y Seguridad de Red
* **`web_server.h` / `web_server.cpp`**
  * **Propósito:** Maneja el enrutamiento de red asíncrono y los sockets del servidor entrante en el Core 0.
  * **Lógica detallada:** Aprovisiona un motor API REST completo exponiendo 19 endpoints operativos. Valida la autenticidad de las solicitudes verificando la cookie `ZENITH_SESSION` contra el token dinámico en RAM. Las entradas de WebSocket se procesan utilizando asignaciones de memoria seguras (`malloc/memcpy`), blindando el sistema contra la corrupción del heap basada en tramas, y endpoints como `/api/config/pins` manejan flujos de datos JSON en vivo para mapear los estados internos del hardware sobre la marcha.

### 6. Controladores de Sensores (Hardware)
* **`nfc.h` / `nfc.cpp`**
  * **Propósito:** Impulsa la lectura de proximidad, auditoría de bloques y ciclos de clonación a través del bus físico SPI.
  * **Lógica detallada:** Controla el chip MFRC522 para comunicarse con tarjetas MIFARE Classic 1K. Valida las claves de autorización (Key A) antes de extraer los bloques de sectores. En modo escritura, apunta a tarjetas mágicas regrabables (CUID/FUID), reescribiendo el Sector 0 y el Bloque 0 para clonar huellas UID.
* **`ultrasonidos.h` / `ultrasonidos.cpp`**
  * **Propósito:** Captura parámetros métricos espaciales mediante reflexión acústica usando el transductor HC-SR04.
  * **Lógica detallada:** Utiliza una Máquina de Estados Finitos (FSM) de hardware 100% no bloqueante. Dispara un breve pulso de activación de 10µs y vincula una rutina de interrupción de bajo nivel (`IRAM_ATTR ecoISR`) al pin ECHO para calcular el tiempo de vuelo del sonido. Esto elimina las llamadas bloqueantes como `pulseIn()`, permitiendo que el Core 1 realice múltiples tareas libremente durante la propagación acústica.
* **`dht.h` / `dht.cpp`**
  * **Propósito:** Extrae parámetros localizados de humedad relativa y temperatura mediante el sensor DHT11.
  * **Lógica detallada:** Cuenta con una implementación personalizada nativa del protocolo 1-Wire libre de dependencias. Aísla temporalmente el procesador de las anomalías de tiempo desactivando las interrupciones del sistema (`noInterrupts()`) durante la ventana de captura *bit-banging* crítica en microsegundos. Implementa una mecánica automática de auto-reintento de 1 segundo para garantizar la integridad de los datos frente a interferencias electromagnéticas ambientales.

### 7. Utilidades e Historiales en Disco
* **`utils.h` / `utils.cpp`**
  * **Propósito:** Marco de ayuda de backend para suavizado matemático, almacenamiento persistente y enrutamiento del bus físico.
  * **Lógica detallada:** Calcula los perfiles reales de estrés de los núcleos utilizando un algoritmo de Media Móvil Exponencial (EMA 30/70) para filtrar picos momentáneos. Gestiona 3 espacios NVS persistentes separados (`zenithmc`, `hwconfig`, `webcred`) para proteger los mapeos de periféricos subyacentes frente a borrados accidentales del Wi-Fi. Utiliza un semáforo explícito Mutex (`i2cMutex`) para coordinar el tráfico I2C compartido en la pantalla LCD mientras rota por las 5 pantallas de telemetría distintas.
* **`sd_card.h` / `sd_card.cpp`** *(Opcional)*
  * **Propósito:** Interconecta una ranura de expansión física MicroSD a través de un bus SPI dedicado.
  * **Lógica detallada:** Salvaguardado bajo la guardia de compilación del preprocesador `#ifdef SD_CS_PIN`. Cuando está presente, intercepta automáticamente las tareas de registro de la base de datos del almacenamiento interno y transmite la salida directamente a los medios de disco físico.

### 8. Capa de Texto (Interfaz CLI)
* **`menus.h` / `menus.cpp`**
  * **Propósito:** Marco visual y lógica de análisis de acciones para la interfaz de consola basada en texto.
  * **Lógica detallada:** Emplea el diseño de búfer `TerminalHibrida` para empaquetar matrices complejas de telemetría (Uptime, estrés del Core, espacio del Heap) antes de lanzarlas a la red, reduciendo la contaminación de paquetes TCP. Cambia el índice de ejecución `programaActivo` para redirigir los estados de ejecución.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:list.svg?color=white"><img src="https://api.iconify.design/lucide:list.svg?color=black" width="26" align="center"></picture> Características Principales

* **Control 100% Inalámbrico:** Acceso completo a la terminal del sistema y salida de diagnóstico a través de Telnet estándar (Puerto 23) y WebSockets sin procesar.
* **Mapeo Dinámico de Pines de Hardware (Plug & Play):** En su primer despliegue, el SO lanza un hotspot de configuración automatizado (`Esp32BlascoOS_Setup`). Los usuarios pueden configurar de manera segura los parámetros de su router local, asignar pines GPIO personalizados y establecer contraseñas web desde cualquier navegador de smartphone sin reconstruir el firmware.
* **Telemetría en Tiempo Real Suavizada por EMA:** Diagnósticos avanzados que detallan:
  * Límites dinámicos de RAM/PSRAM (Total, Asignada, Libre).
  * Ocupación del mapa de almacenamiento de archivos locales (LittleFS / Tarjeta SD).
  * Cargas matemáticas reales del procesador multinúcleo y perfiles de temperatura del silicio.
* **Base de Datos Respaldada por Tiempo NTP:** Rutinas de registro en segundo plano totalmente automatizadas. Una vez sincronizado con los relojes atómicos de la red, el firmware añade una línea de telemetría estructurada de 10 columnas en la hoja de cálculo CSV del sistema cada 2 horas de forma continua.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:git-compare.svg?color=white"><img src="https://api.iconify.design/lucide:git-compare.svg?color=black" width="26" align="center"></picture> El Problema vs La Solución

Desarrollar y probar múltiples proyectos de hardware en un solo microcontrolador suele ser un caos. 

| Sin Blasco OS | Con Blasco OS |
| :--- | :--- |
| Conexión USB obligatoria para probar | **100% Inalámbrico** vía Telnet |
| Flashear el firmware en cada cambio físico de pin | **Configuración Runtime** vía Captive Portal |
| Monitorización básica vía Serial | **Telemetría avanzada** (RAM, Flash, Temp, CPU) |
| Actualizaciones por cable | **Soporte OTA** (Over-The-Air) |
| Proyectos acoplados que rompen el código | **Arquitectura Modular** ("Cajones" independientes) |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:blocks.svg?color=white"><img src="https://api.iconify.design/lucide:blocks.svg?color=black" width="26" align="center"></picture> Módulos Activos

Actualmente, el sistema operativo cuenta con tres proyectos principales integrados:

### 1. Estación de Clonación NFC Pro (V14)
Un módulo avanzado de auditoría y clonación RFID utilizando el hardware **MFRC522**.
* **Lectura Profunda:** Extrae toda la información de la tarjeta (incluyendo el Sector 0 de máxima seguridad) y la salvaguarda en la RAM.
* **Clonación Física:** Permite inyectar datos en el Sector 0 de tarjetas mágicas regrabables (CUID/FUID).

### 2. Radar Ultrasónico (V3)
Módulo de telemetría física utilizando el sensor de distancia **HC-SR04**.
* **Ejecución Asíncrona (ISR):** Flujo 100% no bloqueante impulsado por interrupciones de hardware; el ESP32 sufre cero micro-congelaciones mientras el sonido rebota.
* **Lectura en Bucle:** Refresco cíclico configurable con tolerancia a fallos térmicos ("Fuera de rango").

### 3. Monitor de Temperatura y Humedad Ambiental (V1)
Módulo de adquisición de datos climáticos locales integrado vía **DHT11**.
* **Protocolo 1-Wire Nativo:** Lectura a bajo nivel (Bit-Banging) que no depende de librerías de terceros, optimizada con retardos por hardware y rutinas de auto-recuperación contra el ruido electromagnético.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:hard-drive.svg?color=white"><img src="https://api.iconify.design/lucide:hard-drive.svg?color=black" width="26" align="center"></picture> Hardware Requerido e Instalación

* **Placa Base:** ESP32 (S3 N16R8 o similar).
* **Módulo NFC:** Lector RFID MFRC522 (Bus SPI).
* **Módulo Distancia:** Sensor HC-SR04.
* **Sensor Climático:** Módulo DHT11 (con resistencia pull-up de 4.7kΩ).
* **Módulo de Almacenamiento (Opcional):** Lector MicroSD (Bus SPI).

> *Nota: El sistema arranca con pines predeterminados, pero todas las conexiones de hardware (RST, SS, TRIG, ECHO, DHT) se pueden reasignar completamente desde el Web Captive Portal sin tocar el código.*

### Despliegue Inicial (Vía Captive Portal):
1. Flashea el código fuente compilado por USB la primera vez utilizando tu IDE preferido.
2. El ESP32 formateará su sistema de archivos interno (LittleFS) y, al no detectar credenciales válidas del hogar, abrirá un Punto de Acceso (AP).
3. Busca en tu teléfono o PC la red Wi-Fi abierta: **`Esp32BlascoOS_Setup`** y conéctate a ella.
4. Un asistente web se abrirá automáticamente. Sigue los pasos para introducir la contraseña de tu router local, configurar tus pines GPIO y definir tus credenciales de administrador web.
5. Al hacer clic en Guardar, el ESP32 se reiniciará, apagará el modo AP y se conectará a tu router doméstico de forma transparente.
6. Abre la consola Serial a `115200 baudios` o revisa la pantalla LCD para descubrir su nueva IP local asignada.
7. Abre esa IP en tu navegador web. ¡Disfruta del entorno!

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:settings.svg?color=white"><img src="https://api.iconify.design/lucide:settings.svg?color=black" width="26" align="center"></picture> Configuración de Compilación (Arduino IDE)

Para garantizar que el proyecto compile correctamente y que la Web UI tenga espacio suficiente para la base de datos de LittleFS, es **obligatorio** aplicar los siguientes ajustes en el menú **Herramientas (Tools)** de Arduino IDE. 

*Esta configuración está optimizada para placas **ESP32-S3 (N16R8)** con 16MB de Flash y 8MB de PSRAM.*

| Parámetro de Configuración | Valor Exacto Requerido |
| :--- | :--- |
| **Board** | `ESP32S3 Dev Module` |
| **USB CDC On Boot** | `Enabled` |
| **CPU Frequency** | `240MHz (WiFi)` |
| **Core Debug Level** | `None` |
| **USB DFU On Boot** | `Disabled` |
| **Erase All Flash Before Sketch Upload** | `Disabled` |
| **Events Run On** | `Core 1` |
| **Flash Mode** | `QIO 80MHz` |
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

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:monitor.svg?color=white"><img src="https://api.iconify.design/lucide:monitor.svg?color=black" width="26" align="center"></picture> Interfaz y Telemetría

A continuación se muestra el entorno de ejecución del Sistema Operativo. Haz clic en los menús desplegables para expandir y ver las capturas de pantalla de la interfaz completa.

<details>
<summary><b>Panel de Control (Telemetría y ejecución en vivo)</b></summary>
<br>

<p><i>Tema Oscuro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/db980fe4-600b-43e1-b8bb-b9c366eb26ee" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/eef43556-0c27-49e9-8d07-eda9577c5c75" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/4f032878-b0b0-406b-81f2-786c12560149" />

<p><i>Tema Claro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55b18cca-1077-499a-81ed-d8acf6d66cc4" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/6c5cba54-ecff-498f-835e-130887f2bf8b" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/bbca1743-3a4e-4fa7-9f1b-6b4601bce200" />
</details>

<details>
<summary><b>Gestión de Base de Datos</b></summary>
<br>

<p><i>Tema Oscuro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/15c40684-d907-4bca-9387-72960a42e2ad" />

<p><i>Tema Claro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/59ef1539-4c79-4654-aa02-3c0c15b51490" />
</details>

<details>
<summary><b>Autenticación (Login) </b></summary>
<br>

<p><i>Tema Oscuro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />

<p><i>Tema Claro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />
</details>


<details>
<summary><b>Configuración Interna</b></summary>
<br>

<p><i>Tema Oscuro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/0ef0f0d3-2d07-4788-a5ba-e5ab27a03350" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/fc443c62-634d-437d-b8ac-b34004d1911f" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/25386ba6-4501-4d4b-b53e-b7bc85d0a78c" />


<p><i>Tema Claroe</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/92536347-e43d-4ae2-8d77-b39f28956280" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/f0722d4c-c2fb-45b4-9eb4-78b2052cc72c" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/a1f0496e-58c5-4706-96f9-b061129aa223" />

</details>

<details>
<summary><b>Configuración Inicial</b></summary>
<br>

<p><i>Tema Oscuro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/7baf8092-9362-4781-b994-7e944e0e2b77" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/c21cca77-21ab-4a27-968c-e9bdb81bb691" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/780ecdca-629f-4bed-ac2a-09ff42127f92" />



<p><i>Tema Claro</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/2d2c4c3b-e5e7-4fae-a8e1-2485f2c790c6" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/8361138e-f22b-478e-94a1-d0e94df2959a" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55d62211-ca00-482a-b7dc-7fac87c9ab99" />

</details>

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:mail.svg?color=white"><img src="https://api.iconify.design/lucide:mail.svg?color=black" width="26" align="center"></picture> Contacto y Patrocinio

Desarrollado con pasión por **Ruben Blasco Armengod**.

Si estás interesado en patrocinar la expansión continua de este proyecto, colaborar en diseños de placas base de hardware personalizadas o integrar nuevos sensores en el núcleo multitarea, comunícate a través de cualquiera de los siguientes canales técnicos:

* **GitHub:** [@rubenblascoa](https://github.com/rubenblascoa)
* **Instagram:** [@rubenblascoa](https://instagram.com/rubenblascoa)
* **Email:** rubenblascoarmengod@gmail.com
