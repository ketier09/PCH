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
  
  // Constantes de diseño para la interfaz
  static constexpr int LINE_SPACING_Y = 40; // Espaciado vertical
  static constexpr int START_Y = 80;        // Posición Y inicial
  
  void dibujarBase();
  void dibujarDato(int x, int y, const char* etiqueta, float valor, const char* unidad);

};

