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
 * @file nfc.cpp
 * @brief Implementación de procesos de lectura, descifrado y sobreescritura de sectores NFC.
 */
#include "nfc.h"        // Inclusión de la interfaz propia
#include "terminal.h"   // Permite imprimir los reportes de éxito o error en la terminal híbrida
#include "menus.h"      // Permite invocar la interfaz de refresco mostrarMenuNFC() al finalizar el ciclo

// ============================================================================
// SUBRUTINA DE INTERROGACIÓN PASIVA Y VOLCADO DE MEMORIA NFC
// ============================================================================
void modoLecturaNFC() {                   
  // Sondeo de bajo nivel en el bus SPI. Si no hay tarjeta presente o no se lee su número de serie, aborta de inmediato
  if (!mfrc522->PICC_IsNewCardPresent() || !mfrc522->PICC_ReadCardSerial()) return;

  byte buffer[18];            // Búfer local temporal (MIFARE lee en bloques de 16 bytes + 2 bytes de CRC)
  byte size = sizeof(buffer); // Descriptor de tamaño obligatorio exigido por la biblioteca
  
  // Desafío criptográfico: Autenticamos el Sector 0 usando el comando Key A estándar y la clave almacenada en RAM
  MFRC522::StatusCode status = mfrc522->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522->uid));
  
  // Evaluamos si el chip destino aceptó el protocolo de cifrado simétrico
  if (status == MFRC522::STATUS_OK) {     
    // Ejecutamos la lectura en crudo del bloque 0 (contiene UID, BCC y datos del fabricante)
    status = mfrc522->MIFARE_Read(0, buffer, &size);
    
    // Si la transferencia de datos a través del bus SPI fue exitosa
    if (status == MFRC522::STATUS_OK) {   
      // Transferencia segura indexada byte a byte hacia la matriz de memoria RAM global
      for (byte j = 0; j < 16; j++) bloqueEscaneado[j] = buffer[j];
      
      memoriaLlena = true; // Elevamos la bandera lógica para indicar que los datos son íntegros y válidos
      Terminal.println("\n[EXITO] Bloque 0 guardado."); // Imprimimos la confirmación en Telnet/Web
    }
  }
  
  // Secuencia mandatoria de apagado para liberar la tarjeta física y detener el cifrado del hardware
  mfrc522->PICC_HaltA(); // Desactiva la bobina de la tarjeta
  mfrc522->PCD_StopCrypto1(); // Libera las unidades criptográficas del lector
  
  vTaskDelay(2000 / portTICK_PERIOD_MS); // Pausa anti-rebote electromagnético de 2 segundos para evitar lecturas fantasmas cíclicas
  modoNFC = 0; // Restablecemos el sub-estado a modo pasivo (espera)
  mostrarMenuNFC(); // Redibujamos de forma asíncrona el menú en la consola del usuario
}

// ============================================================================
// SUBRUTINA DE MODULACIÓN OPERATIVA Y ESCRITURA EN TARJETAS DESTINO
// ============================================================================
void modoEscrituraNFC() {                 
  // Sondeo del bus SPI. Si no encuentra ninguna tarjeta preparada en el campo inclinable, retorna
  if (!mfrc522->PICC_IsNewCardPresent() || !mfrc522->PICC_ReadCardSerial()) return;
  
  // Autenticación mandatoria del bloque de destino antes de intentar operaciones de alteración de celdas
  MFRC522::StatusCode status = mfrc522->PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 0, &key, &(mfrc522->uid));
  
  // Si la tarjeta destino valida la clave criptográfica de acceso
  if (status == MFRC522::STATUS_OK) {     
    // Intentamos la inyección del bloque de memoria mediante comando directo de escritura SPI
    // Nota crítica: Requiere que la tarjeta objetivo sea de tipo "Magic Card" uid-changeable (CUID/FUID)
    status = mfrc522->MIFARE_Write(0, bloqueAEscribir, 16);
    
    // Despacho de logs de diagnóstico según el resultado eléctrico devuelto por el hardware
    if (status == MFRC522::STATUS_OK) Terminal.println("\n[EXITO] Tarjeta Clonada.");
    else Terminal.println("\n[ERROR] Tarjeta no grabable.");
  }
  
  // Apagado físico del canal inductivo de radiofrecuencia
  mfrc522->PICC_HaltA();
  mfrc522->PCD_StopCrypto1();
  
  vTaskDelay(3000 / portTICK_PERIOD_MS); // Espera extendida de 3 segundos para asegurar la fijación de la EEPROM física del tag
  modoNFC = 0; // Regreso seguro a la máquina de estados en reposo
  mostrarMenuNFC(); // Refresca los terminales conectados
}