#pragma once
#include <Arduino.h>

// Definimos un "molde" para usar un sensor ultrasónico.
// Este sensor mide la distancia hasta la superficie del agua
// y, con fórmulas, se puede calcular el flujo del canal.
struct ultrasonico {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;  
  // Un "candado de seguridad" para evitar errores cuando
  // varias partes del programa usan la misma variable al mismo tiempo.

  // Constantes generales
  static constexpr float ESCALA   = 100.0f;            // Factor de ajuste (corrección de medidas)
  static constexpr float manningInverso  = 1.0f/0.013f; // Constante usada en la fórmula de Manning

  // Pines del sensor ultrasónico
  const byte trig;        // Pin que envía el pulso (disparo ultrasónico)
  const byte echo;        // Pin que recibe el eco (rebote del pulso)

  // Funciones que se ejecutan automáticamente cuando el eco empieza y termina
  void (*echoChange)();

  // Parámetros físicos del canal
  const float techo;      // Nivel máximo posible
  const float piso;       // Nivel mínimo posible
  const float ancho;      // Ancho del canal
  const float raizCuadrada_pendiente; // Pendiente del canal (inclinación)

  // Constructor:
  // Cuando se crea el sensor, se indican:
  // - qué pines usa (trig, echo),
  // - qué funciones controlan el eco,
  // - y los parámetros físicos del canal.
  ultrasonico(byte t, byte e, void (*i1)(), float te, float pi, float a, float pe);

  // Variables que se van actualizando con las mediciones
  float nivel;               // Nivel actual del agua (en metros)
  volatile uint32_t disparo; // Momento en que se disparó el pulso
  volatile uint32_t duracion;// Tiempo que tardó en volver el eco

  // Métodos (acciones que puede hacer el sensor)
  void set_up();     // Prepara los pines e interrupciones para usar el sensor
  float reading();   // Mide la distancia y calcula el nivel del agua
  float flujo();     // Calcula el flujo de agua en el canal usando el nivel
};


