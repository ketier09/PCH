#pragma once
#include <Arduino.h>

class Led {
  public:
  Led(uint8_t R,  uint8_t G,  uint8_t B,
      uint8_t r1, uint8_t g1, uint8_t b1,
      uint8_t r2, uint8_t g2, uint8_t b2,
      uint8_t r3, uint8_t g3, uint8_t b3,
      uint8_t r4, uint8_t g4, uint8_t b4
      );

  void set_up();
  void establecer_estado(uint8_t estado_deseado);

private:
  static constexpr int n = 4;

  const uint8_t pin[3];
  const int estados[n][3];
};