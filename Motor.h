#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class motor {
public:
  static constexpr int n = 4;

  motor(byte p, int e1, int e2, int e3, int e4);

  void set_up();
  void siguiente_estado();

private:
  const byte pin;
  const int estados[n];

  Servo servo;

  int estado = 0;
};
