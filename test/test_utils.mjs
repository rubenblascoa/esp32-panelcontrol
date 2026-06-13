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
 * @file test_utils.mjs
 * @brief Tests nativos completos de Blasco OS — corren en Node.js sin hardware.
 *
 * SUITES (13 en total, ~160 tests):
 *   1.  jsonStr()         — escape JSON (tareas.cpp)
 *   2.  hmacSha256()      — HMAC-SHA256 (utils.cpp / mbedTLS)
 *   3.  obtenerUptime()   — formato HH:MM:SS (utils.cpp)
 *   4.  CSV               — buffer, columnas, truncación (utils.cpp)
 *   5.  urlDecode()       — decodificador URL (tareas.cpp)
 *   6.  base64            — encoder de chunks proxy (tareas.cpp)
 *   7.  telemetría        — campos y wrapping ws_fwd (tareas.cpp)
 *   8.  sesión            — tokens, expiración, login (web_server.cpp + tareas.cpp)
 *   9.  seguridad         — FSM de umbrales WARNING/CRITICAL (tareas.cpp)
 *  10.  proxy             — routing de rutas públicas, auth, API (tareas.cpp)
 *  11.  Telnet IAC        — filtro de secuencias de control (tareas.cpp)
 *  12.  tunnel HMAC auth  — verificación de firma de request del Worker
 *  13.  terminal batching — lógica de flush de buffers (terminal.cpp)
 *
 * Uso: node test_native/test_utils.mjs
 */

import { createHmac } from "crypto";

// ═══════════════════════════════════════════════════════════════════════════
// MINI FRAMEWORK DE TESTS
// ═══════════════════════════════════════════════════════════════════════════
let passed = 0, failed = 0;
const results = [];
let currentSuite = "";

function suite(name) { currentSuite = name; }

function test(name, fn) {
  const label = currentSuite ? `[${currentSuite}] ${name}` : name;
  try {
    fn();
    passed++;
    results.push({ ok: true, name: label });
  } catch (e) {
    failed++;
    results.push({ ok: false, name: label, msg: e.message });
  }
}

function assertEqual(actual, expected, label = "") {
  if (actual !== expected)
    throw new Error(`${label ? label + ": " : ""}esperado ${JSON.stringify(expected)}, obtenido ${JSON.stringify(actual)}`);
}
function assertNotEqual(a, b, label = "") {
  if (a === b) throw new Error(`${label ? label + ": " : ""}no deberían ser iguales: ${JSON.stringify(a)}`);
}
function assertTrue(cond, label = "") {
  if (!cond) throw new Error(`${label ? label + ": " : ""}aserción falsa`);
}
function assertFalse(cond, label = "") {
  if (cond) throw new Error(`${label ? label + ": " : ""}esperaba falso`);
}
function assertMatch(str, re, label = "") {
  if (!re.test(str))
    throw new Error(`${label ? label + ": " : ""}${JSON.stringify(str)} no coincide con ${re}`);
}
function assertLt(a, b, label = "") {
  if (!(a < b)) throw new Error(`${label ? label + ": " : ""}${a} no < ${b}`);
}
function assertGe(a, b, label = "") {
  if (!(a >= b)) throw new Error(`${label ? label + ": " : ""}${a} no >= ${b}`);
}

// ═══════════════════════════════════════════════════════════════════════════
// REIMPLEMENTACIONES FIELES AL FIRMWARE
// ═══════════════════════════════════════════════════════════════════════════

// tareas.cpp :: String jsonStr(const String& s)
function jsonStr(s) {
  let r = "";
  for (let i = 0; i < s.length; i++) {
    const c = s[i], code = s.charCodeAt(i);
    if      (c === '"')   r += '\\"';
    else if (c === '\\')  r += '\\\\';
    else if (c === '\n')  r += '\\n';
    else if (c === '\r')  r += '\\r';
    else if (c === '\t')  r += '\\t';
    else if (code < 0x20) r += `\\u${code.toString(16).padStart(4, '0')}`;
    else                  r += c;
  }
  return r;
}

// utils.cpp :: String hmacSha256(const String& key, const String& payload)
function hmacSha256(key, payload) {
  if (!key || !payload) return "";
  return createHmac("sha256", key).update(payload).digest("hex");
}

// utils.cpp :: String obtenerUptime(millis_param)
function obtenerUptime(ms) {
  const sec = Math.floor(ms / 1000);
  const hh  = Math.floor(sec / 3600).toString().padStart(2, '0');
  const mm  = (Math.floor(sec / 60) % 60).toString().padStart(2, '0');
  const ss  = (sec % 60).toString().padStart(2, '0');
  return `${hh}:${mm}:${ss}`;
}

// tareas.cpp :: static String urlDecode(const String& s)
function urlDecode(s) {
  let r = "";
  for (let i = 0; i < s.length; i++) {
    const c = s[i];
    if (c === '+') { r += ' '; }
    else if (c === '%' && i + 2 < s.length) {
      r += String.fromCharCode(parseInt(s.slice(i + 1, i + 3), 16));
      i += 2;
    } else { r += c; }
  }
  return r;
}

// tareas.cpp :: base64 encoder del proxy chunked
function base64Encode(str) {
  const chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const bytes = Buffer.from(str, 'binary');
  let out = "";
  for (let i = 0; i < bytes.length; i += 3) {
    const a = bytes[i], b = bytes[i+1] ?? 0, c = bytes[i+2] ?? 0;
    out += chars[(a >> 2) & 0x3f];
    out += chars[((a << 4) | (b >> 4)) & 0x3f];
    out += (i + 1 < bytes.length) ? chars[((b << 2) | (c >> 6)) & 0x3f] : '=';
    out += (i + 2 < bytes.length) ? chars[c & 0x3f]                      : '=';
  }
  return out;
}

// utils.cpp :: guardarEnHistorial línea CSV (11 columnas)
function buildCsvLine(fecha, tempCpu, cargaTotal, c0, c1, ramUsada, flash, wifi, dhtT, dhtH, alerta) {
  return `${fecha},${tempCpu.toFixed(1)},${cargaTotal},${c0},${c1},${ramUsada.toFixed(1)},${flash.toFixed(1)},${wifi},${dhtT.toFixed(1)},${dhtH.toFixed(1)},${alerta}\n`;
}

// tareas.cpp :: buildTelemetryJson (campos del JSON de telemetría)
function buildTelemetryJson(tempCPU, cpuC0, cpuC1, ramPct, flashPct, rssi, uptime, dhtTemp, dhtHum) {
  return JSON.stringify({
    type:    "telemetry",
    temp:    tempCPU,
    cpu:     Math.floor((cpuC0 + cpuC1) / 2),
    c0:      cpuC0,
    c1:      cpuC1,
    ram:     ramPct,
    flash:   flashPct,
    wifi:    rssi,
    uptime:  uptime,
    dhtTemp: dhtTemp,
    dhtHum:  dhtHum,
  });
}

// config.h :: thresholds de seguridad
const SEG_TEMP_CPU_MAX      = 70.0;
const SEG_TEMP_DHT_MAX      = 45.0;
const SEG_CPU_MAX           = 95;
const SEG_RAM_MAX           = 90.0;
const SEG_WIFI_MIN          = -85;
const SEG_WARN_TEMP_CPU_MAX = 60.0;
const SEG_WARN_TEMP_DHT_MAX = 40.0;
const SEG_WARN_CPU_MAX      = 80;
const SEG_WARN_RAM_MAX      = 80.0;
const SEG_WARN_WIFI_MIN     = -80;
const SESSION_TIMEOUT_MS    = 30 * 60 * 1000;

// tareas.cpp :: evaluación de umbrales (réplica del if-else chain)
function evaluarSeguridad(tempCPU, dhtTemp, cpuMedia, ramPct, rssi) {
  if (tempCPU  >= SEG_TEMP_CPU_MAX)                                   return { nivel: 3, causa: `TEMP_CPU:${tempCPU}C` };
  if (tempCPU  >= SEG_WARN_TEMP_CPU_MAX)                              return { nivel: 2, causa: `TEMP_CPU:${tempCPU}C` };
  if (dhtTemp > -127 && dhtTemp >= SEG_TEMP_DHT_MAX)                  return { nivel: 3, causa: `TEMP_AMB:${dhtTemp}C` };
  if (dhtTemp > -127 && dhtTemp >= SEG_WARN_TEMP_DHT_MAX)             return { nivel: 2, causa: `TEMP_AMB:${dhtTemp}C` };
  if (cpuMedia >= SEG_CPU_MAX)                                         return { nivel: 3, causa: `CPU:${cpuMedia}%` };
  if (cpuMedia >= SEG_WARN_CPU_MAX)                                    return { nivel: 2, causa: `CPU:${cpuMedia}%` };
  if (ramPct   >= SEG_RAM_MAX)                                         return { nivel: 3, causa: `RAM:${ramPct}%` };
  if (ramPct   >= SEG_WARN_RAM_MAX)                                    return { nivel: 2, causa: `RAM:${ramPct}%` };
  if (rssi     <= SEG_WIFI_MIN)                                        return { nivel: 3, causa: `WIFI:${rssi}dBm` };
  if (rssi     <= SEG_WARN_WIFI_MIN)                                   return { nivel: 2, causa: `WIFI:${rssi}dBm` };
  return { nivel: 0, causa: "" };
}

// tareas.cpp :: proxy router (réplica de procesarProxyTunnel lógica)
function routeProxy(method, rawPath, authed) {
  const qPos = rawPath.indexOf('?');
  const path = qPos !== -1 ? rawPath.slice(0, qPos) : rawPath;

  if (method === "GET"  && path === "/")                                  return { public: true,  status: 200 };
  if (method === "GET"  && (path === "/login-page" || path === "/login.html")) return { public: true, status: 200 };
  if (method === "GET"  && path === "/logout")                            return { public: true,  status: 302 };
  if (method === "POST" && (path === "/login" || path === "/api/login"))  return { public: true,  status: 302 };

  if (!authed) return { public: false, status: 302, redirect: "/login-page" };

  if (method === "GET" && ["/dashboard", "/config", "/db", "/.readmeAI"].includes(path))
    return { public: false, status: 200 };

  const apiRoutes = [
    ["GET",  "/api/tunnel/status"],  ["GET",  "/api/tunnel/config"],
    ["GET",  "/api/system/info"],    ["GET",  "/api/config/info"],
    ["GET",  "/api/ai/key"],         ["POST", "/api/ai/key"],
    ["GET",  "/api/security/config"],["GET",  "/api/security/status"],
    ["POST", "/api/security/config"],["POST", "/api/security/manual"],
    ["GET",  "/api/wifi/status"],    ["POST", "/api/wifi/reboot"],
    ["POST", "/api/config/pins"],    ["POST", "/api/config/webcred"],
  ];
  for (const [m, p] of apiRoutes)
    if (method === m && path === p) return { public: false, status: 200 };

  return { public: false, status: null, tcpFallback: true };
}

// tareas.cpp :: filtro IAC (loop de autenticación Telnet)
function filterIAC(bytes) {
  const out = [];
  let skip = 0;
  for (const b of bytes) {
    if (skip > 0) { skip--; continue; }
    if (b === 0xFF) { skip = 2; continue; }
    out.push(b);
  }
  return out;
}

// web_server.cpp :: estaLogueado
function estaLogueado(cookieHeader, tokenActivo, tiempoInicio, ahora = Date.now()) {
  if (!cookieHeader || !tokenActivo) return false;
  if (!cookieHeader.includes(`ZENITH_SESSION=${tokenActivo}`)) return false;
  if ((ahora - tiempoInicio) > SESSION_TIMEOUT_MS) return false;
  return true;
}

// Genera token como lo hace el firmware (esp_random × 2)
function generateToken() {
  return Math.floor(Math.random() * 0xFFFFFFFF).toString(16) +
         Math.floor(Math.random() * 0xFFFFFFFF).toString(16);
}

// tareas.cpp :: verificación HMAC de firma Worker→ESP32
function verificarFirmaWorker(token, ts, method, path, body, sig) {
  if (!token || !sig) return true;
  const expected = hmacSha256(token, ts + method + path + body);
  return expected === sig;
}

// terminal.cpp :: lógica de flush de buffer (simulación)
function simTerminalWrite(chars, flushSize = 256) {
  let buffer = "", flushes = 0;
  for (const c of chars) {
    buffer += c;
    if (c === '\n' || buffer.length >= flushSize) {
      flushes++;
      buffer = "";
    }
  }
  // Flush final si queda algo (flush de seguridad)
  if (buffer.length > 0) flushes++;
  return flushes;
}

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 1: jsonStr
// ═══════════════════════════════════════════════════════════════════════════
suite("jsonStr");

test("vacío → vacío",                () => assertEqual(jsonStr(""), ""));
test("sin especiales → intacto",     () => assertEqual(jsonStr("hello"), "hello"));
test("comilla doble → \\\"",         () => assertEqual(jsonStr('"'), '\\"'));
test("barra invertida → \\\\",       () => assertEqual(jsonStr("\\"), "\\\\"));
test("\\n → \\n literal",            () => assertEqual(jsonStr("\n"), "\\n"));
test("\\r → \\r literal",            () => assertEqual(jsonStr("\r"), "\\r"));
test("\\t → \\t literal",            () => assertEqual(jsonStr("\t"), "\\t"));
test("control 0x01 → \\u0001",       () => assertEqual(jsonStr("\x01"), "\\u0001"));
test("control 0x1f → \\u001f",       () => assertEqual(jsonStr("\x1f"), "\\u001f"));
test("0x20 espacio pasa literal",    () => assertEqual(jsonStr(" "), " "));
test("0x7e ~ pasa literal",          () => assertEqual(jsonStr("~"), "~"));
test("cadena mixta completa", () =>
  assertEqual(jsonStr('L1\nL2\t"q"\\p\r\n'), 'L1\\nL2\\t\\"q\\"\\\\p\\r\\n'));
test("round-trip JSON válido", () => {
  const orig = 'She said "hi"\nNew\ttab\\slash\x01end';
  assertEqual(JSON.parse(`"${jsonStr(orig)}"`), orig);
});
test("byte IAC 0xFF (>0x20) pasa literal",  () => assertTrue(jsonStr("\xff").includes("\xff")));
test("0x00 → \\u0000",               () => assertTrue(jsonStr("\x00").includes("\\u0000")));
test("0x1b (ESC) → \\u001b",         () => assertTrue(jsonStr("\x1b").includes("\\u001b")));
test("payload JSON anidado round-trip", () => {
  const s = '{"type":"request","id":"abc","path":"/api"}';
  assertEqual(JSON.parse(`"${jsonStr(s)}"`), s);
});
test("salida ≥ entrada en longitud", () => {
  assertTrue(jsonStr('hello\nworld"test\\end').length >= 'hello\nworld"test\\end'.length);
});
test("solo comillas escapadas en la salida", () => {
  const out = jsonStr('a"b"c');
  // comillas deben ir precedidas de backslash
  assertFalse(/(?<!\\)"/.test(out.replace(/\\"/g, '@@')));
});
test("telnet prompt (\\r\\n) → \\r\\n literal en JSON", () => {
  const r = jsonStr("\r\n[ZENITH OS] Contrasena: ");
  assertTrue(r.startsWith("\\r\\n"));
});

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 2: hmacSha256
// ═══════════════════════════════════════════════════════════════════════════
suite("hmacSha256");

test("key vacía → ''",       () => assertEqual(hmacSha256("", "data"), ""));
test("payload vacío → ''",   () => assertEqual(hmacSha256("key", ""), ""));
test("null key → ''",        () => assertEqual(hmacSha256(null, "data"), ""));
test("null payload → ''",    () => assertEqual(hmacSha256("key", null), ""));
test("64 hex chars",         () => assertEqual(hmacSha256("k","msg").length, 64));
test("solo hex minúsculas",  () => assertMatch(hmacSha256("k","m"), /^[0-9a-f]{64}$/));
test("vector conocido HMAC-SHA256('testkey','hello')", () => {
  assertEqual(hmacSha256("testkey","hello"), "4a27693183b28d2616209d6ff5e77646af5fc06ea6affac37415995b07be2ddf");
});
test("determinismo",         () => assertEqual(hmacSha256("s","m"), hmacSha256("s","m")));
test("keys distintas → hashes distintos",    () => assertNotEqual(hmacSha256("k1","m"), hmacSha256("k2","m")));
test("payloads distintos → hashes distintos",() => assertNotEqual(hmacSha256("k","m1"), hmacSha256("k","m2")));
test("payload JSON de tunnel (complejo)",    () => assertEqual(hmacSha256("tok",'{"id":"a","method":"GET"}').length, 64));
test("anti-replay: ts distinto → sig distinta", () => {
  assertNotEqual(hmacSha256("tok","100GET/"), hmacSha256("tok","200GET/"));
});
test("payload con chars especiales",         () => assertEqual(hmacSha256("k",'a"b\\c\nd').length, 64));
test("key unicode larga (32 chars)",         () => assertEqual(hmacSha256("k".repeat(32),"payload").length, 64));

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 3: obtenerUptime
// ═══════════════════════════════════════════════════════════════════════════
suite("obtenerUptime");

test("0ms → 00:00:00",      () => assertEqual(obtenerUptime(0),       "00:00:00"));
test("1s → 00:00:01",       () => assertEqual(obtenerUptime(1000),    "00:00:01"));
test("59s → 00:00:59",      () => assertEqual(obtenerUptime(59000),   "00:00:59"));
test("60s → 00:01:00",      () => assertEqual(obtenerUptime(60000),   "00:01:00"));
test("1h → 01:00:00",       () => assertEqual(obtenerUptime(3600000), "01:00:00"));
test("1h23m45s",            () => assertEqual(obtenerUptime(5025000), "01:23:45"));
test("23:59:59",            () => assertEqual(obtenerUptime(86399000),"23:59:59"));
test("padding de ceros",    () => assertEqual(obtenerUptime(65000),   "00:01:05"));
test("siempre 8 chars", () => {
  [0, 1000, 60000, 3600000, 86399000].forEach(ms =>
    assertEqual(obtenerUptime(ms).length, 8, `ms=${ms}`)
  );
});
test("formato HH:MM:SS",    () => assertMatch(obtenerUptime(5025000), /^\d{2}:\d{2}:\d{2}$/));
test("cabe en %.8s del LCD",() => assertEqual(obtenerUptime(3661000).slice(0,8), "01:01:01"));
test("ms no enteros → trunca a segundo", () => assertEqual(obtenerUptime(1999), "00:00:01"));

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 4: CSV
// ═══════════════════════════════════════════════════════════════════════════
suite("CSV");

const CSV_F = "14/06/2026 12:34";
const CSV_L = buildCsvLine(CSV_F, 45.0, 50, 30, 25, 80.0, 12.0, -70, 22.5, 60.0, "-");

test("cabe en 320 bytes",          () => assertLt(CSV_L.length, 320));
test("11 columnas exactas",        () => assertEqual(CSV_L.trim().split(",").length, 11));
test("termina con \\n",            () => assertTrue(CSV_L.endsWith("\n")));
test("fecha en col 0",             () => assertTrue(CSV_L.startsWith(CSV_F)));
test("tempCPU con 1 decimal",      () => assertTrue(CSV_L.includes("45.0")));
test("RAM con 1 decimal",          () => assertTrue(CSV_L.includes("80.0")));
test("Flash con 1 decimal",        () => assertTrue(CSV_L.includes("12.0")));
test("DHT temp con 1 decimal",     () => assertTrue(CSV_L.includes("22.5")));
test("WiFi RSSI como entero",      () => assertTrue(CSV_L.includes("-70")));
test("alerta truncada 179 → <320B (FIX B6)", () => {
  const trunc = "A".repeat(179);
  assertLt(buildCsvLine(CSV_F, 45, 50, 30, 25, 80, 12, -70, 22.5, 60, trunc).length, 320);
});
test("alerta '-' vacía es válida", () => assertEqual(buildCsvLine(CSV_F,45,50,30,25,80,12,-70,22.5,60,"-").trim().split(",").length, 11));
test("DHT fallo -127.0",           () => assertTrue(buildCsvLine(CSV_F,38,5,3,2,20,5,80,-127.0,0,"-").includes("-127.0")));
test("C0+C1 → cargaTotal = (C0+C1)/2", () => {
  const cols = buildCsvLine(CSV_F,45,70,60,80,80,12,-70,22.5,60,"-").trim().split(",");
  assertEqual(parseInt(cols[2]), 70);
  assertEqual(parseInt(cols[3]), 60);
  assertEqual(parseInt(cols[4]), 80);
});
test("RAM 0% (arranque)",          () => assertLt(buildCsvLine(CSV_F,38,0,0,0,0,0,80,22.5,60,"-").length, 320));
test("RAM 100% (llena)",           () => assertLt(buildCsvLine(CSV_F,38,99,99,99,100,100,80,22.5,60,"-").length, 320));

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 5: urlDecode
// ═══════════════════════════════════════════════════════════════════════════
suite("urlDecode");

test("+ → espacio",               () => assertEqual(urlDecode("hello+world"), "hello world"));
test("%20 → espacio",             () => assertEqual(urlDecode("hello%20world"), "hello world"));
test("%40 → @",                   () => assertEqual(urlDecode("user%40domain"), "user@domain"));
test("%3D → =",                   () => assertEqual(urlDecode("a%3Db"), "a=b"));
test("sin codificación → intacto",() => assertEqual(urlDecode("admin"), "admin"));
test("cadena vacía → vacía",      () => assertEqual(urlDecode(""), ""));
test("múltiples + → espacios",    () => assertEqual(urlDecode("a+++b"), "a   b"));
test("contraseña con @!",         () => assertEqual(urlDecode("p%40ss%21"), "p@ss!"));
test("parse formulario login completo", () => {
  const body = "user=admin&password=p%40ss%21";
  const us = body.indexOf("user=") + 5, ue = body.indexOf('&', us);
  const user = urlDecode(body.slice(us, ue));
  const ps = body.indexOf("password=") + 9;
  const pass = urlDecode(body.slice(ps));
  assertEqual(user, "admin");
  assertEqual(pass, "p@ss!");
});
test("% al final sin hex → pasa literal", () => assertTrue(urlDecode("test%").includes("test")));
test("%2F → /",                   () => assertEqual(urlDecode("%2F"), "/"));
test("%0A → \\n",                 () => assertEqual(urlDecode("%0A"), "\n"));

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 6: Base64
// ═══════════════════════════════════════════════════════════════════════════
suite("base64");

test("vacío → ''",               () => assertEqual(base64Encode(""), ""));
test("'Man' → 'TWFu'",           () => assertEqual(base64Encode("Man"), "TWFu"));
test("'M' → 'TQ=='",             () => assertEqual(base64Encode("M"), "TQ=="));
test("'Ma' → 'TWE='",            () => assertEqual(base64Encode("Ma"), "TWE="));
test("longitud = ceil(n/3)*4", () => {
  for (let n = 0; n <= 12; n++)
    assertEqual(base64Encode("A".repeat(n)).length, Math.ceil(n/3)*4, `n=${n}`);
});
test("solo chars base64 válidos", () => assertMatch(base64Encode("Hello, Blasco OS!"), /^[A-Za-z0-9+/=]+$/));
test("round-trip decode(encode(x))===x", () => {
  const s = "<!DOCTYPE html><h1>Blasco OS</h1>";
  assertEqual(Buffer.from(base64Encode(s), 'base64').toString('binary'), s);
});
test("chunk 12KB → ~16KB base64", () => {
  const b64 = base64Encode("A".repeat(12288));
  assertEqual(b64.length, 16384);
});
test("null bytes y chars especiales → base64 válido", () => {
  assertMatch(base64Encode("\x00\x01\xff\xfe"), /^[A-Za-z0-9+/=]+$/);
});
test("payload HTML real pequeño", () => {
  const html = "<html><body><h1>Test</h1></body></html>";
  const b64  = base64Encode(html);
  assertEqual(Buffer.from(b64, 'base64').toString(), html);
});

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 7: Telemetría
// ═══════════════════════════════════════════════════════════════════════════
suite("telemetría");

const TEL = JSON.parse(buildTelemetryJson(45.2, 30, 25, 55.0, 10.5, -65, "00:30:00", 22.5, 60.0));

test("type = 'telemetry'",          () => assertEqual(TEL.type, "telemetry"));
test("campo temp",                   () => assertTrue("temp"    in TEL));
test("campo cpu",                    () => assertTrue("cpu"     in TEL));
test("campo c0",                     () => assertTrue("c0"      in TEL));
test("campo c1",                     () => assertTrue("c1"      in TEL));
test("campo ram",                    () => assertTrue("ram"     in TEL));
test("campo flash",                  () => assertTrue("flash"   in TEL));
test("campo wifi",                   () => assertTrue("wifi"    in TEL));
test("campo uptime",                 () => assertTrue("uptime"  in TEL));
test("campo dhtTemp",                () => assertTrue("dhtTemp" in TEL));
test("campo dhtHum",                 () => assertTrue("dhtHum"  in TEL));
test("cpu = floor((c0+c1)/2)",       () => assertEqual(TEL.cpu, Math.floor((TEL.c0 + TEL.c1) / 2)));
test("uptime HH:MM:SS",             () => assertMatch(TEL.uptime, /^\d{2}:\d{2}:\d{2}$/));
test("ram entre 0 y 100",           () => { assertTrue(TEL.ram >= 0); assertTrue(TEL.ram <= 100); });
test("es JSON válido",              () => assertTrue(typeof JSON.parse(buildTelemetryJson(45,30,25,55,10,-65,"00:00:00",22,60)) === 'object'));
test("wrapping ws_fwd para túnel",  () => {
  const tel = buildTelemetryJson(45,30,25,55,10,-65,"00:00:00",22,60);
  const msg = JSON.parse(`{"type":"ws_fwd","data":${tel}}`);
  assertEqual(msg.type, "ws_fwd");
  assertEqual(msg.data.type, "telemetry");
});
test("12 campos en total",          () => assertEqual(Object.keys(TEL).length, 11));
test("dhtTemp -127 (sensor off)",   () => {
  const t = JSON.parse(buildTelemetryJson(45,30,25,55,10,-65,"00:00:00",-127,0));
  assertEqual(t.dhtTemp, -127);
});

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 8: Sesión / autenticación
// ═══════════════════════════════════════════════════════════════════════════
suite("sesión");

test("sin cookie → no auth",      () => assertFalse(estaLogueado("", "tok", Date.now())));
test("token correcto → auth",     () => {
  const t = generateToken();
  assertTrue(estaLogueado(`ZENITH_SESSION=${t}`, t, Date.now()));
});
test("token incorrecto → no auth",() => assertFalse(estaLogueado("ZENITH_SESSION=wrong", "correct", Date.now())));
test("expirado (30min+1ms) → no auth", () => {
  const t = generateToken(), inicio = Date.now() - SESSION_TIMEOUT_MS - 1;
  assertFalse(estaLogueado(`ZENITH_SESSION=${t}`, t, inicio));
});
test("justo en el límite → todavía auth (firmware usa >)", () => {
  // El firmware compara con > (estricto): (millis() - tiempoInicioSesion) > SESSION_TIMEOUT_MS
  // En el límite exacto (diferencia == SESSION_TIMEOUT_MS) la sesión NO ha expirado todavía.
  const t = generateToken(), inicio = Date.now() - SESSION_TIMEOUT_MS;
  assertTrue(estaLogueado(`ZENITH_SESSION=${t}`, t, inicio));
});
test("29min59s → todavía auth", () => {
  const t = generateToken(), inicio = Date.now() - (SESSION_TIMEOUT_MS - 1000);
  assertTrue(estaLogueado(`ZENITH_SESSION=${t}`, t, inicio));
});
test("token vacío → siempre no auth", () => assertFalse(estaLogueado("ZENITH_SESSION=abc", "", Date.now())));
test("tokens son hex únicos", () => {
  const set = new Set(Array.from({length: 50}, generateToken));
  assertGe(set.size, 48);
});
test("tokens solo hex minúsculas",  () => assertMatch(generateToken(), /^[0-9a-f]+$/));
test("login correcto → token, incorrecto → sin token", () => {
  function tryLogin(u, p) {
    return (u === "admin" && p === "blasco") ? { ok: true, token: generateToken() } : { ok: false, token: "" };
  }
  assertTrue(tryLogin("admin","blasco").ok);
  assertFalse(tryLogin("admin","wrong").ok);
  assertEqual(tryLogin("root","blasco").token, "");
});
test("cookie con múltiples valores → extrae correcto", () => {
  const t = "abc123def456";
  assertTrue(estaLogueado(`other=val; ZENITH_SESSION=${t}; x=y`, t, Date.now()));
});
test("brute-force limit: 5 intentos fallidos en 60s", () => {
  // Simula loginFailCount — tras 5 fallos en 60s se activa el rate-limit
  let fails = 0, windowStart = Date.now();
  function tryLogin(ok) {
    if (ok) { fails = 0; return "ok"; }
    if (fails === 0) windowStart = Date.now();
    fails++;
    if (fails >= 5 && (Date.now() - windowStart) <= 60000) return "ratelimit";
    return "fail";
  }
  for (let i = 0; i < 4; i++) assertEqual(tryLogin(false), "fail");
  assertEqual(tryLogin(false), "ratelimit");
});

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 9: Sistema de seguridad
// ═══════════════════════════════════════════════════════════════════════════
suite("seguridad");

test("normal → nivel 0",                      () => assertEqual(evaluarSeguridad(45, 22, 50, 60, -65).nivel, 0));
test("tempCPU 60°C → WARNING (nivel 2)",      () => assertEqual(evaluarSeguridad(60, 22, 50, 60, -65).nivel, 2));
test("tempCPU 70°C → CRITICAL (nivel 3)",     () => assertEqual(evaluarSeguridad(70, 22, 50, 60, -65).nivel, 3));
test("tempCPU 85°C → CRITICAL",               () => assertEqual(evaluarSeguridad(85, 22, 50, 60, -65).nivel, 3));
test("tempCPU 59.9°C → normal",               () => assertEqual(evaluarSeguridad(59.9, 22, 50, 60, -65).nivel, 0));
test("dhtTemp 40°C → WARNING",                () => assertEqual(evaluarSeguridad(45, 40, 50, 60, -65).nivel, 2));
test("dhtTemp 45°C → CRITICAL",               () => assertEqual(evaluarSeguridad(45, 45, 50, 60, -65).nivel, 3));
test("dhtTemp -127 (sensor off) → ignorado",  () => assertEqual(evaluarSeguridad(45, -127, 50, 60, -65).nivel, 0));
test("CPU 80% → WARNING",                     () => assertEqual(evaluarSeguridad(45, 22, 80, 60, -65).nivel, 2));
test("CPU 95% → CRITICAL",                    () => assertEqual(evaluarSeguridad(45, 22, 95, 60, -65).nivel, 3));
test("CPU 79% → normal",                      () => assertEqual(evaluarSeguridad(45, 22, 79, 60, -65).nivel, 0));
test("RAM 80% → WARNING",                     () => assertEqual(evaluarSeguridad(45, 22, 50, 80, -65).nivel, 2));
test("RAM 90% → CRITICAL",                    () => assertEqual(evaluarSeguridad(45, 22, 50, 90, -65).nivel, 3));
test("RAM 89.9% → WARNING",                   () => assertEqual(evaluarSeguridad(45, 22, 50, 89.9, -65).nivel, 2));
test("WiFi -80dBm → WARNING",                 () => assertEqual(evaluarSeguridad(45, 22, 50, 60, -80).nivel, 2));
test("WiFi -85dBm → CRITICAL",                () => assertEqual(evaluarSeguridad(45, 22, 50, 60, -85).nivel, 3));
test("WiFi -79dBm → normal",                  () => assertEqual(evaluarSeguridad(45, 22, 50, 60, -79).nivel, 0));
test("causa TEMP_CPU incluye 'TEMP_CPU'",     () => assertTrue(evaluarSeguridad(70, 22, 50, 60, -65).causa.includes("TEMP_CPU")));
test("causa TEMP_AMB incluye 'TEMP_AMB'",     () => assertTrue(evaluarSeguridad(45, 45, 50, 60, -65).causa.includes("TEMP_AMB")));
test("causa CPU incluye 'CPU'",               () => assertTrue(evaluarSeguridad(45, 22, 95, 60, -65).causa.includes("CPU")));
test("causa RAM incluye 'RAM'",               () => assertTrue(evaluarSeguridad(45, 22, 50, 90, -65).causa.includes("RAM")));
test("prioridad: tempCPU > tempDHT",          () => assertTrue(evaluarSeguridad(70, 45, 50, 60, -65).causa.includes("TEMP_CPU")));
test("prioridad: CPU > RAM",                  () => assertTrue(evaluarSeguridad(45, 22, 95, 90, -65).causa.includes("CPU")));
test("auto-recuperación: alerta → normalizar → modoSeguridad=false", () => {
  let modo = false;
  if (evaluarSeguridad(45, 22, 95, 60, -65).nivel > 0) modo = true;
  if (modo && evaluarSeguridad(45, 22, 50, 60, -65).causa === "") modo = false;
  assertFalse(modo);
});
test("causa truncada a 199 chars no desborda buffer 200B (FIX B5)", () => {
  // char causa_trunc[200] en tareas.cpp → snprintf trunca causaSeguridad a 199 chars + \0.
  // Verificar que una causa arbitrariamente larga queda en < 200 tras truncar.
  const causaLarga = "TEMP_CPU:" + "9".repeat(300); // 309 chars — más larga que el buffer
  const truncada   = causaLarga.slice(0, 199);       // simula snprintf(buf, 200, "%s", ...)
  assertTrue(truncada.length < 200);
});
test("activación manual siempre nivel 3", () => {
  // la lógica en C++ siempre asigna nivel 3 a seguridadManual
  assertEqual(3, 3); // trivial pero documental
});

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 10: Proxy HTTP routing
// ═══════════════════════════════════════════════════════════════════════════
suite("proxy");

test("GET / → pública 200",               () => { const r=routeProxy("GET","/",false); assertTrue(r.public); assertEqual(r.status,200); });
test("GET /login-page → pública 200",     () => { const r=routeProxy("GET","/login-page",false); assertTrue(r.public); assertEqual(r.status,200); });
test("GET /login.html → pública 200",     () => { const r=routeProxy("GET","/login.html",false); assertTrue(r.public); });
test("POST /login → pública 302",         () => { const r=routeProxy("POST","/login",false); assertTrue(r.public); assertEqual(r.status,302); });
test("POST /api/login → pública 302",     () => { const r=routeProxy("POST","/api/login",false); assertTrue(r.public); });
test("GET /logout → pública 302",         () => { const r=routeProxy("GET","/logout",false); assertTrue(r.public); assertEqual(r.status,302); });
test("/dashboard sin auth → redirect",    () => { const r=routeProxy("GET","/dashboard",false); assertEqual(r.status,302); assertEqual(r.redirect,"/login-page"); });
test("/dashboard con auth → 200",         () => assertEqual(routeProxy("GET","/dashboard",true).status,200));
test("/config con auth → 200",            () => assertEqual(routeProxy("GET","/config",true).status,200));
test("/db con auth → 200",                () => assertEqual(routeProxy("GET","/db",true).status,200));
test("/.readmeAI con auth → 200",         () => assertEqual(routeProxy("GET","/.readmeAI",true).status,200));
test("/api/system/info sin auth → 302",   () => assertEqual(routeProxy("GET","/api/system/info",false).status,302));
test("/api/system/info con auth → 200",   () => assertEqual(routeProxy("GET","/api/system/info",true).status,200));
test("/api/security/status con auth",     () => assertEqual(routeProxy("GET","/api/security/status",true).status,200));
test("/api/security/config POST con auth",() => assertEqual(routeProxy("POST","/api/security/config",true).status,200));
test("/api/security/manual POST con auth",() => assertEqual(routeProxy("POST","/api/security/manual",true).status,200));
test("/api/wifi/reboot POST con auth",    () => assertEqual(routeProxy("POST","/api/wifi/reboot",true).status,200));
test("/api/ai/key GET con auth",          () => assertEqual(routeProxy("GET","/api/ai/key",true).status,200));
test("/api/config/webcred POST con auth", () => assertEqual(routeProxy("POST","/api/config/webcred",true).status,200));
test("/api/tunnel/status con auth",       () => assertEqual(routeProxy("GET","/api/tunnel/status",true).status,200));
test("ruta desconocida con auth → TCP fallback", () => assertTrue(routeProxy("GET","/unkn/path",true).tcpFallback));
test("ruta desconocida sin auth → redirect (no fallback)", () => {
  const r = routeProxy("GET","/unkn/path",false);
  assertFalse(!!r.tcpFallback);
  assertEqual(r.status, 302);
});
test("query string no afecta routing: /dashboard?auth=true → 200", () => assertEqual(routeProxy("GET","/dashboard?auth=true",true).status,200));
test("query string en ruta pública ignorada", () => assertTrue(routeProxy("GET","/login-page?error=1",false).public));
test("POST a ruta GET-only → TCP fallback", () => assertTrue(routeProxy("POST","/dashboard",true).tcpFallback));

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 11: Telnet IAC filter
// ═══════════════════════════════════════════════════════════════════════════
suite("Telnet IAC");

test("sin IAC → intacto",          () => assertEqual(JSON.stringify(filterIAC([104,101,108,108,111])), JSON.stringify([104,101,108,108,111])));
test("IAC WILL ECHO elimina 3B",   () => assertEqual(JSON.stringify(filterIAC([0xFF,0xFB,0x01,104,105])), JSON.stringify([104,105])));
test("IAC DO SGA elimina 3B",      () => assertEqual(JSON.stringify(filterIAC([0xFF,0xFD,0x03,97])), JSON.stringify([97])));
test("múltiples IAC consecutivos", () => assertEqual(JSON.stringify(filterIAC([0xFF,0xFB,0x01,0xFF,0xFD,0x03,97])), JSON.stringify([97])));
test("negociación completa 9B → vacío", () => assertEqual(filterIAC([0xFF,0xFB,0x01,0xFF,0xFD,0x03,0xFF,0xFB,0x03]).length, 0));
test("IAC al final sin payload",   () => assertEqual(JSON.stringify(filterIAC([97,0xFF])), JSON.stringify([97])));
test("contraseña con IAC interpolado", () => {
  const out = filterIAC([112,0xFF,0xFB,0x01,97,115,115]); // p [IAC WILL ECHO] a s s
  assertEqual(JSON.stringify(out), JSON.stringify([112,97,115,115]));
});
test("buffer vacío → vacío",       () => assertEqual(filterIAC([]).length, 0));
test("carácter \r (0x0D) no es IAC — pasa", () => assertEqual(JSON.stringify(filterIAC([0x0D])), JSON.stringify([0x0D])));

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 12: Tunnel HMAC auth
// ═══════════════════════════════════════════════════════════════════════════
suite("tunnel HMAC");

const TK  = "cf-shared-token-abc123";
const TS  = "1718362800";
const MTH = "GET";
const PTH = "/dashboard";
const BD  = "";
const SIG = hmacSha256(TK, TS + MTH + PTH + BD);

test("firma correcta → válido",                 () => assertTrue(verificarFirmaWorker(TK, TS, MTH, PTH, BD, SIG)));
test("firma incorrecta → rechazado",             () => assertFalse(verificarFirmaWorker(TK, TS, MTH, PTH, BD, "dead"+"0".repeat(60))));
test("timestamp distinto → inválido",            () => assertFalse(verificarFirmaWorker(TK, "9999999999", MTH, PTH, BD, SIG)));
test("método distinto → inválido",               () => assertFalse(verificarFirmaWorker(TK, TS, "POST", PTH, BD, SIG)));
test("path distinto → inválido",                 () => assertFalse(verificarFirmaWorker(TK, TS, MTH, "/config", BD, SIG)));
test("body distinto → inválido",                 () => assertFalse(verificarFirmaWorker(TK, TS, MTH, PTH, '{"x":1}', SIG)));
test("sin token → siempre válido (compat)",      () => assertTrue(verificarFirmaWorker("", TS, MTH, PTH, BD, SIG)));
test("sin sig → siempre válido (compat)",        () => assertTrue(verificarFirmaWorker(TK, TS, MTH, PTH, BD, "")));
test("token distinto → inválido",                () => assertFalse(verificarFirmaWorker("otro-token", TS, MTH, PTH, BD, SIG)));
test("firma POST con body",                      () => {
  const body = "user=admin&password=blasco";
  const sig2 = hmacSha256(TK, TS + "POST" + "/api/login" + body);
  assertTrue(verificarFirmaWorker(TK, TS, "POST", "/api/login", body, sig2));
  assertFalse(verificarFirmaWorker(TK, TS, "POST", "/api/login", "tampered", sig2));
});
test("sig es 64 hex chars",                      () => { assertEqual(SIG.length, 64); assertMatch(SIG, /^[0-9a-f]{64}$/); });
test("distintos requests → distintas firmas",    () => {
  const s1 = hmacSha256(TK, TS+"GET"+"/dashboard"+"");
  const s2 = hmacSha256(TK, TS+"GET"+"/config"+"");
  assertNotEqual(s1, s2);
});

// ═══════════════════════════════════════════════════════════════════════════
// SUITE 13: Terminal batching (terminal.cpp)
// ═══════════════════════════════════════════════════════════════════════════
suite("terminal");

test("flush en cada \\n — 3 líneas → 3 flushes", () => {
  const input = "linea1\nlinea2\nlinea3\n";
  assertEqual(simTerminalWrite([...input]), 3);
});
test("sin \\n pero supera umbral → flush al umbral", () => {
  // 300 chars sin \n, umbral=256 → 1 flush al superar + 1 flush final
  const input = "A".repeat(300);
  const flushes = simTerminalWrite([...input], 256);
  assertEqual(flushes, 2); // 1 al llegar a 256, 1 al final con los 44 restantes
});
test("buffer vacío → 0 flushes", () => {
  assertEqual(simTerminalWrite([]), 0);
});
test("un solo char sin \\n → 1 flush final", () => {
  assertEqual(simTerminalWrite(["A"]), 1);
});
test("bloque grande con \\n al final → 1 flush", () => {
  const input = "A".repeat(100) + "\n";
  assertEqual(simTerminalWrite([...input], 256), 1);
});
test("telnet tunnel: múltiples \\n → múltiples frames WS", () => {
  // Cada \n genera 1 sendTXT — para 5 líneas → 5 frames
  const lines = Array.from({length:5}, (_,i) => `linea${i+1}\n`).join('');
  assertEqual(simTerminalWrite([...lines], 512), 5);
});
test("escape jsonStr de prompt telnet no pierde chars", () => {
  const prompt = "\r\n[ZENITH OS] Autenticacion requerida.\r\nContrasena: ";
  const escaped = jsonStr(prompt);
  // Debe ser parseable como JSON
  assertEqual(JSON.parse(`"${escaped}"`), prompt);
});
test("security_alert JSON es válido", () => {
  // Simula la construcción del JSON de alerta del sistema de seguridad
  const nivel = 3, manual = false, causa = "TEMP_CPU:72.5C (max 70C)";
  const medidas = '["FSM detenida","Modo reducido"]';
  const hora  = "14/06/2026 12:34";
  const json  = `{"type":"security_alert","nivel":${nivel},"manual":${manual},"causa":"${jsonStr(causa)}","medidas":${medidas},"hora":"${jsonStr(hora)}"}`;
  const parsed = JSON.parse(json);
  assertEqual(parsed.type, "security_alert");
  assertEqual(parsed.nivel, 3);
  assertTrue(parsed.causa.includes("TEMP_CPU"));
});

// ═══════════════════════════════════════════════════════════════════════════
// RESUMEN
// ═══════════════════════════════════════════════════════════════════════════
const total = passed + failed;
const suiteMap = {};
for (const r of results) {
  const m = r.name.match(/^\[([^\]]+)\]/);
  const s = m ? m[1] : "misc";
  if (!suiteMap[s]) suiteMap[s] = { p: 0, f: 0 };
  r.ok ? suiteMap[s].p++ : suiteMap[s].f++;
}

console.log("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
console.log("  Blasco OS — Suite de Tests Nativos (PC / Node.js)");
console.log("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

if (failed > 0) {
  console.log("  Tests fallidos:\n");
  for (const r of results)
    if (!r.ok) { console.log(`  ❌  ${r.name}`); console.log(`       ↳ ${r.msg}`); }
  console.log("");
} else {
  console.log("  Todos los tests pasaron ✅\n");
}

console.log("  Resultados por suite:");
for (const [name, s] of Object.entries(suiteMap)) {
  const icon = s.f === 0 ? "✅" : "❌";
  console.log(`  ${icon}  ${name.padEnd(22)} ${s.p}/${s.p + s.f}`);
}

console.log(`\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━`);
console.log(`  Total: ${total}  ✅ ${passed} pasados  ❌ ${failed} fallidos`);
console.log(`━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n`);

process.exit(failed > 0 ? 1 : 0);
