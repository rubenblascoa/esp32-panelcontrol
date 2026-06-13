#!/usr/bin/env node
/**
 * telnet-tunnel.js — Proxy TCP→WebSocket para acceso Telnet al ESP32 vía túnel Cloudflare
 *
 * Uso:
 *   node telnet-tunnel.js <WORKER_URL>
 *   Ejemplo: node telnet-tunnel.js esp32blascoos.workers.dev
 *
 * Después conectar con:
 *   telnet localhost            (Windows CMD / PuTTY)
 *   telnet 127.0.0.1 23        (explícito)
 *
 * Requisitos:
 *   Node.js 18+ (incluye WebSocket nativo, sin dependencias externas)
 *
 * Puerto local: 23 (requiere permisos de administrador en Windows).
 * Si el puerto 23 está bloqueado, cambia LOCAL_PORT a 2323 y usa:
 *   telnet localhost 2323
 */

const net = require('net');

// ── Configuración ───────────────────────────────────────────────────────────
const LOCAL_PORT  = process.env.PORT  || 23;
const WORKER_HOST = process.argv[2];

if (!WORKER_HOST) {
  console.error('Uso: node telnet-tunnel.js <WORKER_URL>');
  console.error('Ejemplo: node telnet-tunnel.js esp32blascoos.workers.dev');
  process.exit(1);
}

const WS_URL = `wss://${WORKER_HOST.replace(/^wss?:\/\//, '')}/telnet`;

// ── Servidor TCP local ───────────────────────────────────────────────────────
const server = net.createServer((socket) => {
  socket.setNoDelay(true);
  const clientAddr = `${socket.remoteAddress}:${socket.remotePort}`;
  console.log(`[PROXY] Cliente conectado: ${clientAddr}`);

  // Crear WebSocket hacia el Worker
  let ws;
  try {
    ws = new WebSocket(WS_URL);
  } catch (e) {
    console.error('[PROXY] Error creando WebSocket:', e.message);
    socket.destroy();
    return;
  }

  let wsReady = false;
  const pendingToWs = [];

  ws.onopen = () => {
    wsReady = true;
    console.log(`[PROXY] WebSocket conectado → ${WS_URL}`);
    // Vaciar buffer pendiente
    for (const chunk of pendingToWs) ws.send(chunk);
    pendingToWs.length = 0;
  };

  ws.onmessage = (evt) => {
    // Datos del ESP32 → reenviar al cliente Telnet TCP
    if (!socket.destroyed) {
      socket.write(String(evt.data));
    }
  };

  ws.onerror = (err) => {
    console.error('[PROXY] Error WebSocket:', err.message || err);
    socket.destroy();
  };

  ws.onclose = () => {
    console.log(`[PROXY] WebSocket cerrado (${clientAddr})`);
    if (!socket.destroyed) socket.end();
  };

  // Datos del cliente Telnet TCP → reenviar al Worker vía WebSocket
  socket.on('data', (chunk) => {
    const text = chunk.toString('utf8');
    if (wsReady) {
      ws.send(text);
    } else {
      pendingToWs.push(text);
    }
  });

  socket.on('end', () => {
    console.log(`[PROXY] Cliente desconectado: ${clientAddr}`);
    if (ws.readyState === WebSocket.OPEN) ws.close();
  });

  socket.on('error', (err) => {
    if (err.code !== 'ECONNRESET') {
      console.error(`[PROXY] Error socket TCP: ${err.message}`);
    }
    if (ws.readyState === WebSocket.OPEN) ws.close();
  });
});

server.on('error', (err) => {
  if (err.code === 'EACCES') {
    console.error(`[ERROR] Puerto ${LOCAL_PORT} requiere permisos de administrador.`);
    console.error('        Ejecuta el CMD como Administrador, o usa PORT=2323 node telnet-tunnel.js ...');
    console.error('        y conéctate con: telnet localhost 2323');
  } else if (err.code === 'EADDRINUSE') {
    console.error(`[ERROR] Puerto ${LOCAL_PORT} ya está en uso.`);
    console.error('        Usa: PORT=2323 node telnet-tunnel.js ...');
  } else {
    console.error('[ERROR] Servidor TCP:', err.message);
  }
  process.exit(1);
});

server.listen(LOCAL_PORT, '127.0.0.1', () => {
  console.log('');
  console.log('╔══════════════════════════════════════════════════════╗');
  console.log('║        ESP32 Blasco OS — Telnet Tunnel Proxy         ║');
  console.log('╠══════════════════════════════════════════════════════╣');
  console.log(`║  Worker:  ${WS_URL.padEnd(42)}║`);
  console.log(`║  Local:   telnet localhost ${String(LOCAL_PORT).padEnd(26)}║`);
  console.log('╚══════════════════════════════════════════════════════╝');
  console.log('');
  console.log('Esperando conexiones Telnet...');
  console.log('Ctrl+C para detener.');
});
