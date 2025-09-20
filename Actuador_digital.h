#pragma once
#include <Arduino.h>

struct actuador_digital {
  const byte pin;

  actuador_digital(byte p);
  bool estado;
  void set_up();
  void cambiar();
};
