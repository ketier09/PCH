#include "PantallaCustom.h"
#include <Fonts/FreeSansBold12pt7b.h>

PantallaCustom::PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst,
                               int cotaCaptacion, int caudalTurbinable, int cotaRio,
                               int caudalCaptacion, int caudalRetorno, int generadoresActivos)
: tft(cs, dc, rst) {}

void PantallaCustom::set_up() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(&FreeSansBold12pt7b);
  dibujarBase();
}

void PantallaCustom::dibujarBase() {
  tft.drawRGBBitmap(10, 10, imagenes[sanBartLogo].pixels, imagenes[sanBartLogo].width, imagenes[sanBartLogo].height);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(100, 30);
  tft.print("MONITOREO PCH");
}

void PantallaCustom::actualizar(dato data[]) {
  tft.fillRect(0, 60, 320, 180, ILI9341_BLACK);

  dibujarDato(10, 80, data[cotaCaptacion].etiqueta, data[cotaCaptacion].valor, data[cotaCaptacion].unidad);
  dibujarDato(10, 120, data[caudalTurbinable].etiqueta, data[caudalTurbinable].valor, data[caudalTurbinable].unidad);
  dibujarDato(10, 160, data[cotaRio].etiqueta, data[cotaRio].valor, data[cotaRio].unidad);
  dibujarDato(10, 200, data[cantidadGeneradoresActivos].etiqueta, data[cantidadGeneradoresActivos].valor, "");
}

void PantallaCustom::dibujarDato(int x, int y, const char* etiqueta, float valor, const char* unidad) {
  tft.setCursor(x, y);
  tft.setTextColor(ILI9341_YELLOW);
  tft.print(etiqueta);
  tft.print(": ");
  tft.setTextColor(ILI9341_GREEN);
  tft.print(valor, 2);
  tft.print(" ");
  tft.print(unidad);
}
