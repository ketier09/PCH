#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class motor {
public:
  motor(byte p, int e1, int e2, int e3, int e4);

  void set_up();
  uint8_t siguiente_estado();
  void showState(uint8_t index);

private:
  static constexpr int n = 4; // Número de estados únicos (0 a 3)
  // 💡 OPTIMIZACIÓN: Longitud del ciclo limpio (0, 1, 2, 3, 2, 1) = 6
  static constexpr int MAX_SEQUENCE_LEN = 2 * n - 2; 

  const byte pin;
  const int estados[n];

  Servo servo;

  int estado_ciclico = 0; // 💡 OPTIMIZACIÓN: Renombrado para claridad
};