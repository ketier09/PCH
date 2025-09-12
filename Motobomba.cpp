#include "Motobomba.h"

motobomba::motobomba(byte p)
  : pin(p) {}

void motobomba::set_up() {
  pinMode(pin, OUTPUT);
}

void motobomba::establecer_estado(byte estado) {
  estado = estado%2;
}
