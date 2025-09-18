#pragma once
#include <Arduino.h>
#include <Servo.h>

// Aquí definimos un "molde" llamado motor.
// Sirve para manejar un motor conectado al Arduino con dos cables de control.
struct motor {
  const byte pin;   // Primer cable que controla el motor

  // Constructor:
  motor(byte p);
  servo servo;
  int estados[4];

  // Métodos (funciones que puede hacer el motor):
  void set_up();     // Prepara los pines del motor para poder usarlos
  void cambiar_estado(byte estado);
};

