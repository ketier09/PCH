// ============================================================================
// Archivo: Motor.h
// Descripción general:
//    Declaración de la clase `motor`, usada para controlar un servo con una
//    secuencia cíclica suave de estados. Incluye un ciclo no lineal de 6 pasos
//    para mejorar la progresión entre aperturas.
//
//    🔧 Relación con issue:
//       Esta interface forma parte de la corrección aplicada en la issue sobre
//       el control de compuerta y la necesidad de estados de transición.
// ============================================================================

#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class motor {
public:
  // Recibe el pin del servo y 4 posiciones base
  motor(byte p, int e1, int e2, int e3, int e4);

  // Inicializa el servo y muestra una secuencia de prueba
  void set_up();

  // Calcula el siguiente estado dentro del ciclo (0–5) y retorna el índice real (0–3)
  uint8_t siguiente_estado();

  // Fuerza manualmente la posición de un estado concreto
  void showState(uint8_t index);

private:
  // Número de estados reales del servo (0,1,2,3)
  static constexpr int n = 4;

  // Longitud total del ciclo simétrico: 0,1,2,3,2,1 = 6 posiciones
  static constexpr int MAX_SEQUENCE_LEN = 2 * n - 2;

  // Pin conectado al servo
  const byte pin;

  // Arreglo con las posiciones reales del servo
  const int estados[n];

  // Objeto servo del ESP32
  Servo servo;

  // Índice dentro del ciclo extendido (0..5)
  int estado_ciclico = 0;
};
