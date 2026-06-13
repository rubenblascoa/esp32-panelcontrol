// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod
//
// Certificados autofirmados EC-256 para HTTPS.
// Solo compilados cuando HTTPS_ENABLED está definido.
//
// Generar los arrays reales con:
//   openssl req -x509 -newkey ec -pkeyopt ec_paramgen_curve:prime256v1 \
//     -keyout key.pem -out cert.pem -days 3650 -nodes \
//     -subj "/CN=esp32-blasco-os.local"
//   xxd -i cert.pem   → copiar a cert_pem[] abajo
//   xxd -i key.pem    → copiar a key_pem[] abajo

#pragma once

#ifdef HTTPS_ENABLED

#include <pgmspace.h>

// ── Certificado PEM (autofirmado EC-256, generado con OpenSSL) ─
static const char cert_pem[] PROGMEM =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIBOTCB4QIUbzcGQn6/mxvjYUcDCRaVkfVp+bkwCgYIKoZIzj0EAwIwIDEeMBwG\n"
  "A1UEAwwVZXNwMzItYmxhc2NvLW9zLmxvY2FsMB4XDTI2MDYxMzEzMDExOVoXDTM2\n"
  "MDYxMDEzMDExOVowIDEeMBwGA1UEAwwVZXNwMzItYmxhc2NvLW9zLmxvY2FsMFkw\n"
  "EwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE77oaEgePgEx5VT/ZX+RIkqpTbk8fcPeE\n"
  "OGpj+lvnMDk1pLsIR5Wv13+h5WQx/bpbQFfiehFQvLsypdguhFUqsDAKBggqhkjO\n"
  "PQQDAgNHADBEAiADY6Ng1b+wxGJBqo+La+nsJAlMl9qletZ38J3+YlyjEgIgXUq6\n"
  "mP1nPvwoOpIT9DI+2W1s7KJ6HvaFJeBY3eQnSUk=\n"
  "-----END CERTIFICATE-----\n";
static const size_t cert_pem_len = sizeof(cert_pem);

// ── Clave privada PEM (EC-256, generada con OpenSSL) ──────────
static const char key_pem[] PROGMEM =
  "-----BEGIN PRIVATE KEY-----\n"
  "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQghTcFaEN4RxllTZ4T\n"
  "v/2aSRnCH1QzwGGtYGd46iT+3SShRANCAATvuhoSgE8ZATx5VT/ZX+RIkqpTbk8f\n"
  "cPeEOGpj+lvnMDk1pLsIR5Wv13+h5WQx/bpbQFfiehFQvLsypdguhFUq\n"
  "-----END PRIVATE KEY-----\n";
static const size_t key_pem_len = sizeof(key_pem);

#endif
