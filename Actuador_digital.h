#pragma once
#include <Arduino.h>

class actuador_digital {
public:
  actuador_digital(byte p);
  void set_up();
  void encender();
  void apagar();
  void cambiar();
  
  static constexpr float kappa = 0.0; //m³/s

private:
  const byte pin;
  bool estado;
};
