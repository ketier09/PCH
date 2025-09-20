#include "Pulsador.h"

pulsador::pulsador(byte p, void (*i)(), bool pr)
  : pin(p), pressed(pr), orden(i), lastRaw(!pr), lastTrigger(0) {}

void pulsador::set_up() {
  pinMode(pin, (pressed == LOW) ? INPUT_PULLUP : INPUT);
}

void pulsador::update() {
  unsigned long now = millis();
  bool raw = digitalRead(pin);

  if (raw != lastRaw) {
    if (raw == !pressed && lastRaw == pressed &&
        (now - lastTrigger) > debounce) {
      orden();
      lastTrigger = now;
    }
    lastRaw = raw;
  }
}
