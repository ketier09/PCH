#include "PantallaCustom.h"

PantallaCustom::PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst,
                               Dato l1, Dato l2, Dato l3,
                               Dato r1, Dato r2, Dato r3)
  : tft(cs, dc, rst) {
  idx[0] = l1; idx[1] = l2; idx[2] = l3;
  idx[3] = r1; idx[4] = r2; idx[5] = r3;
}

void PantallaCustom::set_up() {
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  dibujarTitulos(data); // primer dibujo inmediato usando global 'data'
}

void PantallaCustom::dibujarTitulos(const dato data[]) {
  // Limpia todo el fondo y reescribe títulos y unidades
  tft.fillScreen(ILI9341_BLACK);

  // IZQUIERDA
  tft.setTextColor(ILI9341_RED);
  tft.setCursor(LX, YT[0]); tft.print(data[idx[0]].etiqueta);

  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(LX, YT[1]); tft.print(data[idx[1]].etiqueta);

  tft.setTextColor(ILI9341_BLUE);
  tft.setCursor(LX, YT[2]); tft.print(data[idx[2]].etiqueta);

  // DERECHA
  tft.setTextColor(ILI9341_YELLOW);
  tft.setCursor(RX, YT[0]); tft.print(data[idx[3]].etiqueta);

  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(RX, YT[1]); tft.print(data[idx[4]].etiqueta);

  tft.setTextColor(ILI9341_MAGENTA);
  tft.setCursor(RX, YT[2]); tft.print(data[idx[5]].etiqueta);
}

void PantallaCustom::borrarCajaValor(bool derecha, int fila) {
  int x = derecha ? RX : LX;
  tft.fillRect(x, YV[fila], W, H, ILI9341_BLACK);
}

void PantallaCustom::imprimirValor(int x, int y, const dato& d) {
  // Si la unidad está vacía/espaciada (GeneradoresActivos), imprime entero
  bool sinUnidad = true;
  for (const char* p = d.unidad; *p; ++p) { if (!isspace(*p)) { sinUnidad = false; break; } }

  tft.setCursor(x, y);
  if (sinUnidad) {
    // entero “bonito”
    tft.print((int)d.valor);
  } else {
    // valor con 2 decimales + unidad
    tft.print(d.valor, 2);
    tft.print(" ");
    tft.print(d.unidad);
  }
}

void PantallaCustom::mostrarValores(const dato data[]) {
  // IZQUIERDA
  tft.setTextColor(ILI9341_RED);
  borrarCajaValor(false, 0); imprimirValor(LX, YV[0], data[idx[0]]);

  tft.setTextColor(ILI9341_GREEN);
  borrarCajaValor(false, 1); imprimirValor(LX, YV[1], data[idx[1]]);

  tft.setTextColor(ILI9341_BLUE);
  borrarCajaValor(false, 2); imprimirValor(LX, YV[2], data[idx[2]]);

  // DERECHA
  tft.setTextColor(ILI9341_YELLOW);
  borrarCajaValor(true, 0);  imprimirValor(RX, YV[0], data[idx[3]]);

  tft.setTextColor(ILI9341_CYAN);
  borrarCajaValor(true, 1);  imprimirValor(RX, YV[1], data[idx[4]]);

  tft.setTextColor(ILI9341_MAGENTA);
  borrarCajaValor(true, 2);  imprimirValor(RX, YV[2], data[idx[5]]);
}

void PantallaCustom::actualizar(const dato data[]) {
  unsigned long ahora = millis();
  if (ahora - tiempoAnterior >= intervalo) {
    tiempoAnterior = ahora;
    dibujarTitulos(data);
  }

  mostrarValores(data);
}

