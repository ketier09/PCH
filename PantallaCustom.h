#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>

class DisplayUI {
public:
  // Pines por defecto (puedes cambiarlos al crear el objeto)
  DisplayUI(uint8_t cs = 5, uint8_t dc = 2, uint8_t rst = 4);

  void begin();                  // Inicializa pantalla, rotación, fuente, limpia, dibuja logo
  void setLabels(const char* const* labels, uint8_t count);
  void tick();                   // Llama frecuentemente desde loop(); actualiza cada intervalo

  // Opcional: cambia el intervalo de refresco (ms)
  void setInterval(uint32_t ms);

private:
  void drawStatic();             // Logo, etc.
  void drawFrame(const char* etiqueta, int cota, int flujo);

  // Lógica de colores segun cota
  void pickColors(int cota, uint16_t& fondo, uint16_t& texto);

private:
  Adafruit_ILI9341 tft;
  const char* const* etiquetas = null
