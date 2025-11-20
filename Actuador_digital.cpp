#include "Actuador_digital.h"

actuador_digital::actuador_digital(byte p)
  : pin(p) {}

void actuador_digital::set_up(){
  pinMode(pin, OUTPUT);
  estado = LOW;
  digitalWrite(pin, estado);
  actuador_digital::cambiar();
  delay(1000);
  actuador_digital::cambiar();
}

void actuador_digital::encender(){
  estado = HIGH;
  digitalWrite(pin, estado);
}

void actuador_digital::apagar(){
  estado = LOW;
  digitalWrite(pin, estado);
}

void actuador_digital::cambiar(){
  estado = !estado;
  digitalWrite(pin, estado);
}
