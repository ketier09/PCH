#include "DisplayUI.h"
#include "Images.h"

#include <SPI.h>
#include <Fonts/FreeSansBold18pt7b.h>

// Colores ILI9341_*
#ifndef ILI9341_BLACK
  #include <Adafruit_ILI9341.h>
#endif

DisplayUI::DisplayUI(uint8_t cs, uint8_t dc, uint8_t rst)
: tft(cs, dc, rst) {}

void DisplayUI::begin() {
  tft.begin();
  tft.setRotation(3);
  tft.setFont(&FreeSansBold18pt7b);
  tft.fillScreen(ILI9341_BLACK);
  drawStatic();
}

void DisplayUI::setLabels(const char* const* labels, uint8_t count) {
  etiquetas = labels;
  etiquetasCount = count;
}

void DisplayUI::setInterval(uint32_t ms) {
  intervalo = ms;
}

void DisplayUI::tick() {
  uint32_t now = millis();
  if (now - tPrev < intervalo) return;
  tPrev = now;

  // Genera valores aleatorios (puedes reemplazarlos por lecturas reales)
  int cota  = random(1, 99);  // metros
  int flujo = random(1, 99);  // m3/s

  if (etiquetas && etiquetasCount > 0) {
    drawFrame(etiquetas[idx], cota, flujo);
    idx = (idx + 1) % etiquetasCount;
  }
}

void DisplayUI::drawStatic() {
  // Limpia zonas
  tft.fillRect(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, ILI9341_BLACK);
  tft.fillRect(0, 0, PANEL_X, PANEL_H, ILI9341_BLACK);

  // Logo (70x70)
  tft.drawRGBBitmap(LOGO_X, LOGO_Y, sanBartLogo, SANBART_W, SANBART_H);
}

void DisplayUI::pickColors(int cota, uint16_t& fondo, uint16_t& texto) {
  if (cota <= 30)       { fondo = ILI9341_RED;    texto = ILI9341_WHITE; }
  else if (cota <= 70)  { fondo = ILI9341_YELLOW; texto = ILI9341_BLACK; }
  else                  { fondo = ILI9341_GREEN;  texto = ILI9341_WHITE; }
}

void DisplayUI::drawFrame(const char* etiqueta, int cota, int flujo) {
  // Redibuja base estática
  drawStatic();

  // Etiqueta del sistema
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(85, 40);
  tft.print(etiqueta);

  // Tarjeta de estado
  uint16_t fondo, texto;
  pickColors(cota, fondo, texto);
  tft.fillRect(CARD_X, CARD_Y, CARD_W, CARD_H, fondo);

  // Textos
  tft.setTextColor(texto);

  // Cota
  tft.setCursor(130, 110);
  tft.print("Cota:");
  tft.setCursor(130, 145);
  tft.print(cota);
  tft.print(" m");

  // Flujo
  tft.setCursor(130, 190);
  tft.print("Flujo:");
  tft.setCursor(130, 225);
  tft.print(flujo);
  tft.print(" m3/s");
}
