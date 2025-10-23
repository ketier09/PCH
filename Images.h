#pragma once
#include <stdint.h>
#include <Arduino.h>

class image {
public:
  constexpr image(const uint16_t* p, int x = 0, int y = 90, int w = 120, int h = 120)
  : pixels(p), x_index(x), y_index(y), width(w), height(h) {}

private:
  const uint16_t* pixels;
  const x_index;
  const y_index;
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
