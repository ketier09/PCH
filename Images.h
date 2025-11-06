#pragma once
#include <stdint.h>
#include <Arduino.h>

class image {
public:
  constexpr image(const uint16_t* p, int x , int y , int w , int h, char* l )
  : pixels(p), x_index(x), y_index(y), width(w), height(h), string_lugar(l) {}

  const uint16_t* pixels;
  const int x_index;
  const int y_index;
  const int width;
  const int height;
  const char* string_lugar;
};

const size_t NUM_IMAGENES = 5;
extern const image imagenes[NUM_IMAGENES];
