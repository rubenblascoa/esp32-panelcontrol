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
 * @file sd_card.cpp
 * @brief Implementacion del modulo de tarjeta SD para almacenamiento de datos CSV.
 */
#include "sd_card.h"

#ifdef SD_CS_PIN
  #include <SPI.h>

  static SPIClass sdSPI;

  bool inicializarSD() {
    sdSPI.begin(SD_CLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    if (!SD.begin(SD_CS_PIN, sdSPI)) {
      Serial.println("❌ [SD] No se pudo inicializar la tarjeta");
      sdDisponible = false;
      return false;
    }
    Serial.println("✅ [SD] Tarjeta SD inicializada correctamente");
    sdDisponible = true;
    return true;
  }

  bool guardarLineaSD(const String& linea) {
    if (!sdDisponible) return false;
    File file = SD.open("/datos.csv", FILE_APPEND);
    if (!file) return false;
    bool ok = file.print(linea);
    file.close();
    return ok;
  }

  bool eliminarCSV_SD() {
    if (!sdDisponible) return false;
    return SD.remove("/datos.csv");
  }

  bool existeCSV_SD() {
    if (!sdDisponible) return false;
    return SD.exists("/datos.csv");
  }

  size_t tamanoCSV_SD() {
    if (!sdDisponible) return 0;
    File file = SD.open("/datos.csv", FILE_READ);
    if (!file) return 0;
    size_t s = file.size();
    file.close();
    return s;
  }

#endif
