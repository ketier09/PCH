#pragma once
#include <Arduino.h>

struct valvula_motobomba {
  const byte valvula;
  const byte motobomba;

  valvula_motobomba(byte v, byte m);
  void set_up();
  void ejecutar_orden(char comando);
  void leer_orden();
};
