#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>

#include "Datos.h"

struct pantalla {

  const byte indices[3]; // índices en el arreglo de datos

  // Constructor
  pantalla(byte t1, byte t2, byte t3);

  // Métodos
  void set_up();
  void enviar(datos dataArray[]);
};
