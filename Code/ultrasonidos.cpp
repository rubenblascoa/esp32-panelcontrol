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
 * @file ultrasonidos.cpp
 * @brief Implementación física del rebote acústico y conversión espacio-temporal.
 */
#include "ultrasonidos.h" // Vinculación con su header correspondiente

// ============================================================================
// CAPTURA DE ECO Y CONVERSIÓN ESCALAR DE DISTANCIA
// ============================================================================
long medirDistanciaFisica() {             
  // Paso 1: Estabilización drástica de la línea de transmisión eléctrica forzando un estado bajo limpio [cite: 1352]
  digitalWrite(TRIG_PIN, LOW); // [cite: 1352]
  delayMicroseconds(2);        // Espera de 2 microsegundos para asentar el pin digital [cite: 1353]
  
  // Paso 2: Generación estricta de la ráfaga de disparo eléctrico hacia el transductor emisor [cite: 1353]
  digitalWrite(TRIG_PIN, HIGH); // Eleva la línea a 3.3V / 5V [cite: 1353]
  delayMicroseconds(10);        // Duración exacta del pulso exigida por el datasheet del HC-SR04 [cite: 1353]
  digitalWrite(TRIG_PIN, LOW);  // Corta la transmisión sónica bajando el pin a cero [cite: 1354]
  
  // Paso 3: Medición bloqueante del ancho del pulso recibido en el pin receptor [cite: 1354]
  // Configuramos un Timeout de 30,000 microsegundos (30ms), límite físico para rebotes a más de 4 metros [cite: 1354]
  long duracion = pulseIn(ECHO_PIN, HIGH, 30000); // [cite: 1354]
  
  // Paso 4: Detección de pérdida de rebote sónico (El eco nunca regresó y el hardware superó el timeout) [cite: 1355]
  if (duracion == 0) return -1; // Retorna código de desbordamiento (Out of bounds) [cite: 1355]
  
  // Paso 5: Ecuación física de conversión: (Tiempo microsegundos * Velocidad del sonido (0.0343 cm/us)) / 2 [cite: 1356]
  // Dividimos entre 2 porque el haz sónico recorre dos veces la distancia (ida hasta el objeto y vuelta al receptor) [cite: 1356]
  return duracion * 0.034 / 2; // Retorna la distancia euclidiana final mapeada en centímetros [cite: 1356]
}