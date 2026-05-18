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
 * @file web_server.cpp
 * @brief Implementación de las peticiones HTTP del servidor web asíncrono y gestión del WebSocket.
 */
#include "web_server.h" // Inclusión de cabecera de definición propia
#include "web_pages.h"  // Permite servir las páginas web estáticas grabadas en PROGMEM [cite: 544, 548, 549]
#include "terminal.h"   // Permite notificar eventos directamente a la terminal dual [cite: 537]
#include "menus.h"      // Permite repintar los menús interactivos tras eventos de conexión [cite: 538]

// ============================================================================
// VALIDACIÓN DE SESIÓN POR COOKIE DE SEGURIDAD
// ============================================================================
bool estaLogueado(AsyncWebServerRequest *request) {
  // Comprobamos de forma estricta si la solicitud entrante contiene el header de Cookies [cite: 23]
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie"); // Extraemos el string completo de cookies del cliente [cite: 23]
    
    // Verificamos que el token del sistema no esté vacío y que la cookie contenga la clave ZENITH_SESSION vinculada a él [cite: 24]
    if (tokenSesionActiva != "" && cookie.indexOf("ZENITH_SESSION=" + tokenSesionActiva) != -1) {
      return true; // Autenticación válida: El usuario tiene permiso de visualización [cite: 24]
    }
  }
  return false; // Acceso denegado de forma por defecto si no supera la validación [cite: 25]
}

// ============================================================================
// MANEJADOR DE EVENTOS ASÍNCRONOS DEL TÚNEL WEBSOCKET
// ============================================================================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) { 
  // Caso 1: Se ha conectado un nuevo cliente web a la URL del socket [cite: 536]
  if (type == WS_EVT_CONNECT) {   
    // Protección anti-desbordamiento de memoria (OOM): Rechaza conexiones si superamos el límite configurado [cite: 18, 536]
    if (ws->count() > MAX_WEBSOCKET_CLIENTS) { 
        client->close(); // Cierra el socket del cliente intruso de manera inmediata [cite: 536]
        return;          // Aborta el procesamiento [cite: 537]
    }
    permitirWebLog = true; // Abre las compuertas lógicas para dejar pasar los caracteres hacia el túnel web [cite: 537]
    Terminal.println("\n[SISTEMA] Dispositivo conectado al SO Blasco mediante Web-Telnet."); // Inyecta log en los canales [cite: 537]
    mostrarMenuPrincipal(); // Dibuja la consola del sistema de forma automática al usuario recién ingresado [cite: 538]
    
    forzarTelemetria = true; // Activa el interruptor global para despachar el JSON de rendimiento sin esperar al ciclo de 5s [cite: 21, 538]
  } 
  // Caso 2: Se recibe una trama de datos desde el navegador web del usuario [cite: 539]
  else if (type == WS_EVT_DATA) { 
    AwsFrameInfo *info = (AwsFrameInfo*)arg; // Mapeo de puntero a la estructura de control de fragmentación de red [cite: 539]
    
    // Validamos que sea una trama de texto, que esté completa (final) y que el tamaño coincida con la longitud leída [cite: 540]
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) { 
      data[len] = 0; // Inserción mandatoria del carácter nulo al final de la matriz para conversión segura a C-String [cite: 540, 541]
      entradaWeb = String((char*)data); // Conversión del búfer de bytes a cadena dinámica de Arduino [cite: 541]
      entradaWeb.trim(); // Limpieza drástica de retornos de carro, tabuladores y espacios en blanco [cite: 541, 542]
      
      // Comando reservado del sistema para reinicio por software en caliente
      if (entradaWeb == "reboot") {
          ESP.restart(); // Ejecuta la interrupción nativa para resetear la placa base [cite: 542]
      } else {
          hayEntradaWeb = true; // Eleva la bandera de sincronización para que el Core 1 procese la orden en su bucle [cite: 17, 543]
      }
    }
  }
}

// ============================================================================
// CONFIGURACIÓN DE RUTAS DEL SERVIDOR HTTP
// ============================================================================
void iniciarServidorWeb() {       
  
  // Ruta 1: Vista pública del formulario de validación de identidad
  server->on("/login-page", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/html", login_html); // Envía el HTML estático comprimido en la Flash 
  });

  // Ruta 2: Endpoint del método POST para el procesamiento de credenciales de acceso
  server->on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
      String u = "", p = ""; // Inicialización de variables locales temporales
      
      // Extracción segura de los parámetros enviados dentro del cuerpo del formulario [cite: 545]
      if(request->hasParam("user", true)) u = request->getParam("user", true)->value(); // [cite: 545]
      if(request->hasParam("password", true)) p = request->getParam("password", true)->value(); // [cite: 545]

      // Validación estricta contra las credenciales del sistema embebido [cite: 545]
      if(u == "admin" && p == "blasco") {
          // Generación pseudo-aleatoria del token de sesión único mezclado con el uptime de CPU [cite: 545]
          tokenSesionActiva = String(random(100000, 999999)) + String(millis()); // [cite: 545]
          
          AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", ""); // Creación de respuesta de redirección HTTP 302 [cite: 545]
          response->addHeader("Location", "/?auth=true"); // Redirige al monitor inyectando la flag de pestaña activa [cite: 545]
          
          // Inyección de la cookie de sesión protegida con flags modernas para mitigar ataques de secuestro (XSS/CSRF) 
          response->addHeader("Set-Cookie", "ZENITH_SESSION=" + tokenSesionActiva + "; Path=/; HttpOnly; SameSite=Strict"); // 
          request->send(response); // Despacha la respuesta al navegador 
      } else {
          request->redirect("/login-page?error=1"); // Redirección inmediata al formulario añadiendo la flag visual de error 
      }
  });

  // Ruta 3: Endpoint para la revocación del token de acceso de usuario
  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      tokenSesionActiva = ""; // Destrucción inmediata del token válido de la memoria RAM del ESP32 [cite: 547]
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", ""); // Respuesta de redirección [cite: 547]
      response->addHeader("Location", "/login-page"); // Envía al usuario de vuelta a la página de login [cite: 547]
      
      // Fuerza la caducidad inmediata de la cookie en el cliente sobreescribiéndola con una fecha del año 1970 [cite: 547]
      response->addHeader("Set-Cookie", "ZENITH_SESSION=deleted; Path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT"); // [cite: 547]
      request->send(response); // Ejecuta el envío [cite: 547]
  });

  // Ruta 4: Raíz del servidor - Dashboard de control principal (Protegido)
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; } // Guardia de seguridad [cite: 548]
    request->send_P(200, "text/html", index_html); // Envía la interfaz principal grabada en PROGMEM [cite: 548]
  });

  // Ruta 5: Endpoint del panel de visor histórico de la base de datos (Protegido)
  server->on("/db", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; } // Guardia de seguridad [cite: 549]
    request->send_P(200, "text/html", db_html); // Envía la interfaz del visor CSV grabada en PROGMEM [cite: 549]
  });

  // Ruta 6: Descarga directa del archivo de registros físicos CSV acumulado (Protegido)
  server->on("/datos.csv", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; } // Bloqueo de intrusos 
    request->send(LittleFS, "/datos.csv", "text/csv"); // Hace streaming directo del archivo desde el chip de memoria física 
  });

  // Ruta 7: Endpoint administrativo para el borrado total de la BBDD (Protegido)
  server->on("/delete-db", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; } // Bloqueo de intrusos 
      
      // Verificamos si el archivo de datos existe físicamente en el volumen de LittleFS 
      if (LittleFS.exists("/datos.csv")) {
          LittleFS.remove("/datos.csv"); // Ejecuta el borrado del archivo liberando los bloques ocupados en la Flash 
          Serial.println("🧹 [SISTEMA] Base de datos borrada desde la web."); // Registra la acción por el puerto serial físico 
          request->send(200, "text/plain", "OK"); // Devuelve confirmación HTTP al frontend 
      } else {
          request->send(404, "text/plain", "Archivo no encontrado"); // Retorna error si el archivo ya fue eliminado 
      }
  });

  // Vinculación formal de eventos e inyección del WebSocket dentro del núcleo del servidor web asíncrono
  ws->onEvent(onWsEvent); // Acopla el callback de eventos [cite: 552]
  server->addHandler(ws); // Registra el WebSocket como un manejador de tramas activo [cite: 552]
  server->begin();        // Abre los sockets y pone al servidor a escuchar peticiones entrantes [cite: 552]
}