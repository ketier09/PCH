#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Fonts/FreeSansBold12pt7b.h>

#include "total.h"
#include "captacion.h"
#include "azud.h"
#include "vertederos.h"
#include "turbina.h"
#include "desarenador.h"
#include "logo.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Datos y variables
int flujos[6];
int cotas[6];
String lugares[6] = {
  "TOTAL", "CAPTACION", "AZUD", "VERTEDEROS", "TURBINA", "DESARENADOR"
};

uint16_t colores[6] = {
  ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE,
  ILI9341_YELLOW, ILI9341_CYAN, ILI9341_MAGENTA
};

unsigned long tiempoAnterior = 0;
const unsigned long intervalo = 5000;
int indiceActual = 0;

void setup() {
  tft.begin();
  tft.setRotation(3);
  tft.setFont(&FreeSansBold12pt7b);
  tft.fillScreen(ILI9341_BLACK);

  // Dibuja el logo una vez
  for (int i = 0; i < cantidadImagenes; i++){
    int j = 0;
    if (i > 0) j = 90;
    tft.drawRGBBitmap(0, j, imagenes[i].pixels, imagenes[i].width, imagenes[i].height);
  }
}

void loop() {
  unsigned long tiempoActual = millis();
  
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    
    uint16_t fondoColor;
    uint16_t textoColor;
    // Limpia toda el área de datos (etiqueta + cota + flujo)
    tft.fillRect(70, 0, 240, 240, ILI9341_BLACK);  // Ajusta según tu layout
    tft.fillRect(0, 0, 320, 320, ILI9341_BLACK);
    // Redibuja el logo
    tft.drawRGBBitmap(0, 0, imagenes[iconoLogo].pixels, imagenes[iconoLogo].width, imagenes[iconoLogo].height);
    
    // Muestra etiqueta del sistema
    tft.setCursor(130, 40);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(lugares[indiceActual]);
    tft.drawRGBBitmap(0, 90, imagenes[indiceActual].pixels, imagenes[indiceActual].width, imagenes[indiceActual].height);
    
    tft.fillRect(120, 70, 320, 240, fondoColor);


tft.setCursor(130, 100);
tft.setTextColor(ILI9341_WHITE);
tft.setCursor(130, 100);
tft.print("Cota: ");
tft.print(cotas[indiceActual]);
tft.print(" msnm");

// Fondo para “Flujo”

tft.setCursor(130, 180);  // Primera línea
tft.setTextColor(ILI9341_WHITE);
tft.print("Flujo: ");
tft.print(flujo);
tft.print(" m3/s");
    // Avanza al siguiente índice
    indiceActual++;
    if (indiceActual >= 6) indiceActual = 0;
  }
}
