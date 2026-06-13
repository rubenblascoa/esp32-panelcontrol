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
 * @file sd_card.h
 * @brief Declaracion de funciones para el modulo de tarjeta SD.
 */
#pragma once

#include "config.h"

#ifdef SD_CS_PIN
  #include <SD.h>

  bool inicializarSD();
  bool guardarLineaSD(const String& linea);
  bool eliminarCSV_SD();
  bool existeCSV_SD();
  size_t tamanoCSV_SD();
  String leerCSV_SD();
#else
  inline bool inicializarSD() { return false; }
  inline bool guardarLineaSD(const String&) { return false; }
  inline bool eliminarCSV_SD() { return false; }
  inline bool existeCSV_SD() { return false; }
  inline size_t tamanoCSV_SD() { return 0; }
  inline String leerCSV_SD() { return ""; }
#endif
