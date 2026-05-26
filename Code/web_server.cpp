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
#include "web_server.h"
#include "web_pages.h"
#include "web_pages.h"
#include "terminal.h"
#include "menus.h"
#include "utils.h"
#include <time.h>
#include <vector>
#include <algorithm>

// ============================================================================
// VALIDACIÓN DE SESIÓN POR COOKIE DE SEGURIDAD
// ============================================================================
bool estaLogueado(AsyncWebServerRequest *request) {
  // Comprobamos de forma estricta si la solicitud entrante contiene el header de Cookies
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie"); // Extraemos el string completo de cookies del cliente
    
    // Verificamos que el token del sistema no esté vacío y que la cookie contenga la clave ZENITH_SESSION vinculada a él
    if (tokenSesionActiva != "" && cookie.indexOf("ZENITH_SESSION=" + tokenSesionActiva) != -1) {
      return true; // Autenticación válida: El usuario tiene permiso de visualización
    }
  }
  return false; // Acceso denegado de forma por defecto si no supera la validación
}

// ============================================================================
// MANEJADOR DE EVENTOS ASÍNCRONOS DEL TÚNEL WEBSOCKET
// ============================================================================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) { 
  // Caso 1: Se ha conectado un nuevo cliente web a la URL del socket
  if (type == WS_EVT_CONNECT) {   
    // Protección anti-desbordamiento de memoria (OOM): Rechaza conexiones si superamos el límite configurado
    if (ws->count() >= MAX_WEBSOCKET_CLIENTS) { 
        client->close(); // Cierra el socket del cliente intruso de manera inmediata
        return;          // Aborta el procesamiento
    }
    // Activar logs web inmediatamente para que el menú se vea sin esperar 30s
    permitirWebLog = true;
    Terminal.println("\n[SISTEMA] Dispositivo conectado al SO Blasco mediante Web-Telnet."); // Inyecta log en los canales
    mostrarMenuPrincipal(); // Dibuja la consola del sistema de forma automática al usuario recién ingresado
    
    forzarTelemetria = true; // Activa el interruptor global para despachar el JSON de rendimiento sin esperar al ciclo de 5s
  } 
  // Caso 2: Se recibe una trama de datos desde el navegador web del usuario
  else if (type == WS_EVT_DATA) { 
    AwsFrameInfo *info = (AwsFrameInfo*)arg; // Mapeo de puntero a la estructura de control de fragmentación de red
    
    // Validamos que sea una trama de texto, que esté completa (final) y que el tamaño coincida con la longitud leída
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) { 
      data[len] = 0; // Inserción mandatoria del carácter nulo al final de la matriz para conversión segura a C-String
      entradaWeb = String((char*)data); // Conversión del búfer de bytes a cadena dinámica de Arduino
      entradaWeb.trim(); // Limpieza drástica de retornos de carro, tabuladores y espacios en blanco
      
      // Comando reservado del sistema para reinicio por software en caliente
      if (entradaWeb == "reboot") {
          ESP.restart(); // Ejecuta la interrupción nativa para resetear la placa base
      } else {
          hayEntradaWeb = true; // Eleva la bandera de sincronización para que el Core 1 procese la orden en su bucle
      }
    }
  }
}

// ============================================================================
// CONFIGURACIÓN DE RUTAS DEL SERVIDOR HTTP
// ============================================================================
void iniciarServidorWeb() {       
  // Forzar zona horaria España
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
  tzset();

  // NOTA: La limpieza de backups se hace en gestionarCopiasSeguridad() desde setup()
  // No duplicar aquí — gestionarCopiasSeguridad() ya limpia a max 10 en cada ejecución.

  // Ruta 1: Vista pública del formulario de validación de identidad
  server->on("/login-page", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/html", login_html); // Envía el HTML estático comprimido en la Flash 
  });

  // Ruta 2: Endpoint del método POST para el procesamiento de credenciales de acceso
  server->on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
      String u = "", p = ""; // Inicialización de variables locales temporales
      
      // Extracción segura de los parámetros enviados dentro del cuerpo del formulario
      if(request->hasParam("user", true)) u = request->getParam("user", true)->value(); 
      if(request->hasParam("password", true)) p = request->getParam("password", true)->value(); 

      // Validación estricta contra las credenciales del sistema embebido
      if(u == "admin" && p == "blasco") {
          // Generación pseudo-aleatoria del token de sesión único mezclado con el uptime de CPU
          tokenSesionActiva = String(random(100000, 999999)) + String(millis()); 
          
          AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", ""); // Creación de respuesta de redirección HTTP 302
          response->addHeader("Location", "/?auth=true"); // Redirige al monitor inyectando la flag de pestaña activa
          
          // Inyección de la cookie de sesión protegida con flags modernas para mitigar ataques de secuestro (XSS/CSRF) 
          response->addHeader("Set-Cookie", "ZENITH_SESSION=" + tokenSesionActiva + "; Path=/; HttpOnly; SameSite=Strict"); 
          request->send(response); // Despacha la respuesta al navegador 
      } else {
          request->redirect("/login-page?error=1"); // Redirección inmediata al formulario añadiendo la flag visual de error 
      }
  });

  // Ruta 3: Endpoint para la revocación del token de acceso de usuario
  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      tokenSesionActiva = ""; // Destrucción inmediata del token válido de la memoria RAM del ESP32
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", ""); // Respuesta de redirección
      response->addHeader("Location", "/login-page"); // Envía al usuario de vuelta a la página de login
      
      // Fuerza la caducidad inmediata de la cookie en el cliente sobreescribiéndola con una fecha del año 1970
      response->addHeader("Set-Cookie", "ZENITH_SESSION=deleted; Path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT"); 
      request->send(response); // Ejecuta el envío
  });

  // Ruta 4: Raíz del servidor - Dashboard de control principal (Protegido)
  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; } // Guardia de seguridad
    request->send_P(200, "text/html", index_html); // Envía la interfaz principal grabada en PROGMEM
  });

  // Ruta 5: Endpoint del panel de visor histórico de la base de datos (Protegido)
  server->on("/db", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; } // Guardia de seguridad
    request->send_P(200, "text/html", db_html); // Envía la interfaz del visor CSV grabada en PROGMEM
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

  // Ruta 8: Endpoint de información del sistema (Protegido)
  server->on("/api/system/info", HTTP_GET, [](AsyncWebServerRequest *request){
      if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; }

      String ip = WiFi.localIP().toString();
      uint32_t heapLibre = ESP.getFreeHeap();
      uint32_t heapTotal = ESP.getHeapSize();
      float heapPct = heapTotal > 0 ? (1.0f - (float)heapLibre / heapTotal) * 100.0f : 0;

      float flashTotal = (float)LittleFS.totalBytes();
      float flashUsado = (float)LittleFS.usedBytes();
      float flashPct   = flashTotal > 0 ? (flashUsado / flashTotal) * 100.0f : 0;

      String json = "{\"chip\":\"ESP32-S3\",\"ip\":\"" + ip + "\",";
      json += "\"flash_kb\":" + String((int)(flashTotal / 1024)) + ",";
      json += "\"flash_pct\":" + String(flashPct, 1) + ",";
      json += "\"psram_kb\":" + String(ESP.getPsramSize() / 1024) + ",";
      json += "\"heap_pct\":" + String(heapPct, 1) + ",";
      json += "\"uptime\":\"" + obtenerUptime() + "\",";
      json += "\"fs_usado_kb\":" + String((int)(flashUsado / 1024)) + "}";

      request->send(200, "application/json", json);
  });

  // Ruta 9: Importar un CSV que reemplaza /datos.csv y el sistema continúa guardando sobre él
  server->on("/import-csv", HTTP_POST,
      [](AsyncWebServerRequest *request){
          if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; }

          String* csvBody = (String*) request->_tempObject;
          if (!csvBody || csvBody->length() == 0) {
              if (csvBody) delete csvBody;
              request->send(400, "text/plain", "Cuerpo vacio");
              return;
          }

          File file = LittleFS.open("/datos.csv", "w");
          if (!file) {
              delete csvBody;
              request->send(500, "text/plain", "Error al abrir el archivo");
              return;
          }

          size_t len = csvBody->length();
          file.print(*csvBody);
          file.close();
          delete csvBody;

          Serial.printf("📥 [SISTEMA] CSV importado correctamente (%d bytes).\n", len);
          request->send(200, "text/plain", "OK");
      },
      nullptr,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
          if (index == 0) {
              request->_tempObject = new String((const char*)data, len);
          } else if (request->_tempObject) {
              ((String*)request->_tempObject)->concat((const char*)data, len);
          }
      }
  );

  // Vinculación del manejador de eventos WebSocket al objeto ws
  ws->onEvent(onWsEvent); // Registra la función de callbacks para conexión, datos y desconexión

  // Registro del WebSocket como manejador del servidor HTTP
  server->addHandler(ws); // Asocia el túnel WebSocket /ws al servidor principal

  // Arranque definitivo del servidor HTTP — sin esta línea el servidor no escucha peticiones
  server->begin(); // *** CRÍTICO: levanta el socket TCP en el puerto 80 ***
  Serial.println("[OK] Servidor web iniciado en puerto 80.");
}