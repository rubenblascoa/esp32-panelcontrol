# 🚀 ESP32 Blasco OS | Multi-Program Telnet Environment

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
4. Abre la terminal de tu PC (o PuTTY) y conéctate: `telnet [IP_DEL_ESP32]`.
5. ¡Disfruta del entorno! Navega usando las teclas `1`, `2`, `M`, `N` o `U`.

---

## 📸 Interfaz y Telemetría Desconectada

A continuación se muestra el entorno de ejecución desconectado y el panel de control del Sistema Operativo interactuando por Telnet:

<p><i>Panel de telemetría y ejecución en vivo: Versión Oscura</i></p>


<img width="1920" height="582" alt="image" src="https://github.com/user-attachments/assets/c8f147e4-a81f-4320-8caf-17223d8ffc37" />

<img width="1916" height="557" alt="image" src="https://github.com/user-attachments/assets/7f70a8a3-0af5-4ead-bf37-acfb4be0a017" />

<img width="1920" height="913" alt="image" src="https://github.com/user-attachments/assets/4c7c79c2-b56f-42b1-a481-ccad7382f6b2" />
<br><br><br>
<p><i>Panel de telemetría y ejecución en vivo: Versión Blanca</i></p>



<img width="1907" height="595" alt="image" src="https://github.com/user-attachments/assets/7b1b0954-86f6-402e-9e74-5cb4805b160a" />

<img width="1906" height="547" alt="image" src="https://github.com/user-attachments/assets/dbe4eba7-4d52-4cff-9aa6-c7855888b681" />

<img width="1904" height="912" alt="image" src="https://github.com/user-attachments/assets/b13a11c2-0409-4bc6-8c57-e63998465a93" />

---

## 📸 Interfaz y Telemetría Activa

A continuación se muestra el entorno de ejecución desconectado y el panel de control del Sistema Operativo interactuando por Telnet:

<p><i>Panel de telemetría y ejecución en vivo: Versión Oscura</i></p>
<br><br><br>


<p><i>Panel de telemetría y ejecución en vivo: Versión Blanca</i></p>







