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
#include "terminal.h"
#include "menus.h"
#include "utils.h"
#include "sd_card.h"
#include <time.h>
#include <vector>
#include <algorithm>

// ============================================================================
// VALIDACIÓN DE SESIÓN POR COOKIE DE SEGURIDAD
// ============================================================================
bool estaLogueado(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    if (tokenSesionActiva != "" && cookie.indexOf("ZENITH_SESSION=" + tokenSesionActiva) != -1) {
      return true;
    }
  }
  return false;
}

// ============================================================================
// MANEJADOR DE EVENTOS ASÍNCRONOS DEL TÚNEL WEBSOCKET
// ============================================================================
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) { 
  if (type == WS_EVT_CONNECT) {   
    if (ws->count() >= MAX_WEBSOCKET_CLIENTS) { 
        client->close();
        return;
    }
    permitirWebLog = true;
    Terminal.println("\n[SISTEMA] Dispositivo conectado al SO Blasco mediante Web-Telnet.");
    mostrarMenuPrincipal();
    forzarTelemetria = true;
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
      String u = "", p = "";
      if(request->hasParam("user", true)) u = request->getParam("user", true)->value(); 
      if(request->hasParam("password", true)) p = request->getParam("password", true)->value(); 

      if(u == webUser && p == webPass) {
          // [FIX] Generación de token con TRNG de hardware del ESP32 (esp_random) en lugar
          // de random() de Arduino, cuyo LFSR sin semilla criptográfica es predecible.
          tokenSesionActiva = String(esp_random(), HEX) + String(esp_random(), HEX);
          
          AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
          response->addHeader("Location", "/?auth=true");
          response->addHeader("Set-Cookie", "ZENITH_SESSION=" + tokenSesionActiva + "; Path=/; HttpOnly; SameSite=Strict"); 
          request->send(response); 
      } else {
          request->redirect("/login-page?error=1");
      }
  });

  server->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request){
      tokenSesionActiva = "";
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

  server->on("/delete-db", HTTP_GET, [](AsyncWebServerRequest *request){
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

  // ==========================================================================
  // ENDPOINTS DE CONFIGURACIÓN INICIAL WIFI (Captive Portal)
  // ==========================================================================

  server->on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request){
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
      WiFi.scanNetworks(true);
    } else if (n == WIFI_SCAN_FAILED) {
      WiFi.scanNetworks(true);
    }
    request->send(200, "application/json", ultimoScan);
  });

  server->on("/api/wifi/configure", HTTP_POST,
    [](AsyncWebServerRequest *request){
      String* body = (String*)request->_tempObject;
      if (!body || body->length() == 0) {
        if (body) delete body;
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Cuerpo vacio\"}");
        return;
      }

      DynamicJsonDocument doc(1024);
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
    Serial.println("[CONFIG] Reset de credenciales solicitado.");
    request->send(200, "application/json", "{\"status\":\"ok\"}");
    borrarCredenciales();
    delay(500);
    ESP.restart();
  });

  server->on("/api/wifi/reboot", HTTP_POST, [](AsyncWebServerRequest *request){
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

  server->onNotFound([](AsyncWebServerRequest *request){
    if (modoConfiguracion && !conexionExitosa) {
      request->send_P(200, "text/html", wifi_setup_html);
    } else {
      request->redirect("/login-page");
    }
  });

  server->on("/import-csv", HTTP_POST,
      [](AsyncWebServerRequest *request){
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
  server->begin();

  delay(200);
  WiFi.scanNetworks(true);
  Serial.println("[WiFi] Scan de redes iniciado en background.");
  Serial.println("[OK] Servidor web iniciado en puerto 80.");
}