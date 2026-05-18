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

[Castellano](README.md) · **English** · [Français](README.fr.md)

---

**The ultimate multi-program execution environment for low-level engineering.**

[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/network/members)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)

**ESP32 Blasco** is a low-level engineering platform and a multi-program execution environment designed exclusively for the ESP32 microcontroller. It acts as a lightweight "Operating System" remotely accessible via **Telnet (Wi-Fi)**. It allows encapsulating and executing multiple hardware projects on the same board, switching between them through a pure text terminal interface (retro/hacker style), completely **wire-free** and **without re-flashing** the firmware.

[Explore the Code](https://github.com/rubenblascoa/esp32-panelcontrol) · [Report a Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Request an Improvement](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Sponsored by
<a href="https://www.pcbway.com/"><img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="280"></a>

> **Hardware powered by PCBWay:** For the development and deployment of ESP32 Blasco OS, having robust hardware is indispensable. I have trusted PCBWay for manufacturing my boards due to the excellent quality of their PCBs and precision in assembly (PCBA). In an environment where high-frequency SPI/I2C connections and real-time telemetry cannot fail, the reliability of their components has been key to ensuring system stability. Their platform is intuitive, customer service is fast, and shipping times are unbeatable. Fully recommended for any engineer or maker looking to step up to professional-grade hardware.

---
</div>

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
