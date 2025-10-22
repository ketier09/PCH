#pragma once
#include <stdint.h>
#include <Arduino.h>

struct image {
  const int width;
  const int height;
  const uint16_t*  pixels;

  constexpr image(int w, int h, const uint16_t* p)
  : width(w), height(h), pixels(p) {}
};

enum coleccionImagenes : uint8_t {
  sanBartLogo,
  total,
  captacion,
  azud,
  vertederos,
  turbina,
  desarenador,

  cantidadImagenes
};

extern const image imagenes[cantidadImagenes];
