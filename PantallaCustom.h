#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

#include "Datos.h"   // trae enum Dato, struct dato y el arreglo data[]

class PantallaCustom {
public:
  // l1..l3 (izq), r1..r3 (der) son índices del enum Dato
  PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst,
                 Dato l1, Dato l2, Dato l3,
                 Dato r1, Dato r2, Dato r3);

  void set_up();
  // Llama esto en loop pasando el arreglo global 'data'
  void actualizar(const dato data[]);

private:
  Adafruit_ILI9341 tft;
  unsigned long tiempoAnterior = 0;
  static constexpr unsigned long intervalo = 10000; // redibujar títulos cada 10 s

  Dato idx[6];     // 0..2 izquierda, 3..5 derecha

  // layout
  static constexpr int LX = 10;
  static constexpr int RX = 160;
  static constexpr int YT[3] = {20, 60, 100};   // y títulos
  static constexpr int YV[3] = {40, 80, 120};   // y valores
  static constexpr int W  = 140;
  static constexpr int H  = 20;

  void dibujarTitulos(const dato data[]);
  void mostrarValores(const dato data[]);
  void borrarCajaValor(bool derecha, int fila);
  // imprime valor con formato: 2 decimales salvo unidades "    " (entero)
  void imprimirValor(int x, int y, const dato& d);
};
