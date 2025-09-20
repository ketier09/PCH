#pragma once
#include <Arduino.h>

struct pulsador {
  static constexpr unsigned long debounce = 50;
  const byte pin;
  const bool pressed;
  void (*orden)();

  pulsador(byte p, void (*i)(), bool pr);

  void set_up();
  void update();

private:
  bool lastRaw;
  unsigned long lastTrigger;
};
