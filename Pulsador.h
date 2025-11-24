#pragma once
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

#include <Arduino.h>

class pulsador {
public:
  // 📌 Constructor
  // p → pin digital del ESP32
  // i → función callback que se ejecutará al liberar el botón
  // active_state → HIGH si el botón activa en 1 lógico, LOW si activa en 0
  pulsador(byte p, void (*i)(), bool active_state);

  void set_up();
  void update();

private:
  // ⏱ Tiempo mínimo entre detecciones para evitar que un mismo rebote
  // cause múltiples activaciones.
  static constexpr unsigned long DEBOUNCE_MS = 400;

  const byte pin;
  // 🔍 Indica si el botón está cableado para ser activo en HIGH
  // Esto permite abstraer la polaridad del hardware.
  const bool isHighActive;

  void (*orden)();

  bool lastRaw;
  unsigned long lastTrigger;
};