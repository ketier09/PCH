#pragma once
#include <Arduino.h>

class pulsador {
public:
  // (Pin, Función Callback, Estado Lógico Activo (HIGH/LOW))
  pulsador(byte p, void (*i)(), bool active_state); // 💡 OPTIMIZACIÓN: Renombrado de pr

  void set_up();
  void update();

private:
  static constexpr unsigned long DEBOUNCE_MS = 100; // 💡 OPTIMIZACIÓN: Renombrado

  const byte pin;
  // 💡 OPTIMIZACIÓN: Variable que indica si el botón es HIGH-Active (e.g. no usa pullup)
  const bool isHighActive; 
  void (*orden)();

  bool lastRaw;
  unsigned long lastTrigger;
};