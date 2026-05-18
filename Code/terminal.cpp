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
 */
#include "terminal.h" // Inclusión del archivo de interfaz propio

// Instanciación física real del objeto global en memoria RAM [cite: 37]
TerminalHibrida Terminal;

// ============================================================================
// IMPLEMENTACIÓN DE MÉTODOS DE LA CLASE TERMINAL HÍBRIDA
// ============================================================================

// Abre la retención temporal: Vacía el búfer de texto web y activa la bandera de modo bloque [cite: 26]
void TerminalHibrida::iniciarBloque() { 
    modoBloque = true;  // Bloquea temporalmente el envío automático caracter por caracter [cite: 26]
    bufferWeb = "";     // Inicializa el string dinámico para recibir una nueva ráfaga [cite: 26]
}

// Cierra la retención y envía la bomba de texto acumulada a la interfaz web [cite: 27]
void TerminalHibrida::enviarBloque() {  
    // Verificación triple: logs web permitidos, clientes web conectados y que el búfer no esté vacío [cite: 27]
    if (permitirWebLog && ws->count() > 0 && bufferWeb.length() > 0) {
        ws->textAll(bufferWeb); // Transmite el string completo a todos los sockets en una sola trama TCP [cite: 27]
    }
    bufferWeb = "";            // Libera de forma inmediata la memoria consumida por el string acumulado [cite: 28]
    modoBloque = false;         // Desactiva la llave maestra, devolviendo el control al flujo normal [cite: 28]
}

// Escritura de un único byte en los canales de comunicación activos [cite: 29]
size_t TerminalHibrida::write(uint8_t c) {   
    // El flujo TCP clásico de Telnet se procesa siempre en directo e inmediato, sin bloqueos 
    if (telnetClient && telnetClient.connected()) {
        telnetClient.write(c); // Escribe el carácter en el socket de red TCP de Putty/CMD [cite: 29]
    }
    
    // Validación de condiciones lógicas para el canal web asíncrono [cite: 30]
    if (permitirWebLog && ws->count() > 0) { 
        bufferWeb += (char)c; // Adjunta el byte convertido a carácter dentro del búfer de memoria [cite: 30]
        
        // Si no estamos agrupando datos en bloque y detectamos un salto de línea, vaciamos el búfer hacia la web [cite: 31]
        if (!modoBloque && c == '\n') { 
            ws->textAll(bufferWeb); // Envía de forma automática la línea terminada a la interfaz web [cite: 31]
            bufferWeb = "";         // Resetea el búfer de línea listo para el siguiente comando [cite: 32]
        }
    }
    return 1; // Retorna 1 byte procesado con éxito según la especificación de la firma abstracta de Arduino [cite: 32]
}

// Escritura en bloque optimizada para matrices de bytes y cadenas de texto completas [cite: 33]
size_t TerminalHibrida::write(const uint8_t *buffer, size_t size) { 
    // Envío directo de la ráfaga completa al terminal Telnet si la sesión está activa y conectada [cite: 33]
    if (telnetClient && telnetClient.connected()) {
        telnetClient.write(buffer, size); // Ejecución de la llamada nativa de socket TCP [cite: 33]
    }
    
    // Procesamiento condicional para la duplicación en consola web [cite: 34]
    if (permitirWebLog && ws->count() > 0) {
        // Ciclo iterativo para procesar cada elemento del búfer de memoria suministrado [cite: 34]
        for(size_t i = 0; i < size; i++) {
            bufferWeb += (char)buffer[i]; // Inserción del carácter actual en el string acumulador [cite: 34]
            
            // Si es un modo normal (no bloque) y se encuentra un salto de línea, se despacha la línea actual [cite: 35]
            if (!modoBloque && buffer[i] == '\n') {
                ws->textAll(bufferWeb); // Transmisión inmediata vía WebSocket [cite: 35]
                bufferWeb = "";         // Vaciado seguro de memoria de la línea enviada [cite: 36]
            }
        }
        
        // Salvaguarda antibloqueo: Si escribimos mucho texto sin saltos de línea (y no es modo bloque),
        // forzamos un envío al superar los 200 caracteres para evitar un error de falta de memoria (OOM) [cite: 36]
        if (!modoBloque && bufferWeb.length() > 200) {
            ws->textAll(bufferWeb); // Fuerza la transmisión de la línea huérfana [cite: 36]
            bufferWeb = "";         // Vaciado preventivo de memoria del búfer [cite: 37]
        }
    }
    return size; // Retorna la cantidad exacta de bytes procesados por la función [cite: 37]
}