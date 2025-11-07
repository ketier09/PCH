#pragma once
#include <Arduino.h>

class actuador_digital {
public:
  actuador_digital(byte p);
  void set_up();
  void encender();
  void apagar();
  void cambiar();

private:
  const byte pin;
  bool estado;
};
