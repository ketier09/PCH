#pragma once
#include <Arduino.h>

// Aquí creamos un "molde" llamado caudalimetro.
// Sirve para guardar todo lo necesario para que el sensor de agua funcione.
struct caudalimetro {
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; 
  // Esto es como un "candado de seguridad":
  // asegura que no haya errores cuando varias partes del programa usan la misma variable.

  // Cada cuánto tiempo queremos calcular el agua que pasó (en milisegundos).
  // Aquí son 2000 ms = 2 segundos.
  static constexpr float periodo_de_las_mediciones = 2000;  

  // El sensor manda unos "clics" (pulsos) cuando pasa agua.
  // Por cada litro, manda aproximadamente 450 clics.
  // Con este número podemos convertir "clics" en "litros por minuto".
  static constexpr float FLOW_CALIBRATION_FACTOR = 450.0f /*clics por litro*/ * periodo_de_las_mediciones / 1000;

  // Factor de ajuste: si después de probar vemos que el sensor mide un poco mal,
  // aquí podemos corregir el resultado multiplicándolo por este número.
  static constexpr float ESCALA = 1.0f;  

  const byte pin;        // Cable del Arduino donde está conectado el sensor.
  void (*isr)();         // Aquí guardamos la función que cuenta los clics del sensor.

  // Esto sirve para crear el caudalímetro y decirle:
  // "usa este pin y esta función para contar clics".
  caudalimetro(byte p, void (*i)());

  uint32_t lastMillis = 0;     // Guarda el último momento en que medimos el agua.
  float flowRate = 0.0f;       // Aquí se guarda el resultado final (cuánta agua pasa).

  volatile uint32_t pulseCount = 0;  
  // Aquí se van contando los clics que envía el sensor.
  // "volatile" significa que puede cambiar en cualquier momento
  // porque otra parte del programa (la interrupción) también lo usa.

  void set_up();      // Prepara el sensor para empezar a contar.
  float reading();    // Calcula cuánta agua pasó en el tiempo definido.
};
