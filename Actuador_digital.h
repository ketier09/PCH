#pragma once
#include <Arduino.h>

class actuador_digital {
public:
  actuador_digital(byte p);
  void set_up();
  void cambiar();

private:
  const byte pin;
  bool estado;
};
