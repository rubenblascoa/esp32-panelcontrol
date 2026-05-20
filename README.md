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

[Español](README.es.md) · **English** · [Français](README.fr.md)

---

**The ultimate multi-program execution environment for low-level engineering.**

![Visitas](https://komarev.com/ghpvc/?username=rubenblascoa&repo=esp32-panelcontrol&label=Views&icon=0&color=121011&style=flat-square)
[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/network/members)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)

**ESP32 Blasco** is a low-level engineering platform and a multi-program execution environment designed exclusively for the ESP32 microcontroller. It acts as a lightweight "Operating System" remotely accessible via **Telnet (Wi-Fi)**. It allows encapsulating and executing multiple hardware projects on the same board, switching between them through a pure text terminal interface (retro/hacker style), completely **wire-free** and **without re-flashing** the firmware.

[Explore the Code](https://github.com/rubenblascoa/esp32-panelcontrol/tree/main/Code) · [Report a Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Request an Improvement](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

<div align="center">

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Hardware Powered by PCBWay

<br>
<a href="https://www.pcbway.com/">
  <img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="300">
</a>
<br><br>
</div>

> **From prototyping to professional manufacturing:** The development of **ESP32 Blasco OS** demands hardware capable of supporting the asynchronous execution of multiple programs in real time. For the physical deployment of this project, I rely on the manufacturing and assembly (PCBA) services of **[PCBWay](https://www.pcbway.com/)**.

In an architecture that delegates 100% of the CPU to FreeRTOS threads, the physical design of the motherboard is critical. The integration of PCBWay into this project responds to very specific technical needs of the low-level environment:

* **Signal Integrity (SPI and I2C):** The NFC cloning module (MFRC522) operates at frequencies where noise is fatal. PCBWay's precise track routing guarantees no electromagnetic interference, preventing data collisions on the bus and ensuring lossless scans.
* **Dissipation for Real Telemetry:** This system extracts stress and temperature data directly from the CPU cores. The quality of the board's copper and fiberglass allows for proper thermal dissipation, vital for keeping the processor stable under continuous load.
* **Assembly Precision (PCBA):** The integration of the ESP32-S3 (N16R8) chip and the soldering of SMD components require exact tolerances so that the final hardware supports the 24/7 execution environment without electrical failures.

The manufacturing experience has been straightforward and seamless, with smooth management of the Gerber and BOM files thanks to the support of their technical team (with a special mention to Liam for facilitating the sponsorship coordination). If you are looking to manufacture your own schematics with industrial quality, the results meet the standards of hardware engineering.

**[Explore PCBWay's manufacturing services here](https://www.pcbway.com/)**

---

</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:folder-tree.svg?color=white"><img src="https://api.iconify.design/lucide:folder-tree.svg?color=black" width="26" align="center"></picture> Architecture and Modular Structure

The firmware implements a strict separation of concerns between hardware and software through an asynchronous multi-core decoupling. The purpose and internal execution logic of each system file are detailed below:

### 1. Entry Core and Orchestration
* **`main.ino`**
  * **Purpose:** It is the physical entry point of the ESP32-S3 and the main file read by the Arduino IDE to compile the entire directory.
  * **Detailed logic:** Contains an empty `loop()` and a `setup()` method responsible for waking up components in a strict cascade (serial port, I2C bus, LCD screen, GPIO pins, Wi-Fi modem, and NTP synchronization). Upon completion, it creates the two FreeRTOS tasks (`taskCore0` and `taskCore1`), passes control to them, and self-destructs using `vTaskDelete(NULL)` to delegate 100% of the CPU to the asynchronous processing threads.

### 2. Configuration and Shared Memory
* **`config.h`**
  * **Purpose:** Defines system libraries, physical pin mappings for the board, and abstractly declares variables shared by both cores.
  * **Detailed logic:** Houses `#include` directives and `#define` statements for the LED, RFID reader, and ultrasonic sensor. It contains global variable signatures preceded by the `extern` keyword, indicating to other `.cpp` files that the variable exists in the common RAM, preventing duplicates and linker errors.
* **`config.cpp`**
  * **Purpose:** Acts as the actual physical space in RAM where the variables from the `config.h` dictionary are created and initialized.
  * **Detailed logic:** Executes only once at startup to reserve the exact space required by the firmware. It stores the real Wi-Fi credentials, memory allocation for web server objects, fixed arrays for MIFARE keys, and command synchronization buffers.

### 3. Duplicated Text Channel
* **`terminal.h`**
  * **Purpose:** Defines the structure of the custom `TerminalHibrida` class.
  * **Detailed logic:** Declares the class inheriting from the native Arduino `Print` library, exposing traditional `.print()` and `.println()` methods, as well as dynamic block controllers (`iniciarBloque()`, `enviarBloque()`).
* **`terminal.cpp`**
  * **Purpose:** Controls the asynchronous algorithm that duplicates text messages in real time.
  * **Detailed logic:** When calling `Terminal.println()`, it intercepts characters, sending them first over the open TCP socket to Telnet (Putty). In parallel, if a web browser is listening, it accumulates characters in a dynamic string (`bufferWeb`) and dispatches them to the WebSocket when finding a newline `\n` (unless in Block Mode). It features a security rule that flushes the buffer if it exceeds 200 characters to prevent memory leaks.

### 4. Graphical Interface and Frontend
* **`web_pages.h`**
  * **Purpose:** Declares three global text constants containing the structured frontend files.
  * **Detailed logic:** Exposes references to `index_html`, `db_html`, and `login_html` accompanied by the `PROGMEM` storage modifier.
* **`web_pages.cpp`**
  * **Purpose:** Stores the exact and massive HTML, CSS, and JavaScript code for the three web control panels.
  * **Detailed logic:** Forces the ESP32-S3 via the `PROGMEM` directive to save these pages directly into the unalterable Flash memory (ROM) transistors. If hosted in conventional RAM, they would consume over 80% of the dynamic Heap, causing memory exhaustion reboots when users connect.

### 5. Routing and Network Security
* **`web_server.h`**
  * **Purpose:** Defines the asynchronous HTTP server functions and the WebSocket callback that processes web frames.
* **`web_server.cpp`**
  * **Purpose:** Acts as the network traffic dispatcher assigned to Core 0.
  * **Detailed logic:**
    * *Security:* Executes the `estaLogueado()` function, inspecting HTTP headers for the `ZENITH_SESSION` cookie linked to the random token in RAM.
    * *HTTP Routes:* Handles requests for `/login` (creating the cookie with `HttpOnly` and `SameSite=Strict` flags), `/logout` (destroys the session), `/datos.csv` (streams the historical file directly from LittleFS to the browser), and `/delete-db` (deletes the physical log file).
    * *WebSocket (onWsEvent):* Captura la trama de red en crudo cuando un usuario escribe en la consola del navegador; si recibe `"reboot"` reinicia la placa, y si recibe comandos de control, los copia en `entradaWeb` e iza la bandera `hayEntradaWeb` para avisar al Core 1.

### 6. Sensor Controllers (Hardware)
* **`nfc.h` / `nfc.cpp`**
  * **Purpose:** Manages proximity reading and cloning cycles via the physical SPI bus.
  * **Detailed logic:** When Core 1 grants access, it continuously interrogates the MFRC522 hardware. In read mode, it performs a cryptographic challenge to the physical tag using MIFARE passwords; if it responds correctly, it dumps Block 0 into the `bloqueEscaneado` variable. In write mode, it injects this data matrix into a blank rewritable card.
* **`ultrasonidos.h` / `ultrasonidos.cpp`**
  * **Purpose:** Measures Euclidean space using high-frequency acoustic bounces.
  * **Detailed logic:** Drives the `TRIG_PIN` to zero potential, emits an ultrasonic pulse by holding the pin high for exactly 10 microseconds, and cuts it off. Immediately after, it executes a high-precision `pulseIn` call on the `ECHO_PIN` with a 30ms grace period. If the echo returns, it calculates the distance in centimeters by dividing the time by two and applying the speed of sound.

### 7. Utilities and Disk Logs
* **`utils.h` / `utils.cpp`**
  * **Purpose:** Provides backend logical support to the firmware for statistics, time, and I2C bus management.
  * **Detailed logic:**
    * `calcularUsoCPU()`: Mathematically estimates the heuristic stress load of the cores based on active tasks.
    * `guardarEnHistorial()`: Opens the `/datos.csv` file in LittleFS and appends a structured text line with the date, internal chip temperature, dynamic CPU loads, RAM/Flash occupancy, and Wi-Fi signal level.
    * `actualizarLCD()`: Controls the physical screen via the `xSemaphoreTake(i2cMutex)` call to lock the I2C bus before writing, avoiding data collisions, rendering visual progress bars (`|====  |`), and automatically rotating every 3 seconds among 4 telemetry pages.

### 8. Text Layer (CLI Interface)
* **`menus.h` / `menus.cpp`**
  * **Purpose:** Contains the visual layout in plain text and the hierarchical navigation tree of the operating system.
  * **Detailed logic:** Manages what the user visualizes when connecting via terminal. Calls `Terminal.iniciarBloque()` to retain characters, prints the console's decorative frames, and concatenates the real-time calculation of PSRAM, processor speed, and Uptime before sending the unified block to the network. Houses the functions that modify the `programaActivo` variable to switch menus.

### 9. Multi-Core Scheduler (FreeRTOS)
* **`tareas.h` / `tareas.cpp`**
  * **Purpose:** Houses the two hardware-distributed infinite loops that replace the conventional Arduino `loop`.
  * **Detailed logic:**
    * `taskCore0` (Assigned to Core 0): Mounts the LittleFS file system (formatting it if corruption is detected). Processes incoming network connections, manages OTA updates, runs the synchronous NTP clock, triggers the database cronjob every 2 hours, and controls the stroboscopic flashing of the status LED.
    * `taskCore1` (Assigned to Core 1): Monitored in parallel if any command has entered via Putty or WebSockets. Depending on the state of `programaActivo`, it asynchronously executes NFC listening on the SPI bus or triggers ultrasonic pings at an exact frequency of 1Hz (every 1000ms) without ever interfering with the other core's network processes.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:list.svg?color=white"><img src="https://api.iconify.design/lucide:list.svg?color=black" width="26" align="center"></picture> Main Features

* **100% Wireless Control:** Full access to the user interface using any Telnet client (Port 23) via the local Wi-Fi network.
* **OTA (Over-The-Air) Updates:** Built-in support to inject new code remotely without a USB connection.
* **Advanced Real-Time Telemetry:** System monitoring including:
  * RAM consumption (Total, Used, Free).
  * Flash Memory state (Storage).
  * Silicon core temperature (`Core Temp`).
  * Processor Frequency (`CPU Speed`).
  * Uninterrupted Activity Time (`Uptime`).
* **Modular Architecture ("Drawers"):** The code separates the master menu from subprograms, allowing the addition of new sensors or projects without breaking others' code.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:git-compare.svg?color=white"><img src="https://api.iconify.design/lucide:git-compare.svg?color=black" width="26" align="center"></picture> The Problem vs The Solution

Developing and testing multiple hardware projects on a single microcontroller is usually a mess. 

| Without Blasco OS | With Blasco OS |
| :--- | :--- |
| Mandatory USB connection to test | **100% Wireless** via Telnet |
| Flash firmware on every change | **Hot-swapping** between modules |
| Basic monitoring via Serial | **Advanced telemetry** (RAM, Flash, Temp, CPU) |
| Updates via cable | **OTA Support** (Over-The-Air) |
| Coupled projects that break code | **Modular Architecture** (Independent "Drawers") |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:blocks.svg?color=white"><img src="https://api.iconify.design/lucide:blocks.svg?color=black" width="26" align="center"></picture> Active Modules

Currently, the operating system has three main integrated projects:

### 1. NFC Cloning Station Pro (V14)
An advanced RFID auditing and cloning module using **MFRC522** hardware.
* **Deep Reading:** Extracts all information from the card and saves it into the ESP32 RAM.
* **Physical Cloning:** Allows injecting data into Sector 0 of rewritable cards.
* **Multi-Source:** Clone from RAM, inject a Hardcoded master key, or type a hex code manually.

### 2. Ultrasonic Radar (V3)
Physical telemetry module using the **HC-SR04** distance sensor.
* **Asynchronous Execution:** Non-blocking flow; handles network requests and web server concurrently.
* **Loop Reading:** Configurable cyclical refresh with fault-tolerant thermal logic.

### 3. Ambient Temperature and Humidity Monitor (V1)
Integrated local climate data acquisition module.
* **Precision Sampling:** Continuous telemetry of relative humidity and ambient temperature.
* **Data Management:** Automatic synchronization of historical readings.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:hard-drive.svg?color=white"><img src="https://api.iconify.design/lucide:hard-drive.svg?color=black" width="26" align="center"></picture> Required Hardware and Installation

* **Base Board:** ESP32 (S3 N16R8 or similar).
* **NFC Module:** MFRC522 RFID Reader (Connected via SPI bus: SDA->D5, RST->D21).
* **Distance Module:** HC-SR04 Sensor (TRIG->D12, ECHO->D14).

### Deployment:
1. Configure your Wi-Fi credentials (`ssid` and `password`) in the source code.
2. Flash the code via USB for the first time using Arduino IDE.
3. Open the Serial console at `115200 baud` to discover the assigned local IP.
4. Open your web browser and navigate to that local IP.
5. Enjoy the environment!

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:settings.svg?color=white"><img src="https://api.iconify.design/lucide:settings.svg?color=black" width="26" align="center"></picture> Compilation Configuration (Arduino IDE)

> **System Access:** The default username and password are set to **`admin` / `blasco`**. This combination can be modified in the main .ino file.

| Configuration Parameter | Exact Required Value |
| :--- | :--- |
| **Board** | `ESP32S3 Dev Module` |
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

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:monitor.svg?color=white"><img src="https://api.iconify.design/lucide:monitor.svg?color=black" width="26" align="center"></picture> Interface and Telemetry

Below is the execution environment of the Operating System. Click on the dropdown menus to expand and view the full interface screenshots.

<details>
<summary><b>Control Panel (Telemetry and live execution)</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/db980fe4-600b-43e1-b8bb-b9c366eb26ee" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/eef43556-0c27-49e9-8d07-eda9577c5c75" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/4f032878-b0b0-406b-81f2-786c12560149" />

<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55b18cca-1077-499a-81ed-d8acf6d66cc4" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/6c5cba54-ecff-498f-835e-130887f2bf8b" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/bbca1743-3a4e-4fa7-9f1b-6b4601bce200" />
</details>

<details>
<summary><b>Database Management</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/15c40684-d907-4bca-9387-72960a42e2ad" />

<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/59ef1539-4c79-4654-aa02-3c0c15b51490" />
</details>

<details>
<summary><b>Authentication Login</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />

<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />
</details>

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:user.svg?color=white"><img src="https://api.iconify.design/lucide:user.svg?color=black" width="26" align="center"></picture> Contact Me

Developed with passion by **Ruben Blasco Armengod**.

* **GitHub:** [rubenblascoa](https://github.com/rubenblascoa)
* **Email:** rubenblascoarmengod@gmail.com
