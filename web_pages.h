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
 * @file web_pages.h
 * @brief Declaración de las páginas HTML incrustadas en memoria Flash (PROGMEM).
 */
#pragma once // Evita colisiones por inclusiones múltiples en tiempo de compilación

#include <Arduino.h> // Necesario para reconocer el modificador PROGMEM y tipos de datos base

// ============================================================================
// DECLARACIÓN DE ARREGLOS CONSTANTES DE TEXTO (Almacenados en Flash)
// ============================================================================

extern const char index_html[] PROGMEM;       // Interfaz de telemetría y consola interactiva WebSocket
extern const char db_html[] PROGMEM;          // Vista del historial CSV almacenado en LittleFS con paginación
extern const char login_html[] PROGMEM;       // Formulario de acceso protegido por cookies seguras
extern const char wifi_setup_html[] PROGMEM;  // Captive portal para configuración WiFi inicial
extern const char config_html[] PROGMEM;      // Panel de configuración del sistema
extern const char landing_html[] PROGMEM;     // Landing page pública de presentación