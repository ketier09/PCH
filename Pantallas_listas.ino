#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Fonts/FreeSansBold12pt7b.h>

#include "ingreso.h"
#include "captacion.h"
#include "garantia.h"
#include "generacion.h"
#include "logo.h"

#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);




// Datos y variables
int variables[4];
String etiquetas[4] = {
  "INGRESO", "CAPTACION", "GARANTIA", "GENERACION"
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
  tft.drawRGBBitmap(0, 0, iconoLogo, 70, 70);
  tft.drawRGBBitmap(0, 90, iconoIngreso, 140, 105);
  tft.drawRGBBitmap(0, 90, iconoCaptacion, 140, 105);
  tft.drawRGBBitmap(0, 90, iconoGarantia, 140, 105);
  tft.drawRGBBitmap(0, 90, iconoGeneracion, 130, 13);

}

void loop() {
  unsigned long tiempoActual = millis();

  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;

    // Genera valores aleatorios
    int cota = random(1, 99);   // metros
    int flujo = random(1, 99); // litros por segundo

    uint16_t fondoColor;
    uint16_t textoColor;

    

    
    // Limpia toda el área de datos (etiqueta + cota + flujo)
tft.fillRect(90, 0, 240, 240, ILI9341_BLACK);
tft.fillRect(0,90, 240, 320, ILI9341_BLACK);  // Ajusta según tu layout
// Redibuja el logo
tft.drawRGBBitmap(0, 0, iconoLogo, 70, 70);

// Muestra etiqueta del sistema
tft.setCursor(130, 40);
tft.setTextColor(ILI9341_WHITE);
tft.print(etiquetas[indiceActual]);
// Mostrar ícono según etiqueta
if (etiquetas[indiceActual] == "INGRESO") {
  tft.drawRGBBitmap(0, 90, iconoIngreso , 140, 105); // Ícono para Total
} else if (etiquetas[indiceActual] == "CAPTACION") {
  tft.drawRGBBitmap(0, 90, iconoCaptacion, 140, 105); // Ícono para Captación
} else if (etiquetas[indiceActual] == "GARANTIA") {
  tft.drawRGBBitmap(0, 90, iconoGarantia, 140, 105); // Ícono para Azud
} else if (etiquetas[indiceActual] == "GENERACION") {
  tft.drawRGBBitmap(0, 90, iconoGeneracion, 130, 130);
} 

// Fondo 



tft.setCursor(130, 100);
tft.setTextColor(ILI9341_WHITE);
tft.setCursor(170, 100);
tft.print("Cota: ");
tft.setCursor(170, 130);
tft.print(cota);
tft.print(" msnm");

// Fondo para “Flujo”

tft.setCursor(170, 180);  // Primera línea
tft.setTextColor(ILI9341_WHITE);
tft.print("Flujo: ");
tft.setCursor(170, 210);
tft.print(flujo);
tft.print(" m3/s");
    // Avanza al siguiente índice
    indiceActual++;
    if (indiceActual >= 4) indiceActual = 0;
  }
}