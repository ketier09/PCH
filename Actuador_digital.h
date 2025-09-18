#pragma once
#include <Arduino.h>

struct valvula_motobomba {
  const byte pin;

  valvula_motobomba(byte p);
  void set_up();
  void ejecutar_orden(char comando);
};
