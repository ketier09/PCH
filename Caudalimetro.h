#pragma once
#include <Arduino.h>

class caudalimetro {
public:

  caudalimetro(byte p);

  void set_up();
  float reading();

private:
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  static constexpr float periodo_de_las_mediciones = 2000; // ms
  // 450 pulsos/L → 450000 pulsos/m^3. Resultado: m^3/s
  static constexpr float PULSES_PER_M3 = 450000.0f;
  static constexpr float FLOW_CALIBRATION_FACTOR =
      PULSES_PER_M3 * (periodo_de_las_mediciones / 1000.0f);
  // 💡 OPTIMIZACIÓN: Si flowRate ya es m³/s, ESCALA debe ser 1.0f para mantener la unidad.
  static constexpr float ESCALA = 1.0f; 

  const byte pin;

  uint32_t lastMillis = 0;
  float    flowRate   = 0.0f;          // m^3/s 
  volatile uint32_t pulseCount = 0;

  // ISR genérica: estática y con arg
  static void IRAM_ATTR isrThunk(void* arg);

};