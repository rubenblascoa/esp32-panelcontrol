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
 * [FIX] Todas las operaciones SPI envueltas con spiMutex para evitar colisiones
 *       con el modulo MFRC522 cuando ambos cores accedan al bus simultaneamente.
 */
#include "sd_card.h"

#ifdef SD_CS_PIN
  #include <SPI.h>

  static SPIClass sdSPI;

  bool inicializarSD() {
    // La inicializacion ocurre en setup(), antes de crear las tareas FreeRTOS,
    // por lo que no necesita mutex aqui.
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
    // [FIX] Proteger acceso SPI con spiMutex
    if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;
    File file = SD.open("/datos.csv", FILE_APPEND);
    bool ok = false;
    if (file) {
      ok = file.print(linea);
      file.close();
    }
    if (spiMutex) xSemaphoreGive(spiMutex);
    return ok;
  }

  bool eliminarCSV_SD() {
    if (!sdDisponible) return false;
    if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;
    bool ok = SD.remove("/datos.csv");
    if (spiMutex) xSemaphoreGive(spiMutex);
    return ok;
  }

  bool existeCSV_SD() {
    if (!sdDisponible) return false;
    if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;
    bool ok = SD.exists("/datos.csv");
    if (spiMutex) xSemaphoreGive(spiMutex);
    return ok;
  }

  size_t tamanoCSV_SD() {
    if (!sdDisponible) return 0;
    if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(200)) != pdTRUE) return 0;
    File file = SD.open("/datos.csv", FILE_READ);
    size_t s = 0;
    if (file) {
      s = file.size();
      file.close();
    }
    if (spiMutex) xSemaphoreGive(spiMutex);
    return s;
  }

#endif
