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

[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)

**ESP32 Blasco** is a low-level engineering platform and a multi-program execution environment designed exclusively for the ESP32-S3 microcontroller. It acts as a lightweight "Operating System" remotely accessible via **Telnet (Wi-Fi)** and WebSockets. It allows encapsulating and executing multiple hardware projects on the same board, switching between them through a pure text terminal interface (retro/hacker style), completely **wire-free** and featuring **dynamic runtime hot-configuration**.

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

>In an architecture that delegates 100% of the CPU to FreeRTOS threads, the physical design of the motherboard is critical. The integration of PCBWay into this project responds to very specific technical needs of the low-level environment:

>* **Signal Integrity (SPI and I2C):** The NFC cloning module (MFRC522) operates at frequencies where noise is fatal. PCBWay's precise track routing guarantees no electromagnetic interference, preventing data collisions on the bus and ensuring lossless scans.
  
>* **Dissipation for Real Telemetry:** This system extracts stress and temperature data directly from the CPU cores. The quality of the board's copper and fiberglass allows for proper thermal dissipation, vital for keeping the processor stable under continuous load.
  
>* **Assembly Precision (PCBA):** The integration of the ESP32-S3 (N16R8) chip and the soldering of SMD components require exact tolerances so that the final hardware supports the 24/7 execution environment without electrical failures.

>The manufacturing experience has been straightforward and seamless, with smooth management of the Gerber and BOM files thanks to the support of their technical team (with a special mention to Liam for facilitating the sponsorship coordination). If you are looking to manufacture your own schematics with industrial quality, the results meet the standards of hardware engineering.

>**[Explore PCBWay's manufacturing services here](https://www.pcbway.com/)**

---

</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:folder-tree.svg?color=white"><img src="https://api.iconify.design/lucide:folder-tree.svg?color=black" width="26" align="center"></picture> Architecture and Modular Structure

The firmware implements a strict separation of concerns between hardware and software through an asynchronous multi-core decoupling. The purpose and internal execution logic of each system file are detailed below:

### 1. Entry Core and Orchestration
* **`main.ino`**
  * **Purpose:** It is the physical entry point of the ESP32-S3 and the main orchestration file.
  * **Detailed logic:** If the system lacks valid network credentials, it automatically provisions a local **Wi-Fi Access Point (AP)** named `Esp32BlascoOS_Setup` to spin up a Captive Portal. Once connected, it sets up the physical I2C bus, loads NVS settings, hooks up the status LED, initializes the FreeRTOS processing cores (`taskCore0` and `taskCore1` with 16KB of stack allocation each), and purges itself calling `vTaskDelete(NULL)` to grant complete CPU control to the scheduler.

### 2. Configuration and System State
* **`config.h` / `config.cpp`**
  * **Purpose:** Abstract layout definition of global memory structures, thresholds, and runtime pin maps.
  * **Detailed logic:** Declares macros for logging streams (`LOG_I`/`LOG_E`) and binds global state variables via the `extern` keyword to prevent duplication across compiler units. Complex peripherals like the `MFRC522` reader are handled strictly as dynamic pointers (`MFRC522*`), enabling complete runtime remapping of GPIO connections from the Captive Portal or REST endpoints without modifying a single line of source code.

### 3. Multi-Core Scheduler (FreeRTOS)
* **`tareas.h` / `tareas.cpp`**
  * **Purpose:** Controls the hardware-distributed infinite processing loops replacing the default Arduino execution flow.
  * **Detailed logic:**
    * **`taskCore0`:** Manages network interfaces and local storage. It handles raw async WebSocket inputs, executes background OTA flashes, throttles NTP syncing, and triggers a periodic file cronjob every 2 hours to back up historical system state logs.
    * **`taskCore1`:** Dedicated entirely to physical hardware operations. It evaluates input console command queues (`cmdQueue`), refreshes the 5-page diagnostic LCD using safe thread guards, and steps through the running states of the active background sensors.

### 4. Graphical Interface and Frontend
* **`web_pages.h` / `web_pages.cpp`**
  * **Purpose:** Stores the unalterable HTML, CSS, and modern JavaScript structures for the remote administration dashboards.
  * **Detailed logic:** Uses the `PROGMEM` keyword to lock the massive web layouts (Dashboard, DB Viewer, Config Panel, Captive Portal, and Login screens) into the Flash memory space. This prevents the assets from polluting the dynamic RAM Heap, completely removing memory-exhaustion reboots during simultaneous client attachments.

### 5. Routing and Network Security
* **`web_server.h` / `web_server.cpp`**
  * **Purpose:** Handles async network routing and incoming server sockets on Core 0.
  * **Detailed logic:** Provisions a complete REST API engine exposing 19 operational endpoints. Validates request authenticity by verifying the `ZENITH_SESSION` cookie against the dynamic token in RAM. WebSocket inputs are processed using safe memory allocations (`malloc/memcpy`), shielding the system against frame-based heap corruption, and endpoints like `/api/config/pins` handle live JSON data streams to map internal hardware states on the fly.

### 6. Sensor Controllers (Hardware)
* **`nfc.h` / `nfc.cpp`**
  * **Purpose:** Drives proximity reading, block auditing, and cloning cycles over the physical SPI bus.
  * **Detailed logic:** Controls the MFRC522 chip to communicate with MIFARE Classic 1K cards. Validates authorization keys (Key A) before extracting sector blocks. In write mode, it targets rewritable magic cards (CUID/FUID), rewriting Sector 0 and Block 0 to clone UID fingerprints.
* **`ultrasonidos.h` / `ultrasonidos.cpp`**
  * **Purpose:** Captures spatial metric parameters via acoustic reflection using the HC-SR04 transductor.
  * **Detailed logic:** Uses a 100% non-blocking hardware Finite State Machine (FSM). Fires a brief 10µs trigger pulse and binds a low-level interrupt routine (`IRAM_ATTR ecoISR`) to the ECHO pin to calculate sound flight time. This eliminates blocking calls like `pulseIn()`, letting Core 1 multitask freely during acoustic propagation.
* **`dht.h` / `dht.cpp`**
  * **Purpose:** Extracts localized relative humidity and temperature parameters via the DHT11 sensor.
  * **Detailed logic:** Features a custom, dependency-free native 1-Wire protocol implementation. Temporarily isolates the processor from timing anomalies by turning off system interrupts (`noInterrupts()`) during the microsecond-critical bit-banging capture window. Implements an automated 1-second auto-retry mechanic to guarantee data integrity against ambient electromagnetic interference.

### 7. Utilities and Disk Logs
* **`utils.h` / `utils.cpp`**
  * **Purpose:** Backend helper framework for mathematical smoothing, persistent storage, and physical bus routing.
  * **Detailed logic:** Computes actual core stress profiles using an Exponential Moving Average algorithm (EMA 30/70) to screen out momentary spikes. Manages 3 separate persistent NVS spaces (`zenithmc`, `hwconfig`, `webcred`) to protect underlying peripheral mappings from accidental Wi-Fi wipes. Uses an explicit Mutex semaphore (`i2cMutex`) to coordinate shared I2C traffic on the LCD display while rolling across 5 distinct telemetry screens.
* **`sd_card.h` / `sd_card.cpp`** *(Optional)*
  * **Purpose:** Interconnects a physical MicroSD card expansion slot over a dedicated SPI bus.
  * **Detailed logic:** Safeguarded under the preprocessor compilation guard `#ifdef SD_CS_PIN`. When present, it automatically intercepts database logging tasks from internal storage and streams the output directly onto physical disk media.

### 8. Text Layer (CLI Interface)
* **`menus.h` / `menus.cpp`**
  * **Purpose:** Visual framework and action parsing logic for the text-based console interface.
  * **Detailed logic:** Employs the `TerminalHibrida` buffering layout to package complex telemetry arrays (Uptime, Core stress, Heap space) before blasting them onto the network fabric, cutting down TCP packet pollution. Changes the running index `programaActivo` to redirect execution states.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:list.svg?color=white"><img src="https://api.iconify.design/lucide:list.svg?color=black" width="26" align="center"></picture> Main Features

* **100% Wireless Control:** Complete access to the system terminal and diagnostic output through standard Telnet (Port 23) and raw WebSockets.
* **Dynamic Hardware Pin Mapping (Plug & Play):** On its first deployment, the OS launches an automated configuration hotspot (`Esp32BlascoOS_Setup`). Users can safely configure their localized router parameters, assign custom GPIO pins, and set web passwords from any smartphone browser without rebuilding the firmware.
* **EMA-Smoothed Real-Time Telemetry:** Advanced diagnostics detailing:
  * Dynamic RAM/PSRAM boundaries (Total, Allocated, Free).
  * Local File Storage map occupancy (LittleFS / SD Card).
  * True multi-core mathematical processor loads and silicon temperature profiles.
* **NTP Time-Backed Database:** Fully automated background logging routines. Once synced to atomic network clocks, the firmware appends a structured 10-column telemetry line into the system CSV spreadsheet every 2 hours continuously.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:git-compare.svg?color=white"><img src="https://api.iconify.design/lucide:git-compare.svg?color=black" width="26" align="center"></picture> The Problem vs The Solution

Developing and testing multiple hardware projects on a single microcontroller is usually a mess. 

| Without Blasco OS | With Blasco OS |
| :--- | :--- |
| Mandatory USB connection to test | **100% Wireless** via Telnet |
| Flash firmware on every physical pin change | **Runtime Configuration** via Captive Portal |
| Basic monitoring via Serial | **Advanced telemetry** (RAM, Flash, Temp, CPU) |
| Updates via cable | **OTA Support** (Over-The-Air) |
| Coupled projects that break code | **Modular Architecture** (Independent "Drawers") |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:blocks.svg?color=white"><img src="https://api.iconify.design/lucide:blocks.svg?color=black" width="26" align="center"></picture> Active Modules

Currently, the operating system has three main integrated projects:

### 1. NFC Cloning Station Pro (V14)
An advanced RFID auditing and cloning module using **MFRC522** hardware.
* **Deep Reading:** Extracts all information from the card and saves it into the ESP32 RAM.
* **Physical Cloning:** Allows injecting data into Sector 0 of rewritable magic cards (CUID/FUID).

### 2. Ultrasonic Radar (V3)
Physical telemetry module using the **HC-SR04** distance sensor.
* **Asynchronous Execution (ISR):** 100% Non-blocking flow driven by hardware interrupts; the ESP32 suffers zero micro-freezes while the sound bounces.
* **Loop Reading:** Configurable cyclical refresh with thermal fault tolerance ("Out of range").

### 3. Ambient Temperature and Humidity Monitor (V1)
Integrated local climate data acquisition module via **DHT11**.
* **Native 1-Wire Protocol:** Low-level reading (Bit-Banging) that relies on no third-party libraries, optimized with hardware delays and auto-recovery routines against electromagnetic noise.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:hard-drive.svg?color=white"><img src="https://api.iconify.design/lucide:hard-drive.svg?color=black" width="26" align="center"></picture> Required Hardware and Installation

* **Base Board:** ESP32 (S3 N16R8 or similar).
* **NFC Module:** MFRC522 RFID Reader (SPI Bus).
* **Distance Module:** HC-SR04 Sensor.
* **Climate Sensor:** DHT11 Module (with 4.7kΩ pull-up resistor).
* **Storage Module (Optional):** MicroSD Reader (SPI Bus).

> *Note: The system boots with default pins, but all hardware connections (RST, SS, TRIG, ECHO, DHT) can be completely reassigned from the Web Captive Portal without touching the code.*

### Initial Deployment (Via Captive Portal):
1. Flash the compiled source code via USB for the first time using your preferred IDE.
2. The ESP32 will format its internal filesystem (LittleFS) and, upon not detecting valid home credentials, will open an Access Point (AP).
3. Search on your phone or PC for the open Wi-Fi network: **`Esp32BlascoOS_Setup`** and connect to it.
4. A web wizard will open automatically. Follow the steps to enter your local router password, configure your GPIO pins, and define your web admin credentials.
5. Upon clicking Save, the ESP32 will reboot, shut down AP mode, and connect to your home router transparently.
6. Open the Serial console at `115200 baud` or check the LCD screen to discover its newly assigned local IP.
7. Open that IP in your web browser. Enjoy the environment!

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:settings.svg?color=white"><img src="https://api.iconify.design/lucide:settings.svg?color=black" width="26" align="center"></picture> Compilation Configuration (Arduino IDE)

To ensure the project compiles correctly and the Web UI has enough space for the LittleFS database, it is **mandatory** to apply the following settings in the Arduino IDE **Tools** menu. 

*This configuration is optimized for **ESP32-S3 (N16R8)** boards featuring 16MB of Flash and 8MB of PSRAM.*

| Configuration Parameter | Exact Required Value |
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
<summary><b>Authentication Login </b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />

<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />
</details>

<details>
<summary><b>Internal Configuration</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/0ef0f0d3-2d07-4788-a5ba-e5ab27a03350" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/fc443c62-634d-437d-b8ac-b34004d1911f" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/25386ba6-4501-4d4b-b53e-b7bc85d0a78c" />


<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/92536347-e43d-4ae2-8d77-b39f28956280" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/f0722d4c-c2fb-45b4-9eb4-78b2052cc72c" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/a1f0496e-58c5-4706-96f9-b061129aa223" />

</details>

<details>
<summary><b>Initial Configuration</b></summary>
<br>

<p><i>Dark Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/7baf8092-9362-4781-b994-7e944e0e2b77" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/c21cca77-21ab-4a27-968c-e9bdb81bb691" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/780ecdca-629f-4bed-ac2a-09ff42127f92" />



<p><i>Light Theme</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/2d2c4c3b-e5e7-4fae-a8e1-2485f2c790c6" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/8361138e-f22b-478e-94a1-d0e94df2959a" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55d62211-ca00-482a-b7dc-7fac87c9ab99" />

</details>

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:mail.svg?color=white"><img src="https://api.iconify.design/lucide:mail.svg?color=black" width="26" align="center"></picture> Contact & Sponsorship

Developed with passion by **Ruben Blasco Armengod**.

If you are interested in sponsoring the continuous expansion of this project, collaborating on custom hardware motherboard layouts, or embedding new sensors into the multi-tasking core, please reach out through any of the following technical channels:

* **GitHub:** [@rubenblascoa](https://github.com/rubenblascoa)
* **Instagram:** [@rubenblascoa](https://instagram.com/rubenblascoa)
* **Email:** rubenblascoarmengod@gmail.com
