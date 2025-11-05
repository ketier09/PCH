#pragma once
#include <Arduino.h>

class RGB {
  public:
  RGB(unit8_t R,  unit8_t G,  unit8_t B,
      unit8_t r1, unit8_t g1, unit8_t b1,
      unit8_t r2, unit8_t g2, unit8_t b2,
      unit8_t r3, unit8_t g3, unit8_t b3,
      unit8_t r4, unit8_t g4, unit8_t b4
      );

  void set_up();

private:
  static constexpr int n = 4;

  const unit8_t pin[3];
  const int estados[n][3];
}