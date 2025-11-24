// ============================================================================
// Archivo: Motor.cpp
// Descripción general:
//    Implementación del controlador de motor/servo para la compuerta. Maneja
//    una secuencia cíclica de 6 pasos (0,1,2,3,2,1) que permite una transición
//    suave entre posiciones, evitando saltos bruscos.
//
//    🔧 Relación con issue:
//       Esta implementación resuelve la issue donde la compuerta avanzaba
//       únicamente en 4 pasos lineales. Aquí se corrige usando un ciclo
//       simétrico de longitud 6 para permitir aperturas intermedias.
//
// Archivos relacionados: Motor.h
// ============================================================================

#include "Motor.h"

// Constructor: almacena el pin y la secuencia de posiciones del servo
motor::motor(byte p, int e1, int e2, int e3, int e4)
  : pin(p), estados{e1, e2, e3, e4}{}

void motor::set_up() {
  // Inicializa el servo con los límites de pulso correctos para ESP32
  servo.attach(pin, 500, 2500);

  // Muestra los estados iniciales como una prueba visual
  for(int i = 0; i < 4; i++){
    delay(500);
    showState(i);
  }
  showState(0);
}

void motor::showState(uint8_t index){
  // Escribe directamente el ángulo asociado al índice
  servo.write(estados[index]);
}

uint8_t motor::siguiente_estado(){
  // Avanza dentro del ciclo 0-5, usando módulo para ciclo infinito
  estado_ciclico = (estado_ciclico + 1) % MAX_SEQUENCE_LEN;

  int posicionIndex;

  // Mapea el índice cíclico (0..5) a una posición válida (0..3)
  if (estado_ciclico < n) {
    // Primera mitad del ciclo (0,1,2,3)
    posicionIndex = estado_ciclico;
  } else {
    // Segunda mitad refleja hacia atrás (2,1)
    posicionIndex = MAX_SEQUENCE_LEN - estado_ciclico;
    // Ejemplos:
    // estado_ciclico = 4 → 6 - 4 = 2
    // estado_ciclico = 5 → 6 - 5 = 1
  }

  // Actualiza el servo a la posición mapeada
  showState(posicionIndex);

  return posicionIndex;
}
