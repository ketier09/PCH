#include "PantallaCustom.h"
#include <Fonts/FreeSansBold12pt7b.h>

// 💡 OPTIMIZACIÓN: Inicialización completa de todos los miembros
PantallaCustom::PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst,
                               int idxCotaCaptacion, int idxCaudalTurbinable, int idxCotaRio,
                               int idxGeneradoresActivos)
: tft(cs, dc, rst),
  cotaCaptacion(idxCotaCaptacion),
  caudalTurbinable(idxCaudalTurbinable),
  cotaRio(idxCotaRio),
  generadoresActivos(idxGeneradoresActivos) {}


void PantallaCustom::set_up() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setFont(&FreeSansBold12pt7b);
  dibujarBase();
}

void PantallaCustom::dibujarBase() {
  tft.drawRGBBitmap(10, 10, imagenes[0].pixels, imagenes[0].width, imagenes[0].height);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(100, 30);
  tft.print("MONITOREO PCH");
}

void PantallaCustom::actualizar(dato data[]) {
  // Limpiar el área de datos
  tft.fillRect(0, 60, 320, 180, ILI9341_BLACK); 

  int y = START_Y; // Inicializar la posición Y

  // LÍNEA 1: Cota Captación
  dibujarDato(10, y,  data[cotaCaptacion].etiqueta, data[cotaCaptacion].valor, data[cotaCaptacion].unidad);
  y += LINE_SPACING_Y;

  // LÍNEA 2: Caudal Turbinable
  dibujarDato(10, y, data[caudalTurbinable].etiqueta, data[caudalTurbinable].valor, data[caudalTurbinable].unidad);
  y += LINE_SPACING_Y;

  // LÍNEA 3: Cota Río
  dibujarDato(10, y, data[cotaRio].etiqueta, data[cotaRio].valor, data[cotaRio].unidad);
  y += LINE_SPACING_Y;

  // LÍNEA 4: Generadores Activos
  dibujarDato(10, y, data[generadoresActivos].etiqueta, data[generadoresActivos].valor, "");
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