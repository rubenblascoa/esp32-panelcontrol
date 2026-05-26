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
 * @file dht.h
 * @brief Interfaz de adquisicion del sensor digital DHT11 de temperatura y humedad.
 *        El objeto DHT de la libreria Adafruit se mantiene como detalle de
 *        implementacion interno en dht.cpp — este header no lo expone.
 */
#pragma once

#include "config.h"

// ============================================================================
// DECLARACION DE FUNCIONES DEL SENSOR DHT11
// ============================================================================

/**
 * @brief Inicializa el bus de comunicacion serie con el sensor DHT11 y configura sus parametros internos.
 */
void inicializarDHT();

/**
 * @brief Lee el registro de temperatura del sensor y devuelve el valor en grados Celsius.
 * @return float Temperatura en C; retorna -127.0 si el sensor no responde o los datos estan corruptos.
 */
float leerTemperatura();

/**
 * @brief Obtiene la humedad relativa capturada por el higrometro digital del modulo DHT11.
 * @return float Humedad relativa en porcentaje (0.0-100.0); retorna -1.0 si la lectura es invalida.
 */
float leerHumedad();

/**
 * @brief Ejecuta una lectura simultanea de temperatura y humedad, actualizando las variables globales
 *        temperaturaActual y humedadActual. Retorna true si ambas lecturas son validas.
 * @return bool Verdadero si el sensor respondio con datos integros, falso en caso de error.
 */
bool actualizarDHT();
