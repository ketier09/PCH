/**
 * ----------------------------------------------------------------------------
 * Actuador_digital (Relé)
 * ----------------------------------------------------------------------------
 * Este módulo encapsula el control de un actuador digital (por ejemplo un relé
 * o una salida ON/OFF) a partir de un pin del microcontrolador.
 *
 * Funcionalidades principales:
 *   - Inicialización segura del pin en modo salida (set_up)
 *   - Encendido y apagado explícitos del actuador
 *   - Cambio de estado (toggle) con una sola llamada
 *
 * Issue relacionada:
 *   - #27 "El actuador digital es un relé": se sugiere renombrar el apartado
 *     en la documentación/web para que quede claro que este módulo controla
 *     específicamente el relé.
 * ----------------------------------------------------------------------------
 */

#pragma once
#include <Arduino.h>

class actuador_digital {
public:
  // Constructor: recibe el número de pin que controlará el actuador
  actuador_digital(byte p);

  // Configura el pin como salida y realiza un pequeño test de funcionamiento
  void set_up();

  // Fuerza el estado del actuador a ENCENDIDO (HIGH)
  void encender();

  // Fuerza el estado del actuador a APAGADO (LOW)
  void apagar();

  // Invierte el estado actual del actuador (de ON a OFF o viceversa)
  void cambiar();
  
  // Coeficiente asociado al actuador (actualmente sin uso, se deja para futuro)
  static constexpr float kappa = 0.0; // m³/s

private:
  const byte pin;   // Pin físico donde está conectado el relé/actuador
  bool estado;      // Estado lógico actual del actuador (true = HIGH, false = LOW)
};
