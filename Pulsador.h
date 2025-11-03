#pragma once
#include <Arduino.h>

class pulsador {
public:
  // (Pin, Función Callback, LOW/HIGH activo)
  pulsador(byte p, void (*i)(), bool pr);

  void set_up();
  void update();

private:
  // Se mantiene el valor de 50ms (estándar de la industria)
  static constexpr unsigned long DEBOUNCE_MS = 50; 

  // Constantes de modo de entrada para set_up()
  static constexpr int INPUT_ACTIVE_HIGH = INPUT;
  static constexpr int INPUT_ACTIVE_LOW = INPUT_PULLUP;

  const byte pin;
  // 'true' si el botón es HIGH-active (INPUT), 'false' si es LOW-active (INPUT_PULLUP)
  const bool isHighActive; 
  void (*orden)(); // Puntero a la función a ejecutar

  // Estado del pin leído en el ciclo anterior
  bool lastRaw; 
  // Momento en que se ejecutó la última acción para evitar rebotes
  unsigned long lastTrigger; 
};
