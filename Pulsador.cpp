/* ----------------------------------------------------------------------------
 * 🔘 Módulo: Pulsador (Button Handler)
 * ----------------------------------------------------------------------------
 * Este módulo implementa un pulsador físico con:
 *   - Detección confiable de flancos por liberación
 *   - Antirrebote por software (debounce)
 *   - Soporte tanto para botones activos en HIGH como activos en LOW
 *   - Callback automático al liberar el botón
 *
 * Esta implementación es robusta y estable para sistemas embebidos en ESP32,
 * asegurando que la acción solo se dispare una vez por pulsación real.
 *
 * 📌 No tiene issues asociadas actualmente.
 * ----------------------------------------------------------------------------
 */

#include "Pulsador.h"

pulsador::pulsador(byte p, void (*i)(), bool active_state)
  // 🔍 Convierte automáticamente el estado activo físico (HIGH/LOW)
  //     en un booleano uniforme para lógica interna.
  : pin(p), 
    isHighActive(active_state == HIGH),  // true → HIGH activo | false → LOW activo
    orden(i), 
    // lastRaw se inicializa al estado contrario del activo para garantizar
    // que la primera lectura detecte un cambio correctamente.
    lastRaw(active_state != HIGH), 
    lastTrigger(0) 
{}

void pulsador::set_up() {
  // 🧠 Si el botón es activo en HIGH, usamos INPUT.
  //     Si es activo en LOW, se asume un botón con resistencia pull-up → INPUT_PULLUP.
  // Esto evita fluctuaciones de voltaje o lectura inestable.
  pinMode(pin, isHighActive ? INPUT : INPUT_PULLUP);
}

// 🔄 update(): Debe ejecutarse frecuentemente en el loop principal.
// Lee el estado, detecta cambios y despacha el callback cuando el botón se libera.
void pulsador::update() {
  unsigned long now = millis();
  bool raw = digitalRead(pin);

  if (raw != lastRaw) {
    // ✔ "isReleased" significa que se detectó el flanco correcto:
    //   - Para HIGH-Active → flanco de bajada
    //   - Para LOW-Active  → flanco de subida
    //
    // Esto unifica el comportamiento, sin importar el hardware.
    const bool isReleased = (raw != isHighActive);

    // ✔ Se dispara la acción SOLO si:
    //   1. Es liberación
    //   2. Se respetó el tiempo de antirrebote
    if (isReleased && (now - lastTrigger) > DEBOUNCE_MS) {
      orden();          // Ejecuta la acción definida por el usuario
      lastTrigger = now;
    }

    lastRaw = raw;       // Actualiza último estado leído
  }
}