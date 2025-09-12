#pragma once
#include <Arduino.h>

struct ultrasonico {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  
  static constexpr float ESCALA   = 1.0f;
  static constexpr float manningInverso  = 1.0f/0.013f;

  const byte trig;
  const byte echo;
  void (*echoRising)();
  void (*echoFalling)();
  const float techo;
  const float piso;
  const float ancho;
  const float raizCuadrada_pendiente;

  ultrasonico(byte t, byte e, void (*i1)(), void (*i2)(), float te, float pi, float a, float pe);

  float nivel;
  
  volatile uint32_t disparo;
  volatile uint32_t duracion;

  void set_up();
  float reading();
  float flujo();
};
