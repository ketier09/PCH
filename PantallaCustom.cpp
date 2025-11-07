#include "PantallaCustom.h"
#include <Fonts/FreeSansBold12pt7b.h>

// 💡 OPTIMIZACIÓN: Inicialización completa de todos los miembros
PantallaCustom::PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst)
: tft(cs, dc, rst) {}


void PantallaCustom::set_up() {
  tft.begin();
  tft.setRotation(3);
  tft.setFont(&FreeSansBold12pt7b);
  tft.fillScreen(ILI9341_BLACK);
}

void PantallaCustom::actualizar(dato data[]) {
  float flujos[NUM_ETIQUETAS] = { 
    data[caudalIngreso].valor, 
    data[caudalCaptacion].valor, 
    data[caudalGarantia].valor, 
    data[caudalGeneracion].valor
  };

  float cotas[NUM_ETIQUETAS] = { 
    data[cotaIngreso].valor, 
    data[cotaCaptacion].valor, 
    data[cotaGarantia].valor, 
    data[cotaGeneracion].valor
  };


  unsigned long tiempoActual = millis();
  if (tiempoActual - tiempoAnterior >= intervalo) {
  tiempoAnterior = tiempoActual;
    
    tft.setTextColor(ILI9341_WHITE);
  
    tft.fillRect(90, 0, 240, 240, ILI9341_BLACK);
    tft.fillRect(0, 90, 240, 320, ILI9341_BLACK);
    dibujar_imagen(0);
    
    tft.setCursor(130, 40);
    tft.print(imagenes[indiceActual].string_lugar);
    dibujar_imagen(indiceActual+1);
    
    mostrarDato(170, 100, "Cota:", cotas[indiceActual], "msnm");
    mostrarDato(170, 180, "Flujo:", flujos[indiceActual], "m3/s");
    
    indiceActual = (indiceActual + 1) % NUM_ETIQUETAS;
  }
}

void PantallaCustom::mostrarDato(int x, int y, const char* etiqueta, float valor, const char* unidad) {
  tft.setCursor(x, y);
  tft.print(etiqueta);
  tft.setCursor(x, y + 30);
  tft.print(valor);
  tft.print(" ");
  tft.print(unidad);
}

void PantallaCustom::dibujar_imagen(uint8_t indice) {
  tft.drawRGBBitmap(imagenes[indice].x_index, imagenes[indice].y_index, imagenes[indice].pixels, imagenes[indice].width, imagenes[indice].height);
}




