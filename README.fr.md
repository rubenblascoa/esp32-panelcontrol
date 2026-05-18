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

[Español](README.es.md) · [English](README.md) · **Français**

---

**L'environnement d'exécution multi-programme ultime pour l'ingénierie bas niveau.**

[![GitHub stars](https://img.shields.io/github/stars/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/rubenblascoa/esp32-panelcontrol?style=flat-square&color=black)](https://github.com/rubenblascoa/esp32-panelcontrol/network/members)
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)

**ESP32 Blasco** est une plateforme d'ingénierie bas niveau et un environnement d'exécution multi-programme conçu exclusivement pour le microcontrôleur ESP32. Il agit comme un léger "Système d'Exploitation" accessible à distance via **Telnet (Wi-Fi)**. Il permet d'encapsuler et d'exécuter plusieurs projets matériels sur la même carte, en basculant entre eux via une interface de terminal en texte pur (style rétro/hacker), complètement **sans câble** et **sans reflasher** le firmware.

[Explorer le Code](https://github.com/rubenblascoa/esp32-panelcontrol/tree/main/Code) · [Signaler un Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Demander une Amélioration](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Parrainé par
<a href="https://www.pcbway.es/"><img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="280"></a>

> **Matériel propulsé par PCBWay :** Pour le développement et le déploiement d'ESP32 Blasco OS, il est indispensable de disposer d'un matériel robuste. J'ai fait confiance à PCBWay pour la fabrication de mis cartes en raison de l'excellente qualité de leurs PCB et de la précision de l'assemblage (PCBA). Dans un environnement où les connexions SPI/I2C à haute fréquence et la télémétrie en temps réel ne peuvent pas échouer, la fiabilité de leurs composants a été essentielle pour assurer la stabilité du système. Leur plateforme est intuitive, le service client est rapide et les délais d'expédition sont imbattables. Entièrement recommandé pour tout ingénieur ou maker souhaitant passer à un matériel de qualité professionnelle.

---
</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:folder-tree.svg?color=white"><img src="https://api.iconify.design/lucide:folder-tree.svg?color=black" width="26" align="center"></picture> Architecture et Structure Modulaire

Le firmware implémente une séparation stricte des responsabilités matérielles et logicielles via un découplage asynchrone multicœur. Voici le but et la logique d'exécution interne de chaque fichier du système :

### 1. Le Noyau d'Entrée et d'Orchestration
* **`main.ino`**
  * **But :** Point de démarrage physique de l'ESP32-S3 et fichier principal lu par l'Arduino IDE pour compiler tout le répertoire.
  * **Logique détaillée :** Contient un `loop()` vide et une méthode `setup()` chargée de réveiller les composants en cascade stricte (port série, bus I2C, écran LCD, broches GPIO, modem Wi-Fi et synchronisation NTP). À la fin, il crée les deux tâches FreeRTOS (`taskCore0` et `taskCore1`) en leur transmettant le contrôle et s'auto-détruit via `vTaskDelete(NULL)` pour déléguer 100% du CPU aux threads asynchrones.

### 2. Configuration et Mémoire Partagée
* **`config.h`**
  * **But :** Définit les bibliothèques système, les mappages de broches physiques de la carte et déclare de manière abstraite les variables partagées par les deux cœurs.
  * **Logique détaillée :** Héberge les directives `#include` et les `#define` de la LED, du lecteur RFID et du capteur à ultrasons. Il contient les signatures des variables globales précédées du mot-clé `extern`, indiquant aux autres fichiers `.cpp` que la variable existe dans la RAM commune, évitant ainsi les duplications et les erreurs de liaison (*linker errors*).
* **`config.cpp`**
  * **But :** Fonctionne comme l'espace physique réel dans la mémoire RAM où les variables du dictionnaire `config.h` sont créées et initialisées.
  * **Logique détaillée :** S'exécute une seule fois au démarrage pour réserver l'espace exact requis par le firmware. Stocke les identifiants Wi-Fi réels, l'allocation mémoire pour les objets du serveur web, les matrices fixes des clés MIFARE et les tampons de synchronisation des commandes.

### 3. Le Canal de Texte Dupliqué
* **`terminal.h`**
  * **But :** Définit la structure de la classe personnalisée `TerminalHibrida`.
  * **Logique détaillée :** Déclare la classe en héritant de la bibliothèque native `Print` d'Arduino, ce qui permet d'exposer les méthodes traditionnelles `.print()` et `.println()`, ainsi que les contrôleurs de blocs dynamiques (`iniciarBloque()`, `enviarBloque()`).
* **`terminal.cpp`**
  * **But :** Contrôle l'algorithme asynchrone qui duplique les messages texte en temps réel.
  * **Logique détaillée :** Lors de l'appel à `Terminal.println()`, il intercepte les caractères en les envoyant d'abord via le socket TCP ouvert vers Telnet (Putty). En parallèle, si un navigateur web est à l'écoute, il accumule les caractères dans une chaîne dynamique (`bufferWeb`) et les distribue au WebSocket lorsqu'il rencontre un saut de ligne `\n` (sauf en Mode Bloc). Il dispose d'une règle de sécurité qui vide le tampon s'il dépasse 200 caractères pour éviter les fuites de mémoire.

### 4. Interface Graphique et Frontend
* **`web_pages.h`**
  * **But :** Déclare trois constantes de texte globales contenant les fichiers structurés du frontend.
  * **Logique détaillée :** Expose les références à `index_html`, `db_html` et `login_html` accompagnées du modificateur de stockage `PROGMEM`.
* **`web_pages.cpp`**
  * **But :** Stocke de manière exacte et massive tout le code HTML, CSS et JavaScript des trois panneaux de contrôle web.
  * **Logique détaillée :** Oblige l'ESP32-S3 via la directive `PROGMEM` à sauvegarder ces pages dans les transistors inalterables de la mémoire Flash (ROM). Si elles étaient hébergées dans la RAM conventionnelle, elles consommeraient plus de 80% du Heap dynamique, provoquant des redémarrages par manque de mémoire lors de la connexion des utilisateurs.

### 5. Routage et Sécurité Réseau
* **`web_server.h`**
  * **But :** Définit les fonctions du serveur HTTP asynchrone et le callback du WebSocket qui traitera les trames web.
* **`web_server.cpp`**
  * **But :** Fonctionne comme le répartiteur de trafic réseau assigné au Core 0.
  * **Logique détaillée :**
    * *Sécurité :* Exécute la fonction `estaLogueado()`, inspectant les en-têtes HTTP à la recherche du cookie `ZENITH_SESSION` lié au jeton aléatoire de la RAM.
    * *Routes HTTP :* Gère les requêtes vers `/login` (en créant le cookie avec les attributs `HttpOnly` et `SameSite=Strict`), `/logout` (détruit la session), `/datos.csv` (effectue un streaming direct du fichier historique depuis LittleFS vers le navigateur) et `/delete-db` (supprime le fichier physique de logs).
    * *WebSocket (onWsEvent) :* Capture la trame réseau brute lorsqu'un utilisateur écrit dans la console du navigateur ; s'il reçoit `"reboot"`, il redémarre la carte, et s'il reçoit des commandes de contrôle, il les copie dans `entradaWeb` et lève le drapeau `hayEntradaWeb` pour avertir le Core 1.

### 6. Contrôleurs de Capteurs (Hardware)
* **`nfc.h` / `nfc.cpp`**
  * **But :** Gère les cycles de lecture et de clonage par proximité via le bus physique SPI.
  * **Logique détaillée :** Lorsque le Core 1 lui donne le feu vert, il interroge en continu le matériel MFRC522. En mode lecture, il effectue un défi cryptographique sur le tag physique à l'aide de mots de passe MIFARE ; s'il répond correctement, il bascule le Bloc 0 dans la variable `bloqueEscaneado`. En mode écriture, il injecte cette matrice de données dans une carte réinscriptible vierge.
* **`ultrasonidos.h` / `ultrasonidos.cpp`**
  * **But :** Mesure l'espace euclidien à l'aide de rebonds acoustiques à haute fréquence.
  * **Logique détaillée :** Place la broche `TRIG_PIN` à un potentiel de zéro, émet une impulsion ultrasonique en maintenant la broche à l'état haut pendant exactement 10 microsecondes, puis la coupe. Immédiatement après, il exécute un appel `pulseIn` de haute précision sur la broche `ECHO_PIN` avec un temps de grâce de 30 ms. Si l'écho revient, il calcule la distance en centimètres en divisant le temps par deux et en appliquant la vitesse du son.

### 7. Utilitaires et Historique sur Disque
* **`utils.h` / `utils.cpp`**
  * **But :** Offre un support logique de backend au firmware pour les statistiques, le temps et la gestion du bus I2C.
  * **Logique détaillée :**
    * `calcularUsoCPU()` : Estime mathématiquement la charge de stress heuristique des cœurs selon les tâches actives.
    * `guardarEnHistorial()` : Ouvre le fichier `/datos.csv` dans LittleFS et ajoute une ligne structurée avec la date, la température interne de la puce, les charges dynamiques du CPU, l'occupation de la RAM/Flash et le niveau de signal Wi-Fi.
    * `actualizarLCD()` : Contrôle l'écran physique via l'appel `xSemaphoreTake(i2cMutex)` pour verrouiller le bus I2C avant l'écriture, évitant ainsi les collisions de données, affichant des barres de progression visuelles (`|====  |`) et alternant automatiquement toutes les 3 secondes entre 4 pages de télémétrie.

### 8. Couche de Texte (Interface CLI)
* **`menus.h` / `menus.cpp`**
  * **But :** Contiene le design visuel en texto plano y el árbol de navegación jerárquico del sistema operativo.
  * **Logique détaillée :** Gère ce que l'utilisateur visualise lorsqu'il se connecte par terminal. Appelle `Terminal.iniciarBloque()` pour retenir les caractères, imprime les cadres décoratifs de la console et concatène le calcul en temps réel de la PSRAM, de la vitesse du processeur et de l'Uptime avant d'envoyer le bloc unifié au réseau. Héberge les fonctions qui modifient la variable `programaActivo` pour changer de menu.

### 9. Le Planificateur Multicœur (FreeRTOS)
* **`tareas.h` / `tareas.cpp`**
  * **But :** Héberge les deux boucles infinies distribuées par le matériel qui remplacent le `loop` conventionnel d'Arduino.
  * **Logique détaillée :**
    * `taskCore0` (Assigné au Noyau 0) : Monte le système de fichiers LittleFS (en le formatant si une corruption est détectée). Traite les connexions réseau entrantes, gère les mises à jour OTA, exécute l'horloge synchrone NTP, déclenche le cronjob de la base de données toutes les 2 heures et contrôle le clignotement stroboscopique de la LED d'état.
    * `taskCore1` (Assigné au Noyau 1) : Surveille en parallèle si une commande est entrée via Putty ou les WebSockets. Selon l'état de `programaActivo`, il exécute de manière asynchrone l'écoute NFC sur le bus SPI ou effectue des tirs d'ultrasons à une fréquence exacte de 1 Hz (toutes les 1000 ms) sans jamais interférer avec les processus réseau de l'autre cœur.

---

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
