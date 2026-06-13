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
#include "certs.h"
#include "web_server.h"
#include "web_pages.h"
#include "readmeai_content.h"
#include "terminal.h"
#include "menus.h"
#include "utils.h"
#include "sd_card.h"
#include <time.h>
#include <nvs_flash.h>
#include <nvs.h>

// ============================================================================
// VALIDACIÓN DE SESIÓN POR COOKIE DE SEGURIDAD
// ============================================================================
bool estaLogueado(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    if (sesion.token != "" && cookie.indexOf("ZENITH_SESSION=" + sesion.token) != -1) {
      // [FIX] Verificar expiración del token (30 min de inactividad)
      if ((millis() - sesion.inicio) > SESSION_TIMEOUT_MS) {
        sesion.token = "";
        sesion.inicio = 0;
        return false;
      }
      // Renovar timestamp en cada request autenticado (sliding window)
      sesion.inicio = millis();
      return true;
    }
  }
  return false;
}

// ============================================================================
// DETECCIÓN DE NAVEGADOR DIRECTO — BLOQUEO DE API DESDE EL NAVEGADOR
// ============================================================================
bool esNavegadorDirecto(AsyncWebServerRequest *request) {
  // [FIX] Los endpoints /api/ nunca deben bloquearse por Accept header —
  // son llamados por fetch() del propio JS de la página y necesitan responder JSON.
  // El check solo tiene sentido para rutas que sirven HTML (evitar acceso directo).
  if (request->url().startsWith("/api/")) return false;
  if (request->hasHeader("Accept")) {
    String accept = request->header("Accept");
    if (accept.indexOf("text/html") != -1) return true;
  }
  return false;
}

// ============================================================================
// MANEJADOR DE EVENTOS ASÍNCRONOS DEL TÚNEL WEBSOCKET
// ============================================================================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) { 
  if (type == WS_EVT_CONNECT) {
    ws->cleanupClients(sistemaWeb.maxClientes);
    if (ws->count() > sistemaWeb.maxClientes) { 
        client->close();
        return;
    }

    // ── TELEMETRÍA INSTANTÁNEA ──────────────────────────────────────────────
    // Enviar telemetría al cliente recién conectado SIN esperar el ciclo de
    // Core 0 (que puede tardar hasta 3 s). sistemaWeb.forzarTelemetria sigue activo como
    // respaldo por si este envío falla.
    sistemaWeb.forzarTelemetria = true;
    {
      int cargaC0 = calcularUsoCPU(0);
      int cargaC1 = calcularUsoCPU(1);
      uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize();
      float ramPct = (ESP.getFreePsram() > 0)
          ? 100.0f - (ESP.getFreePsram() * 100.0f / ramTotal)
          : 100.0f - (ESP.getFreeHeap() * 100.0f / ESP.getHeapSize());
      float flashTotal = (float)LittleFS.totalBytes();
      float flashUsado = (float)LittleFS.usedBytes();

      StaticJsonDocument<320> tel;
      tel["type"]    = "telemetry";
      tel["temp"]    = temperatureRead();
      tel["cpu"]     = (cargaC0 + cargaC1) / 2;
      tel["c0"]      = cargaC0;
      tel["c1"]      = cargaC1;
      tel["ram"]     = (int)ramPct;
      tel["flash"]   = (flashTotal > 0) ? round((flashUsado * 100.0f / flashTotal) * 100.0f) / 100.0f : 0.0f;
      tel["wifi"]    = WiFi.RSSI();
      tel["uptime"]  = obtenerUptime();
      tel["dhtTemp"] = sensores.temperatura;
      tel["dhtHum"]  = sensores.humedad;
      String telOut;
      serializeJson(tel, telOut);
      client->text(telOut);
    }

    // ── CSV INSTANTÁNEO ─────────────────────────────────────────────────────
    // Enviar historial CSV al cliente recién conectado para que la gráfica y
    // la tabla de la DB se pueblen al instante sin fetch HTTP adicional.
    {
      String csv;
      #ifdef SD_CS_PIN
      if (sdDisponible && existeCSV_SD()) {
        csv = leerCSV_SD();
      } else
      #endif
      if (LittleFS.exists("/datos.csv")) {
        File f = LittleFS.open("/datos.csv", "r");
        if (f) {
          const int MAX_LINEAS = 500;
          int totalLineas = 0;
          while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) totalLineas++;
          }
          f.seek(0);
          int saltar = totalLineas - MAX_LINEAS;
          if (saltar < 0) saltar = 0;
          int idx = 0;
          while (f.available() && idx < saltar) {
            f.readStringUntil('\n');
            idx++;
          }
          while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) csv += line + "\n";
          }
          f.close();
        }
      }
      if (csv.length() > 0) {
          // Construir JSON manualmente igual que el túnel — ArduinoJson no
          // puede alojar el CSV completo en stack sin heap dinámico.
          String csvMsg = "{\"type\":\"csv_data\",\"text\":\"";
          csvMsg += jsonStr(csv);  // escapa comillas, barras y saltos
          csvMsg += "\"}";
          client->text(csvMsg);
      }
    }

    // ── SEGURIDAD: estado actual en tiempo real ──────────────────────────────
    // Enviar el estado de seguridad al cliente recién conectado. Si hay una
    // alerta activa, el cliente mostrará el toast inmediatamente sin esperar
    // el próximo ciclo del loop de seguridad (puede tardar hasta 3s).
    if (seguridad.modo) {
      char buf[800];
      snprintf(buf, sizeof(buf),
          "{\"type\":\"security_alert\",\"habilitado\":true,\"nivel\":%d,\"manual\":%s,\"causa\":\"%s\",\"medidas\":%s,\"hora\":\"%s\"}",
          seguridad.nivel,
          seguridad.manual ? "true" : "false",
          jsonStr(seguridad.causa).c_str(),
          seguridad.medidas.c_str(),
          jsonStr(obtenerFechaHora()).c_str());
      client->text(buf);
    }
  }
  else if (type == WS_EVT_DISCONNECT) {
    ws->cleanupClients(); // Liberar el slot del cliente que acaba de irse
  }
  else if (type == WS_EVT_DATA) { 
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) { 
      char* cmd = (char*)malloc(len + 1);
      if (!cmd) return;
      memcpy(cmd, data, len);
      cmd[len] = 0;
      
      char* end = cmd + len - 1;
      while (end > cmd && (*end == ' ' || *end == '\n' || *end == '\r')) *end-- = 0;
      if (strcmp(cmd, "reboot") == 0) {
          free(cmd);
          ESP.restart();
      } else if (strcmp(cmd, "__ready__") == 0) {
          free(cmd);
          sistemaWeb.permitirLog = true;
          sistemaWeb.pedirMenu = true; // El loop de Core0 mandará el menú en el próximo ciclo
      } else {
          if (xQueueSend(cmdQueue, &cmd, 0) != pdTRUE) {
              free(cmd);
          }
      }
    }
  }
}

// ============================================================================
// CONFIGURACIÓN DE RUTAS DEL SERVIDOR HTTP
// ============================================================================
void iniciarServidorWeb() {       
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1);
  tzset();

  server->on("/login-page", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/html", login_html);
  });

  server->on("/login", HTTP_POST, [](AsyncWebServerRequest *request){
      // ── Rate limiting: 5 fallos en 60s → bloqueo 5 minutos ──
      static int loginFailCount = 0;
      static unsigned long windowStart = 0;
      static unsigned long blockUntil = 0;
      unsigned long now = millis();

      if (blockUntil > 0) {
        if (now < blockUntil) {
          request->send(429, "text/plain", "Demasiados intentos. Espere 5 minutos.");
          return;
        }
        blockUntil = 0;
        loginFailCount = 0;
        windowStart = 0;
      }

      String u = "", p = "";
      if(request->hasParam("user", true)) u = request->getParam("user", true)->value(); 
      if(request->hasParam("password", true)) p = request->getParam("password", true)->value(); 

      if(u == webUser && p == webPass) {
          loginFailCount = 0;
          windowStart = 0;
          blockUntil = 0;

          sesion.token = String(esp_random(), HEX) + String(esp_random(), HEX);
          sesion.inicio = millis();
          
          AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
          response->addHeader("Location", "/dashboard?auth=true");
          #ifdef HTTPS_ENABLED
          response->addHeader("Set-Cookie", "ZENITH_SESSION=" + sesion.token + "; Path=/; Secure; HttpOnly; SameSite=Strict");
          #else
          response->addHeader("Set-Cookie", "ZENITH_SESSION=" + sesion.token + "; Path=/; HttpOnly; SameSite=Strict");
          #endif
          request->send(response); 
      } else {
          if (loginFailCount == 0) windowStart = now;
          loginFailCount++;
          if (loginFailCount >= 5 && (now - windowStart) <= 60000) {
            blockUntil = now + 300000;
          } else if (loginFailCount >= 5) {
            loginFailCount = 1;
            windowStart = now;
          }
          request->redirect("/login-page?error=1");
      }
  });

  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      sesion.token = "";
      sesion.inicio = 0;  // [FIX] Limpiar timestamp de sesión
      AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
      response->addHeader("Location", "/login-page");
      response->addHeader("Set-Cookie", "ZENITH_SESSION=deleted; Path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT"); 
      request->send(response);
  });

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if (modoConfiguracion && !conexionExitosa) {
      request->send_P(200, "text/html", wifi_setup_html);
      return;
    }
    request->send_P(200, "text/html", landing_html);
  });

  server->on("/.readmeAI", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; }
    request->send_P(200, "text/plain; charset=utf-8", readmeai_content);
  });

  server->on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; }
    request->send_P(200, "text/html", index_html);
  });

  server->on("/db", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; }
    request->send_P(200, "text/html", db_html);
  });

  // ── /config (panel de configuración del sistema)
  server->on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    if(!estaLogueado(request)) { request->redirect("/login-page"); return; }
    request->send_P(200, "text/html", config_html);
  });

  server->on("/datos.csv", HTTP_GET, [](AsyncWebServerRequest *request){ 
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; }
    String path = "/datos.csv";
    if (path.indexOf("..") != -1) { request->send(400, "text/plain", "Ruta invalida"); return; }
    #ifdef SD_CS_PIN
    if (sdDisponible && existeCSV_SD()) {
      request->send(SD, path, "text/csv");
      return;
    }
    #endif
    request->send(LittleFS, path, "text/csv");
  });

  server->on("/api/csv", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
    int page = request->hasParam("page") ? request->getParam("page")->value().toInt() : 0;
    int size = request->hasParam("size") ? request->getParam("size")->value().toInt() : 100;
    if (size < 1) size = 1;
    if (size > 500) size = 500;

    // Dos pasadas: primera cuenta líneas, segunda lee desde el offset con prepend
    // para entregar newest-first sin vector<String> en heap.
    auto leerPagina = [&](fs::FS &fs) -> bool {
      File f = fs.open("/datos.csv", FILE_READ);
      if (!f) return false;

      // Primera pasada: contar líneas
      int totalLines = 0;
      while (f.available()) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) totalLines++;
      }

      // Calcular offset de inicio
      int startLine = totalLines - (page + 1) * size;
      if (startLine < 0) startLine = 0;
      int count = min(size, totalLines - startLine);

      // Segunda pasada: saltar hasta startLine
      f.seek(0);
      int idx = 0;
      while (f.available() && idx < startLine) {
        f.readStringUntil('\n');
        idx++;
      }

      // Leer líneas con prepend para newest-first
      String chunk;
      chunk.reserve((size_t)count * 100);
      int lin = 0;
      while (f.available() && lin < count) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
          chunk = line + "\n" + chunk;
          lin++;
        }
      }
      f.close();

      AsyncWebServerResponse *resp = request->beginResponse(200, "text/csv", chunk);
      resp->addHeader("X-Total-Lines", String(totalLines));
      request->send(resp);
      return true;
    };

  #ifdef SD_CS_PIN
    if (sdDisponible && existeCSV_SD()) {
      if (leerPagina(SD)) return;
    }
  #endif
    if (!LittleFS.exists("/datos.csv")) { request->send(404, "text/plain", "No data"); return; }
    leerPagina(LittleFS);
  });

  server->on("/delete-db", HTTP_GET, [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; }
      String path = "/datos.csv";
      if (path.indexOf("..") != -1) { request->send(400, "text/plain", "Ruta invalida"); return; }
      
      if (sdDisponible && existeCSV_SD()) {
          eliminarCSV_SD();
          Serial.println("🧹 [SISTEMA] Base de datos borrada desde la web (SD).");
          request->send(200, "text/plain", "OK");
      } else if (LittleFS.exists("/datos.csv")) {
          LittleFS.remove("/datos.csv");
          Serial.println("🧹 [SISTEMA] Base de datos borrada desde la web.");
          request->send(200, "text/plain", "OK");
      } else {
          request->send(404, "text/plain", "Archivo no encontrado");
      }
  });

  server->on("/api/system/info", HTTP_GET, [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; }

      uint32_t heapLibre = ESP.getFreeHeap();
      uint32_t heapTotal = ESP.getHeapSize();
      float heapPct = heapTotal > 0 ? (1.0f - (float)heapLibre / heapTotal) * 100.0f : 0;

      float flashTotal = (float)LittleFS.totalBytes();
      float flashUsado = (float)LittleFS.usedBytes();
      float flashPct   = flashTotal > 0 ? (flashUsado / flashTotal) * 100.0f : 0;

      StaticJsonDocument<256> doc;
      doc["chip"] = "ESP32-S3";
      doc["ip"] = WiFi.localIP().toString();
      doc["flash_kb"] = (int)(flashTotal / 1024);
      doc["flash_pct"] = flashPct;
      doc["psram_kb"] = (int)(ESP.getPsramSize() / 1024);
      doc["heap_pct"] = heapPct;
      doc["uptime"] = obtenerUptime();
      doc["fs_usado_kb"] = (int)(flashUsado / 1024);

      String out;
      serializeJson(doc, out);
      request->send(200, "application/json", out);
  });

  // ── GET /api/config/info → IP, SSID, RSSI, pines actuales, usuario web
  server->on("/api/config/info", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
    StaticJsonDocument<256> doc;
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["user"] = webUser;
    doc["pins"]["nfcRst"] = RST_PIN;
    doc["pins"]["nfcSs"] = SS_PIN;
    doc["pins"]["trigPin"] = TRIG_PIN;
    doc["pins"]["echoPin"] = ECHO_PIN;
    doc["pins"]["dhtPin"] = DHT_PIN;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // ── POST /api/config/webcred → cambia usuario y contraseña web (valida pass actual)
  server->on("/api/config/webcred", HTTP_POST,
    [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
      String* body = (String*)request->_tempObject;
      if(!body) { request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cuerpo vacio\"}"); return; }
      StaticJsonDocument<512> doc;
      if(deserializeJson(doc, *body)) { delete body; request->send(400, "application/json", "{\"status\":\"error\"}"); return; }
      delete body;
      const char* user    = doc["user"];
      const char* curPass = doc["currentPass"];
      const char* newPass = doc["newPass"];
      if(!user || !curPass || !newPass) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Faltan campos\"}");
        return;
      }
      if(webPass != String(curPass)) {
        request->send(403, "application/json", "{\"status\":\"error\",\"message\":\"Contrasena actual incorrecta\"}");
        return;
      }
      guardarCredencialesWeb(String(user), String(newPass));
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    },
    nullptr,
    [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t idx, size_t total){
      if(idx == 0) r->_tempObject = new String((const char*)data, len);
      else if(r->_tempObject) ((String*)r->_tempObject)->concat((const char*)data, len);
    }
  );

  // ── POST /api/config/pins → guarda mapeo de pines GPIO en NVS
  server->on("/api/config/pins", HTTP_POST,
    [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
      String* body = (String*)request->_tempObject;
      if(!body) { request->send(400, "application/json", "{\"status\":\"error\"}"); return; }
      StaticJsonDocument<256> doc;
      if(deserializeJson(doc, *body)) { delete body; request->send(400, "application/json", "{\"status\":\"error\"}"); return; }
      delete body;
      int nfcRst  = doc["nfcRst"]  | RST_PIN;
      int nfcSs   = doc["nfcSs"]   | SS_PIN;
      int trigPin = doc["trigPin"] | TRIG_PIN;
      int echoPin = doc["echoPin"] | ECHO_PIN;
      int dhtPin  = doc["dhtPin"]  | DHT_PIN;
      guardarConfigHardware(nfcRst, nfcSs, trigPin, echoPin, dhtPin);
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    },
    nullptr,
    [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t idx, size_t total){
      if(idx == 0) r->_tempObject = new String((const char*)data, len);
      else if(r->_tempObject) ((String*)r->_tempObject)->concat((const char*)data, len);
    }
  );

  // ── GET /api/ai/key → devuelve la API Key de Gemini desde NVS al JS del dashboard
  // [SEC] Bloqueado desde navegador directo (esNavegadorDirecto). Requiere cookie de sesión.
  // No-store para evitar que proxies o caches intermedios guarden la respuesta.
  // La key nunca va a localStorage — vive en RAM (variable aiKey) y se descarta al cerrar el tab.
  server->on("/api/ai/key", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
    String key = cargarClaveAI();
    StaticJsonDocument<512> doc;
    doc["key"] = key;
    doc["configured"] = (key.length() > 0);
    String out;
    serializeJson(doc, out);
    AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", out);
    resp->addHeader("Cache-Control", "no-store, no-cache, must-revalidate");
    request->send(resp);
  });

  // ── POST /api/ai/key → guarda la API Key de Gemini en NVS
  server->on("/api/ai/key", HTTP_POST,
    [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
      String* body = (String*)request->_tempObject;
      if(!body) { request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cuerpo vacio\"}"); return; }
      StaticJsonDocument<512> doc;
      if(deserializeJson(doc, *body)) { delete body; request->send(400, "application/json", "{\"status\":\"error\"}"); return; }
      delete body;
      const char* key = doc["key"];
      if(!key) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Falta key\"}");
        return;
      }
      guardarClaveAI(String(key));
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    },
    nullptr,
    [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t idx, size_t total){
      if(idx == 0) r->_tempObject = new String((const char*)data, len);
      else if(r->_tempObject) ((String*)r->_tempObject)->concat((const char*)data, len);
    }
  );

  // ==========================================================================
  // ENDPOINTS SISTEMA DE SEGURIDAD
  // ==========================================================================

  // ── GET /api/security/config → estado actual del sistema de seguridad
  server->on("/api/security/config", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
    StaticJsonDocument<256> doc;
    doc["habilitado"]   = seguridad.habilitado;
    doc["modoAlerta"]   = seguridad.modo;
    doc["causa"]        = seguridad.causa;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // ── POST /api/security/config → activa/desactiva el sistema de seguridad
  server->on("/api/security/config", HTTP_POST,
    [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
      String* body = (String*)request->_tempObject;
      if(!body) { request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cuerpo vacio\"}"); return; }
      StaticJsonDocument<256> doc;
      if(deserializeJson(doc, *body)) { delete body; request->send(400, "application/json", "{\"status\":\"error\"}"); return; }
      delete body;
      // [FIX] El default era |true — si el cliente omitía el campo,
      // el sistema de seguridad se activaba silenciosamente. Ahora
      // el campo es obligatorio y al desactivar se limpia el estado.
      if (!doc.containsKey("habilitado")) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Falta campo habilitado\"}");
        return;
      }
      bool habilitado = doc["habilitado"].as<bool>();
      seguridad.habilitado = habilitado;
      if (!habilitado) {
        seguridad.manual = false;
        seguridad.modo   = false;
        seguridad.causa  = "";
      }
      LOG_I("SEGURIDAD", "Sistema %s por usuario", habilitado ? "ACTIVADO" : "DESACTIVADO");
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    },
    nullptr,
    [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t idx, size_t total){
      if(idx == 0) r->_tempObject = new String((const char*)data, len);
      else if(r->_tempObject) ((String*)r->_tempObject)->concat((const char*)data, len);
    }
  );

  // ── GET /api/security/status → estado en tiempo real (para polling ligero)
  server->on("/api/security/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
    StaticJsonDocument<256> doc;
    doc["habilitado"] = seguridad.habilitado;
    doc["modoAlerta"] = seguridad.modo;
    doc["nivel"]      = seguridad.modo ? seguridad.nivel : 0;
    doc["manual"]     = seguridad.manual;
    doc["causa"]      = seguridad.modo ? seguridad.causa : "";
    if (seguridad.modo && seguridad.medidas.length() > 0) {
      doc["medidas"] = serialized(seguridad.medidas);
    } else {
      doc["medidas"] = serialized("[]");
    }
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // ── POST /api/security/manual → activa/desactiva la seguridad manualmente
  server->on("/api/security/manual", HTTP_POST,
    [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
      String* body = (String*)request->_tempObject;
      if(!body) { request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cuerpo vacio\"}"); return; }
      StaticJsonDocument<256> doc;
      if(deserializeJson(doc, *body)) { delete body; request->send(400, "application/json", "{\"status\":\"error\"}"); return; }
      delete body;
      bool activar = doc["activar"] | false;
      seguridad.manual = activar;
      if (!activar) {
        seguridad.modo = false;
        seguridad.causa = "";
      }
      LOG_I("SEGURIDAD", "Activacion manual %s por usuario", activar ? "ACTIVADA" : "DESACTIVADA");
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    },
    nullptr,
    [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t idx, size_t total){
      if(idx == 0) r->_tempObject = new String((const char*)data, len);
      else if(r->_tempObject) ((String*)r->_tempObject)->concat((const char*)data, len);
    }
  );

  // ==========================================================================
  // ENDPOINTS TÚNEL CLOUDFLARE WORKER
  // ==========================================================================

  // ── GET /api/tunnel/config → devuelve host, path, token y estado del túnel
  server->on("/api/tunnel/config", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
    String host, path, token;
    bool dummy = false;
    cargarConfigCFWorker(host, path, token, dummy);
    StaticJsonDocument<384> doc;
    doc["host"]       = host;
    doc["path"]       = path.length() > 0 ? path : "/esp-tunnel";
    doc["token"]      = token;
    doc["habilitado"] = tunel.habilitado;
    doc["conectado"]  = tunel.conectado;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // ── POST /api/tunnel/config → guarda host/path/token en NVS y activa/desactiva el túnel
  server->on("/api/tunnel/config", HTTP_POST,
    [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
      String* body = (String*)request->_tempObject;
      if(!body) { request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cuerpo vacio\"}"); return; }
      StaticJsonDocument<512> doc;
      if(deserializeJson(doc, *body)) { delete body; request->send(400, "application/json", "{\"status\":\"error\"}"); return; }
      delete body;
      const char* host  = doc["host"];
      const char* path  = doc["path"]  | "/esp-tunnel";
      const char* token = doc["token"] | "";
      bool habilitado   = doc["habilitado"] | false;
      if(!host || strlen(host) == 0) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Falta host\"}");
        return;
      }
      guardarConfigCFWorker(String(host), String(path), String(token), habilitado);
      tunel.host       = String(host);
      tunel.path       = String(path);
      tunel.token      = String(token);
      tunel.habilitado = habilitado;
      tunel.conectado  = false;
      tunel.ws.disconnect();
      if (habilitado) {
        WiFi.setSleep(false);
        tunel.ws.beginSSL(tunel.host.c_str(), 443, tunel.path.c_str());
        tunel.ws.setReconnectInterval(5000);
      }
      request->send(200, "application/json", "{\"status\":\"ok\"}");
    },
    nullptr,
    [](AsyncWebServerRequest *r, uint8_t *data, size_t len, size_t idx, size_t total){
      if(idx == 0) r->_tempObject = new String((const char*)data, len);
      else if(r->_tempObject) ((String*)r->_tempObject)->concat((const char*)data, len);
    }
  );

  // ── GET /api/tunnel/status → estado de conexión en tiempo real
  server->on("/api/tunnel/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    if(!estaLogueado(request)) { request->send(401, "application/json", "{\"error\":\"unauth\"}"); return; }
    StaticJsonDocument<128> doc;
    doc["habilitado"] = tunel.habilitado;
    doc["conectado"]  = tunel.conectado;
    doc["host"]       = tunel.host;
    String out;
    serializeJson(doc, out);
    request->send(200, "application/json", out);
  });

  // ==========================================================================
  // ENDPOINTS DE CONFIGURACIÓN INICIAL WIFI (Captive Portal)
  // ==========================================================================

  server->on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    static String ultimoScan = "[]";
    int n = WiFi.scanComplete();

    if (n >= 0) {
      String resultado = "[";
      for (int i = 0; i < n; i++) {
        if (i > 0) resultado += ",";
        String ssid = WiFi.SSID(i);
        ssid.replace("\\", "\\\\");
        ssid.replace("\"", "\\\"");
        resultado += "{\"ssid\":\"" + ssid + "\",";
        resultado += "\"rssi\":"  + String(WiFi.RSSI(i)) + ",";
        resultado += "\"encryption\":" + String(WiFi.encryptionType(i)) + "}";
      }
      resultado += "]";
      ultimoScan = resultado;
      WiFi.scanDelete();
      WiFi.scanNetworks(true, true); // async=true, show_hidden=true (detecta hotspots)
    } else if (n == WIFI_SCAN_FAILED) {
      WiFi.scanNetworks(true, true); // async=true, show_hidden=true (detecta hotspots)
    }
    request->send(200, "application/json", ultimoScan);
  });

  server->on("/api/wifi/configure", HTTP_POST,
    [](AsyncWebServerRequest *request){
      if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
      String* body = (String*)request->_tempObject;
      if (!body || body->length() == 0) {
        if (body) delete body;
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cuerpo vacio\"}");
        return;
      }

      // [FIX] StaticJsonDocument en lugar de DynamicJsonDocument — evita fragmentación de heap
      // en ESP32 con alta actividad. 1KB es suficiente para SSID+pass+5 pines+credenciales web.
      StaticJsonDocument<1024> doc;
      DeserializationError err = deserializeJson(doc, *body);
      delete body;
      if (err) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"JSON invalido\"}");
        return;
      }

      const char* ssid = doc["ssid"];
      const char* password = doc["password"];

      if (!ssid || strlen(ssid) == 0) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"SSID requerido\"}");
        return;
      }
      if (!password) password = "";

      guardarCredenciales(ssid, password);
      guardarSetupCompletado();

      if (doc.containsKey("nfcRst") || doc.containsKey("nfcSs") ||
          doc.containsKey("trigPin") || doc.containsKey("echoPin") || doc.containsKey("dhtPin")) {
        int nfcRst  = doc["nfcRst"]  | RST_PIN;
        int nfcSs   = doc["nfcSs"]   | SS_PIN;
        int trigPin = doc["trigPin"] | TRIG_PIN;
        int echoPin = doc["echoPin"] | ECHO_PIN;
        int dhtPin  = doc["dhtPin"]  | DHT_PIN;
        guardarConfigHardware(nfcRst, nfcSs, trigPin, echoPin, dhtPin);
      }

      if (doc.containsKey("webUser") && doc.containsKey("webPass")) {
        const char* wu = doc["webUser"];
        const char* wp = doc["webPass"];
        if (strlen(wu) > 0 && strlen(wp) > 0) {
          guardarCredencialesWeb(String(wu), String(wp));
        }
      }

      conexionEnProgreso = true;
      conexionExitosa = false;
      conexionIP = "";
      inicioConexion = millis();
      WiFi.begin(ssid, password);

      Serial.printf("[CONFIG] Conectando a %s...\n", ssid);
      request->send(200, "application/json", "{\"status\":\"ok\"}");
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

  server->on("/api/wifi/status", HTTP_GET, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    String json;
    if (!conexionEnProgreso && conexionExitosa) {
      json = "{\"status\":\"connected\",\"ip\":\"" + conexionIP + "\"}";
    } else if (conexionEnProgreso && WiFi.status() == WL_CONNECTED) {
      conexionExitosa = true;
      conexionIP = WiFi.localIP().toString();
      conexionEnProgreso = false;
      modoConfiguracion = false;
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      dnsServer.stop();
      Serial.printf("[CONFIG] Conectado! IP: %s. AP desactivado.\n", conexionIP.c_str());
      json = "{\"status\":\"connected\",\"ip\":\"" + conexionIP + "\"}";
    } else if (conexionEnProgreso) {
      unsigned long elapsed = millis() - inicioConexion;
      int pct = constrain(elapsed * 100 / 15000, 5, 99);
      if (elapsed > 15000) {
        json = "{\"status\":\"failed\",\"message\":\"Tiempo de espera agotado (15s)\"}";
        conexionEnProgreso = false;
      } else {
        String msg = (elapsed < 5000) ? "Conectando..." : "Esperando respuesta del router...";
        json = "{\"status\":\"connecting\",\"progress\":" + String(pct) + ",\"message\":\"" + msg + "\"}";
      }
    } else {
      json = "{\"status\":\"idle\"}";
    }
    request->send(200, "application/json", json);
  });

  server->on("/api/wifi/reset", HTTP_POST, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    Serial.println("[CONFIG] Reset de credenciales solicitado.");
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    borrarCredenciales();
    delay(500);
    ESP.restart();
  });

  server->on("/api/wifi/reboot", HTTP_POST, [](AsyncWebServerRequest *request){
    if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    Serial.println("[CONFIG] Reiniciando ESP32 para aplicar configuración WiFi...");
    delay(500);
    ESP.restart();
  });

  server->on("/setup", HTTP_GET, [](AsyncWebServerRequest *request){
    if (modoConfiguracion) {
      request->send_P(200, "text/html", wifi_setup_html);
    } else {
      request->redirect("/");
    }
  });

  // [FIX] Eliminado onNotFound duplicado — ESPAsyncWebServer solo mantiene el último registrado.
  // La lógica del captive portal se fusionó en el único onNotFound al final de iniciarServidorWeb().

  server->on("/import-csv", HTTP_POST,
      [](AsyncWebServerRequest *request){
          if(esNavegadorDirecto(request)) { request->redirect("/"); return; }
          if(!estaLogueado(request)) { request->send(401, "text/plain", "Acceso Denegado"); return; }

          String path = "/datos.csv";
          if (path.indexOf("..") != -1) { request->send(400, "text/plain", "Ruta invalida"); return; }

          String* csvBody = (String*) request->_tempObject;
          if (!csvBody || csvBody->length() == 0) {
              if (csvBody) delete csvBody;
              request->send(400, "text/plain", "Cuerpo vacio");
              return;
          }

          size_t len = csvBody->length();
          #ifdef SD_CS_PIN
          if (sdDisponible) {
              File file = SD.open(path, FILE_WRITE);
              if (!file) {
                  delete csvBody;
                  request->send(500, "text/plain", "Error al abrir el archivo en SD");
                  return;
              }
              file.print(*csvBody);
              file.close();
          } else {
          #endif
              File file = LittleFS.open(path, "w");
              if (!file) {
                  delete csvBody;
                  request->send(500, "text/plain", "Error al abrir el archivo");
                  return;
              }
              file.print(*csvBody);
              file.close();
          #ifdef SD_CS_PIN
          }
          #endif
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

  ws->onEvent(onWsEvent);
  server->addHandler(ws);

  // [FIX] Único onNotFound — combina captive portal y redirección general.
  // ESPAsyncWebServer solo preserva el último onNotFound registrado; tener dos
  // hace que el primero (captive portal) sea ignorado silenciosamente.
  server->onNotFound([](AsyncWebServerRequest *request) {
    if (modoConfiguracion && !conexionExitosa) {
      request->send_P(200, "text/html", wifi_setup_html);
    } else {
      request->redirect("/");
    }
  });

  server->begin();

  delay(200);
  WiFi.scanNetworks(true, true); // async=true, show_hidden=true (detecta hotspots)
  Serial.println("[WiFi] Scan de redes iniciado en background (incl. redes ocultas).");
  Serial.println("[OK] Servidor web iniciado en puerto 80.");
}