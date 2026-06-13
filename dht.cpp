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
 * @file dht.cpp
 * @brief Implementacion nativa del protocolo 1-Wire del sensor DHT11.
 *        Sin dependencia de libreria externa — usa solo Arduino core (digitalRead/pinMode/delayMicroseconds).
 */
#include "dht.h"
#include "terminal.h"
#include "menus.h"

// ============================================================================
// LECTURA NATIVA DEL PROTOCOLO DHT11
// Retorna true si la lectura es valida. Escribe temperatura y humedad en los punteros.
// ============================================================================
static bool leerDHT11Raw(float &temp, float &hum) {
  uint8_t data[5] = {0, 0, 0, 0, 0};

  // --- Señal de inicio: MCU tira LOW 18ms, luego suelta la linea ---
  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, HIGH); // Asegurar nivel alto antes de iniciar
  delay(2);
  digitalWrite(DHT_PIN, LOW);
  delay(20);                   // 20ms > 18ms mínimo del protocolo DHT11
  pinMode(DHT_PIN, INPUT_PULLUP);
  delayMicroseconds(40);       // Espera a que el sensor tome el control del bus

  // Pequeña pausa para que el bus se asiente tras el cambio a INPUT
  delayMicroseconds(5);

  // --- Deshabilitar interrupciones durante la lectura crítica de temporización ---
  // En ESP32-S3 a 240MHz, las interrupciones del WiFi/FreeRTOS alteran micros()
  noInterrupts();

  // --- Respuesta del sensor: esperar LOW (80us) luego HIGH (80us) ---
  // Timeout generoso para dar margen al DHT11 (lento vs DHT22)
  unsigned long t = micros();
  while (digitalRead(DHT_PIN) == HIGH) { if (micros() - t > 300) { interrupts(); return false; } }
  t = micros();
  while (digitalRead(DHT_PIN) == LOW)  { if (micros() - t > 120) { interrupts(); return false; } }
  t = micros();
  while (digitalRead(DHT_PIN) == HIGH) { if (micros() - t > 120) { interrupts(); return false; } }

  // --- Leer 40 bits (5 bytes) ---
  for (int i = 0; i < 40; i++) {
    // Esperar flanco de subida (inicio de bit)
    t = micros();
    while (digitalRead(DHT_PIN) == LOW)  { if (micros() - t > 60) { interrupts(); return false; } }
    // Medir duracion del HIGH: <40us = 0, >40us = 1 (umbral más robusto para DHT11)
    t = micros();
    while (digitalRead(DHT_PIN) == HIGH) { if (micros() - t > 80) { interrupts(); return false; } }
    if (micros() - t > 40) data[i / 8] |= (1 << (7 - (i % 8)));
  }

  // --- Re-habilitar interrupciones ---
  interrupts();

  // --- Verificar checksum ---
  if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) return false;

  hum  = data[0] + data[1] * 0.1f;
  temp = data[2] + data[3] * 0.1f;
  return true;
}

// ============================================================================
// CONTADOR DE ERRORES Y RECUPERACIÓN AUTOMÁTICA
// ============================================================================
static uint8_t  fallosDHT = 0;
static uint32_t ultimoReinitDHT = 0;
static const uint8_t  MAX_FALLOS_DHT = 5;
static const uint32_t REINIT_COOLDOWN_MS = 3000;

void reinicializarDHT() {
  pinMode(DHT_PIN, INPUT_PULLUP);
  delay(100);
  LOG_I("DHT", "Bus reinicializado tras %d fallos consecutivos", fallosDHT);
}

void resetFallosDHT() { fallosDHT = 0; }
uint8_t getFallosDHT() { return fallosDHT; }

// ============================================================================
// INICIALIZACION
// ============================================================================
void inicializarDHT() {
  pinMode(DHT_PIN, INPUT_PULLUP);
  delay(1000); // DHT11 necesita 1s tras el encendido antes de la primera lectura

  float t, h;
  if (!leerDHT11Raw(t, h)) {
    Terminal.println("[DHT] Error: Sensor no detectado en la inicializacion.");
  } else {
    Terminal.println("[DHT] Sensor DHT11 inicializado correctamente.");
  }
}

// ============================================================================
// LECTURA INDIVIDUAL DE TEMPERATURA (C)
// ============================================================================
float leerTemperatura() {
  float t, h;
  if (!leerDHT11Raw(t, h)) {
    Terminal.println("[DHT] Error: Lectura de temperatura invalida.");
    return -127.0;
  }
  return t;
}

// ============================================================================
// LECTURA INDIVIDUAL DE HUMEDAD RELATIVA (%)
// ============================================================================
float leerHumedad() {
  float t, h;
  if (!leerDHT11Raw(t, h)) {
    Terminal.println("[DHT] Error: Lectura de humedad invalida.");
    return -1.0;
  }
  return h;
}

// ============================================================================
// LECTURA COMPUESTA CON ACTUALIZACION DE VARIABLES GLOBALES
// ============================================================================
bool actualizarDHT() {
  float t, h;
  for (int intento = 0; intento < 3; intento++) {
    if (leerDHT11Raw(t, h)) {
      sensores.temperatura = t;
      sensores.humedad     = h;
      fallosDHT = 0;
      return true;
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
  fallosDHT++;
  if (fallosDHT >= MAX_FALLOS_DHT) {
    uint32_t ahora = millis();
    if (ahora - ultimoReinitDHT > REINIT_COOLDOWN_MS) {
      reinicializarDHT();
      fallosDHT = 0;
      ultimoReinitDHT = ahora;
    }
  }
  return false;
}

// ============================================================================
// CALLBACKS DEL MÓDULO (Module Registry)
// ============================================================================
void dhtEntrada(const String& cmd) {
  procesarEntradaDHT(cmd);
}

const Modulo moduloDHT = {
  "DHT11 Temp/Hum",
  NULL,
  NULL,
  NULL,
  mostrarMenuDHT,
  dhtEntrada
};
