#pragma once
#include <stdint.h>
#include <Arduino.h>

class image {
public:
  constexpr image(const uint16_t* p, int x = 0, int y = 90, int w = 120, int h = 120)
  : pixels(p), x_index(x), y_index(y), width(w), height(h) {}

  const uint16_t* pixels;
  const int x_index;
  const int y_index;
  const int width;
  const int height;

private:

  static constexpr uint8_t NUM_IMAGENES = 5;
};

extern const image imagenes[NUM_IMAGENES];
