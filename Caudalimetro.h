// Caudalimetro.h
#pragma once
#include <Arduino.h>

struct caudalimetro {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  static constexpr float periodo_de_las_mediciones = 2000; // ms
  // 450 pulsos/L → 450000 pulsos/m^3. Resultado: m^3/s
  static constexpr float FLOW_CALIBRATION_FACTOR =
      450000.0f * (periodo_de_las_mediciones / 1000.0f);

  static constexpr float ESCALA = 100.0f;

  const byte pin;

  caudalimetro(byte p);

  uint32_t lastMillis = 0;
  float    flowRate   = 0.0f;          // m^3/s (antes de ESCALA)
  volatile uint32_t pulseCount = 0;

  void set_up();
  float reading();

  // ISR genérica: estática y con arg
  static void IRAM_ATTR isrThunk(void* arg);
};
