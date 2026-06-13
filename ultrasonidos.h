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
 * @file ultrasonidos.h
 * @brief Interfaz de captura y disparo del transductor de ultrasonidos HC-SR04.
 */
#pragma once

#include "config.h"
#include "modules.h"

// ============================================================================
// DECLARACIÓN DE FUNCIONES DE ULTRASONIDOS
// ============================================================================

/**
 * @brief Genera un pulso sónico y mide el tiempo de retorno para calcular la distancia en centímetros.
 * @return long Distancia entera calculada en centímetros; retorna -1 si se supera el timeout de rebote.
 */
long medirDistanciaFisica();
void reinicializarUltrasonidos();
void ultrasonidosRegistrarFallo();
uint8_t ultrasonidosGetFallos();
void ultrasonidosResetFallos();
void usLoop();
void usEntrada(const String& cmd);
extern const Modulo moduloUS;