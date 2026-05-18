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
 * @file tareas.h
 * @brief Declaración de las tareas asíncronas para el planificador en paralelo FreeRTOS.
 */
#pragma once // Directiva de preprocesamiento que impide colisiones por dobles inclusiones en el compilador

#include "config.h" // Inclusión del archivo base para el reconocimiento de tipos y variables compartidas

// ============================================================================
// DECLARACIÓN DE HILOS INFINITOS FREERTOS
// ============================================================================

/**
 * @brief Tarea destinada al Núcleo 0 (Core 0). Administra el WiFi, OTA, WebSockets, base de datos y LCD.
 * @param pvParameters Puntero genérico a parámetros de FreeRTOS (asignado a NULL por defecto).
 */
void taskCore0(void * pvParameters); // 

/**
 * @brief Tarea destinada al Núcleo 1 (Core 1). Controla los sensores físicos SPI/GPIO y el procesamiento de menús.
 * @param pvParameters Puntero genérico a parámetros de FreeRTOS (asignado a NULL por defecto).
 */
void taskCore1(void * pvParameters); //