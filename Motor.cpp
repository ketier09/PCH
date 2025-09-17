#include "Motor.h"

// Cuando se crea un motor, hay que decirle en qué dos pines (cables)
// del Arduino está conectado. Un motor necesita dos para poder girar
// en ambos sentidos.
motor::motor(byte p1, byte p2)
  : pin1(p1), pin2(p2) {}

// Esta función prepara los dos pines para que el Arduino
// los use como "interruptores" de salida y así poder controlar el motor.
void motor::set_up() {
  pinMode(pin1, OUTPUT);   // El primer cable funcionará como salida
  pinMode(pin2, OUTPUT);   // El segundo cable también funcionará como salida
}
