#include "Motor.h"

motor::motor(byte p, int e1, int e2, int e3, int e4)
  : pin(p), estados{e1, e2, e3, e4}{}

void motor::set_up() {
  servo.attach(pin, 500, 2500);
}

void motor::siguiente_estado(){
  int m = 2*n;
  estado = (estado + 1) % m;
  if(estado >= n){
    estado = m - 1 - estado; // (8 - 1 - 7 = 0)
  }
  servo.write(estados[estado]);
}
