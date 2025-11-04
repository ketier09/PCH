#include "Motor.h"

motor::motor(byte p, int e1, int e2, int e3, int e4)
  : pin(p), estados{e1, e2, e3, e4}{}

void motor::set_up() {
  servo.attach(pin, 500, 2500);
}

void motor::siguiente_estado(){
  // 1. Avanza el contador cíclico (0, 1, 2, 3, 4, 5, 0, ...)
  estado_ciclico = (estado_ciclico + 1) % MAX_SEQUENCE_LEN; // % 6
  
  int posicionIndex;
  
  // 2. Mapea el contador cíclico (0-5) a la posición real (0-3)
  if (estado_ciclico < n) {
    posicionIndex = estado_ciclico; // Indices: 0, 1, 2, 3
  } else {
    // Indices: 4 -> 2, 5 -> 1
    // La fórmula 6 - estado_ciclico es más simple:
    // 6 - 4 = 2 (estado[2])
    // 6 - 5 = 1 (estado[1])
    posicionIndex = MAX_SEQUENCE_LEN - estado_ciclico;
  }
  
  servo.write(estados[posicionIndex]);
}