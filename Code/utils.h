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
 * @file utils.h
 * @brief Declaración de funciones de diagnóstico de CPU, tiempos NTP, LittleFS y control del LCD.
 */
#pragma once // Directiva que impide al preprocesador duplicar cabeceras en el proceso de compilación

#include "config.h" // Inclusión mandatoria para interactuar con tipos de datos de Arduino y objetos globales

// ============================================================================
// DECLARACIÓN DE FUNCIONES DE UTILIDAD Y APOYO SISTEMA
// ============================================================================

/**
 * @brief Calcula el porcentaje real de uso de CPU de un núcleo comparando tiempo de trabajo vs tiempo de ciclo.
 * @param coreID Identificador numérico del núcleo a evaluar (0 o 1).
 * @return uint32_t Porcentaje entero de uso real de CPU (0 a 100%).
 */
uint32_t calcularUsoCPU(int coreID); // [cite: 524]

/**
 * @brief Construye una cadena formateada con las horas, minutos y segundos transcurridos desde el arranque.
 * @return String Cadena de texto con formato de reloj "HH:MM:SS".
 */
String obtenerUptime(); // [cite: 531]

/**
 * @brief Consulta el chip de tiempo interno para formatear la fecha y hora obtenida por el servidor NTP.
 * @return String Cadena de texto con formato de calendario estándar "DD/MM/YYYY HH:MM".
 */
String obtenerFechaHora(); // [cite: 532, 534]

/**
 * @brief Abre el volumen LittleFS en modo adjuntar y añade una línea CSV con las métricas actuales.
 */
void guardarEnHistorial(); // [cite: 552]

/**
 * @brief Orquesta la paginación de datos del sistema en la pantalla LCD I2C protegiendo el bus mediante Mutex.
 */
void actualizarLCD(); // [cite: 560]