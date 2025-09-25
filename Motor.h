#pragma once
#include <Arduino.h>
#include <Servo.h>

// Aquí definimos un "molde" llamado motor.
// Sirve para manejar un motor conectado al Arduino con dos cables de control.
struct motor {
  const byte pin;   // Primer cable que controla el motor
  const int estados[4];
  const int n;

  // Constructor:
  motor(byte p, int e1, int e2, int e3, int e4);
  Servo servo;

  // Métodos (funciones que puede hacer el motor):
  void set_up();     // Prepara los pines del motor para poder usarlos
  void siguiente_estado();
};


