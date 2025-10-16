#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>
#include "Datos.h"
#include "Images.h"

class PantallaCustom {
public:
  PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst,
                 int cotaCaptacion, int caudalTurbinable, int cotaRio,
                 int caudalCaptacion, int caudalRetorno, int generadoresActivos);

  void set_up();
  void actualizar(dato data[]);

private:
  Adafruit_ILI9341 tft;
  void dibujarBase();
  void dibujarDato(int x, int y, const char* etiqueta, float valor, const char* unidad);
};
