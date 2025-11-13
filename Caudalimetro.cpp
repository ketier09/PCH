#include "Caudalimetro.h"

caudalimetro::caudalimetro(byte p) : pin(p) {}

void caudalimetro::set_up() {
  if (!digitalPinIsValid(pin)) {
    lastError = "Pin inválido para caudalímetro";
    Serial.print(F("\n[Caudalímetro] "));
    Serial.println(lastError);
    return;
  }

  pinMode(pin, INPUT);

  int irq = digitalPinToInterrupt(pin);
  if (irq == NOT_AN_INTERRUPT) {
    lastError = "El pin no soporta interrupciones";
    Serial.print(F("\n[Caudalímetro] "));
    Serial.println(lastError);
    return;
  }

  attachInterruptArg(irq, &caudalimetro::isrThunk, this, FALLING);
  initialized = true;
}

void IRAM_ATTR caudalimetro::isrThunk(void* arg) {
  auto* self = static_cast<caudalimetro*>(arg);

  // Evita desbordes del contador
  if (self->pulseCount < UINT32_MAX) {
    self->pulseCount++;
  } else {
    self->errorFlag = true;
  }
}

float caudalimetro::reading() {
  if (!initialized) {
    lastError = "Error: se llamó a reading() antes de set_up()";
    Serial.print(F("\n[Caudalímetro] "));
    Serial.println(lastError);
    return -1;  // Valor inválido
  }

  if (errorFlag) {
    lastError = "Overflow: pulseCount llegó a UINT32_MAX";
    Serial.print(F("\n[Caudalímetro] "));
    Serial.println(lastError);
    errorFlag = false;
  }

  const uint32_t now = millis();
  if (now - lastMillis >= periodo_de_las_mediciones) {
    lastMillis = now;

    uint32_t pulses;
    portENTER_CRITICAL(&mux);
    pulses = pulseCount;
    pulseCount = 0;
    portEXIT_CRITICAL(&mux);

    flowRate = pulses / FLOW_CALIBRATION_FACTOR;
  }
  return flowRate * kappa;
}

const char* caudalimetro::getLastError() {
  return lastError;
}

