#pragma once
#include <Arduino.h>

#include "Datos.h"          // Aquí está la definición de la estructura 'datos'

// "pantalla" es como un pequeño panel que muestra 3 datos.
// Al crearla, le diremos cuáles 3 datos (por su posición/índice) queremos mostrar.
struct pantalla {
  const byte mosi;
  const byte sck;
  const byte ss;
  // Constructor:
  // Cuando hacemos una "pantalla", elegimos qué 3 datos mostrará
  // (por ejemplo: 0 = nivel del río, 1 = caudal, 2 = generadores activos).
  pantalla(byte m, byte k, byte s);

  // Prepara la pantalla antes de usarla (encenderla, configurarla, etc.).
  // De momento solo es el "lugar" donde pondremos esa preparación.
  void set_up();

  // Muestra en la pantalla los 3 datos elegidos.
  // 'dataArray' es la lista completa de mediciones;
  // esta función toma solo los 3 indicados en 'indices' y los presenta.
  void enviar(datos dataArray[]);
};
