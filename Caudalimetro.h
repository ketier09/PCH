#pragma once
#include <Arduino.h>

struct caudalimetro {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  
  static constexpr float periodo_de_las_mediciones = 2000;
  static constexpr float FLOW_CALIBRATION_FACTOR = 450.0f/*pulsos por litro*/*periodo_de_las_mediciones/1000;
  static constexpr float ESCALA = 1.0f;
  
  const byte pin;
  void (*isr)();

  caudalimetro(byte p, void (*i)());

  uint32_t lastMillis = 0;
  float flowRate = 0.0f;
  
  volatile uint32_t pulseCount = 0;
  void set_up();
  float reading();
};
