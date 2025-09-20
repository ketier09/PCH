#include "Actuador_digital.h"

actuador_digital::actuador_digital(byte p)
  : pin(p) {}

void actuador_digital::set_up(){
  pinMode(pin, OUTPUT);
  estado = LOW;
  digitalWrite(pin, estado);
}

void actuador_digital::cambiar(){
  estado = !estado;
  digitalWrite(pin, estado);
}
