#pragma once
#include <Arduino.h>

class pulsador {
public:
  static constexpr unsigned long debounce = 50;

  pulsador(byte p, void (*i)(), bool pr);

  void set_up();
  void update();

private:
  const byte pin;
  const bool pressed;
  void (*orden)();

  bool lastRaw;
  unsigned long lastTrigger;
};
