#include "Motor.h"

motor::motor(byte p1, byte p2)
  : pin1(p1), pin2(p2) {}

void motor::set_up() {
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
}
