#include "Pulsador.h"

// Se cambia 'pressed' por 'isHighActive' para mayor claridad.
pulsador::pulsador(byte p, void (*i)(), bool pr)
  : pin(p), isHighActive(pr), orden(i), lastRaw(!pr), lastTrigger(0) {}

void pulsador::set_up() {
  // Configuración más clara usando las constantes
  int mode = isHighActive ? INPUT_ACTIVE_HIGH : INPUT_ACTIVE_LOW;
  pinMode(pin, mode);
}

void pulsador::update() {
  const unsigned long now = millis();
  const bool raw = digitalRead(pin);

  // 1. Detección de Transición: Solo si el estado ha cambiado
  if (raw != lastRaw) {
    
    // 2. Transición a Estado "No Presionado" (Asumiendo trigger en la liberación):
    // El estado 'no presionado' es el opuesto a 'isHighActive'.
    const bool isReleased = (raw == !isHighActive); 
    
    // 3. Validación de Debounce y Ejecución:
    if (isReleased && (now - lastTrigger) > DEBOUNCE_MS) {
      // Se asume que la acción 'orden()' debe ejecutarse al soltar el botón,
      // que es el comportamiento implementado en el código original.
      orden();
      lastTrigger = now;
    }
    
    // 4. Actualizar Estado para el siguiente ciclo
    lastRaw = raw;
  }
}
