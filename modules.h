// MIT License
//
// Copyright (c) 2026 Ruben Blasco Armengod
//
// Registro de módulos — cada módulo se autodescribe mediante un struct
// con punteros a función, eliminando el acoplamiento del switch(FSM_GET())
// en tareas.cpp.

#pragma once

#include "config.h"

typedef struct {
  const char* nombre;
  void (*init)();
  void (*loop)();
  void (*stop)();
  void (*menu)();
  void (*entrada)(const String& cmd);
} Modulo;
