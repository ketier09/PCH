#include "RGB.h"

RGB::RGB(unit8_t R,  unit8_t G,  unit8_t B,
      unit8_t r1, unit8_t g1, unit8_t b1,
      unit8_t r2, unit8_t g2, unit8_t b2,
      unit8_t r3, unit8_t g3, unit8_t b3,
      unit8_t r4, unit8_t g4, unit8_t b4
      )
  : pin{R,G,B}, estados{{r1,g1,b1}, {r2,g2,b2}, {r3,g3,b3}, {r4,g4,b4}}{}

void RGB::set_up() {
  for(int i = 0; i < 3; i++){
    pinMode(pin[i], OUTPUT);
  }
}

void RGB::establecer_estado(uint8_t estado_deseado) {
  for(int i = 0; i < 3; i++){
    digitalWrite(pin[i],estados[estado_deseado][i])
  }
}