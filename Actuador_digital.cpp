#include "Actuador_digital.h"

valvula_motobomba::valvula_motobomba(byte p)
  : pin(p) {}

void valvula_motobomba::set_up(){
  pinMode(pin, OUTPUT);
}

void valvula_motobomba::apagar(){
  digitalWrite(pin, LOW);
}

void valvula_motobomba::encender(){
  digitalWrite(pin, HIGH);
}
