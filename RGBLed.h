#pragma once
#include <Arduino.h>

class RGBLed {
public:
  enum LedType { ANODO_COMUN, CATODO_COMUN };

  struct Color {
    uint8_t r, g, b;
  };

  // Constructor: define pines y tipo de LED
  RGBLed(uint8_t pinR, uint8_t pinG, uint8_t pinB, LedType tipo = ANODO_COMUN);

  // Inicializa pines y PWM
  void set_up();

  // Muestra un color (por índice 0–3)
  void showColor(uint8_t index);

  // Define un color manualmente
  void setColor(uint8_t r, uint8_t g, uint8_t b);

private:
  uint8_t _rPin, _gPin, _bPin;
  LedType _tipo;

  static const uint8_t _colorCount = 5;
  const Color _colors[_colorCount] = {
    {0,   0,   0},     // Negro / apagado
    {245, 106, 0},     // Naranja
    {72,  219, 35},    // Verde brillante
    {252, 0,   0},     // Rojo brillante
    {255, 255, 255}
  };

  uint8_t mapLevel(uint8_t v) const;
};
