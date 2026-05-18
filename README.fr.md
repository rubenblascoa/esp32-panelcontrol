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

[Castellano](README.md) · [English](README.en.md) · **Français**

---

**L'environnement d'exécution multi-programme ultime pour l'ingénierie bas niveau.**

[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/network/members)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)

**ESP32 Blasco** est une plateforme d'ingénierie bas niveau et un environnement d'exécution multi-programme conçu exclusivement pour le microcontrôleur ESP32. Il agit comme un léger "Système d'Exploitation" accessible à distance via **Telnet (Wi-Fi)**. Il permet d'encapsuler et d'exécuter plusieurs projets matériels sur la même carte, en basculant entre eux via une interface de terminal en texte pur (style rétro/hacker), complètement **sans câble** et **sans reflasher** le firmware.

[Explorer le Code](https://github.com/rubenblascoa/esp32-panelcontrol) · [Signaler un Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Demander une Amélioration](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Parrainé par
<a href="https://www.pcbway.es/"><img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="280"></a>

> **Matériel propulsé par PCBWay :** Pour le développement et le déploiement d'ESP32 Blasco OS, il est indispensable de disposer d'un matériel robuste. J'ai fait confiance à PCBWay pour la fabrication de mis cartes en raison de l'excellente qualité de leurs PCB et de la précision de l'assemblage (PCBA). Dans un environnement où les connexions SPI/I2C à haute fréquence et la télémétrie en temps réel ne peuvent pas échouer, la fiabilité de leurs composants a été essentielle pour assurer la stabilité du système. Leur plateforme est intuitive, le service client est rapide et les délais d'expédition sont imbattables. Entièrement recommandé pour tout ingénieur ou maker souhaitant passer à un matériel de qualité professionnelle.

---
</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:list.svg?color=white"><img src="https://api.iconify.design/lucide:list.svg?color=black" width="26" align="center"></picture> Caractéristiques Principales

* **Contrôle 100% Sans Fil :** Accès complet à l'interface utilisateur via n'importe quel client Telnet (Port 23) sur le réseau Wi-Fi local.
* **Mises à jour OTA (Over-The-Air) :** Support intégré pour injecter du nouveau code à distance sans connexion USB.
* **Télémétrie Avancée en Temps Réel :** Surveillance du système comprenant :
  * Consommation de RAM (Totale, Utilisée, Libre).
  * État de la mémoire Flash (Stockage).
  * Température des cœurs de silicium (`Temp Core`).
  * Fréquence du processeur (`Vit. CPU`).
  * Temps d'activité ininterrompu (`Uptime`).
* **Architecture Modulaire ("Tiroirs") :** Le code sépare le menu maître des sous-programmes, permettant d'ajouter de nouveaux capteurs ou projets sans casser le code des autres.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:git-compare.svg?color=white"><img src="https://api.iconify.design/lucide:git-compare.svg?color=black" width="26" align="center"></picture> Le Problème vs La Solution

Développer et tester plusieurs projets matériels sur un seul microcontrolleur est généralement un chaos.

| Sans Blasco OS | Avec Blasco OS |
| :--- | :--- |
| Connexion USB obligatoire pour tester | **100% Sans Fil** via Telnet |
| Flashear le firmware à chaque changement | **Échange à chaud** entre les modules |
| Surveillance basique via la liaison Série | **Télémétrie avancée** (RAM, Flash, Temp, CPU) |
| Mises à jour par câble | **Support OTA** (Over-The-Air) |
| Projets couplés qui cassent le code | **Architecture Modulaire** ("Tiroirs" indépendants) |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:blocks.svg?color=white"><img src="https://api.iconify.design/lucide:blocks.svg?color=black" width="26" align="center"></picture> Modules Actifs

Actuellement, le système d'exploitation comprend trois projets principaux intégrés :

### 1. Station de Clonage NFC Pro (V14)
Un module avancé d'audit et de clonage RFID utilisant le matériel **MFRC522**.
* **Lecture Profonde :** Extrait toutes les informations de la carte et les sauvegarde dans la RAM de l'ESP32.
* **Clonación Física :** Permet d'injecter des données dans le Secteur 0 de cartes réinscriptibles.
* **Multi-Source :** Cloner depuis la RAM, injecter une clé maîtresse (Hardcoded) ou un code hexadécimal.

### 2. Radar à Ultrasons (V3)
Module de télémétrie physique utilisant le capteur de distance **HC-SR04**.
* **Exécution Asynchrone :** Flux non bloquant ; gère les requêtes réseau et le serveur web.
* **Lecture en Boucle :** Rafraîchissement cyclique avec une logique de tolérance aux pannes thermiques.

### 3. Moniteur de Température et d'Humidité Ambiante (V1)
Module intégré d'acquisition de données climatiques locales.
* **Échantillonnage de Précision :** Télémétrie continue de l'humidité relative et de la température.
* **Gestion des Données :** Synchronisation automatique des lectures historiques.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:hard-drive.svg?color=white"><img src="https://api.iconify.design/lucide:hard-drive.svg?color=black" width="26" align="center"></picture> Matériel Requis et Installation

* **Carte de Base :** ESP32 (S3 N16R8 ou similaire).
* **Module NFC :** Lecteur RFID MFRC522 (Connecté par bus SPI : SDA->D5, RST->D21).
* **Module de Distance :** Capteur HC-SR04 (TRIG->D12, ECHO->D14).

### Mise en Route :
1. Configurez vos identifiants Wi-Fi (`ssid` et `password`) dans le code source.
2. Flashez le code via USB pour la première fois à l'aide d'Arduino IDE.
3. Ouvrez la console Série à `115200 bauds` pour découvrir l'adresse IP locale attribuée.
4. Ouvrez votre navigateur web et accédez à cette IP locale.
5. Profitez de l'environnement !

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:settings.svg?color=white"><img src="https://api.iconify.design/lucide:settings.svg?color=black" width="26" align="center"></picture> Configuration de Compilation (Arduino IDE)

> **Accès au système :** L'identifiant et le mot de passe définis par défaut sont **`admin` / `blasco`**. Cette combinaison est modifiable dans le fichier .ino principal.

| Paramètre de Configuration | Valeur Exacte Requise |
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

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:monitor.svg?color=white"><img src="https://api.iconify.design/lucide:monitor.svg?color=black" width="26" align="center"></picture> Interface et Télémétrie

Voici l'environnement d'exécution du système d'exploitation. Cliquez sur les menus déroulants pour voir les captures d'écran de l'interface complète.

<details>
<summary><b>Panneau de Contrôle (Télémétrie et exécution en direct)</b></summary>
<br>

<p><i>Version Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/db980fe4-600b-43e1-b8bb-b9c366eb26ee" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/eef43556-0c27-49e9-8d07-eda9577c5c75" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/4f032878-b0b0-406b-81f2-786c12560149" />

<p><i>Version Claire</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55b18cca-1077-499a-81ed-d8acf6d66cc4" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/6c5cba54-ecff-498f-835e-130887f2bf8b" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/bbca1743-3a4e-4fa7-9f1b-6b4601bce200" />
</details>

<details>
<summary><b>Gestion de la Base de Données</b></summary>
<br>

<p><i>Version Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/15c40684-d907-4bca-9387-72960a42e2ad" />

<p><i>Version Claire</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/59ef1539-4c79-4654-aa02-3c0c15b51490" />
</details>

<details>
<summary><b>Authentification Connexion</b></summary>
<br>

<p><i>Version Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />

<p><i>Version Claire</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />
</details>

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:user.svg?color=white"><img src="https://api.iconify.design/lucide:user.svg?color=black" width="26" align="center"></picture> Contactez-moi

Développé avec passion par **Ruben Blasco Armengod**.

* **GitHub:** [rubenblascoa](https://github.com/rubenblascoa)
* **Email:** rubenblascoarmengod@gmail.com
