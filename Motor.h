#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

struct motor {
  static constexpr int n = 4;

  const byte pin;
  const int estados[n];

  motor(byte p, int e1, int e2, int e3, int e4);

  ESP32Servo servo;  // CAMBIAR Servo por ESP32Servo

  int estado = 0;
  void set_up();
  void siguiente_estado();
};
