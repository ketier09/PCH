#include "Led.h"

Led::Led(uint8_t R,  uint8_t G,  uint8_t B,
      uint8_t r1, uint8_t g1, uint8_t b1,
      uint8_t r2, uint8_t g2, uint8_t b2,
      uint8_t r3, uint8_t g3, uint8_t b3,
      uint8_t r4, uint8_t g4, uint8_t b4
      )
  : pin{R,G,B}, estados{{r1,g1,b1}, {r2,g2,b2}, {r3,g3,b3}, {r4,g4,b4}}{}

void Led::set_up() {
  for(int i = 0; i < 3; i++){
    pinMode(pin[i], OUTPUT);
  }
}

void Led::establecer_estado(uint8_t estado_deseado) {
  for(int i = 0; i < 3; i++){
    digitalWrite(pin[i],estados[estado_deseado][i]);
  }
}