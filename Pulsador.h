#pragma once
#include <Arduino.h>

class pulsador {
public:
  pulsador(byte p, void (*i)(), bool pr);

  void set_up();
  void update();

private:
  static constexpr unsigned long debounce = 50;

  const byte pin;
  const bool pressed;
  void (*orden)();

  bool lastRaw;
  unsigned long lastTrigger;
};
