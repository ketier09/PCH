// ============================================================================
// Archivo: RGBLed.h
// Declaración de clase para controlar un LED RGB mediante PWM en ESP32.
//
// EXPLICACIÓN GENERAL:
// Esta clase abstrae el control del LED RGB permitiendo definir:
//   - Tipo de LED (ánodo o cátodo común)
//   - Colores predefinidos
//   - Asignación de pines
//
// Issue Relacionada: #24
// La declaración incorpora comentarios para recordar que la secuencia de prueba
// definida en set_up() debe usarse para validar físicamente el correcto
// funcionamiento del LED RGB en las primeras pruebas.
// ============================================================================

#pragma once
#include <Arduino.h>

class RGBLed {
public:
  enum LedType { ANODO_COMUN, CATODO_COMUN };

  struct Color {
    uint8_t r, g, b;  // Intensidades para los tres canales (0–255)
  };

  // Constructor general: define pines y tipo de LED.
  RGBLed(uint8_t pinR, uint8_t pinG, uint8_t pinB, LedType tipo = ANODO_COMUN);

  // Inicializa pines, PWM y realiza prueba de colores (⚠ Issue #24)
  void set_up();

  // Selecciona un color de la lista interna por índice (0–4)
  void showColor(uint8_t index);

  // Asigna manualmente valores RGB
  void setColor(uint8_t r, uint8_t g, uint8_t b);

private:
  uint8_t _rPin, _gPin, _bPin;
  LedType _tipo;

  // Colores predefinidos para pruebas visuales
  static const uint8_t _colorCount = 5;
  const Color _colors[_colorCount] = {
    {0,   0,   0},     // 0 -> Apagado
    {245, 106, 0},     // 1 -> Naranja
    {72,  219, 35},    // 2 -> Verde brillante
    {252, 0,   0},     // 3 -> Rojo brillante
    {255, 255, 255}    // 4 -> Blanco
  };

  // Mapeo de brillo según el tipo de LED
  uint8_t mapLevel(uint8_t v) const;
};
