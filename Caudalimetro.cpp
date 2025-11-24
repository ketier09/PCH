// ============================================================================
// 📌 Archivo: Caudalimetro.cpp
// 📘 Descripción general
// Este archivo contiene la implementación completa del módulo caudalímetro.
// Se encarga de:
//    - Configurar el pin y registrar la ISR.
//    - Contar pulsos provenientes del sensor.
//    - Calcular frecuencia → caudal L/min.
//    - Aplicar un filtro exponencial móvil (EMA) para estabilizar la lectura.
//    - Convertir la salida final a m³/s.
//
// 🔧 Issue relacionada indirecta:
//    - Lecturas ruidosas o fluctuantes → solucionado con EMA (suavizador).
//    - No tiene issues relacionadas con Firestore.
// ============================================================================

#include "Caudalimetro.h"

caudalimetro::caudalimetro(byte p) 
  : pin(p) {}

// Configura el pin y adjunta la interrupción
void caudalimetro::set_up() {

  // Entrada con pull-up interno para sensores tipo efecto Hall
  pinMode(pin, INPUT_PULLUP);

  // Usa attachInterruptArg para pasar "this" de forma segura
  attachInterruptArg(
    digitalPinToInterrupt(pin),
    isrThunk,
    this,
    FALLING  // El sensor genera pulsos al caer la señal
  );
}

// ISR estática: recibe puntero genérico y lo convierte a objeto real
void IRAM_ATTR caudalimetro::isrThunk(void *p) {
  auto *self = static_cast<caudalimetro*>(p);
  self->instance_isrThunk();
}

// ISR real: solo incrementa contador (máximo rendimiento)
void caudalimetro::instance_isrThunk() {
  pulseCount++;
}

// Lectura general del sensor (cada periodo)
// Devuelve caudal filtrado en m³/s
float caudalimetro::reading() {

  const uint32_t now = millis();

  // Solo calcula cuando pasa el periodo definido (1s)
  if (now - lastMillis >= (uint32_t)periodo_de_las_mediciones) {

    lastMillis = now;

    // Copia protegida de pulsos (porque pulseCount se modifica en la ISR)
    uint32_t pulses;
    portENTER_CRITICAL(&mux);
    pulses = pulseCount;
    pulseCount = 0;  // Reiniciar para el siguiente ciclo
    portEXIT_CRITICAL(&mux);

    // Frecuencia = pulsos / segundo
    float pulseFrequency = (float)pulses * (1000.0f / periodo_de_las_mediciones);

    // Conversión a L/min según calibración
    flowRate = pulseFrequency / FLOW_CALIBRATION_FACTOR;

    // Filtro exponencial móvil (primer valor toma directo)
    if (isnan(flowRate_f)) {
      flowRate_f = flowRate;
    } else {
      flowRate_f = suavizador * flowRate + (1.0f - suavizador) * flowRate_f;
    }
  }

  // Devuelve lectura en m³/s
  return flowRate_f * kappa;
}
