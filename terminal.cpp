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
 * @file terminal.cpp
 * @brief Implementación de las funciones de escritura y control de flujo de la TerminalHibrida.
 *
 * OPTIMIZACIONES DE RENDIMIENTO (PERF-001 .. PERF-004):
 *
 * PERF-001 — Batching Telnet LAN
 *   ANTES: telnetClient.write(c) por cada byte → N llamadas TCP para una línea de N chars.
 *   AHORA: bufferTelnet acumula bytes; se envía en un solo write(buf, n) en flushTelnet()
 *          cuando aparece '\n' o el buffer supera TELNET_FLUSH_SIZE.
 *          Impacto: latencia de eco reducida de ~N×1ms a ~1ms por línea.
 *
 * PERF-002 — Batching Telnet túnel (Cloudflare Worker)
 *   ANTES: _flushTelnetTunnel(&c, 1) por cada byte → un WS frame por carácter.
 *   AHORA: bufferTelnetTunnel acumula; se envía en un solo tunel.ws.sendTXT() por línea
 *          o cuando supera TELNET_TUNNEL_FLUSH_SIZE.
 *          Impacto: de N frames WS a 1 frame por línea; reduce RTT drásticamente.
 *
 * PERF-003 — Web console: flush solo en salto de línea o umbral
 *   Sin cambios de lógica, pero write(buf, size) ahora no itera byte a byte cuando
 *   no hay clientes web activos (early-exit antes del bucle).
 *
 * PERF-004 — write(buf, size) sin iteración cuando Telnet está disponible
 *   ANTES: bucle for i→size con bufferWeb += char[i] y check de '\n' en cada iteración.
 *   AHORA: concat(buffer, size) directo al bufferTelnet + flush, evitando el bucle.
 */

#include "terminal.h"
#include "utils.h"    // jsonStr() para escapar texto JSON al túnel

// Instanciación física real del objeto global en memoria RAM
TerminalHibrida Terminal;

// Tamaño máximo del buffer Telnet LAN antes de forzar flush (aunque no haya '\n')
static const size_t TELNET_FLUSH_SIZE = 256;

// Tamaño máximo del buffer Telnet túnel antes de forzar flush
static const size_t TELNET_TUNNEL_FLUSH_SIZE = 512;

// ============================================================================
// HELPERS PRIVADOS DE FLUSH
// ============================================================================

// Envía bufferTelnet al socket TCP en una sola llamada write() y lo vacía.
// Una sola llamada TCP vs N llamadas: reduce drásticamente el overhead de red.
void TerminalHibrida::flushTelnet() {
    if (bufferTelnet.length() == 0) return;
    if (telnetClient && telnetClient.connected()) {
        telnetClient.write((const uint8_t*)bufferTelnet.c_str(), bufferTelnet.length());
    }
    bufferTelnet = "";
}

// Envía bufferTelnetTunnel al Worker en un único WS frame y lo vacía.
// Un frame WS vs N frames: reduce el número de round-trips al Worker.
void TerminalHibrida::flushTelnetTunnel() {
    if (bufferTelnetTunnel.length() == 0) return;
    if (tunel.telnetActivo && tunel.conectado) {
        String msg;
        msg.reserve(32 + bufferTelnetTunnel.length() * 2);
        msg = "{\"type\":\"telnet_fwd\",\"data\":\"";
        msg += jsonStr(bufferTelnetTunnel);
        msg += "\"}";
        tunel.ws.sendTXT(msg);
    }
    bufferTelnetTunnel = "";
}

// ============================================================================
// HELPER PRIVADO: despacha bufferWeb a LAN y/o túnel según clientes activos
// ============================================================================

static void _flushBuffer(const String& buffer) {
    if (ws != nullptr && ws->count() > 0) {
        ws->textAll(buffer);
    }
    if (tunel.conectado && tunel.wsCount > 0) {
        String msg;
        msg.reserve(32 + buffer.length() * 2);
        msg = "{\"type\":\"ws_fwd\",\"data\":\"";
        msg += jsonStr(buffer);
        msg += "\"}";
        tunel.ws.sendTXT(msg);
    }
}

// ============================================================================
// IMPLEMENTACIÓN DE MÉTODOS PÚBLICOS
// ============================================================================

void TerminalHibrida::iniciarBloque() {
    modoBloque = true;
    bufferWeb  = "";
}

void TerminalHibrida::enviarBloque() {
    if (sistemaWeb.permitirLog && bufferWeb.length() > 0) {
        String copy = bufferWeb;
        bufferWeb = "";
        _flushBuffer(copy);
    } else {
        bufferWeb = "";
    }
    modoBloque = false;
}

// ============================================================================
// write(uint8_t c) — un único carácter
// ============================================================================
// PERF-001 / PERF-002: acumula en buffer y solo hace flush al encontrar '\n'
// o cuando el buffer supera el umbral. Elimina la llamada TCP/WS por carácter.
// ============================================================================
size_t TerminalHibrida::write(uint8_t c) {
    // — Telnet LAN —
    // Acumular en bufferTelnet; flush en '\n' o al superar el umbral.
    if (telnetClient && telnetClient.connected()) {
        bufferTelnet += (char)c;
        if (c == '\n' || bufferTelnet.length() >= TELNET_FLUSH_SIZE) {
            flushTelnet();
        }
    }

    // — Telnet túnel —
    // Acumular en bufferTelnetTunnel; flush en '\n' o al superar el umbral.
    if (tunel.telnetActivo && telnet.autenticado && tunel.conectado) {
        bufferTelnetTunnel += (char)c;
        if (c == '\n' || bufferTelnetTunnel.length() >= TELNET_TUNNEL_FLUSH_SIZE) {
            flushTelnetTunnel();
        }
    }

    // — Web console (WebSocket / túnel) —
    bool hayLan   = (ws != nullptr && ws->count() > 0);
    bool hayTunel = (tunel.conectado && tunel.wsCount > 0);
    if (sistemaWeb.permitirLog && (hayLan || hayTunel)) {
        bufferWeb += (char)c;
        if (!modoBloque && c == '\n') {
            String copy = bufferWeb;
            bufferWeb = "";
            _flushBuffer(copy);
        }
    }

    return 1;
}

// ============================================================================
// write(const uint8_t* buffer, size_t size) — bloque de bytes
// ============================================================================
// PERF-001 / PERF-002 / PERF-004: concat directo al bufferTelnet y flush
// al final del bloque. Evita el bucle byte-a-byte para Telnet.
// Para web console mantiene el bucle para detectar '\n' mid-buffer.
// ============================================================================
size_t TerminalHibrida::write(const uint8_t* buffer, size_t size) {
    if (size == 0) return 0;

    // — Telnet LAN —
    // concat() directo del bloque completo, luego un solo flush.
    if (telnetClient && telnetClient.connected()) {
        bufferTelnet.concat((const char*)buffer, size);
        // Flush inmediato si hay '\n' en el bloque o supera el umbral
        bool tieneNewline = (memchr(buffer, '\n', size) != nullptr);
        if (tieneNewline || bufferTelnet.length() >= TELNET_FLUSH_SIZE) {
            flushTelnet();
        }
    }

    // — Telnet túnel —
    if (tunel.telnetActivo && telnet.autenticado && tunel.conectado) {
        bufferTelnetTunnel.concat((const char*)buffer, size);
        bool tieneNewline = (memchr(buffer, '\n', size) != nullptr);
        if (tieneNewline || bufferTelnetTunnel.length() >= TELNET_TUNNEL_FLUSH_SIZE) {
            flushTelnetTunnel();
        }
    }

    // — Web console —
    bool hayLan   = (ws != nullptr && ws->count() > 0);
    bool hayTunel = (tunel.conectado && tunel.wsCount > 0);
    if (sistemaWeb.permitirLog && (hayLan || hayTunel)) {
        for (size_t i = 0; i < size; i++) {
            bufferWeb += (char)buffer[i];
            if (!modoBloque && buffer[i] == '\n') {
                String copy = bufferWeb;
                bufferWeb = "";
                _flushBuffer(copy);
            }
        }
        // Flush de seguridad si el buffer crece sin encontrar '\n'
        if (!modoBloque && bufferWeb.length() > 200) {
            String copy = bufferWeb;
            bufferWeb = "";
            _flushBuffer(copy);
        }
    }

    return size;
}
