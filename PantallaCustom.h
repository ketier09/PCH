/**
 * ---------------------------------------------------------------------------
 * Archivo: PantallaCustom.h
 * ---------------------------------------------------------------------------
 * 📝 EXPLICACIÓN GENERAL
 * Cabecera de la clase encargada del manejo gráfico en pantalla TFT.
 * Define:
 *   - Constructor con pines CS/DC/RST.
 *   - Métodos de inicialización y actualización visual.
 *   - Control del ciclo de imágenes.
 *   - Impresión del texto y reducción de decimales.
 *
 * 🔎 ISSUE RELACIONADA: #26 “El texto de la pantalla se corta”
 *   Esta cabecera es clave para entender cómo se estructura el contenido
 *   mostrado y por qué algunos textos pueden exceder el espacio disponible.
 * ---------------------------------------------------------------------------
 */

#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <stdint.h>
#include "Datos.h"
#include "Images.h"

class PantallaCustom {
public:

  // Constructor → recibe los pines de la pantalla
  PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst);

  // Configura fuente, rotación y mensaje inicial
  void set_up();

  // Actualiza cíclicamente las imágenes y datos
  void actualizar(dato data[]);

private:
  // Instancia principal del controlador ILI9341
  Adafruit_ILI9341 tft;

  // Número total de etiquetas que se rotan en pantalla
  static constexpr uint8_t NUM_ETIQUETAS = 4;

  // Tiempo entre transiciones
  static constexpr unsigned long intervalo = 5000;

  // Variables internas del sistema de rotación
  unsigned long tiempoAnterior = 0;
  int indiceActual = 0;

  // Etiquetas opcionales (actualmente no usadas)
  const char* etiquetas[NUM_ETIQUETAS];

  // Funciones internas de dibujo
  void dibujar_imagen(uint8_t indice);
  void mostrarDato(int x, int y, const char* etiqueta, float valor, const char* unidad);

  // Ajusta el número de decimales mostrados → agregado para evitar Issue #26
  double reducirDecimales(double numero, int decimales);
};
