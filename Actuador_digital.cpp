#include "Actuador_digital.h"

actuador_digital::actuador_digital(byte p)
  : pin(p) {}

void actuador_digital::set_up(){
  pinMode(pin, OUTPUT);
}

void actuador_digital::apagar(){
  digitalWrite(pin, LOW);
}

void actuador_digital::encender(){
  digitalWrite(pin, HIGH);
}
