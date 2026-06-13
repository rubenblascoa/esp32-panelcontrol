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

#include "nfc.h"
#include "terminal.h"
#include "menus.h"
#include <SPI.h>

// ============================================================================
// CONTADOR DE ERRORES Y RECUPERACIÓN AUTOMÁTICA
// ============================================================================
static uint8_t  fallosNFC = 0;
static uint32_t ultimoReinitNFC = 0;
static const uint8_t  MAX_FALLOS_NFC = 5;
static const uint32_t REINIT_COOLDOWN_NFC_MS = 5000;

void nfcReinicializar() {
  if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(200)) != pdTRUE) return;

  delete mfrc522;
  mfrc522 = nullptr;

  SPI.end();
  delay(50);
  SPI.begin(18, 13, 11, SS_PIN);
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);
  mfrc522->PCD_Init();

  if (spiMutex) xSemaphoreGive(spiMutex);

  LOG_W("NFC", "MFRC522 reinicializado tras %d fallos consecutivos", fallosNFC);
  fallosNFC = 0;
}

void nfcRegistrarFallo() {
  fallosNFC++;
  if (fallosNFC >= MAX_FALLOS_NFC) {
    uint32_t ahora = millis();
    if (ahora - ultimoReinitNFC > REINIT_COOLDOWN_NFC_MS) {
      nfcReinicializar();
      ultimoReinitNFC = ahora;
    }
  }
}

uint8_t nfcGetFallos() { return fallosNFC; }
void nfcResetFallos() { fallosNFC = 0; }

#define NUM_NFC_KEYS 20

static const byte NFC_KEY_DICT[NUM_NFC_KEYS][6] = {
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
  {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5},
  {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5},
  {0x4D, 0x3A, 0x99, 0xC3, 0x51, 0xDD},
  {0x1A, 0x98, 0x2C, 0x7E, 0x45, 0x9A},
  {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
  {0x71, 0x4C, 0x5C, 0x88, 0x6E, 0x97},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x0F},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
  {0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0},
  {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5},
  {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5},
  {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
  {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC},
  {0x11, 0x22, 0x33, 0x44, 0x55, 0x66},
  {0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56},
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
  {0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F},
};

static MFRC522::MIFARE_Key sectorKeyExitosas[16];
static bool sectorAutenticado[16];

static void printHexByte(byte b) {
  if (b < 0x10) Terminal.print("0");
  Terminal.print(b, HEX);
}

static bool autenticarSector(byte sector) {
  byte blockBase = sector * 4;

  if (sector > 0 && sectorAutenticado[sector - 1]) {
    MFRC522::StatusCode status = mfrc522->PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockBase,
      &sectorKeyExitosas[sector - 1], &(mfrc522->uid));
    if (status == MFRC522::STATUS_OK) {
      sectorKeyExitosas[sector] = sectorKeyExitosas[sector - 1];
      sectorAutenticado[sector] = true;
      return true;
    }
    mfrc522->PCD_StopCrypto1();
  }

  for (int k = 0; k < NUM_NFC_KEYS; k++) {
    MFRC522::MIFARE_Key testKey;
    memcpy(testKey.keyByte, NFC_KEY_DICT[k], 6);

    MFRC522::StatusCode status = mfrc522->PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockBase,
      &testKey, &(mfrc522->uid));

    if (status == MFRC522::STATUS_OK) {
      sectorKeyExitosas[sector] = testKey;
      sectorAutenticado[sector] = true;
      return true;
    }

    mfrc522->PCD_StopCrypto1();
  }

  sectorAutenticado[sector] = false;
  return false;
}

void modoLecturaNFC() {
  if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE) return;

  if (!mfrc522->PICC_IsNewCardPresent() || !mfrc522->PICC_ReadCardSerial()) {
    if (spiMutex) xSemaphoreGive(spiMutex);
    return;
  }

  for (int i = 0; i < 16; i++) sectorAutenticado[i] = false;

  byte buffer[18];
  byte size = sizeof(buffer);

  nfc.dumpValido = false;
  bool algunSectorOK = false;

  Terminal.println("\n[NFC] Dump completo MIFARE 1K (64 bloques, diccionario de claves):");
  Terminal.println("      Sec | Blo | Clave            | Datos (hex)");
  Terminal.println("----------------------------------------------------------");

  for (byte sector = 0; sector < 16; sector++) {
    byte blockBase = sector * 4;

    if (!autenticarSector(sector)) {
      Terminal.printf("      %2d  |  -- | NO KEY           | Sector saltado\n", sector);
      continue;
    }

    Terminal.printf("      %2d  |     | ", sector);
    for (byte j = 0; j < 6; j++) { printHexByte(sectorKeyExitosas[sector].keyByte[j]); }
    Terminal.println();

    for (byte blk = 0; blk < 4; blk++) {
      byte blockNum = blockBase + blk;
      size = sizeof(buffer);
      MFRC522::StatusCode status = mfrc522->MIFARE_Read(blockNum, buffer, &size);

      if (status == MFRC522::STATUS_OK) {
        byte* dst = &nfc.dump[blockNum * 16];
        for (byte j = 0; j < 16; j++) dst[j] = buffer[j];

        Terminal.printf("      %2d  |  %d  | ", sector, blk);
        for (byte j = 0; j < 16; j++) { printHexByte(buffer[j]); Terminal.print(" "); }
        Terminal.println();
        algunSectorOK = true;
      } else {
        Terminal.printf("      %2d  |  %d  | Read FAIL\n", sector, blk);
      }
    }

    mfrc522->PCD_StopCrypto1();
  }

  if (algunSectorOK) {
    for (byte j = 0; j < 16; j++) nfc.bloqueEscaneado[j] = nfc.dump[j];
    nfc.memoriaLlena = true;
    nfc.dumpValido = true;
    nfcResetFallos();
  } else {
    nfcRegistrarFallo();
  }

  Terminal.println("----------------------------------------------------------");
  if (nfc.dumpValido) Terminal.println("[EXITO] Dump completo de 64 bloques guardado en RAM.");
  else            Terminal.println("[AVISO] No se pudo leer ningun sector de la tarjeta.");

  mfrc522->PICC_HaltA();
  mfrc522->PCD_StopCrypto1();

  if (spiMutex) xSemaphoreGive(spiMutex);

  vTaskDelay(2000 / portTICK_PERIOD_MS);
  nfc.modo = 0;
  mostrarMenuNFC();
}

void modoEscrituraNFC() {
  if (spiMutex && xSemaphoreTake(spiMutex, pdMS_TO_TICKS(100)) != pdTRUE) return;

  if (!mfrc522->PICC_IsNewCardPresent() || !mfrc522->PICC_ReadCardSerial()) {
    if (spiMutex) xSemaphoreGive(spiMutex);
    return;
  }

  Terminal.println("\n[NFC] Clonando tarjeta destino...");

  bool algunSectorEscrito = false;

  for (byte sector = 0; sector < 16; sector++) {
    byte blockBase = sector * 4;

    bool authOK = false;

    if (sectorAutenticado[sector]) {
      MFRC522::StatusCode status = mfrc522->PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockBase,
        &sectorKeyExitosas[sector], &(mfrc522->uid));
      if (status == MFRC522::STATUS_OK) {
        authOK = true;
      } else {
        mfrc522->PCD_StopCrypto1();
      }
    }

    if (!authOK) {
      for (int k = 0; k < NUM_NFC_KEYS; k++) {
        MFRC522::MIFARE_Key testKey;
        memcpy(testKey.keyByte, NFC_KEY_DICT[k], 6);

        MFRC522::StatusCode status = mfrc522->PCD_Authenticate(
          MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockBase,
          &testKey, &(mfrc522->uid));

        if (status == MFRC522::STATUS_OK) {
          sectorKeyExitosas[sector] = testKey;
          sectorAutenticado[sector] = true;
          authOK = true;
          break;
        }

        mfrc522->PCD_StopCrypto1();
      }
    }

    if (!authOK) {
      Terminal.printf("  [ERROR] Sector %d: Auth FAIL (ninguna clave funciona)\n", sector);
      continue;
    }

    for (byte blk = 0; blk < 4; blk++) {
      byte blockNum = blockBase + blk;
      byte* dataPtr = &nfc.dump[blockNum * 16];

      MFRC522::StatusCode status = mfrc522->MIFARE_Write(blockNum, dataPtr, 16);

      if (status == MFRC522::STATUS_OK) {
        Terminal.printf("  Sector %2d, Bloque %2d: OK\n", sector, blk);
        algunSectorEscrito = true;
      } else {
        Terminal.printf("  Sector %2d, Bloque %2d: FAIL\n", sector, blk);
      }
    }

    mfrc522->PCD_StopCrypto1();
  }

  if (algunSectorEscrito) {
    nfcResetFallos();
    Terminal.println("\n[EXITO] Proceso de clonacion finalizado.");
  } else {
    nfcRegistrarFallo();
    Terminal.println("\n[ERROR] No se pudo escribir ningun sector en la tarjeta.");
  }

  mfrc522->PICC_HaltA();
  mfrc522->PCD_StopCrypto1();

  if (spiMutex) xSemaphoreGive(spiMutex);

  vTaskDelay(3000 / portTICK_PERIOD_MS);
  nfc.modo = 0;
  mostrarMenuNFC();
}

// ============================================================================
// CALLBACKS DEL MÓDULO (Module Registry)
// ============================================================================
void nfcLoop() {
  if (nfc.modo == 0 && (millis() - tiempoUltimoMenuNFC >= intervaloMenu)) mostrarMenuNFC();
  if (nfc.modo == 1) modoLecturaNFC();
  if (nfc.modo == 4) modoEscrituraNFC();
}

void nfcEntrada(const String& cmd) {
  procesarEntradaNFC(cmd);
}

const Modulo moduloNFC = {
  "NFC Cloner v14",
  NULL,
  nfcLoop,
  NULL,
  mostrarMenuNFC,
  nfcEntrada
};
