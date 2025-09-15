#pragma once                  // Evita que este archivo se incluya dos veces en el programa
#include <Arduino.h>          // Incluye las funciones básicas de Arduino

// Definimos una estructura llamada "caudalimetro"
// que contendrá todo lo necesario para medir el flujo de agua
struct caudalimetro {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; 
  // "mux" es un candado de seguridad (protección) para manejar variables compartidas 
  // entre el programa principal y las interrupciones

  // Tiempo entre mediciones (en milisegundos). Aquí: 2000 ms = 2 segundos
  static constexpr float periodo_de_las_mediciones = 2000;  

  // Factor de calibración: el caudalímetro envía 450 pulsos por litro
  // Se ajusta al periodo de medición para calcular correctamente el caudal
  static constexpr float FLOW_CALIBRATION_FACTOR = 450.0f/*pulsos por litro*/ * periodo_de_las_mediciones / 1000;

  // Factor de escala para ajustar el resultado final (sirve para correcciones)
  static constexpr float ESCALA = 1.0f;  

  const byte pin;        // Pin de Arduino al que está conectado el sensor
  void (*isr)();         // Puntero a la función que contará los pulsos (interrupción)

  // Constructor: permite crear un caudalímetro indicando el pin y la función ISR
  caudalimetro(byte p, void (*i)());

  uint32_t lastMillis = 0;     // Guarda el tiempo de la última medición
  float flowRate = 0.0f;       // Aquí se guardará el caudal calculado

  volatile uint32_t pulseCount = 0;  
  // Cuenta de pulsos recibidos desde el caudalímetro (volatile porque cambia en la interrupción)

  void set_up();      // Función para configurar el pin y la interrupción
  float reading();    // Función para calcular el caudal en base a los pulsos
};
