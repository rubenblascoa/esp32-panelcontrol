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
[![License: MIT](https://img.shields.io/badge/License-MIT-lightgrey.svg?style=flat-square)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-black?style=flat-square)](#)
![Views](https://komarev.com/ghpvc/?username=rubenblascoa&repo=esp32-panelcontrol&label=Views&icon=0&color=121011&style=flat-square)

**ESP32 Blasco** est une plateforme d'ingénierie bas niveau et un environnement d'exécution multi-programme conçu exclusivement pour le microcontrôleur ESP32-S3. Il agit comme un léger "Système d'Exploitation" accessible à distance via **Telnet (Wi-Fi)** et WebSockets. Il permet d'encapsuler et d'exécuter plusieurs projets matériels sur la même carte, en basculant entre eux via une interface de terminal en texte pur (style rétro/hacker), complètement **sans fil** et avec une **configuration dynamique à chaud**.

[Explorer le Code](https://github.com/rubenblascoa/esp32-panelcontrol/tree/main/Code) · [Signaler un Bug](https://github.com/rubenblascoa/esp32-panelcontrol/issues) · [Demander une Amélioration](https://github.com/rubenblascoa/esp32-panelcontrol/issues)

---

<div align="center">

### <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:zap.svg?color=white"><img src="https://api.iconify.design/lucide:zap.svg?color=black" width="22" align="center"></picture> Matériel Propulsé par PCBWay

<br>
<a href="https://www.pcbway.com/">
  <img src="https://www.image2url.com/r2/default/images/1779125298301-64f9e1cb-9abb-470b-8871-f272256b85a6.png" alt="PCBWay Logo" width="300">
</a>
<br><br>
</div>

> **Du prototypage à la fabrication professionnelle :** Le développement d'**ESP32 Blasco OS** exige un matériel capable de supporter l'exécution asynchrone de multiples programmes en temps réel. Pour le déploiement physique de ce projet, je fais confiance aux services de fabrication et d'assemblage (PCBA) de **[PCBWay](https://www.pcbway.com/)**.

> Dans une architecture qui délègue 100 % du CPU aux threads de FreeRTOS, la conception physique de la carte mère est critique. L'intégration de PCBWay dans ce projet répond à des besoins techniques très spécifiques de l'environnement de bas niveau :

>* **Intégrité du Signal (SPI et I2C) :** Le module de clonage NFC (MFRC522) fonctionne à des fréquences où le bruit est fatal. Le routage précis des pistes de PCBWay garantit l'absence d'interférences électromagnétiques, prévenant les collisions de données sur le bus et assurant des balayages sans perte.
  
>* **Dissipation pour la Télémétrie Réelle :** Ce système extrait les données de stress et de température directement des cœurs du CPU. La qualité du cuivre et de la fibre de verre de la carte permet une dissipation thermique adéquate, vitale pour maintenir le processeur stable sous une charge continue.
  
>* **Précision d'Assemblage (PCBA) :** L'intégration de la puce ESP32-S3 (N16R8) et la soudure des composants CMS exigent des tolérances exactes pour que le matériel final supporte l'environnement d'exécution 24/7 sans pannes électriques.

> L'expérience de fabrication a été directe et sans heurts, avec une gestion fluide des fichiers Gerber et de la nomenclature (BOM) grâce au soutien de leur équipe technique (avec une mention spéciale à Liam pour avoir facilité la coordination du sponsoring). Si vous cherchez à fabriquer vos propres schémas avec une qualité industrielle, les résultats répondent aux normes de l'ingénierie matérielle.

>**[Découvrez les services de fabrication de PCBWay ici](https://www.pcbway.com/)**

---

</div>

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:folder-tree.svg?color=white"><img src="https://api.iconify.design/lucide:folder-tree.svg?color=black" width="26" align="center"></picture> Architecture et Structure Modulaire

Le firmware implémente une séparation stricte des responsabilités matérielles et logicielles via un découplage asynchrone multicœur. Voici le but et la logique d'exécution interne de chaque fichier du système :

### 1. Noyau d'Entrée et d'Orchestration
* **`main.ino`**
  * **But :** C'est le point d'entrée physique de l'ESP32-S3 et le fichier d'orchestration principal.
  * **Logique détaillée :** Si le système manque d'identifiants réseau valides, il déploie automatiquement un **Point d'Accès (AP) Wi-Fi** local nommé `Esp32BlascoOS_Setup` pour lancer un Portail Captif (Captive Portal). Une fois connecté, il configure le bus I2C physique, charge les paramètres NVS, connecte la LED d'état, initialise les cœurs de traitement FreeRTOS (`taskCore0` et `taskCore1` avec 16 Ko de pile chacun) et s'auto-détruit en appelant `vTaskDelete(NULL)` pour confier le contrôle total du CPU au planificateur.

### 2. Configuration et État du Système
* **`config.h` / `config.cpp`**
  * **But :** Définition abstraite de la conception des structures de mémoire globale, des seuils et des cartes de broches au moment de l'exécution.
  * **Logique détaillée :** Déclare des macros pour les flux de journalisation (`LOG_I`/`LOG_E`) et lie les variables d'état global via le mot-clé `extern` pour éviter les duplications entre les unités de compilation. Les périphériques complexes tels que le lecteur `MFRC522` sont traités strictement comme des pointeurs dynamiques (`MFRC522*`), permettant de remapper entièrement les connexions GPIO au moment de l'exécution depuis le Portail Captif ou les endpoints REST sans modifier une seule ligne de code source.

### 3. Planificateur Multicœur (FreeRTOS)
* **`tareas.h` / `tareas.cpp`**
  * **But :** Contrôle les boucles de traitement infinies distribuées par le matériel qui remplacent le flux d'exécution par défaut d'Arduino.
  * **Logique détaillée :**
    * **`taskCore0` :** Gère les interfaces réseau et le stockage local. Traite les entrées asynchrones brutes du WebSocket, exécute les flashages OTA en arrière-plan, régule la synchronisation NTP et déclenche un cronjob de fichiers périodique toutes les 2 heures pour sauvegarder les registres de l'état historique du système.
    * **`taskCore1` :** Entièrement dédié aux opérations physiques du matériel. Évalue les files d'attente de commandes de la console d'entrée (`cmdQueue`), rafraîchit le LCD de diagnostic de 5 pages à l'aide de protections de threads sûres, et avance à travers les états d'exécution des capteurs actifs en arrière-plan.

### 4. Interface Graphique et Frontend
* **`web_pages.h` / `web_pages.cpp`**
  * **But :** Stocke les structures inaltérables HTML, CSS et JavaScript moderne pour les panneaux d'administration à distance.
  * **Logique détaillée :** Utilise le mot-clé `PROGMEM` pour verrouiller les conceptions web massives (Dashboard, Visionneur de BD, Panneau de Configuration, Portail Captif et écrans de Login) dans l'espace de la mémoire Flash. Cela empêche les ressources de polluer le Heap de la RAM dynamique, éliminant complètement les redémarrages dus à l'épuisement de la mémoire lors des connexions simultanées de clients.

### 5. Routage et Sécurité Réseau
* **`web_server.h` / `web_server.cpp`**
  * **But :** Gère le routage réseau asynchrone et les sockets du serveur entrant sur le Core 0.
  * **Logique détaillée :** Fournit un moteur API REST complet exposant 19 endpoints opérationnels. Valide l'authenticité des requêtes en vérifiant le cookie `ZENITH_SESSION` par rapport au jeton dynamique en RAM. Les entrées WebSocket sont traitées en utilisant des allocations de mémoire sécurisées (`malloc/memcpy`), protégeant le système contre la corruption du heap basée sur les trames, et des endpoints comme `/api/config/pins` gèrent les flux de données JSON en direct pour cartographier les états internes du matériel à la volée.

### 6. Contrôleurs de Capteurs (Matériel)
* **`nfc.h` / `nfc.cpp`**
  * **But :** Pilote la lecture de proximité, l'audit de blocs et les cycles de clonage à travers le bus physique SPI.
  * **Logique détaillée :** Contrôle la puce MFRC522 pour communiquer avec les cartes MIFARE Classic 1K. Valide les clés d'autorisation (Key A) avant d'extraire les blocs de secteurs. En mode écriture, il cible les cartes magiques réinscriptibles (CUID/FUID), réécrivant le Secteur 0 et le Bloc 0 pour cloner les empreintes UID.
* **`ultrasonidos.h` / `ultrasonidos.cpp`**
  * **But :** Capture les paramètres métriques spatiaux par réflexion acoustique à l'aide du transducteur HC-SR04.
  * **Logique détaillée :** Utilise une Machine à États Finis (FSM) matérielle 100 % non bloquante. Déclenche une brève impulsion d'activation de 10 µs et lie une routine d'interruption de bas niveau (`IRAM_ATTR ecoISR`) à la broche ECHO pour calculer le temps de vol du son. Cela élimine les appels bloquants comme `pulseIn()`, permettant au Core 1 d'effectuer plusieurs tâches librement pendant la propagation acoustique.
* **`dht.h` / `dht.cpp`**
  * **But :** Extrait les paramètres localisés d'humidité relative et de température via le capteur DHT11.
  * **Logique détaillée :** Comprend une implémentation personnalisée native du protocole 1-Wire sans dépendances. Isole temporairement le processeur des anomalies de temps en désactivant les interruptions du système (`noInterrupts()`) pendant la fenêtre de capture *bit-banging* critique en microsecondes. Implémente une mécanique automatique d'auto-réessai d'une seconde pour garantir l'intégrité des données face aux interférences électromagnétiques environnementales.

### 7. Utilitaires et Historiques sur Disque
* **`utils.h` / `utils.cpp`**
  * **But :** Cadre d'aide de backend pour le lissage mathématique, le stockage persistant et le routage du bus physique.
  * **Logique détaillée :** Calcule les profils réels de stress des cœurs en utilisant un algorithme de Moyenne Mobile Exponentielle (EMA 30/70) pour filtrer les pics momentanés. Gère 3 espaces NVS persistants séparés (`zenithmc`, `hwconfig`, `webcred`) pour protéger les mappages de périphériques sous-jacents contre les suppressions accidentelles du Wi-Fi. Utilise un sémaphore explicite Mutex (`i2cMutex`) pour coordonner le trafic I2C partagé sur l'écran LCD pendant la rotation entre les 5 écrans de télémétrie distincts.
* **`sd_card.h` / `sd_card.cpp`** *(Optionnel)*
  * **But :** Interconnecte une fente d'expansion physique MicroSD via un bus SPI dédié.
  * **Logique détaillée :** Sauvegardé sous la garde de compilation du préprocesseur `#ifdef SD_CS_PIN`. Lorsqu'il est présent, il intercepte automatiquement les tâches de journalisation de la base de données du stockage interne et transmet la sortie directement aux supports de disque physique.

### 8. Couche de Texte (Interface CLI)
* **`menus.h` / `menus.cpp`**
  * **But :** Cadre visuel et logique d'analyse d'actions pour l'interface de console basée sur du texte.
  * **Logique détaillée :** Emploie la conception de tampon `TerminalHibrida` pour empaqueter des matrices complexes de télémétrie (Uptime, stress du Core, espace du Heap) avant de les lancer sur le réseau, réduisant la pollution des paquets TCP. Modifie l'index d'exécution `programaActivo` pour rediriger les états d'exécution.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:list.svg?color=white"><img src="https://api.iconify.design/lucide:list.svg?color=black" width="26" align="center"></picture> Caractéristiques Principales

* **Contrôle 100 % Sans Fil :** Accès complet au terminal du système et à la sortie de diagnostic via Telnet standard (Port 23) et WebSockets bruts.
* **Mappage Dynamique des Broches Matérielles (Plug & Play) :** Lors de son premier déploiement, l'OS lance un hotspot de configuration automatisé (`Esp32BlascoOS_Setup`). Les utilisateurs peuvent configurer en toute sécurité les paramètres de leur routeur local, attribuer des broches GPIO personnalisées et définir des mots de passe web depuis n'importe quel navigateur de smartphone sans reconstruire le firmware.
* **Télémétrie en Temps Réel Lissée par EMA :** Diagnostics avancés détaillant :
  * Limites dynamiques de RAM/PSRAM (Totale, Allouée, Libre).
  * Occupation de la carte de stockage des fichiers locaux (LittleFS / Carte SD).
  * Charges mathématiques réelles du processeur multicœur et profils de température du silicium.
* **Base de Données Sauvegardée par Temps NTP :** Routines de journalisation en arrière-plan entièrement automatisées. Une fois synchronisé avec les horloges atomiques du réseau, le firmware ajoute une ligne de télémétrie structurée de 10 colonnes dans la feuille de calcul CSV du système toutes les 2 heures en continu.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:git-compare.svg?color=white"><img src="https://api.iconify.design/lucide:git-compare.svg?color=black" width="26" align="center"></picture> Le Problème vs La Solution

Développer et tester plusieurs projets matériels sur un seul microcontrôleur est souvent un chaos. 

| Sans Blasco OS | Avec Blasco OS |
| :--- | :--- |
| Connexion USB obligatoire pour tester | **100 % Sans Fil** via Telnet |
| Flasher le firmware à chaque changement physique de broche | **Configuration Runtime** via Portail Captif |
| Surveillance basique via Série | **Télémétrie avancée** (RAM, Flash, Temp, CPU) |
| Mises à jour par câble | **Support OTA** (Over-The-Air) |
| Projets couplés qui cassent le code | **Architecture Modulaire** ("Tiroirs" indépendants) |

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:blocks.svg?color=white"><img src="https://api.iconify.design/lucide:blocks.svg?color=black" width="26" align="center"></picture> Modules Actifs

Actuellement, le système d'exploitation comprend trois projets principaux intégrés :

### 1. Station de Clonage NFC Pro (V14)
Un module avancé d'audit et de clonage RFID utilisant le matériel **MFRC522**.
* **Lecture Profonde :** Extrait toutes les informations de la carte (y compris le Secteur 0 de sécurité maximale) et les sauvegarde dans la RAM.
* **Clonage Physique :** Permet d'injecter des données dans le Secteur 0 de cartes magiques réinscriptibles (CUID/FUID).

### 2. Radar à Ultrasons (V3)
Module de télémétrie physique utilisant le capteur de distance **HC-SR04**.
* **Exécution Asynchrone (ISR) :** Flux 100 % non bloquant piloté par des interruptions matérielles ; l'ESP32 ne subit aucune micro-congélation pendant que le son rebondit.
* **Lecture en Boucle :** Rafraîchissement cyclique configurable avec tolérance aux pannes thermiques ("Hors de portée").

### 3. Moniteur de Température et Humidité Ambiante (V1)
Module d'acquisition de données climatiques locales intégré via **DHT11**.
* **Protocole 1-Wire Natif :** Lecture de bas niveau (Bit-Banging) qui ne dépend d'aucune bibliothèque tierce, optimisée avec des délais matériels et des routines d'auto-récupération contre le bruit électromagnétique.

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:hard-drive.svg?color=white"><img src="https://api.iconify.design/lucide:hard-drive.svg?color=black" width="26" align="center"></picture> Matériel Requis et Installation

* **Carte de Base :** ESP32 (S3 N16R8 ou similaire).
* **Module NFC :** Lecteur RFID MFRC522 (Bus SPI).
* **Module de Distance :** Capteur HC-SR04.
* **Capteur Climatique :** Module DHT11 (avec résistance pull-up de 4.7kΩ).
* **Module de Stockage (Optionnel) :** Lecteur MicroSD (Bus SPI).

> *Remarque : Le système démarre avec des broches par défaut, mais toutes les connexions matérielles (RST, SS, TRIG, ECHO, DHT) peuvent être entièrement réaffectées depuis le Web Captive Portal sans toucher au code.*

### Déploiement Initial (Via Portail Captif) :
1. Flashez le code source compilé par USB pour la première fois en utilisant votre IDE préféré.
2. L'ESP32 formatera son système de fichiers interne (LittleFS) et, s'il ne détecte pas d'identifiants valides de la maison, ouvrira un Point d'Accès (AP).
3. Recherchez sur votre téléphone ou PC le réseau Wi-Fi ouvert : **`Esp32BlascoOS_Setup`** et connectez-vous y.
4. Un assistant web s'ouvrira automatiquement. Suivez les étapes pour entrer le mot de passe de votre routeur local, configurer vos broches GPIO et définir vos identifiants d'administrateur web.
5. En cliquant sur Enregistrer, l'ESP32 redémarrera, désactivera le mode AP et se connectera de manière transparente à votre routeur domestique.
6. Ouvrez la console Série à `115200 bauds` ou vérifiez l'écran LCD pour découvrir sa nouvelle IP locale assignée.
7. Ouvrez cette IP dans votre navigateur web. Profitez de l'environnement !

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:settings.svg?color=white"><img src="https://api.iconify.design/lucide:settings.svg?color=black" width="26" align="center"></picture> Configuration de Compilation (Arduino IDE)

Pour s'assurer que le projet se compile correctement et que l'interface Web dispose de suffisamment d'espace pour la base de données de LittleFS, il est **obligatoire** d'appliquer les paramètres suivants dans le menu **Outils (Tools)** d'Arduino IDE. 

*Cette configuration est optimisée pour les cartes **ESP32-S3 (N16R8)** avec 16 Mo de Flash et 8 Mo de PSRAM.*

| Paramètre de Configuration | Valeur Exacte Requise |
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

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:monitor.svg?color=white"><img src="https://api.iconify.design/lucide:monitor.svg?color=black" width="26" align="center"></picture> Interface et Télémétrie

Voici l'environnement d'exécution du Système d'Exploitation. Cliquez sur les menus déroulants pour développer et voir les captures d'écran de l'interface complète.

<details>
<summary><b>Panneau de Contrôle (Télémétrie et exécution en direct)</b></summary>
<br>

<p><i>Thème Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/db980fe4-600b-43e1-b8bb-b9c366eb26ee" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/eef43556-0c27-49e9-8d07-eda9577c5c75" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/4f032878-b0b0-406b-81f2-786c12560149" />

<p><i>Thème Clair</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55b18cca-1077-499a-81ed-d8acf6d66cc4" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/6c5cba54-ecff-498f-835e-130887f2bf8b" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/bbca1743-3a4e-4fa7-9f1b-6b4601bce200" />
</details>

<details>
<summary><b>Gestion de Base de Données</b></summary>
<br>

<p><i>Thème Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/15c40684-d907-4bca-9387-72960a42e2ad" />

<p><i>Thème Clair</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/59ef1539-4c79-4654-aa02-3c0c15b51490" />
</details>

<details>
<summary><b>Authentification (Login)</b></summary>
<br>

<p><i>Thème Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/be11690f-6344-40b0-aa56-73fbde6f885b" />

<p><i>Thème Clair</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/72f2c465-19c3-456b-9e2b-9e858a1d2273" />
</details>

<details>
<summary><b>Configuration Interne</b></summary>
<br>

<p><i>Thème Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/0ef0f0d3-2d07-4788-a5ba-e5ab27a03350" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/fc443c62-634d-437d-b8ac-b34004d1911f" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/25386ba6-4501-4d4b-b53e-b7bc85d0a78c" />


<p><i>Thème Clair</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/92536347-e43d-4ae2-8d77-b39f28956280" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/f0722d4c-c2fb-45b4-9eb4-78b2052cc72c" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/a1f0496e-58c5-4706-96f9-b061129aa223" />

</details>

<details>
<summary><b>Configuration Initiale</b></summary>
<br>

<p><i>Thème Sombre</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/7baf8092-9362-4781-b994-7e944e0e2b77" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/c21cca77-21ab-4a27-968c-e9bdb81bb691" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/780ecdca-629f-4bed-ac2a-09ff42127f92" />



<p><i>Thème Clair</i></p>
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/2d2c4c3b-e5e7-4fae-a8e1-2485f2c790c6" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/8361138e-f22b-478e-94a1-d0e94df2959a" />
<img width="100%" alt="image" src="https://github.com/user-attachments/assets/55d62211-ca00-482a-b7dc-7fac87c9ab99" />

</details>

---

## <picture><source media="(prefers-color-scheme: dark)" srcset="https://api.iconify.design/lucide:mail.svg?color=white"><img src="https://api.iconify.design/lucide:mail.svg?color=black" width="26" align="center"></picture> Contact et Sponsoring

Développé avec passion par **Ruben Blasco Armengod**.

Si vous êtes intéressé par le parrainage de l'expansion continue de ce projet, par la collaboration sur des conceptions de cartes mères matérielles personnalisées ou par l'intégration de nouveaux capteurs dans le noyau multitâche, veuillez me contacter via l'un des canaux techniques suivants :

* **GitHub:** [@rubenblascoa](https://github.com/rubenblascoa)
* **Instagram:** [@rubenblascoa](https://instagram.com/rubenblascoa)
* **Email:** rubenblascoarmengod@gmail.com
