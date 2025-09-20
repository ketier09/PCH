#pragma once
#include <Arduino.h>

struct actuador_digital {
  const byte pin;

  actuador_digital(byte p);
  void set_up();
  void apagar();
  void encender();
};
