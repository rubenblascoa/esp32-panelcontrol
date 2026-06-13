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
 * @file tareas.cpp
 * @brief Implementación de los lazos infinitos multitarea distribuidos por hardware en el ESP32-S3.
 */
#include "tareas.h"     // Vinculación de prototipos propios
#include "web_server.h" // Habilita la inicialización de los endpoints web 
#include "utils.h"      // Permite el acceso a los diagnósticos de CPU, base de datos y LCD 
#include "menus.h"      // Habilita la inyección de los menús visuales de terminal 

// [FIX B7] El estado de seguridad (FSM + midiendoDistancia + nfc.modo) se protege
// con segMutex. FSM se lee ANTES de tomar el mutex para evitar llamar xSemaphoreTake
// anidado (FSM_GET usa fsmMutex). Las variables volatile garantizan visibilidad en Core 1.
#include "nfc.h"        // Permite invocar las subrutinas de interrogación del bus SPI 
#include "ultrasonidos.h" // Habilita la captura de datos del transductor sónico 
#include "terminal.h"   // Habilita la impresión por el canal dual híbrido 
#include "dht.h"        // Habilita la inicialización y lectura del sensor DHT11 de temperatura y humedad
#include "modules.h"    // Registro de módulos (table-driven dispatch)
#include "sd_card.h"    // Habilita la comprobación de tamaño del CSV en SD
#include "web_pages.h"         // PROGMEM strings: login_html, index_html, config_html, db_html
#include "readmeai_content.h"  // PROGMEM string: readmeai_content (servido en /.readmeAI)

// ============================================================================
// TÚNEL CLOUDFLARE — COLA DE PETICIONES NO BLOQUEANTE
// ============================================================================
// El callback WS solo parsea y encola; el proxy HTTP se procesa en el loop
// principal de Core 0 con yields controlados. Sin bloqueos, sin watchdog.
// ============================================================================
struct TunnelReq {
  bool   pendiente = false;
  String id, method, path, body, hdrJson;
};
static TunnelReq cfReq;
// tunel.wsCount vive en config.cpp (extern en config.h)
static bool enviarDatosIniciales = false;

String jsonStr(const String& s) {
  String r;
  // Reservamos 2× la longitud de entrada: el peor caso (todos los chars escapables)
  // duplica el tamaño. Para CSV grandes esto evita múltiples realocaciones del String.
  r.reserve(s.length() * 2);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    switch (c) {
      case '"':  r += "\\\""; break;
      case '\\': r += "\\\\"; break;
      case '\n': r += "\\n";  break;
      case '\r': r += "\\r";  break;
      case '\t': r += "\\t";  break;
      default:
        if ((unsigned char)c < 0x20) { char b[8]; snprintf(b, sizeof(b), "\\u%04x", (unsigned char)c); r += b; }
        else r += c;
    }
  }
  return r;
}

// Helpers para servir peticiones del túnel directamente (sin TCP)
// Tamaño de cada chunk de body (12KB: reduce rondas WS para páginas HTML grandes ~91KB de ~23 a ~8 chunks)
static const size_t PROXY_CHUNK_SIZE = 12288;

// Envía la respuesta al Worker en chunks para evitar serializar JSON de cuerpos
// HTML grandes (91KB index_html → escapeado JSON sería ~182KB bloqueando ~2s).
// Protocolo:
//   1. "response_start" con status + headers
//   2. N × "response_chunk" con fragmentos base64 del body
//   3. "response_end" para indicar fin de stream
// Para cuerpos pequeños (<= PROXY_CHUNK_SIZE) se usa el camino rápido (1 mensaje).
static void enviarRespuestaProxy(int sc, const String& ct, const String& body, const String& extraHdrs) {
  // Construir objeto de headers reutilizable
  String hdrs = "\"Content-Type\":\"" + ct + "\"";
  if (extraHdrs.length() > 0) {
    int ls = 0;
    while (ls < (int)extraHdrs.length()) {
      int le = extraHdrs.indexOf('\n', ls);
      if (le == -1) le = extraHdrs.length();
      String h = extraHdrs.substring(ls, le);
      if (h.length() > 0 && h.charAt(h.length()-1) == '\r') h.remove(h.length()-1);
      int co = h.indexOf(':');
      if (co != -1) {
        String k = h.substring(0, co); k.trim();
        String v = h.substring(co + 1); v.trim();
        hdrs += ",\"" + jsonStr(k) + "\":\"" + jsonStr(v) + "\"";
      }
      ls = le + 1;
    }
  }

  const size_t bodyLen = body.length();

  // Camino rápido: cuerpos pequeños en un solo mensaje (JSON clásico, sin chunks)
  if (bodyLen <= PROXY_CHUNK_SIZE) {
    String out;
    out.reserve(512 + hdrs.length() + bodyLen * 2);
    out += "{\"type\":\"response\",\"id\":\"";
    out += jsonStr(cfReq.id);
    out += "\",\"status\":";
    out += String(sc);
    out += ",\"headers\":{";
    out += hdrs;
    out += "},\"body\":\"";
    out += jsonStr(body);
    out += "\"}";
    tunel.ws.sendTXT(out);
    return;
  }

  // Camino chunked: mandar header + chunks base64 + fin
  // 1. response_start (sin body)
  String start;
  start.reserve(512 + hdrs.length());
  start += "{\"type\":\"response_start\",\"id\":\"";
  start += jsonStr(cfReq.id);
  start += "\",\"status\":";
  start += String(sc);
  start += ",\"size\":";
  start += String(bodyLen);
  start += ",\"headers\":{";
  start += hdrs;
  start += "}}";
  tunel.ws.sendTXT(start);

  // 2. chunks base64 del body (base64 es ~33% más grande pero evita escaping char-by-char)
  static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  size_t offset = 0;
  while (offset < bodyLen) {
    size_t chunkLen = min(PROXY_CHUNK_SIZE, bodyLen - offset);
    // Base64 de chunkLen bytes → ceil(chunkLen/3)*4 chars
    size_t b64Len = ((chunkLen + 2) / 3) * 4;
    String b64;
    b64.reserve(b64Len);
    for (size_t i = 0; i < chunkLen; i += 3) {
      uint8_t a = (uint8_t)body.charAt(offset + i);
      uint8_t b = (i + 1 < chunkLen) ? (uint8_t)body.charAt(offset + i + 1) : 0;
      uint8_t c = (i + 2 < chunkLen) ? (uint8_t)body.charAt(offset + i + 2) : 0;
      b64 += b64chars[(a >> 2) & 0x3F];
      b64 += b64chars[((a << 4) | (b >> 4)) & 0x3F];
      b64 += (i + 1 < chunkLen) ? b64chars[((b << 2) | (c >> 6)) & 0x3F] : '=';
      b64 += (i + 2 < chunkLen) ? b64chars[c & 0x3F] : '=';
    }
    String msg;
    msg.reserve(120 + b64Len + cfReq.id.length() * 2);
    msg += "{\"type\":\"response_chunk\",\"id\":\"";
    msg += jsonStr(cfReq.id);
    msg += "\",\"data\":\"";
    msg += b64;
    msg += "\"}";
    tunel.ws.sendTXT(msg);
    offset += chunkLen;
    // Yield para no bloquear el watchdog entre chunks
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  // 3. response_end
  char endBuf[256];
  snprintf(endBuf, sizeof(endBuf),
      "{\"type\":\"response_end\",\"id\":\"%s\"}",
      jsonStr(cfReq.id).c_str());
  tunel.ws.sendTXT(endBuf);
}

static bool estaAuthedViaTunnel(const String& hdrJson) {
  if (sesion.token.length() == 0) return false;
  // [FIX] Verificar expiración igual que estaLogueado() en web_server.cpp
  if (sesion.inicio > 0 && (millis() - sesion.inicio) > SESSION_TIMEOUT_MS) {
    sesion.token = "";
    sesion.inicio = 0;
    return false;
  }
  // 1024 bytes: los headers del navegador incluyen cookie (hasta ~200 chars) + varios más
  StaticJsonDocument<1024> d;
  if (hdrJson.length() == 0 || deserializeJson(d, hdrJson)) return false;
  JsonObject o = d.as<JsonObject>();
  for (JsonPair kv : o) {
    String k = kv.key().c_str(); k.toLowerCase();
    if (k == "cookie" && strstr(kv.value().as<const char*>(), ("ZENITH_SESSION=" + sesion.token).c_str())) {
      sesion.inicio = millis();  // [FIX] Renovar sliding window en cada peticion autenticada
      return true;
    }
  }
  return false;
}

static String urlDecode(const String& s) {
  String r;
  for (size_t i = 0; i < s.length(); i++) {
    char c = s.charAt(i);
    if (c == '+') r += ' ';
    else if (c == '%' && i + 2 < s.length()) {
      char h[3] = { s.charAt(i+1), s.charAt(i+2), 0 };
      r += (char)strtol(h, NULL, 16);
      i += 2;
    } else r += c;
  }
  return r;
}

// ============================================================================
// PROXY HELPERS — cada función maneja un grupo de rutas
// ============================================================================

// Helper: serializa un StaticJsonDocument a String y lo envía como respuesta JSON.
template<size_t N>
static void proxyEnviarJson(int sc, StaticJsonDocument<N>& doc) {
  String out;
  serializeJson(doc, out);
  enviarRespuestaProxy(sc, "application/json", out, "");
}

// Rutas públicas (sin autenticación): landing, login, logout.
// Devuelve true si la petición fue manejada.
static bool proxy_servirPaginaPublica(const String& method, const String& path, const String& body) {
  if (method == "GET" && path == "/") {
    enviarRespuestaProxy(200, "text/html", FPSTR(landing_html), "");
    return true;
  }
  if (method == "GET" && (path == "/login-page" || path == "/login.html")) {
    enviarRespuestaProxy(200, "text/html", FPSTR(login_html), "");
    return true;
  }
  if (method == "GET" && path == "/logout") {
    sesion.token = "";
    String extra = "Location: /login-page\r\nSet-Cookie: ZENITH_SESSION=deleted; Path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT";
    enviarRespuestaProxy(302, "text/html", "", extra);
    return true;
  }
  if (method == "POST" && (path == "/login" || path == "/api/login")) {
    String u, p;
    int us = body.indexOf("user=");
    if (us != -1) {
      us += 5; int ue = body.indexOf('&', us);
      if (ue == -1) ue = body.length();
      u = urlDecode(body.substring(us, ue));
    }
    int ps = body.indexOf("password=");
    if (ps != -1) {
      ps += 9; int pe = body.indexOf('&', ps);
      if (pe == -1) pe = body.length();
      p = urlDecode(body.substring(ps, pe));
    }
    String extra;
    if (u == webUser && p == webPass) {
      sesion.token  = String(esp_random(), HEX) + String(esp_random(), HEX);
      sesion.inicio = millis();  // [FIX] Iniciar ventana de sesión
      extra = "Location: /dashboard?auth=true\r\nSet-Cookie: ZENITH_SESSION=" + sesion.token + "; Path=/; HttpOnly; SameSite=Strict";
      enviarRespuestaProxy(302, "text/html", "", extra);
    } else {
      extra = "Location: /login-page?error=1";
      enviarRespuestaProxy(302, "text/html", "", extra);
    }
    return true;
  }
  return false;
}

// Rutas HTML protegidas (requieren auth): dashboard, config, db, readmeAI.
// Devuelve true si la petición fue manejada.
static bool proxy_servirPaginaProtegida(const String& method, const String& path) {
  if (method != "GET") return false;
  if (path == "/dashboard") { enviarRespuestaProxy(200, "text/html", FPSTR(index_html),  ""); return true; }
  if (path == "/config")    { enviarRespuestaProxy(200, "text/html", FPSTR(config_html), ""); return true; }
  if (path == "/db")        { enviarRespuestaProxy(200, "text/html", FPSTR(db_html),     ""); return true; }
  if (path == "/.readmeAI") { enviarRespuestaProxy(200, "text/plain; charset=utf-8", FPSTR(readmeai_content), ""); return true; }
  return false;
}

// Endpoints de API (JSON): sistema, config, seguridad, túnel, wifi.
// Devuelve true si la petición fue manejada.
static bool proxy_servirAPI(const String& method, const String& path, const String& body) {
  if (method == "GET" && path == "/api/tunnel/status") {
    StaticJsonDocument<64> doc;
    doc["connected"]  = tunel.conectado;
    doc["habilitado"] = tunel.habilitado;
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "GET" && path == "/api/tunnel/config") {
    StaticJsonDocument<256> doc;
    doc["host"]       = tunel.host;
    doc["path"]       = tunel.path;
    doc["habilitado"] = tunel.habilitado;
    doc["token"]      = tunel.token;
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "GET" && path == "/api/system/info") {
    float flashTotal = (float)LittleFS.totalBytes();
    float flashUsado = (float)LittleFS.usedBytes();
    uint32_t heapLibre = ESP.getFreeHeap();
    uint32_t heapTotal = ESP.getHeapSize();
    StaticJsonDocument<256> doc;
    doc["chip"]        = "ESP32-S3";
    doc["ip"]          = WiFi.localIP().toString();
    doc["flash_kb"]    = (int)(flashTotal / 1024);
    doc["flash_pct"]   = flashTotal > 0 ? (flashUsado / flashTotal) * 100.0f : 0.0f;
    doc["psram_kb"]    = (int)(ESP.getPsramSize() / 1024);
    doc["heap_pct"]    = heapTotal > 0 ? (1.0f - (float)heapLibre / heapTotal) * 100.0f : 0.0f;
    doc["uptime"]      = obtenerUptime();
    doc["fs_usado_kb"] = (int)(flashUsado / 1024);
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "GET" && path == "/api/config/info") {
    StaticJsonDocument<256> doc;
    doc["ip"]   = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["user"] = webUser;
    doc["pins"]["nfcRst"]  = RST_PIN;
    doc["pins"]["nfcSs"]   = SS_PIN;
    doc["pins"]["trigPin"] = TRIG_PIN;
    doc["pins"]["echoPin"] = ECHO_PIN;
    doc["pins"]["dhtPin"]  = DHT_PIN;
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "GET" && path == "/api/ai/key") {
    StaticJsonDocument<512> doc;
    doc["key"] = cargarClaveAI();
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "POST" && path == "/api/ai/key") {
    StaticJsonDocument<512> jdoc;
    if (!deserializeJson(jdoc, body)) {
      const char* key = jdoc["key"];
      if (key) guardarClaveAI(String(key));
    }
    StaticJsonDocument<32> resp; resp["status"] = "ok";
    proxyEnviarJson(200, resp);
    return true;
  }
  if (method == "GET" && path == "/api/security/config") {
    StaticJsonDocument<128> doc;
    doc["habilitado"] = seguridad.habilitado;
    doc["modoAlerta"] = seguridad.modo;
    doc["causa"]      = seguridad.modo ? seguridad.causa : "";
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "GET" && path == "/api/security/status") {
    // [FIX] Campo "manual" necesario para que el JS sincronice el botón de activación.
    StaticJsonDocument<256> doc;
    doc["habilitado"] = seguridad.habilitado;
    doc["modoAlerta"] = seguridad.modo;
    doc["nivel"]      = seguridad.modo ? (int)seguridad.nivel : 0;
    doc["manual"]     = seguridad.manual;
    doc["causa"]      = seguridad.modo ? seguridad.causa : "";
    if (seguridad.modo && seguridad.medidas.length() > 0)
      doc["medidas"] = serialized(seguridad.medidas);
    else
      doc["medidas"] = serialized("[]");
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "POST" && path == "/api/security/config") {
    // [FIX] El JS manda JSON, no form-urlencoded.
    StaticJsonDocument<128> jdoc;
    bool habilitado = false;
    if (!deserializeJson(jdoc, body) && jdoc.containsKey("habilitado"))
      habilitado = jdoc["habilitado"].as<bool>();
    seguridad.habilitado = habilitado;
    if (!habilitado) { seguridad.manual = false; seguridad.modo = false; seguridad.causa = ""; }
    LOG_I("SEGURIDAD", "Sistema %s por usuario desde /config (tunel)", habilitado ? "ACTIVADO" : "DESACTIVADO");
    StaticJsonDocument<32> resp; resp["status"] = "ok";
    proxyEnviarJson(200, resp);
    return true;
  }
  if (method == "POST" && path == "/api/security/manual") {
    StaticJsonDocument<128> jdoc;
    bool activar = false;
    if (!deserializeJson(jdoc, body) && jdoc.containsKey("activar"))
      activar = jdoc["activar"].as<bool>();
    seguridad.manual = activar;
    if (!activar) { seguridad.modo = false; seguridad.causa = ""; }
    LOG_I("SEGURIDAD", "Activacion manual %s por usuario desde tunel", activar ? "ACTIVADA" : "DESACTIVADA");
    StaticJsonDocument<32> resp; resp["status"] = "ok";
    proxyEnviarJson(200, resp);
    return true;
  }
  if (method == "GET" && path == "/api/wifi/status") {
    StaticJsonDocument<128> doc;
    if (!conexionEnProgreso && conexionExitosa) {
      doc["status"] = "connected";
      doc["ip"]     = WiFi.localIP().toString();
    } else {
      doc["status"] = "idle";
    }
    proxyEnviarJson(200, doc);
    return true;
  }
  if (method == "POST" && path == "/api/wifi/reboot") {
    StaticJsonDocument<32> resp; resp["status"] = "ok";
    proxyEnviarJson(200, resp);
    delay(300);
    ESP.restart();
    return true;
  }
  if (method == "POST" && path == "/api/config/pins") {
    StaticJsonDocument<256> jdoc;
    if (!deserializeJson(jdoc, body)) {
      guardarConfigHardware(
        jdoc["nfcRst"]  | RST_PIN,
        jdoc["nfcSs"]   | SS_PIN,
        jdoc["trigPin"] | TRIG_PIN,
        jdoc["echoPin"] | ECHO_PIN,
        jdoc["dhtPin"]  | DHT_PIN
      );
    }
    StaticJsonDocument<32> resp; resp["status"] = "ok";
    proxyEnviarJson(200, resp);
    return true;
  }
  if (method == "POST" && path == "/api/config/webcred") {
    StaticJsonDocument<512> jdoc;
    if (!deserializeJson(jdoc, body)) {
      const char* user    = jdoc["user"];
      const char* curPass = jdoc["currentPass"];
      const char* newPass = jdoc["newPass"];
      if (!user || !curPass || !newPass) {
        StaticJsonDocument<64> err; err["status"] = "error"; err["message"] = "Faltan campos";
        proxyEnviarJson(400, err);
        return true;
      }
      if (webPass != String(curPass)) {
        StaticJsonDocument<64> err; err["status"] = "error"; err["message"] = "Contrasena actual incorrecta";
        proxyEnviarJson(403, err);
        return true;
      }
      guardarCredencialesWeb(String(user), String(newPass));
    }
    StaticJsonDocument<32> resp; resp["status"] = "ok";
    proxyEnviarJson(200, resp);
    return true;
  }
  return false; // No manejado — ir a TCP fallback
}

// TCP fallback: reenvía la petición al servidor HTTP local y retransmite la respuesta.
static void proxy_tcpFallback(const String& fullPath) {
  WiFiClient client;
  client.setTimeout(1500); // [PERF] El ESP32 local responde en <50ms

  bool conectado = false;
  IPAddress ips[] = { IPAddress(127,0,0,1), WiFi.localIP(), WiFi.softAPIP() };
  for (int i = 0; i < 3; i++) {
    if (ips[i] == IPAddress(0,0,0,0)) continue;
    if (client.connect(ips[i], 80)) { conectado = true; break; }
  }
  if (!conectado) {
    LOG_W("PROXY", "TCP fallback: no se pudo conectar para %s", fullPath.c_str());
    enviarRespuestaProxy(502, "text/plain", "ERR:unreachable", "");
    return;
  }

  client.printf("%s %s HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n",
                cfReq.method.c_str(), fullPath.c_str());

  StaticJsonDocument<512> hdrDoc;
  if (cfReq.hdrJson.length() > 0) deserializeJson(hdrDoc, cfReq.hdrJson);
  JsonObject hdrObj = hdrDoc.as<JsonObject>();
  for (JsonPair kv : hdrObj) {
    String k = kv.key().c_str(); k.toLowerCase();
    if (k == "host" || k == "connection" || k == "content-length" || k == "accept-encoding") continue;
    client.printf("%s: %s\r\n", kv.key().c_str(), kv.value().as<const char*>());
  }

  if (cfReq.body.length() > 0) {
    client.printf("Content-Length: %u\r\n", cfReq.body.length());
    client.print("\r\n");
    client.print(cfReq.body);
  } else {
    client.print("\r\n");
  }

  unsigned long deadline = millis() + 1500;
  String raw;
  uint8_t buf[512];
  while ((client.connected() || client.available()) && millis() < deadline) {
    if (client.available()) {
      int r = client.read(buf, sizeof(buf));
      if (r > 0) raw.concat((const char*)buf, r);
    } else {
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
  }
  client.stop();

  if (raw.length() == 0) {
    LOG_W("PROXY", "TCP fallback: respuesta vacía para %s", fullPath.c_str());
    enviarRespuestaProxy(502, "text/plain", "ERR:no response", "");
    return;
  }

  int hEnd = raw.indexOf("\r\n\r\n");
  if (hEnd == -1) hEnd = raw.indexOf("\n\n");
  if (hEnd == -1) { enviarRespuestaProxy(502, "text/plain", "ERR:bad HTTP", ""); return; }

  int sc = 502;
  int nl = raw.indexOf('\n');
  if (nl != -1) {
    String sl = raw.substring(0, nl);
    int a = sl.indexOf(' '), b2 = sl.indexOf(' ', a + 1);
    if (a != -1 && b2 != -1) sc = sl.substring(a + 1, b2).toInt();
  }

  int bStart = hEnd + (raw.charAt(hEnd) == '\r' ? 4 : 2);
  String respBody = raw.substring(bStart);

  String respCt  = "text/plain";
  String hdrsOut = "";
  String hs      = raw.substring(0, hEnd);
  int ls2        = hs.indexOf('\n') + 1;
  while (ls2 < (int)hs.length()) {
    int le2 = hs.indexOf('\n', ls2);
    if (le2 == -1) le2 = hs.length();
    String h = hs.substring(ls2, le2);
    if (h.length() > 0 && h.charAt(h.length() - 1) == '\r') h.remove(h.length() - 1);
    if (h.length() == 0) break;
    int co = h.indexOf(':');
    if (co != -1) {
      String k = h.substring(0, co); k.trim();
      String v = h.substring(co + 1); v.trim();
      String kl = k; kl.toLowerCase();
      if (kl == "content-type") {
        respCt = v;
      } else if (kl != "transfer-encoding" && kl != "connection" && kl != "keep-alive" && kl != "content-encoding") {
        hdrsOut += k + ": " + v + "\r\n";
      }
    }
    ls2 = le2 + 1;
  }

  enviarRespuestaProxy(sc, respCt, respBody, hdrsOut);
}

// ── Orquestador principal ─────────────────────────────────────────────────────
// Procesa una petición proxy: rutas públicas → auth → páginas protegidas → API → TCP fallback.
static void procesarProxyTunnel() {
  if (!cfReq.pendiente) return;

  const String method  = cfReq.method;
  const String body    = cfReq.body;
  const String hdrJson = cfReq.hdrJson;
  cfReq.pendiente = false;

  // Separar pathname de query string (ej: /dashboard?auth=true → /dashboard)
  String path     = cfReq.path;
  String fullPath = path;
  int qPos = path.indexOf('?');
  if (qPos != -1) path = path.substring(0, qPos);

  LOG_I("PROXY", "%s %s", method.c_str(), path.c_str());

  // 1. Rutas públicas — no requieren autenticación
  if (proxy_servirPaginaPublica(method, path, body)) return;

  // 2. Comprobar autenticación antes de continuar
  if (!estaAuthedViaTunnel(hdrJson)) {
    enviarRespuestaProxy(302, "text/html", "", "Location: /login-page");
    return;
  }

  // 3. Páginas HTML protegidas
  if (proxy_servirPaginaProtegida(method, path)) return;

  // 4. Endpoints de API (JSON)
  if (proxy_servirAPI(method, path, body)) return;

  // 5. TCP fallback para cualquier ruta no manejada directamente
  proxy_tcpFallback(fullPath);
}


// ============================================================================
// HELPER: construye el JSON de telemetría (reutilizado en ws_open y en el loop)
// ============================================================================
static String buildTelemetryJson() {
  int cargaC0 = calcularUsoCPU(0);
  int cargaC1 = calcularUsoCPU(1);
  uint32_t ramTotal = ESP.getPsramSize() > 0 ? ESP.getPsramSize() : ESP.getHeapSize();
  float ramPct = (ESP.getFreePsram() > 0)
      ? round((100.0f - (ESP.getFreePsram() * 100.0f / ramTotal)) * 100.0f) / 100.0f
      : round((100.0f - (ESP.getFreeHeap() * 100.0f / ESP.getHeapSize())) * 100.0f) / 100.0f;
  float flashTotal = (float)LittleFS.totalBytes();
  float flashUsado = (float)LittleFS.usedBytes();

  StaticJsonDocument<320> tel;
  tel["type"]    = "telemetry";
  tel["temp"]    = temperatureRead();
  tel["cpu"]     = (cargaC0 + cargaC1) / 2;
  tel["c0"]      = cargaC0;
  tel["c1"]      = cargaC1;
  tel["ram"]     = ramPct;
  tel["flash"]   = (flashTotal > 0) ? round((flashUsado * 100.0f / flashTotal) * 100.0f) / 100.0f : 0.0f;
  tel["wifi"]    = WiFi.RSSI();
  tel["uptime"]  = obtenerUptime();
  tel["dhtTemp"] = sensores.temperatura;
  tel["dhtHum"]  = sensores.humedad;
  String out;
  serializeJson(tel, out);
  return out;
}

// ============================================================================
// CORE 0: PILA DE RED, WiFi, HTTP, DNS, TELNET, WEBSOCKET, BBDD
// ============================================================================
void taskCore0(void * pvParameters) {
  if (!LittleFS.begin(true, "/spiffs", 10, "ffat")) {
      if (!LittleFS.begin(true)) Serial.println("❌ [ERROR] Flash no montada");
      else Serial.println("✅ [OK] Flash montada en Modo Rescate");
  } else {
      Serial.println("✅ [OK] Flash montada correctamente");
  }
  if (sdDisponible) {
      if (tamanoCSV_SD() > 8388608) {
          eliminarCSV_SD();
          Serial.println("🧹 [SISTEMA] Base de datos en SD superó límite. Limpieza ejecutada.");
      }
  } else if (LittleFS.exists("/datos.csv")) {
      File dbFile = LittleFS.open("/datos.csv", "r");
      if (dbFile) {
          size_t fileSize = dbFile.size();
          dbFile.close();
          if (fileSize > 8388608) {
              LittleFS.remove("/datos.csv");
              Serial.println("🧹 [SISTEMA] Base de datos superó límite. Limpieza ejecutada.");
          }
      }
  }

  telnetServer.begin();
  iniciarServidorWeb();

  // --- CARGAR CONFIGURACIÓN DEL TÚNEL DESDE NVS ---
  // En cada arranque se recuperan host/path/token y el flag de habilitación
  // para no depender de que el usuario reconfigure el túnel tras un reinicio.
  {
    String nvsHost, nvsPath, nvsToken;
    bool nvsHabilitado = false;
    cargarConfigCFWorker(nvsHost, nvsPath, nvsToken, nvsHabilitado);
    if (nvsHost.length() > 0) {
      tunel.host  = nvsHost;
      tunel.path  = nvsPath;
      tunel.token = nvsToken;
      // Migración: si no existe la clave "habilitado" en NVs (actualización desde
      // firmware anterior), asumir true para no romper la conectividad existente.
      {
        Preferences prefs;
        prefs.begin("cfworker", true);
        tunel.habilitado = prefs.isKey("habilitado") ? nvsHabilitado : true;
        prefs.end();
      }
      LOG_I("TUNNEL", "Config recuperada de NVS: host=%s habilitado=%d", nvsHost.c_str(), tunel.habilitado);
    }
  }

  // --- INICIALIZACIÓN DEL TÚNEL CLOUDFLARE WORKER ---
  // Callback que actualiza tunel.conectado y enruta mensajes entrantes del Worker
  tunel.ws.onEvent([](WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
      case WStype_CONNECTED:
        tunel.conectado = true;
        // [FIX-TUN-1] Resetear contador de browsers al reconectar — el Worker
        // mandará ws_open por cada browser activo, reconstruyendo el contador limpio.
        // Sin este reset, un tunel.wsCount fantasma hace que el ESP32 crea que hay
        // browsers cuando no los hay (p.ej. tras hibernación del DO de Cloudflare).
        tunel.wsCount = 0;
        LOG_I("TUNNEL", "Conectado al Worker: %s%s", tunel.host.c_str(), tunel.path.c_str());
        // Enviar token de autenticación al Worker nada más conectar
        if (tunel.token.length() > 0) {
          String auth = "{\"type\":\"auth\",\"token\":\"" + tunel.token + "\"}";
          tunel.ws.sendTXT(auth);
        }
        break;
      case WStype_DISCONNECTED:
        tunel.conectado = false;
        // [FIX-TUN-2] Resetear tunel.wsCount en desconexión — si el Worker muere
        // sin mandar ws_close, el contador queda positivo indefinidamente.
        tunel.wsCount = 0;
        LOG_I("TUNNEL", "Desconectado del Worker. Reintentando en 5s...");
        break;
      case WStype_TEXT: {
        // Callback ultraligero: solo parsea y encola. El proxy pesado va en el loop.
        // [FIX B9] 768 bytes es insuficiente para requests con body >~300 chars.
        // Con 4096 bytes caben headers HTTP + body de requests normales sin descarte silencioso.
        StaticJsonDocument<4096> r;
        if (deserializeJson(r, (const char*)payload, length)) break;
        const char* type = r["type"];

        // Datos de navegador reenviados por el túnel (telemetría/comandos)
        if (type && strcmp(type, "ws_fwd") == 0) {
          const char* data = r["data"];
          if (data && strlen(data) > 0) {
            if (ws) ws->textAll(data);
            char* cmd = (char*)malloc(strlen(data) + 1);
            if (cmd) {
              strcpy(cmd, data);
              char* end = cmd + strlen(cmd) - 1;
              while (end > cmd && (*end == ' ' || *end == '\n' || *end == '\r')) *end-- = 0;
              if (strcmp(cmd, "reboot") == 0) { free(cmd); ESP.restart(); }
              else if (xQueueSend(cmdQueue, &cmd, 0) != pdTRUE) free(cmd);
            }
          }
          break;
        }

        // Navegador conectó/desconectó vía túnel
        if (type && strcmp(type, "ws_open") == 0) {
          tunel.wsCount++; sistemaWeb.forzarTelemetria = true;
          sistemaWeb.permitirLog = true;
          enviarDatosIniciales = true;
          LOG_I("TUNNEL", "Navegador vía túnel (+%d)", tunel.wsCount);

          // ── TELEMETRÍA INSTANTÁNEA por túnel ──────────────────────────────
          // sistemaWeb.forzarTelemetria espera al siguiente ciclo (hasta 3 s). Mandamos
          // la telemetría actual ahora mismo para que el dashboard cargue
          // al instante sin esperar el intervalo del loop de Core 0.
          {
            String telOut = buildTelemetryJson();
            String msg;
            msg.reserve(telOut.length() + 24);
            msg = "{\"type\":\"ws_fwd\",\"data\":";
            msg += telOut;
            msg += "}";
            tunel.ws.sendTXT(msg);
          }
          break;
        }
        if (type && strcmp(type, "ws_close") == 0) {
          if (tunel.wsCount > 0) tunel.wsCount--;
          LOG_I("TUNNEL", "Navegador vía túnel (-%d)", tunel.wsCount);
          break;
        }

        // Sesión Telnet entrante por túnel
        if (type && strcmp(type, "telnet_open") == 0) {
          if (telnetClient && telnetClient.connected()) telnetClient.stop();
          tunel.telnetActivo = true;
          telnet.autenticado  = false;
          telnet.intentos     = 0;
          telnet.ultimaActividad = millis();
          String prompt = "\r\n[ZENITH OS] Autenticacion requerida.\r\nContrasena: ";
          tunel.ws.sendTXT("{\"type\":\"telnet_fwd\",\"data\":\"" + jsonStr(prompt) + "\"}");
          LOG_I("TUNNEL", "Sesion Telnet via tunel abierta");
          break;
        }

        if (type && strcmp(type, "telnet_close") == 0) {
          tunel.telnetActivo = false;
          telnet.autenticado  = false;
          sistemaWeb.permitirLog = false;
          LOG_I("TUNNEL", "Sesion Telnet via tunel cerrada");
          break;
        }

        if (type && strcmp(type, "telnet_data") == 0) {
          const char* tdata = r["data"];
          if (!tdata) break;
          telnet.ultimaActividad = millis();
          if (!telnet.autenticado) {
            String passIntento = String(tdata);
            passIntento.trim();
            if (passIntento == webPass) {
              telnet.autenticado = true;
              telnet.intentos    = 0;
              String ok = "\r\n[ZENITH OS] Acceso concedido. Bienvenido.\r\n";
              tunel.ws.sendTXT("{\"type\":\"telnet_fwd\",\"data\":\"" + jsonStr(ok) + "\"}");
              if (tunel.wsCount == 0) { tunel.wsCount = 1; sistemaWeb.permitirLog = true; }
            } else {
              telnet.intentos++;
              if (telnet.intentos >= 3) {
                String ko = "\r\n[SISTEMA] Demasiados intentos. Conexion cerrada.\r\n";
                tunel.ws.sendTXT("{\"type\":\"telnet_fwd\",\"data\":\"" + jsonStr(ko) + "\"}");
                tunel.telnetActivo = false;
              } else {
                String ko = "\r\n[SISTEMA] Contrasena incorrecta. Intento " + String(telnet.intentos) + "/3. Contrasena: ";
                tunel.ws.sendTXT("{\"type\":\"telnet_fwd\",\"data\":\"" + jsonStr(ko) + "\"}");
              }
            }
            break;
          }
          if (strcmp(tdata, "reboot") == 0) { ESP.restart(); break; }
          char* cmd = (char*)malloc(strlen(tdata) + 1);
          if (cmd) {
            strcpy(cmd, tdata);
            char* end = cmd + strlen(cmd) - 1;
            while (end > cmd && (*end == ' ' || *end == '\n' || *end == '\r')) *end-- = 0;
            if (xQueueSend(cmdQueue, &cmd, 0) != pdTRUE) free(cmd);
          }
          break;
        }

        // Petición HTTP proxy (existente)
        if (!type || strcmp(type, "request") != 0) break;
        if (cfReq.pendiente) break;

        // ── Verificación HMAC-SHA256 de origen ───────────────────────────────
        // El Worker firma cada request con el token compartido. Rechazar si
        // la firma no coincide (evita peticiones de terceros que lleguen al
        // WebSocket mediante un ataque de inyección).
        {
          const char* sig   = r["headers"]["x-zenith-sig"]  | "";
          const char* ts    = r["headers"]["x-zenith-time"] | "";
          if (tunel.token.length() > 0 && strlen(sig) > 0 && strlen(ts) > 0) {
            String payload = String(ts) + String(r["method"] | "GET") + String(r["path"] | "/") + String(r["body"] | "");
            String expected = hmacSha256(tunel.token, payload);
            if (expected.length() > 0 && expected != sig) {
              LOG_W("TUNNEL", "HMAC inválido — request ignorado");
              break;
            }
          }
        }

        cfReq.id       = r["id"]     | "";
        cfReq.method   = r["method"] | "GET";
        cfReq.path     = r["path"]   | "/";
        cfReq.body     = r["body"]   | "";
        // Serializar cabeceras a JSON string para reusarlas después
        JsonObject h = r["headers"];
        if (!h.isNull()) {
          StaticJsonDocument<768> hd;
          hd["h"] = h;
          serializeJson(hd["h"], cfReq.hdrJson);
        } else {
          cfReq.hdrJson = "";
        }
        cfReq.pendiente = true;
        // [FIX-TUN-7] No tocar tunel.wsCount aquí — un request HTTP no implica
        // que haya un browser WebSocket activo. El contador se gestiona solo
        // vía ws_open/ws_close. En cambio, forzar telemetría sí tiene sentido
        // para que el dashboard reciba datos al cargar la página.
        sistemaWeb.forzarTelemetria = true;
        break;
      }
      case WStype_ERROR:
        tunel.conectado = false;
        LOG_E("TUNNEL", "Error en WebSocket del Worker");
        break;
      default:
        break;
    }
  });

  // Arrancar el cliente si el túnel está configurado y habilitado en NVS
  // [FIX] beginSSL se llama en el primer ciclo del loop, no aqui — ver tunnelBeginSslLlamado.
  // Habia una race condition: si beginSSL se llamaba aqui Y el loop lo volvia a llamar
  // antes de que tunel.conectado se pusiera true, el segundo beginSSL reseteaba
  // el SSL handshake en curso y dejaba el tunel permanentemente desconectado.

  unsigned long lastTelemetryTime = 0;
  unsigned long lastNtpCheck = 0;
  int ultimaHoraGuardada = -1;
  bool primerGuardado = true;

  uint32_t inicioCiclo0 = micros();
  for(;;) {
    if (!sistemaWeb.listo) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        continue;
    }

    uint32_t ahora = micros();
    tiempoCicloCore[0] = ahora - inicioCiclo0;
    inicioCiclo0 = ahora;

    ArduinoOTA.handle();
    dnsServer.processNextRequest();
    yield();

    // --- TELNET ---
    if (telnetServer.hasClient()) {
      if (telnetClient && telnetClient.connected()) {
          telnetServer.available().stop();
      } else {
        telnetClient = telnetServer.available();
        telnet.autenticado = false;
        telnet.intentos = 0;
        telnet.ultimaActividad = millis();

        // Negociación Telnet: el servidor toma control del eco
        // IAC WILL ECHO  — el servidor hará el eco (cliente debe callar el suyo)
        // IAC WONT LINEMODE — modo carácter a carácter, no línea completa
        const uint8_t negociacion[] = {
          0xFF, 0xFB, 0x01,  // IAC WILL ECHO
          0xFF, 0xFD, 0x03,  // IAC DO SUPPRESS-GO-AHEAD
          0xFF, 0xFB, 0x03   // IAC WILL SUPPRESS-GO-AHEAD
        };
        telnetClient.write(negociacion, sizeof(negociacion));

        telnetClient.print("\r\n[ZENITH OS] Autenticacion requerida.\r\nContrasena: ");
      }
    }

    // Autenticación carácter a carácter — el servidor controla el eco
    // Se filtran secuencias IAC (0xFF) para que no corrompan el buffer
    if (telnetClient && telnetClient.connected() && !telnet.autenticado && telnetClient.available() > 0) {
      static String passBuffer = "";
      static bool iacPending = false;
      static int  iacSkip    = 0;
      while (telnetClient.available() > 0) {
        uint8_t c = telnetClient.read();

        // Filtrar secuencias IAC (3 bytes: 0xFF + cmd + opt)
        if (iacSkip > 0)   { iacSkip--;  continue; }
        if (c == 0xFF)     { iacSkip = 2; continue; }
        // Ignorar caracteres de control que no sean CR/LF/BS/DEL
        if (c == '\n') continue; // ignorar LF suelto
        if (c == '\r') {
          if (passBuffer.length() == 0) continue;
          String passIntento = passBuffer;
          passBuffer = "";
          telnetClient.print("\r\n");
          if (passIntento == webPass) {
            telnet.autenticado = true;
            telnet.intentos = 0;
            Terminal.println("\n[SISTEMA] Autenticacion correcta. Conectado al SO Blasco.");
            mostrarMenuPrincipal();
          } else {
            telnet.intentos++;
            if (telnet.intentos >= 3) {
              telnetClient.print("[SISTEMA] Demasiados intentos fallidos. Conexion cerrada.\r\n");
              telnetClient.stop();
              telnet.autenticado = false;
              telnet.intentos = 0;
              passBuffer = "";
            } else {
              telnetClient.print("[SISTEMA] Contrasena incorrecta. Intento ");
              telnetClient.print(telnet.intentos);
              telnetClient.print("/3.\r\nContrasena: ");
            }
          }
          break;
        } else if (c == 127 || c == 8) {
          if (passBuffer.length() > 0) {
            passBuffer.remove(passBuffer.length() - 1);
            telnetClient.print("\b \b");
          } else {
            // Buffer vacío: el cliente ya movió el cursor hacia atrás.
            // Reescribimos la línea entera para forzarlo de vuelta al final del prompt.
            telnetClient.print("\rContrasena: ");
          }
        } else if (c >= 32 && c < 127) {
          passBuffer += (char)c;
          telnetClient.print('*'); // eco enmascarado, sin revelar longitud real
        }
      }
    }

    // --- TELNET TIMEOUT (5 min inactividad) ---
    if (telnetClient && telnetClient.connected() && millis() - telnet.ultimaActividad > 300000) {
        telnetClient.println("\r\n[SISTEMA] Conexion cerrada por inactividad (5 min).");
        telnetClient.stop();
        telnet.autenticado = false;
        telnet.intentos = 0;
    }
    // Timeout Telnet vía túnel
    if (tunel.telnetActivo && millis() - telnet.ultimaActividad > 300000) {
        String msg = "\r\n[SISTEMA] Sesion cerrada por inactividad (5 min).\r\n";
        tunel.ws.sendTXT("{\"type\":\"telnet_fwd\",\"data\":\"" + jsonStr(msg) + "\"}");
        tunel.telnetActivo = false;
        telnet.autenticado  = false;
    }

    // --- WEBSOCKET TELEMETRÍA ---
    bool hayTeleCli = (ws->count() > 0) || (tunel.wsCount > 0);
    if (hayTeleCli) {
        // Menú solicitado por __ready__ (LAN) o por túnel con delay
        if (sistemaWeb.pedirMenu) {
            sistemaWeb.pedirMenu = false;
            sistemaWeb.permitirLog = true;
            mostrarMenuPrincipal();
        }
        // Solo para túnel sin LAN: activar inmediatamente cuando hay clientes túnel
        if (!sistemaWeb.permitirLog && tunel.wsCount > 0 && ws->count() == 0) {
            sistemaWeb.permitirLog = true;
            sistemaWeb.pedirMenu = true;
        }
        if (sistemaWeb.forzarTelemetria || (millis() - lastTelemetryTime >= 3000)) {
            sistemaWeb.forzarTelemetria = false;
            lastTelemetryTime = millis();

            String out = buildTelemetryJson();

            if (ws->count() > 0) {
                ws->textAll(out);
                ws->cleanupClients();
            }
            if (tunel.wsCount > 0 && tunel.conectado) {
                String tm;
                tm.reserve(out.length() + 24);
                tm = "{\"type\":\"ws_fwd\",\"data\":";
                tm += out;
                tm += "}";
                tunel.ws.sendTXT(tm);
            }
        }
        // Enviar datos iniciales (CSV) al navegador que acaba de conectar
        if (enviarDatosIniciales && tunel.conectado && tunel.wsCount > 0) {
            enviarDatosIniciales = false;
            String csv;
            if (sdDisponible && existeCSV_SD()) {
                csv = leerCSV_SD();
            } else if (LittleFS.exists("/datos.csv")) {
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
                String msg = "{\"type\":\"ws_fwd\",\"data\":{\"type\":\"csv_data\",\"text\":\"" + jsonStr(csv) + "\"}}";
                tunel.ws.sendTXT(msg);
            }
        }
    } else {
        sistemaWeb.tiempoInicio = 0;
        sistemaWeb.permitirLog = false;
    }

    // --- SISTEMA DE SEGURIDAD — monitorización cada 3s (sincronizado con telemetría) ---
    {
      static unsigned long ultimaRevisionSeg = 0;
      static int alertasEnviadas = 0;
      // [FIX] Ignición: no evaluar durante los primeros 30s de vida del ESP32.
      // En el arranque la CPU va al 100%, la temperatura del die es alta y la RAM
      // está en proceso de inicialización — sin este margen se generan falsas alertas.
      static const unsigned long SEG_BOOT_DELAY_MS = 30000UL;
      // ── Estado previo a la alerta (para restaurar FSM en recuperación) ──
      static uint8_t fsmGuardado = 0;

      if (millis() - ultimaRevisionSeg >= 3000) {
        ultimaRevisionSeg = millis();

        // Si el usuario desactivó la seguridad desde /config, limpiar estado
        if (!seguridad.habilitado) {
          if (seguridad.modo) {
            seguridad.modo = false;
            alertasEnviadas = 0;
            String ok = "{\"type\":\"security_ok\",\"hora\":\"" + jsonStr(obtenerFechaHora()) + "\",\"desactivado\":true}";
            if (ws->count() > 0) ws->textAll(ok);
            if (tunel.conectado && tunel.wsCount > 0) {
              tunel.ws.sendTXT("{\"type\":\"ws_fwd\",\"data\":" + ok + "}");
            }
            LOG_I("SEGURIDAD", "Sistema desactivado por usuario.");
          }
          // Restaurar FSM si se quedó detenida por una alerta previa
          if (FSM_GET() == 0 && fsmGuardado != 0) FSM_SET(fsmGuardado);
          // No usar continue: el resto del loop debe seguir ejecutándose
        } else {
          // [FIX] No evaluar umbrales durante el arranque — valores estabilizados tras 30s
          if (millis() >= SEG_BOOT_DELAY_MS) {
          float tempCPU  = temperatureRead();
          int   cpuMedia = (calcularUsoCPU(0) + calcularUsoCPU(1)) / 2;
          float ramTotal = ESP.getPsramSize() > 0 ? (float)ESP.getPsramSize() : (float)ESP.getHeapSize();
          float ramLibre = ESP.getFreePsram() > 0 ? (float)ESP.getFreePsram() : (float)ESP.getFreeHeap();
          float ramPct   = (1.0f - ramLibre / ramTotal) * 100.0f;
          int   rssi     = WiFi.RSSI();

          // ── Activar manualmente si el usuario lo ha solicitado ────────
          if (seguridad.manual && !seguridad.modo) {
            // [FIX B4] Escrituras de String heap bajo segMutex.
            // [FIX B7] Leer FSM antes del mutex, luego aplicar cambios de estado
            // con escrituras simples a volatile — no se puede usar xSemaphoreTake
            // ni malloc dentro de taskENTER_CRITICAL, así que usamos volatile +
            // segMutex para la atomicidad necesaria en este contexto.
            int fsmActual = FSM_GET(); // leer FSM ANTES de tomar el mutex
            if (segMutex) xSemaphoreTake(segMutex, portMAX_DELAY);
            seguridad.modo    = true;
            seguridad.causa   = "ACTIVACION MANUAL";
            seguridad.nivel   = 3;
            alertasEnviadas  = 0;
            fsmGuardado      = fsmActual;
            seguridad.medidas = "[";
            // Aplicar medidas — volatile garantiza visibilidad en Core 1
            if (fsmActual == 2) { midiendoDistancia = false; seguridad.medidas += "\"Distancia detenida\","; }
            if (fsmActual != 0) { FSM_SET(0);                seguridad.medidas += "\"FSM detenida\","; }
            if (nfc.modo != 0)   { nfc.modo = 0;               seguridad.medidas += "\"NFC detenido\","; }
            seguridad.medidas    += "\"Modo reducido\"]";
            seguridad.ultimaAlerta  = "Seguridad: ACTIVACION MANUAL";
            if (segMutex) xSemaphoreGive(segMutex);
            LOG_E("SEGURIDAD", "ALERTA MANUAL [nivel 3] activada por usuario.");
          }

          // ── Detectar prioridad y causa (CRITICAL → WARNING por parámetro) ──
          String causaDetectada = "";
          uint8_t nivelDetectado = 0;

          // CPU temperature
          if (tempCPU >= SEG_TEMP_CPU_MAX) { nivelDetectado = 3; causaDetectada = "TEMP_CPU:"  + String(tempCPU,  1) + "C (max " + String(SEG_TEMP_CPU_MAX, 0) + "C)"; }
          else if (tempCPU >= SEG_WARN_TEMP_CPU_MAX) { nivelDetectado = 2; causaDetectada = "TEMP_CPU:"  + String(tempCPU,  1) + "C (warn " + String(SEG_WARN_TEMP_CPU_MAX, 0) + "C)"; }

          // Ambient temperature
          else if (sensores.temperatura > -127.0f && sensores.temperatura >= SEG_TEMP_DHT_MAX) { nivelDetectado = 3; causaDetectada = "TEMP_AMB:"  + String(sensores.temperatura, 1) + "C (max " + String(SEG_TEMP_DHT_MAX, 0) + "C)"; }
          else if (sensores.temperatura > -127.0f && sensores.temperatura >= SEG_WARN_TEMP_DHT_MAX) { nivelDetectado = 2; causaDetectada = "TEMP_AMB:"  + String(sensores.temperatura, 1) + "C (warn " + String(SEG_WARN_TEMP_DHT_MAX, 0) + "C)"; }

          // CPU load
          else if (cpuMedia >= SEG_CPU_MAX) { nivelDetectado = 3; causaDetectada = "CPU:"  + String(cpuMedia) + "% (max " + String(SEG_CPU_MAX) + "%)"; }
          else if (cpuMedia >= SEG_WARN_CPU_MAX) { nivelDetectado = 2; causaDetectada = "CPU:"  + String(cpuMedia) + "% (warn " + String(SEG_WARN_CPU_MAX) + "%)"; }

          // RAM
          else if (ramPct >= SEG_RAM_MAX) { nivelDetectado = 3; causaDetectada = "RAM:"  + String(ramPct, 1) + "% (max " + String(SEG_RAM_MAX, 0) + "%)"; }
          else if (ramPct >= SEG_WARN_RAM_MAX) { nivelDetectado = 2; causaDetectada = "RAM:"  + String(ramPct, 1) + "% (warn " + String(SEG_WARN_RAM_MAX, 0) + "%)"; }

          // WiFi (INFO level only)
          else if (WiFi.isConnected() && rssi <= SEG_WIFI_MIN) { nivelDetectado = 3; causaDetectada = "WIFI:" + String(rssi) + "dBm (min " + String(SEG_WIFI_MIN) + "dBm)"; }
          else if (WiFi.isConnected() && rssi <= SEG_WARN_WIFI_MIN) { nivelDetectado = 2; causaDetectada = "WIFI:" + String(rssi) + "dBm (warn " + String(SEG_WARN_WIFI_MIN) + "dBm)"; }

          if (causaDetectada.length() > 0 && !seguridad.modo) {
            // [FIX B4] Proteger escrituras de String heap bajo segMutex.
            // [FIX B7] FSM se lee antes del mutex para no llamar xSemaphoreTake
            // dentro de una sección crítica (prohibido en FreeRTOS).
            int fsmActual = FSM_GET();
            if (segMutex) xSemaphoreTake(segMutex, portMAX_DELAY);
            seguridad.modo   = true;
            seguridad.causa  = causaDetectada;
            seguridad.nivel  = nivelDetectado;
            alertasEnviadas = 0;

            if (nivelDetectado >= 3) {
              // ── Medidas solo en nivel CRITICAL ──────────────────────────
              seguridad.medidas = "[";
              fsmGuardado = fsmActual;
              if (fsmActual == 2) { midiendoDistancia = false; seguridad.medidas += "\"Distancia detenida\","; }
              if (fsmActual != 0) { FSM_SET(0);                seguridad.medidas += "\"FSM detenida\","; }
              if (nfc.modo != 0)   { nfc.modo = 0;               seguridad.medidas += "\"NFC detenido\","; }
              seguridad.medidas   += "\"Modo reducido\"]";
              seguridad.ultimaAlerta = "Seguridad: " + causaDetectada;
            } else {
              seguridad.medidas = "";
            }
            if (segMutex) xSemaphoreGive(segMutex);
            LOG_E("SEGURIDAD", "ALERTA [nivel %d]: %s", seguridad.nivel, causaDetectada.c_str());
          }

          // ── Broadcast de la alerta ──────────────────────────────────────
          if (seguridad.modo) {
            // Consola: cada 2 warms envía 1 (alterna)
            if (alertasEnviadas % 2 == 0) {
              Terminal.iniciarBloque();
              Terminal.println("\n==================================================");
              Terminal.println("          ⚠  ALERTA DE SEGURIDAD  ⚠             ");
              Terminal.println("==================================================");
              Terminal.println("");
              Terminal.print  ("  Prioridad: ");        Terminal.println(seguridad.nivel == 3 ? "CRITICA" : seguridad.nivel == 2 ? "ADVERTENCIA" : "INFORMACION");
              Terminal.print  ("  Parametro: ");         Terminal.println(seguridad.causa);
              if (seguridad.medidas.length() > 0) {
                Terminal.print  ("  Medidas: ");           Terminal.println(seguridad.medidas);
                Terminal.println("  El sistema operara en modo reducido.");
              }
              Terminal.println("");
              Terminal.println("==================================================");
              Terminal.enviarBloque();
            }
            // Popups: siempre se envían
            String medidasJson = seguridad.medidas.length() > 0 ? seguridad.medidas : "[]";
            // [FIX B5] Truncar causa antes del snprintf. Sin truncar, una causa larga
            // puede superar los 800 bytes del buffer y corromper la pila de Core 0.
            char causa_trunc[200];
            snprintf(causa_trunc, sizeof(causa_trunc), "%s", seguridad.causa.c_str());
            char buf[800];
            snprintf(buf, sizeof(buf),
                "{\"type\":\"security_alert\",\"nivel\":%d,\"manual\":%s,\"causa\":\"%s\",\"medidas\":%s,\"hora\":\"%s\"}",
                seguridad.nivel,
                seguridad.manual ? "true" : "false",
                causa_trunc,
                medidasJson.c_str(),
                jsonStr(obtenerFechaHora()).c_str());
            String alerta = buf;
            if (ws->count() > 0) ws->textAll(alerta);
            if (tunel.conectado && tunel.wsCount > 0) {
              String fwd;
              fwd.reserve(strlen(buf) + 40);
              fwd += "{\"type\":\"ws_fwd\",\"data\":";
              fwd += buf;
              fwd += "}";
              tunel.ws.sendTXT(fwd);
            }
            alertasEnviadas++;
          }

          // ── Recuperación (auto y manual) ─────────────────────────────
          {
            static bool modoSeguridadAntes = false;

            // [FIX B2] Auto-recuperación ANTES de calcular transicionSalida.
            // En la versión anterior, transicionSalida se evaluaba antes del auto-recovery,
            // así que cuando causaDetectada se normalizaba y seguridad.modo pasaba a false,
            // la transición ya no se detectaba → no se enviaba security_ok, no se
            // restauraba fsmGuardado, seguridad.nivel nunca volvía a 0.
            if (seguridad.modo && !seguridad.manual && causaDetectada.length() == 0) {
              seguridad.modo = false;
            }

            // Ahora sí calculamos la transición, después del auto-recovery
            bool transicionSalida = modoSeguridadAntes && !seguridad.modo;
            modoSeguridadAntes = seguridad.modo;

            // Restaurar estado si acabamos de salir del modo seguridad
            if (transicionSalida) {
              alertasEnviadas  = 0;
              seguridad.nivel  = 0;
              seguridad.medidas = "";
              if (FSM_GET() == 0 && fsmGuardado != 0) FSM_SET(fsmGuardado);
              Terminal.iniciarBloque();
              Terminal.println("\n--------------------------------------------------");
              Terminal.println("  ✓ SEGURIDAD: Parametros normalizados.");
              Terminal.print  ("  Hora: "); Terminal.println(obtenerFechaHora());
              Terminal.println("  Sistema vuelve a operacion normal.");
              Terminal.println("--------------------------------------------------");
              Terminal.enviarBloque();
              String ok = "{\"type\":\"security_ok\",\"hora\":\"" + jsonStr(obtenerFechaHora()) + "\"}";
              if (ws->count() > 0) ws->textAll(ok);
              if (tunel.conectado && tunel.wsCount > 0) {
                tunel.ws.sendTXT("{\"type\":\"ws_fwd\",\"data\":" + ok + "}");
              }
              LOG_I("SEGURIDAD", "Parametros normalizados. Sistema recuperado.");
            }
          }
          } // ← cierra if (millis() >= SEG_BOOT_DELAY_MS)
        } // ← cierra else (monitorización habilitada)
      }
    }

    // --- CRONJOB CSV (Cada 2 horas usando hora NTP, verificado cada 30s) ---
    unsigned long ahoraMillis = millis();
    if (ahoraMillis - lastNtpCheck >= 30000) {
        lastNtpCheck = ahoraMillis;

        if (primerGuardado) {
            guardarEnHistorial();
            primerGuardado = false;
        }

        struct tm timeinfo;
        if (getLocalTime(&timeinfo) && timeinfo.tm_year > 100) {
            if (timeinfo.tm_hour % 2 == 0 && timeinfo.tm_min == 0 && timeinfo.tm_hour != ultimaHoraGuardada) {
                guardarEnHistorial();
                ultimaHoraGuardada = timeinfo.tm_hour;
            }
        }
    }

    // [FIX B3] Escritura atómica del par trabajo/ciclo bajo cpuMutex
    if (cpuMutex && xSemaphoreTake(cpuMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
      tiempoProcesamientoCore[0] = micros() - inicioCiclo0;
      xSemaphoreGive(cpuMutex);
    } else {
      tiempoProcesamientoCore[0] = micros() - inicioCiclo0;
    }

    if (tunel.habilitado && tunel.host.length() > 0) {
        static bool tunnelBeginSslLlamado  = false;
        static bool wifiEstabaConectado    = false;
        static unsigned long wifiConectadoDesde = 0;

        bool wifiOK = WiFi.isConnected();

        // Detectar flanco de reconexion WiFi — registrar el instante en que WiFi sube
        if (wifiOK && !wifiEstabaConectado) {
            wifiConectadoDesde = millis();
            LOG_I("TUNNEL", "WiFi (re)conectado — esperando estabilizacion TCP...");
        }
        wifiEstabaConectado = wifiOK;

        if (wifiOK) {
            // [FIX] Esperar 2s tras conectar/reconectar WiFi antes de llamar beginSSL.
            // El stack TCP/IP del ESP32 tarda ~1s en asignar IP y estabilizar rutas.
            // Llamar beginSSL demasiado pronto causa handshakes TLS fallidos que la
            // libreria no reintenta correctamente, dejando el tunel muerto.
            bool wifiEstabilizado = (millis() - wifiConectadoDesde) >= 2000UL;

            if (!tunnelBeginSslLlamado && wifiEstabilizado) {
                WiFi.setSleep(false);
                tunel.ws.beginSSL(tunel.host.c_str(), 443, tunel.path.c_str());
                tunel.ws.setReconnectInterval(5000);
                tunnelBeginSslLlamado = true;
                LOG_I("TUNNEL", "beginSSL llamado (WiFi estable %lums)", millis() - wifiConectadoDesde);
            }

            if (tunnelBeginSslLlamado) {
                tunel.ws.loop();
                procesarProxyTunnel();
            }
        }

        // Si WiFi se cayo: resetear flag para que al reconectar se haga beginSSL limpio.
        // Sin este reset, tunnelBeginSslLlamado quedaba true con la conexion TCP muerta
        // y setReconnectInterval nunca recuperaba el tunel.
        if (!wifiOK && tunnelBeginSslLlamado) {
            tunel.conectado     = false;
            tunnelBeginSslLlamado = false;
            LOG_I("TUNNEL", "WiFi caido — tunnel reset, esperando reconexion");
        }
    }

    vTaskDelay(2 / portTICK_PERIOD_MS); // [PERF] 10→5→2ms: máxima responsividad del túnel WS y Telnet
  }
}

// ============================================================================
// TAREA INDEPENDIENTE DHT11 (Core 1, prioridad 0)
// Evita que noInterrupts() (~5ms) bloquee el loop principal de Core 1
// ============================================================================
void taskDHT(void * pvParameters) {
  inicializarDHT();
  unsigned long tiempoUltimoDHT = 0;
  for (;;) {
    if (millis() - tiempoUltimoDHT >= 3000UL) {
        if (!(seguridad.modo && seguridad.nivel >= 3)) {
          tiempoUltimoDHT = millis();
          actualizarDHT();
        }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ============================================================================
// REGISTRO DE MÓDULOS (table-driven FSM dispatch)
// ============================================================================
const Modulo* modulos[] = { &moduloNFC, &moduloUS, &moduloDHT };
const int numModulos = sizeof(modulos) / sizeof(modulos[0]);

// ============================================================================
// CORE 1: LCD, NEOPIXEL, TERMINAL INPUT, SENSORES FSM (DHT11 en taskDHT)
// ============================================================================
void taskCore1(void * pvParameters) {
  unsigned long tiempoUltimoLCD = 0;
  unsigned long tiempoUltimoLED = 0;

  uint32_t inicioCiclo1 = micros();
  for(;;) {
    uint32_t ahora = micros();
    tiempoCicloCore[1] = ahora - inicioCiclo1;
    inicioCiclo1 = ahora;

    // --- LCD (cada 3s) ---
    if (millis() - tiempoUltimoLCD >= 3000) {
        tiempoUltimoLCD = millis();
        actualizarLCD();
    }

    // --- NEOPIXEL ---
    bool hayEspectadoresLED = (telnetClient && telnetClient.connected() && telnet.autenticado) || (ws->count() > 0);
    if (hayEspectadoresLED) {
      if (millis() - tiempoUltimoLED >= 500) {
        tiempoUltimoLED = millis();
        ledState = !ledState;
        if (ledState) {
            pixel.setPixelColor(0, pixel.Color(0, 0, 70));
        } else {
            pixel.setPixelColor(0, pixel.Color(0, 0, 0));
        }
        pixel.show();
      }
    } else {
      pixel.setPixelColor(0, pixel.Color(0, 25, 0));
      pixel.show();
    }

    // --- ENTRADA DE TEXTO (Telnet / WebSocket) ---
    String entrada = "";
    if (telnetClient && telnetClient.connected() && telnet.autenticado && telnetClient.available() > 0) {
      static String cmdBuffer = "";
      static int    cmdIacSkip = 0;
      // [PERF] Buffer de eco: acumular caracteres de eco y enviarlos en un solo write TCP
      // en vez de una llamada por carácter. Reduce el overhead TCP de N×1ms a ~1ms por tecla.
      String ecoBuffer = "";
      while (telnetClient.available() > 0) {
        uint8_t c = telnetClient.read();
        // Filtrar secuencias IAC
        if (cmdIacSkip > 0) { cmdIacSkip--; continue; }
        if (c == 0xFF)      { cmdIacSkip = 2; continue; }
        if (c == '\n') continue;
        if (c == '\r') {
          if (cmdBuffer.length() == 0) continue;
          entrada = cmdBuffer;
          entrada.trim();
          cmdBuffer = "";
          ecoBuffer += "\r\n"; // acumular en vez de print directo
          telnet.ultimaActividad = millis();
          break;
        } else if (c == 127 || c == 8) {
          if (cmdBuffer.length() > 0) {
            cmdBuffer.remove(cmdBuffer.length() - 1);
            ecoBuffer += "\b \b";
          } else {
            ecoBuffer += " \b";
          }
        } else if (c >= 32 && c < 127) {
          cmdBuffer += (char)c;
          ecoBuffer += (char)c; // acumular eco en vez de print directo
        }
      }
      // Flush del eco acumulado: un solo write TCP para todos los caracteres leídos
      if (ecoBuffer.length() > 0) {
        telnetClient.write((const uint8_t*)ecoBuffer.c_str(), ecoBuffer.length());
      }
    } else {
      char* cmdPtr = NULL;
      if (xQueueReceive(cmdQueue, &cmdPtr, 0) == pdTRUE && cmdPtr) {
          if (strcmp(cmdPtr, "__ready__") == 0) {
              sistemaWeb.permitirLog = true;
              sistemaWeb.pedirMenu = true;
              free(cmdPtr);
          } else {
              entrada = String(cmdPtr);
              entrada.trim();
              free(cmdPtr);
          }
      }
    }

    bool hayEspectadores = (telnetClient && telnetClient.connected() && telnet.autenticado) || (ws->count() > 0) || (tunel.conectado && tunel.wsCount > 0);

    // -- Bloqueo de consola en modo CRITICAL --
    if (seguridad.modo && seguridad.nivel >= 3 && entrada.length() > 0) {
      Terminal.iniciarBloque();
      Terminal.println("");
      Terminal.println("==================================================");
      Terminal.println("           ALERTA DE SEGURIDAD              ");
      Terminal.println("==================================================");
      Terminal.println("");
      Terminal.print  ("  Prioridad: "); Terminal.println("CRITICA");
      Terminal.print  ("  Parametro: "); Terminal.println(seguridad.causa);
      if (seguridad.medidas.length() > 0) {
        Terminal.print  ("  Medidas: "); Terminal.println(seguridad.medidas);
        Terminal.println("  El sistema operara en modo reducido.");
        Terminal.println("");
      }
      Terminal.println("==================================================");
      Terminal.println("  Consola bloqueada. Resuelve la alerta en /config.");
      Terminal.println("==================================================");
      Terminal.enviarBloque();
      entrada = "";
    }

    // --- FSM (table-driven con Module Registry) ---
    if (FSM_GET() == 0) {
      if (millis() - tiempoUltimoMenuPrincipal >= intervaloMenu && hayEspectadores) mostrarMenuPrincipal();
      if (entrada.length() > 0) procesarMenuPrincipal(entrada);
    }
    else if (FSM_GET() >= 1 && FSM_GET() <= numModulos) {
      int idx = FSM_GET() - 1;
      if (entrada.length() > 0 && modulos[idx]->entrada) modulos[idx]->entrada(entrada);
      if (modulos[idx]->loop) modulos[idx]->loop();
    }
    else if (FSM_GET() == 4) {
      // Estado de confirmación de reinicio
      if (entrada.length() > 0) {
        entrada.toUpperCase();
        if (entrada == "S") {
          Terminal.println("\n[SISTEMA] Reiniciando placa...");
          Terminal.println("[SISTEMA] Reconectando en unos segundos.");
          vTaskDelay(1500 / portTICK_PERIOD_MS);
          ESP.restart();
        } else {
          // Cualquier otra tecla: cancelar y volver al menú
          FSM_SET(0);
          Terminal.println("\n[SISTEMA] Reinicio cancelado.");
          mostrarMenuPrincipal();
        }
      }
    }

    // [FIX B3] Escritura atómica del par trabajo/ciclo bajo cpuMutex
    if (cpuMutex && xSemaphoreTake(cpuMutex, pdMS_TO_TICKS(2)) == pdTRUE) {
      tiempoProcesamientoCore[1] = micros() - inicioCiclo1;
      xSemaphoreGive(cpuMutex);
    } else {
      tiempoProcesamientoCore[1] = micros() - inicioCiclo1;
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}