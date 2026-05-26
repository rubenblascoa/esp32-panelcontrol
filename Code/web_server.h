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
 * @file web_server.h
 * @brief Declaración de las rutas HTTP, eventos WebSocket y control de accesos del servidor.
 */
#pragma once // Evita la duplicidad de inclusiones en los archivos del compilador

#include "config.h" // Inclusión mandatoria para interactuar con los punteros de red globales

// ============================================================================
// DECLARACIÓN DE FUNCIONES DE RED
// ============================================================================

/**
 * @brief Inicializa el servidor web asíncrono, vincula los endpoints HTTP y el gestor del WebSocket.
 */
void iniciarServidorWeb(); //

/**
 * @brief Intercepta los encabezados de la solicitud HTTP para validar la presencia de la cookie de sesión activa.
 * @param request Puntero a la estructura de la petición asíncrona del cliente.
 * @return true Si el token de la cookie coincide perfectamente con el token de sesión global en la RAM.
 * @return false Si no hay cookie, está vacía o el token es inválido.
 */
bool estaLogueado(AsyncWebServerRequest *request); //

/**
 * @brief Función de callback (manejador de interrupciones) para el procesamiento de tramas de datos vía WebSocket.
 * @param server Puntero al servidor WebSocket que originó el evento.
 * @param client Puntero al cliente específico que envía o recibe datos.
 * @param type Tipo de evento WebSocket recibido (Conexión, Desconexión, Datos, Ping).
 * @param arg Parámetros adicionales o metadatos de la trama de red.
 * @param data Puntero a la matriz de bytes del búfer de datos entrantes.
 * @param len Longitud en bytes de la trama de datos actual.
 */
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len); //