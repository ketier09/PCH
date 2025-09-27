#pragma once
#include <Arduino.h>
#include <math.h>

struct ultrasonico {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  static constexpr float ESCALA = 1.0f; // m/cm
  static constexpr float NIVEL_0 = 0.0f;
  static constexpr float manningInverso = 1.0f / 0.013f;

  const byte trig;
  const byte echo;

  const float techo;  // m
  const float piso;   // m
  const float ancho;  // m
  const float raizCuadrada_pendiente; // = sqrt(S)

  ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe);

  float nivel = NAN;
  volatile uint32_t disparo = 0;
  volatile uint32_t duracion = 0;

  void set_up();
  void disparar();
  float reading(uint32_t timeout_us = 30000); // ~5 m máx
  float flujo();

  static void IRAM_ATTR isrThunk(void* arg);
};


