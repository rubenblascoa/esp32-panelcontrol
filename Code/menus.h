// MIT License
// 
// Copyright (c) 2026 Ruben Blasco Armengod
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
 * @file menus.h
 * @brief Declaración de las pantallas de menús de usuario y parsers de comandos para la terminal dual.
 */
#pragma once // Garantiza que el compilador procese esta cabecera una sola vez por unidad de traducción

#include "config.h" // Inclusión mandatoria para interactuar con tipos y estados globales

// ============================================================================
// DECLARACIÓN DE FUNCIONES DE INTERFAZ DE TEXTO (CLI)
// ============================================================================

/**
 * @brief Agrupa y transmite la pantalla del menú de inicio junto con el reporte extendido de telemetría.
 */
void mostrarMenuPrincipal(); // [cite: 535, 626]

/**
 * @brief Procesa el comando ingresado en el menú inicial para conmutar entre los módulos de hardware.
 * @param entrada Cadena de texto limpia con el carácter o comando del usuario.
 */
void procesarMenuPrincipal(String entrada); // [cite: 535, 640]

/**
 * @brief Renderiza el panel específico de la estación de clonación NFC mostrando el estado de la memoria RAM.
 */
void mostrarMenuNFC(); // [cite: 535, 642]

/**
 * @brief Filtra e interpreta los comandos ingresados por el usuario dentro del entorno del módulo RFID.
 * @param entrada Cadena de comando ingresada desde el Telnet o la interfaz web.
 */
void procesarEntradaNFC(String entrada); // [cite: 535, 648]

/**
 * @brief Dibuja las opciones disponibles para el control del radar ultrasónico de distancia HC-SR04.
 */
void mostrarMenuUltrasonidos(); // [cite: 535, 663]