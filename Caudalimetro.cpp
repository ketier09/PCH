#include "Caudalimetro.h"

caudalimetro::caudalimetro(byte p) : pin(p) {}

void caudalimetro::set_up() {
  pinMode(pin, INPUT_PULLDOWN);

  attachInterruptArg(digitalPinToInterrupt(pin), isrThunk, this, FALLING);
  
}

void IRAM_ATTR caudalimetro::isrThunk(void *p) {
  auto *self = static_cast<caudalimetro*>(p);
  self->instance_isrThunk();
}
void caudalimetro::instance_isrThunk() {
  pulseCount++;
}

float caudalimetro::reading() {

  const uint32_t now = millis();
  if (now - lastMillis >= (uint32_t)periodo_de_las_mediciones) {
    lastMillis = now;

    uint32_t pulses;
    portENTER_CRITICAL(&mux);
    pulses = pulseCount;
    pulseCount = 0;
    portEXIT_CRITICAL(&mux);
    
    // Cálculo de Frecuencia (Pulsos/s)
    float pulseFrequency = (float)pulses * (1000.0f / periodo_de_las_mediciones);

    // Cálculo del caudal instantáneo (L/min)
    flowRate = pulseFrequency / FLOW_CALIBRATION_FACTOR;
    
    // --- Aplicación del Filtro Exponencial Móvil (EMA) ---
    if (isnan(flowRate_f)) {
      // Inicialización: si es el primer valor, se toma directamente
      flowRate_f = flowRate; 
    } else {
      flowRate_f = suavizador * flowRate + (1.0f - suavizador) * flowRate_f;
    }

  }
  
  // Retorna el valor filtrado (L/min) convertido a m³/s
  return flowRate_f * kappa;
}