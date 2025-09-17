#pragma once
#include <Arduino.h>

// Aquí definimos un "molde" llamado motor.
// Sirve para manejar un motor conectado al Arduino con dos cables de control.
struct motor {
  const byte pin1;   // Primer cable que controla el motor
  const byte pin2;   // Segundo cable que controla el motor

  // Constructor:
  // Cuando creamos un motor, le decimos qué dos pines del Arduino se usarán.
  motor(byte p1, byte p2);

  // Métodos (funciones que puede hacer el motor):
  void set_up();     // Prepara los pines del motor para poder usarlos
  void adelante();   // Hace que el motor gire hacia adelante
  void atras();      // Hace que el motor gire hacia atrás
  void detener();    // Apaga el motor (se detiene)
};
