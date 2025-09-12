#pragma once
#include <Arduino.h>

struct motobomba {
  
  static constexpr byte estados_motobomba[3] = { 0 };
  const byte pin;

  motobomba(byte p);

  void set_up();
  void establecer_estado(byte estado);
};
