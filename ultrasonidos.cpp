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
 * @brief Implementación del sensor HC-SR04 con interrupción no bloqueante.
 *        Reemplaza pulseIn() por interrupción en ECHO para no bloquear FreeRTOS.
 */
#include "ultrasonidos.h"
#include "terminal.h"
#include "menus.h"

// Variables compartidas volátiles para la ISR
static volatile unsigned long ecoStart = 0;
static volatile unsigned long ecoDuration = 0;
static volatile bool ecoCompletado = false;

// Estado interno de la máquina de estados
static bool esperandoEco = false;
static unsigned long trigMicros = 0;
static unsigned long ultimoTrigMs = 0;

// ============================================================================
// CONTADOR DE ERRORES Y RECUPERACIÓN AUTOMÁTICA
// ============================================================================
static uint8_t  fallosUltrasonidos = 0;
static uint32_t ultimoReinitUS = 0;
static const uint8_t  MAX_FALLOS_US = 10;
static const uint32_t REINIT_COOLDOWN_US_MS = 3000;

void reinicializarUltrasonidos() {
  detachInterrupt(digitalPinToInterrupt(ECHO_PIN));
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
  esperandoEco = false;
  ecoCompletado = false;
  ecoDuration = 0;
  LOG_W("US", "Pines TRIG/ECHO reinicializados tras %d fallos", fallosUltrasonidos);
  fallosUltrasonidos = 0;
}

void ultrasonidosRegistrarFallo() {
  fallosUltrasonidos++;
  if (fallosUltrasonidos >= MAX_FALLOS_US) {
    uint32_t ahora = millis();
    if (ahora - ultimoReinitUS > REINIT_COOLDOWN_US_MS) {
      reinicializarUltrasonidos();
      ultimoReinitUS = ahora;
    }
  }
}

uint8_t ultrasonidosGetFallos() { return fallosUltrasonidos; }
void ultrasonidosResetFallos() { fallosUltrasonidos = 0; }

// ISR para CHANGE en ECHO_PIN — detecta flanco de subida o bajada
void IRAM_ATTR ecoISR() {
    if (digitalRead(ECHO_PIN) == HIGH) {
        ecoStart = micros();
    } else {
        ecoDuration = micros() - ecoStart;
        ecoCompletado = true;
    }
}

long medirDistanciaFisica() {
    if (!esperandoEco) {
        if (millis() - ultimoTrigMs < 200) return -1;

        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);

        ecoCompletado = false;
        ecoDuration = 0;
        trigMicros = micros();
        attachInterrupt(digitalPinToInterrupt(ECHO_PIN), ecoISR, CHANGE);
        esperandoEco = true;
        ultimoTrigMs = millis();
        return -1;
    }

    if (ecoCompletado) {
        ecoCompletado = false;
        esperandoEco = false;
        detachInterrupt(digitalPinToInterrupt(ECHO_PIN));

        if (ecoDuration == 0) { ultrasonidosRegistrarFallo(); return -2; }

        ultrasonidosResetFallos();
        return ecoDuration * 0.034 / 2;
    }

    if (micros() - trigMicros > 30000) {
        esperandoEco = false;
        detachInterrupt(digitalPinToInterrupt(ECHO_PIN));
        ultrasonidosRegistrarFallo();
        return -2;
    }

    return -1;
}

// ============================================================================
// CALLBACKS DEL MÓDULO (Module Registry)
// ============================================================================
void usLoop() {
  if (midiendoDistancia) {
    long d = medirDistanciaFisica();
    if (d >= 0) {
      Terminal.print(" -> Distancia: "); Terminal.print(d); Terminal.println(" cm");
    } else if (d == -2) {
      Terminal.println(" -> Distancia: FUERA DE RANGO");
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void usEntrada(const String& cmd_in) {
  String cmd = cmd_in;
  cmd.toUpperCase();
  if (cmd == "M") {
    midiendoDistancia = false; FSM_SET(0); mostrarMenuPrincipal();
  } else if (cmd == "U") {
    midiendoDistancia = false; mostrarMenuUltrasonidos();
  } else if (cmd == "1") {
    midiendoDistancia = true;
    Terminal.println("\n[+] RADAR ACTIVADO...");
  } else if (cmd == "2") {
    midiendoDistancia = false;
    Terminal.println("\n[-] Radar detenido."); mostrarMenuUltrasonidos();
  }
}

const Modulo moduloUS = {
  "Ultrasonidos HC-SR04",
  NULL,
  usLoop,
  NULL,
  mostrarMenuUltrasonidos,
  usEntrada
};
