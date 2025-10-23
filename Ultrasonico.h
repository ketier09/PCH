#pragma once
#include <Arduino.h>
#include <math.h>

class ultrasonico {
public:

  ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe);

  void set_up();
  float reading(uint32_t timeout_us = 30000); // ~5 m máx
  float flujo();

  static void IRAM_ATTR isrThunk(void* arg);

private:
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  static constexpr float kappa = 1.0f;
  static constexpr float ESCALA = 1.0f; // m/cm (Se mantiene 1.0f para no romper el código, ver sugerencia 2)
  static constexpr float NIVEL_0 = 1268.0f;
  static constexpr float manningInverso = 1.0f / 0.013f;
  static constexpr float suavizador = 0.30f; //Para evitar transiciones drásticas

  const byte trig;
  const byte echo;

  const float techo;  // m
  const float piso;   // m
  const float ancho;  // m
  const float raizCuadrada_pendiente; // = sqrt(S)

  float nivel = NAN;
  float nivel_f = nivel;
  volatile uint32_t disparo = 0;
  volatile uint32_t duracion = 0;

  void disparar();
};
