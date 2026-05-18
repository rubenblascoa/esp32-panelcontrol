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
 * @file nfc.h
 * @brief Declaración de las subrutinas de control para el lector RFID MFRC522.
 */
#pragma once // Directiva de preprocesador que impide la doble inclusión de este header

#include "config.h" // Inclusión mandatoria para interactuar con el objeto mfrc522 global

// ============================================================================
// DECLARACIÓN DE FUNCIONES DE HARDWARE NFC
// ============================================================================

/**
 * @brief Realiza el escaneo continuo del bus SPI para interceptar tarjetas, autenticarse y volcar el bloque 0.
 */
void modoLecturaNFC(); //

/**
 * @brief Detecta una tarjeta mágica o destino y escribe en su bloque 0 la información previamente respaldada.
 */
void modoEscrituraNFC(); //