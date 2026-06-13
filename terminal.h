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
 * @file terminal.h
 * @brief Declaración de la clase TerminalHibrida para salida dual (Telnet + Web) antibloqueo.
 */
#pragma once // Directiva de preprocesador que garantiza que este archivo se incluya una única vez por compilación

#include "config.h" // Inclusión mandatoria del archivo de configuración global para conocer el estado de la red y flags

/**
 * @class TerminalHibrida
 * @brief Clase derivada de Print para gestionar de forma asíncrona flujos de datos hacia Telnet y WebSockets.
 */
class TerminalHibrida : public Print { 
private:
  String bufferWeb;          // Búfer de memoria dinámico para concatenar caracteres destinados al WebSocket
  String bufferTelnet;       // Búfer de batching para Telnet LAN: acumula hasta \n o 256 bytes antes del write TCP
  String bufferTelnetTunnel; // Búfer de batching para Telnet túnel: agrupa chars en un solo WS frame por línea
  bool modoBloque;           // Bandera de control para agrupar cadenas largas y evitar saturar la pila de red

  void flushTelnet();        // Vuelca bufferTelnet al socket TCP en una sola llamada
  void flushTelnetTunnel();  // Vuelca bufferTelnetTunnel al Worker en un solo WS frame

public:
  TerminalHibrida() : bufferWeb(""), bufferTelnet(""), bufferTelnetTunnel(""), modoBloque(false) {}

  /**
   * @brief Detiene temporalmente el envío automático de líneas a la web para agrupar mensajes en bloque.
   */
  void iniciarBloque(); // Método de apertura de retención 

  /**
   * @brief Libera el búfer acumulado y envía la ráfaga de texto completa a todos los clientes web activos.
   */
  void enviarBloque();  // Método de cierre y transmisión masiva 

  /**
   * @brief Escribe un único carácter en los flujos de salida activos (Telnet directo / Web condicional).
   * @param c Carácter binario o ASCII a procesar.
   * @return size_t Cantidad de bytes escritos de manera exitosa (siempre devuelve 1 en este flujo).
   */
  size_t write(uint8_t c) override; // Sobrescritura del método virtual puro obligatorio de la clase madre Print 
  
  /**
   * @brief Escribe un arreglo (array) de bytes de longitud determinada en los flujos de salida de red.
   * @param buffer Puntero al bloque de memoria que contiene los datos.
   * @param size Tamaño en bytes del bloque de memoria a transmitir.
   * @return size_t Cantidad total de bytes procesados y escritos en los buffers.
   */
  size_t write(const uint8_t *buffer, size_t size) override; // Sobrescritura optimizada para ráfagas de datos 
};

// ============================================================================
// EXPORTACIÓN DEL OBJETO TERMINAL
// ============================================================================
extern TerminalHibrida Terminal; // Declaración externa de la instancia única para acceso global desde otros .cpp 