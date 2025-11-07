#include "Pulsador.h"

pulsador::pulsador(byte p, void (*i)(), bool active_state)
  // 💡 OPTIMIZACIÓN: Uso de isHighActive
  : pin(p), isHighActive(active_state == HIGH), orden(i), 
    // lastRaw se inicializa a la inversa del estado activo
    lastRaw(active_state != HIGH), lastTrigger(0) {}

void pulsador::set_up() {
  // 💡 OPTIMIZACIÓN: Lógica más clara para INPUT/INPUT_PULLUP
  // Si es HIGH activo (isHighActive=true), usa INPUT. Si es LOW activo, usa INPUT_PULLUP.
  pinMode(pin, isHighActive ? INPUT : INPUT_PULLUP);
}

void pulsador::update() {
  unsigned long now = millis();
  bool raw = digitalRead(pin);

  // Detecta el cambio de estado
  if (raw != lastRaw) {
    // 💡 OPTIMIZACIÓN: La detección original ya es para la LIBERACIÓN (raw = NO activo).
    // isReleased es true cuando el pin vuelve a su estado de reposo (no activo)
    const bool isReleased = (raw != isHighActive); 

    // Se dispara la acción al liberar (flanco de bajada si es HIGH-Active, flanco de subida si es LOW-Active)
    if (isReleased && (now - lastTrigger) > DEBOUNCE_MS) { 
      orden();
      lastTrigger = now;
    }
    lastRaw = raw;
  }
}