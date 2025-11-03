#pragma once
#include <Arduino.h>
#include <math.h>

class ultrasonico {
public:

  // El constructor no cambia: (byte t, byte e, int c, float te, float pi, float a, float pe)
  ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe);

  void set_up();
  float reading(uint32_t timeout_us = 30000); // ~5 m máx
  float flujo();

  static void IRAM_ATTR isrThunk(void* arg);

private:
  // Se mantiene la implementación de la exclusión mutua para la ISR
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  // --- Constantes de Calibración y Físicas ---
  // Conversión de microsegundos a centímetros (343.2 m/s / 2 * 1e6 us/s = 0.01716 cm/us)
  // El valor original era 58.0, que es la conversión inversa: 1 / 0.01716 ~ 58.27
  // Se define la velocidad para mayor claridad:
  static constexpr float VELOCIDAD_SONIDO_CM_US = 0.01716f; // (cm/us) = (343.2 / 2) / 10000
  static constexpr float CM_POR_US = 1.0f / 58.0f; // Constante utilizada en el código original (cm/us)
  
  static constexpr float ESCALA = 1.0f; // m/cm
  static constexpr float NIVEL_0 = 1268.0f; // msnm
  static constexpr float manning = 0.013f; // Coeficiente de rugosidad de Manning
  static constexpr float manningInverso = 1.0f / manning; 
  static constexpr float suavizador = 0.30f; 
  static constexpr float kappa = 1.0f; // Factor de corrección de flujo

  const byte trig;
  const byte echo;

  // Los miembros de la clase no cambian
  const float techo;
  const float piso;
  const float ancho;
  const float raizCuadrada_pendiente;

  float nivel = NAN;
  float nivel_f = nivel;
  volatile uint32_t disparo = 0;
  volatile uint32_t duracion = 0;

  void disparar();
};
