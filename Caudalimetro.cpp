// Caudalimetro.cpp
#include "Caudalimetro.h"

caudalimetro::caudalimetro(byte p) : pin(p) {}

void caudalimetro::set_up() {
  pinMode(pin, INPUT);
  // Usa attachInterruptArg para pasar "this" a la ISR
  attachInterruptArg(digitalPinToInterrupt(pin), &caudalimetro::isrThunk, this, FALLING);
}

void IRAM_ATTR caudalimetro::isrThunk(void* arg) {
  auto* self = static_cast<caudalimetro*>(arg);
  // Incremento mínimo en ISR. No hace falta sección crítica aquí:
  // la lectura ya se protege con portENTER_CRITICAL en reading().
  self->pulseCount++;
}

float caudalimetro::reading() {
  const uint32_t now = millis();
  if (now - lastMillis >= periodo_de_las_mediciones) {
    lastMillis = now;

    uint32_t pulses;
    portENTER_CRITICAL(&mux);
    pulses = pulseCount;
    pulseCount = 0;
    portEXIT_CRITICAL(&mux);

    // m^3/s (pulsos / (pulsos/m^3 * s)) = m^3/s
    flowRate = pulses / FLOW_CALIBRATION_FACTOR;
  }
  return flowRate * ESCALA;
}

