#include "MiServo.h"

// Cuando se crea un motor, hay que decirle en qué dos pines (cables)
// del Arduino está conectado. Un motor necesita dos para poder girar
// en ambos sentidos.
motor::motor(byte p)
  : pin(p) {}

const int motor::estados[] = {1, 2, 3, 4, 5};
const size_t motor::n = sizeof(estados) / sizeof(estados[0]);

// Esta función prepara los dos pines para que el Arduino
// los use como "interruptores" de salida y así poder controlar el motor.
void motor::set_up() {
  servo.attach(pin, estados[0], estados[n-1]);  // El segundo cable también funcionará como salida
}

void motor::siguiente_estado(){
  int m = 2*n;
  estado++;
  estado = estado % m;
  if(estado >= n){
    estado = m-estado-2;
  }
  servo.write(estados[estado]);
}





