#include "Pantalla.h"

// Cuando se crea una pantalla, se le dicen tres "índices"
// (números que señalan qué datos queremos mostrar).
pantalla::pantalla(byte m, byte k, byte s)
  : mosi(m), sck(k), ss(s) {}

// Esta función sirve para preparar la pantalla antes de usarla.
// Por ahora está vacía, pero aquí iría el código de inicialización
// (por ejemplo, encender la pantalla o configurarla).
void pantalla::set_up() {
}

// Esta función se encarga de enviar información a la pantalla.
// Recibe un arreglo (lista) con muchos datos.
// Solo muestra los 3 que fueron elegidos al crear la pantalla.
void pantalla::enviar(datos dataArray[]) {

}
