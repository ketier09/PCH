#pragma once
#include <Arduino.h>

class caudalimetro {
public:

  caudalimetro(byte p);

  void set_up();
  float reading();
  const char* getLastError();

private:
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  static constexpr float periodo_de_las_mediciones = 1.0f; // ms (Se usa float para cálculos)
  // 450 pulsos/L
  static constexpr float PULSES_PER_LITTER = 450.0f;
  static constexpr float FLOW_CALIBRATION_FACTOR =
      PULSES_PER_LITTER * (periodo_de_las_mediciones / 1000.0f);
  static constexpr float suavizador = 0.3f;
  static constexpr float kappa = 1.0f; //min*m³/s*L

  const byte pin;

  uint32_t lastMillis = 0;
  float    flowRate   = NAN;
  float    flowRate_f = NAN;
  bool initialized = false;

  volatile uint32_t pulseCount = 0;

  // ISR genérica: estática y con arg
  static void IRAM_ATTR isrThunk(void* arg);
};