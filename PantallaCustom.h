#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>
#include "Datos.h"
#include "Images.h"

class PantallaCustom {
public:
  // 💡 OPTIMIZACIÓN: Simplificación del constructor a solo los 4 índices usados
  PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst);

  void set_up();
  void actualizar(dato data[]);

private:
  Adafruit_ILI9341 tft;
  
  void dibujar_imagen(uint8_t indice);
};


