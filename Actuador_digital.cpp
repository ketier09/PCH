/**
 * ----------------------------------------------------------------------------
 * Implementación de actuador_digital
 * ----------------------------------------------------------------------------
 * Controla un actuador digital (relé) a través de un pin del microcontrolador.
 *
 * Detalles relevantes:
 *   - Durante set_up() se inicializa el pin como salida y se coloca en LOW.
 *   - Se hace un pequeño "parpadeo" inicial (ON–OFF) para verificar que el
 *     relé responde correctamente al arranque de la maqueta.
 *
 * Issues relacionadas:
 *   - #27 "El actuador digital es un relé": este archivo implementa justamente
 *     el comportamiento de dicho relé, por lo que cualquier cambio de nombre
 *     o documentación debe tener en cuenta esta clase.
 * ----------------------------------------------------------------------------
 */

#include "Actuador_digital.h"

// Constructor: guarda el pin que controlará el actuador
actuador_digital::actuador_digital(byte p)
  : pin(p) {}

// Inicializa el pin como salida y realiza un test rápido del actuador
void actuador_digital::set_up(){
  pinMode(pin, OUTPUT);

  // Estado inicial: APAGADO para evitar activaciones inesperadas
  estado = LOW;
  digitalWrite(pin, estado);

  // Pequeña prueba: enciende 1 s y vuelve a apagar
  actuador_digital::cambiar();
  delay(1000);
  actuador_digital::cambiar();
}

// Enciende el actuador (pone el pin en HIGH)
void actuador_digital::encender(){
  estado = HIGH;
  digitalWrite(pin, estado);
}

// Apaga el actuador (pone el pin en LOW)
void actuador_digital::apagar(){
  estado = LOW;
  digitalWrite(pin, estado);
}

// Invierte el estado actual del actuador
void actuador_digital::cambiar(){
  estado = !estado;
  digitalWrite(pin, estado);
}
