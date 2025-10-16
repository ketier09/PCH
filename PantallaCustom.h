#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <Fonts/FreeSansBold18pt7b.h>

#include "Images.h"
#include "Datos.h"   // trae enum Dato, struct dato y el arreglo data[]

struct PantallaCustom {

  static constexpr unsigned long intervalo = 5000;

  Adafruit_ILI9341 tft;
  Dato idx[6];     // 0..2 izquierda, 3..5 derecha
  uint16_t colores[6] = {
    ILI9341_RED, ILI9341_GREEN, ILI9341_BLUE,
    ILI9341_YELLOW, ILI9341_CYAN, ILI9341_MAGENTA
  };

  PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst,
                 Dato l1, Dato l2, Dato l3,
                 Dato r1, Dato r2, Dato r3);

  uint32_t lastMillis = 0;
  int indiceActual = 0;


  void set_up();
  void actualizar(const dato data[]);
};


