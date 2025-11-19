#pragma once
#include <Arduino.h>

class caudalimetro {
public:

  caudalimetro(byte p);

  void set_up();
  float reading();

  static void IRAM_ATTR isrThunk(void *p);
  void IRAM_ATTR instance_isrThunk();

private:
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  // 1. CORRECCIÓN: Periodo de medición a 1 segundo para estabilidad.
  static constexpr float periodo_de_las_mediciones = 1000.0f; // ms (1 segundo para toma de pulsos)
  
  // 450 pulsos/L
  static constexpr float PULSES_PER_LITTER = 450.0f;
  
  // 2. CORRECCIÓN: Factor de calibración (Hz por L/min)
  // 450 pulsos / 60 segundos = 7.5 Hz por L/min
  static constexpr float FLOW_CALIBRATION_FACTOR = PULSES_PER_LITTER / 60.0f; 
  
  static constexpr float suavizador = 0.3f;
  static constexpr float kappa = 1.0f; //min*m³/s*L

  const byte pin;

  uint32_t lastMillis = 0;
  float    flowRate   = NAN;
  float    flowRate_f = NAN;
  bool initialized = false;

  volatile uint32_t pulseCount = 0;
};