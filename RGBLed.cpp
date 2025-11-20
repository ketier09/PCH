#include "RGBLed.h"

RGBLed::RGBLed(uint8_t pinR, uint8_t pinG, uint8_t pinB, LedType tipo)
: _rPin(pinR), _gPin(pinG), _bPin(pinB), _tipo(tipo) {}

void RGBLed::set_up() {
  pinMode(_rPin, OUTPUT);
  pinMode(_gPin, OUTPUT);
  pinMode(_bPin, OUTPUT);

  // Configura PWM (para ESP32 core 3.x)
  analogWriteFrequency(_rPin, 5000);
  analogWriteFrequency(_gPin, 5000);
  analogWriteFrequency(_bPin, 5000);

  analogWriteResolution(_rPin, 8);
  analogWriteResolution(_gPin, 8);
  analogWriteResolution(_bPin, 8);

  // Apaga el LED al iniciar
  for(int i; i < 4; i++) {
    showColor(i);
    delay(10);
  }
  setColor(0, 0, 0);
}

void RGBLed::showColor(uint8_t index) {
  if (index >= _colorCount) index = _colorCount - 1;
  Color c = _colors[index];
  setColor(c.r, c.g, c.b);
}

void RGBLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
  analogWrite(_rPin, mapLevel(r));
  analogWrite(_gPin, mapLevel(g));
  analogWrite(_bPin, mapLevel(b));
}

uint8_t RGBLed::mapLevel(uint8_t v) const {
  return (_tipo == ANODO_COMUN) ? (uint8_t)(255 - v) : v;
}