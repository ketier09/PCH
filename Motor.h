#pragma once
#include <Arduino.h>

struct motor {
  const byte pin1;
  const byte pin2;

  // Constructor
  motor(byte p1, byte p2);

  // Métodos
  void set_up();
  void adelante();
  void atras();
  void detener();
};
