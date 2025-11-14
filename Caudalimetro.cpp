#include "Caudalimetro.h"

caudalimetro::caudalimetro(byte p) : pin(p) {}

void caudalimetro::set_up() {
  // ... (código de set_up() sin cambios)
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
    flowRate = (pulseFrequency / FLOW_CALIBRATION_FACTOR) * 60.0f;
    
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

const char* caudalimetro::getLastError() {
  return lastError;
}