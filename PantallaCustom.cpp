#include "PantallaCustom.h"
#include <Fonts/FreeSansBold12pt7b.h>

// 💡 OPTIMIZACIÓN: Inicialización completa de todos los miembros
PantallaCustom::PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst)
: tft(cs, dc, rst) {}


void PantallaCustom::set_up() {

}

void PantallaCustom::actualizar(dato data[]) {
  
}

void PantallaCustom::dibujar_imagen(uint8_t indice) {
  tft.drawRGBBitmap(images[indice].x_index, images[indice].y_index, images[indice].pixels, images[indice].width, images[indice].height);
}



