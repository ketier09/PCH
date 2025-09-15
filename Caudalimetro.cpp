#include "Caudalimetro.h"   // Incluimos el archivo de cabecera con la definición anterior

// Constructor: inicializa el pin y la función de interrupción
caudalimetro::caudalimetro(byte p, void (*i)())
  : pin(p), isr(i) {}

// Configuración inicial del caudalímetro
void caudalimetro::set_up() {
  pinMode(pin, INPUT_PULLUP);   // Configura el pin como entrada con resistencia interna
  attachInterrupt(digitalPinToInterrupt(pin), isr, FALLING);  
  // Asocia la función de interrupción "isr" al pin del sensor
  // Se activa cuando la señal del sensor baja (FALLING)
}

// Función para leer el caudal
float caudalimetro::reading() {
  // Comprueba si ya pasaron "periodo_de_las_mediciones" (2 segundos por defecto)
  if (millis() - lastMillis >= periodo_de_las_mediciones) {
    lastMillis = millis();   // Actualiza el tiempo de la última medición
    
    uint32_t pulses;         // Variable temporal para guardar los pulsos contados

    // Bloque crítico: protegemos el acceso a pulseCount con un candado
    portENTER_CRITICAL(&mux);
    pulses = pulseCount;     // Copiamos el número de pulsos acumulados
    pulseCount = 0;          // Reiniciamos el contador para la siguiente medición
    portEXIT_CRITICAL(&mux);

    // Calculamos el caudal: pulsos / factor de calibración
    flowRate = pulses / FLOW_CALIBRATION_FACTOR;
  }

  // Devolvemos el caudal corregido por el factor de escala
  return flowRate * ESCALA;
}
