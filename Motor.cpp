#include "Motor.h"

// Cuando se crea un motor, hay que decirle en qué dos pines (cables)
// del Arduino está conectado. Un motor necesita dos para poder girar
// en ambos sentidos.
motor::motor(byte p, int e1, int e2, int e3, int e4)
  : pin(p), estados{e1, e2, e3, e4} {}

// Esta función prepara los dos pines para que el Arduino
// los use como "interruptores" de salida y así poder controlar el motor.
void motor::set_up() {
  servo.attach(pin, 500, 2500);  // El segundo cable también funcionará como salida
}

void motor::siguiente_estado(){
  int m = 2*n;
  estado = (estado + 1) % m;
  if(estado >= n){
    estado = m-estado-2;
  }
  servo.write(estados[estado]);
}





