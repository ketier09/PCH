#pragma once
#include <stdint.h>
#include <Arduino.h>

class image {
public:
  constexpr image(const uint16_t* p, int w = 120, int h = 120)
  : pixels(p), width(w), height(h) {}

private:
  const uint16_t* pixels;
  const int width;
  const int height;
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
