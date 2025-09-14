#include "Caudalimetro.h"

caudalimetro::caudalimetro(byte p, void (*i)())
  : pin(p), isr(i) {}

void caudalimetro::set_up() {
  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin), isr, FALLING);
}

float caudalimetro::reading() {
  if (millis() - lastMillis >= periodo_de_las_mediciones) {
    lastMillis = millis();
    uint32_t pulses;
    portENTER_CRITICAL(&mux);
    pulses = pulseCount;
    pulseCount = 0;
    portEXIT_CRITICAL(&mux);
    flowRate = pulses / FLOW_CALIBRATION_FACTOR;
  }
  return flowRate * ESCALA;
}
