// ============================================================================
// Archivo: RGBLed.cpp
// Sistema: Control de LED RGB (con soporte para ánodo común y cátodo común)
// Autor: ChatGPT (comentarios agregados)
//
// EXPLICACIÓN GENERAL:
// Este módulo implementa el control de un LED RGB en un ESP32 utilizando PWM.
// Permite encender colores manualmente o mediante índices predeterminados.
// Se añadió desglose de PWM, mapeo para ánodo/cátodo común y secuencia inicial.
//
// ⚠ Issue Relacionada: #24
// La issue #24 reportaba que "el LED RGB no fue probado correctamente",
// por lo que aquí se agregan comentarios aclaratorios sobre la secuencia de test
// y recomendaciones para validarlo correctamente en hardware real.
// ============================================================================

#include "RGBLed.h"

RGBLed::RGBLed(uint8_t pinR, uint8_t pinG, uint8_t pinB, LedType tipo)
: _rPin(pinR), _gPin(pinG), _bPin(pinB), _tipo(tipo) {}


void RGBLed::set_up() {
  // Configuración de pines como salida para cada canal RGB
  pinMode(_rPin, OUTPUT);
  pinMode(_gPin, OUTPUT);
  pinMode(_bPin, OUTPUT);

  // --------------------------------------------------------------------------
  // CONFIGURACIÓN DE PWM
  // ESP32 (core 3.x) usa analogWriteFrequency y analogWriteResolution como APIs
  // compatibles. Se establece 5kHz y resolución de 8 bits.
  // --------------------------------------------------------------------------
  analogWriteFrequency(_rPin, 5000);
  analogWriteFrequency(_gPin, 5000);
  analogWriteFrequency(_bPin, 5000);

  analogWriteResolution(_rPin, 8);
  analogWriteResolution(_gPin, 8);
  analogWriteResolution(_bPin, 8);

  // --------------------------------------------------------------------------
  // SECUENCIA DE PRUEBA ⚠ (Issue #24)
  //
  // Esta secuencia recorre los colores definidos en _colors[]
  // para validar visualmente que el LED responde correctamente.
  // IMPORTANTE:
  //   - En hardware real, el LED debe mostrar: apagado → naranja → verde → rojo → blanco.
  //   - Si alguno de esos colores no aparece, revisar cableado o tipo de LED.
  // --------------------------------------------------------------------------
  for(int i = 0; i < 4; i++) {
    showColor(i);
    delay(500);
  }

  // Finalmente apaga el LED
  setColor(0, 0, 0);
}


void RGBLed::showColor(uint8_t index) {
  // Asegura que el índice no exceda el tamaño del arreglo
  if (index >= _colorCount) index = _colorCount - 1;

  // Obtiene el color correspondiente
  Color c = _colors[index];

  // Aplica el color al LED
  setColor(c.r, c.g, c.b);
}


void RGBLed::setColor(uint8_t r, uint8_t g, uint8_t b) {
  // Se envía PWM a cada canal mapeando el valor según el tipo de LED
  analogWrite(_rPin, mapLevel(r));
  analogWrite(_gPin, mapLevel(g));
  analogWrite(_bPin, mapLevel(b));
}


uint8_t RGBLed::mapLevel(uint8_t v) const {
  // --------------------------------------------------------------------------
  // MAPEADO SEGÚN TIPO DE LED
  // ANODO_COMUN:  el encendido es inverso (0 = encendido, 255 = apagado)
  // CATODO_COMUN: el encendido es normal  (0 = apagado, 255 = encendido)
  // --------------------------------------------------------------------------
  return (_tipo == ANODO_COMUN) ? (uint8_t)(255 - v) : v;
}
