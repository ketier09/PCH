// =======================================================================
// 📌 Archivo: Ultrasonico.h
// 📘 Descripción general
//
// Este archivo declara la clase `ultrasonico`, encargada de:
//   - Configurar el sensor ultrasónico (trig/echo)
//   - Capturar el tiempo del pulso mediante interrupciones
//   - Convertir tiempo → distancia → nivel hidráulico
//   - Aplicar filtro EMA
//   - Calcular caudal mediante fórmula de Manning
//
// No tiene issues asociadas directamente.
//
// =======================================================================

#pragma once
#include <Arduino.h>
#include <math.h>

class ultrasonico {
public:

  // Constructor: parámetros físicos del canal y offsets
  ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe);

  void set_up();
  float reading(uint32_t timeout_us = 30000); // lectura de nivel
  float flujo();                               // cálculo de caudal

  // ISR estáticas para redirigir a métodos de instancia
  static void IRAM_ATTR isrThunk(void *p);
  void IRAM_ATTR instance_isrThunk();

private:
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  // --- Constantes físicas y de calibración ---
  static constexpr float MM_POR_US = 5.0f / 29.0f; // mm/us según velocidad sonido
  static constexpr float ESCALA = 0.1f;            // mm → m
  static constexpr float NIVEL_0 = 1268.0f;        // offset de referencia
  static constexpr float manningInverso = 1.0f / 0.013f;
  static constexpr float suavizador = 0.50f;       // EMA
  static constexpr float kappa = 2.3f;             // factor conversión final

  // Pines físicos del sensor
  const byte trig;
  const byte echo;

  // Parámetros del canal hidráulico
  const float techo;
  const float piso;
  const float ancho;
  const float raizCuadrada_pendiente;

  float nivel   = NAN;
  float nivel_f = nivel;
  bool advertencias = true;

  // Variables de medición
  volatile uint32_t disparo = 0;
  volatile uint32_t duracion = 0;

  void disparar();
};
