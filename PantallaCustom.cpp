/**
 * ---------------------------------------------------------------------------
 * Archivo: PantallaCustom.cpp
 * ---------------------------------------------------------------------------
 * 📝 EXPLICACIÓN GENERAL
 * Este módulo gestiona toda la presentación visual en la pantalla TFT ILI9341:
 *   - Configuración inicial de la pantalla y tipografías.
 *   - Rotación, colores, mensajes iniciales.
 *   - Actualización cíclica de los valores de caudal y cota.
 *   - Renderizado de imágenes y texto en posiciones específicas.
 *
 * 🔎 ISSUE RELACIONADA: #26 “El texto de la pantalla se corta”
 *   Esta clase está directamente relacionada con el problema reportado,
 *   ya que el corte de texto ocurre al imprimir valores + unidades en
 *   zonas con espacio limitado. Los comentarios explican dónde puede
 *   ajustarse el layout para corregir esa issue.
 * ---------------------------------------------------------------------------
 */

#include "PantallaCustom.h"
#include <Fonts/FreeSansBold12pt7b.h>

// 💡 El constructor simplemente pasa los pines al objeto TFT.
PantallaCustom::PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst)
: tft(cs, dc, rst) {}


void PantallaCustom::set_up() {
  tft.begin();
  tft.setRotation(3);  // Rotación usada en toda la maqueta
  tft.setFont(&FreeSansBold12pt7b); // Fuente grande → importante en Issue #26

  // 🔴🟢🔵 Secuencia de test visual
  tft.fillScreen(ILI9341_RED);   delay(500);
  tft.fillScreen(ILI9341_GREEN); delay(500);
  tft.fillScreen(ILI9341_BLUE);  delay(500);

  // Mensaje cuando no hay WiFi configurado
  tft.fillScreen(ILI9341_BLACK);
  tft.print(F("\n\n\n\nEsperando internet\nConéctate a la red\n'ESP_CONFIG-[ID de LA TARJETA]'\ny entra a http://192.168.4.1\npara cargar credenciales"));
}


void PantallaCustom::actualizar(dato data[]) {

  // 👉 Para Issue #26: estos valores a veces imprimen demasiados caracteres.
  float flujos[NUM_ETIQUETAS] = { 
    reducirDecimales(data[caudalIngreso].valor,     0),
    reducirDecimales(data[caudalCaptacion].valor,   0),
    reducirDecimales(data[caudalGarantia].valor,    0),
    reducirDecimales(data[caudalGeneracion].valor,  0)
  };

  float cotas[NUM_ETIQUETAS] = { 
    reducirDecimales(data[cotaIngreso].valor,     0),
    reducirDecimales(data[cotaCaptacion].valor,   0),
    reducirDecimales(data[cotaGarantia].valor,    0),
    reducirDecimales(data[cotaGeneracion].valor,  0)
  };

  unsigned long tiempoActual = millis();

  // ⏱ Control del intervalo de actualización
  if (tiempoActual - tiempoAnterior >= intervalo) {
    tiempoAnterior = tiempoActual;
    
    tft.setTextColor(ILI9341_WHITE);

    // Limpieza de zonas antes de redibujar (evita artefactos)
    tft.fillRect(90, 0, 240, 240, ILI9341_BLACK);
    tft.fillRect(0, 90, 240, 320, ILI9341_BLACK);

    // Dibuja imagen principal y secundarias
    dibujar_imagen(0);

    tft.setCursor(130, 40);
    tft.print(imagenes[indiceActual+1].string_lugar);  // 👉 Texto largo → ISSUE #26
    dibujar_imagen(indiceActual+1);

    // 👉 En estas impresiones puede cortarse la unidad "msnm"
    mostrarDato(170, 100, "Cota:",  cotas[indiceActual], "msnm");
    mostrarDato(170, 180, "Flujo:", flujos[indiceActual], "m3/s");

    indiceActual = (indiceActual + 1) % NUM_ETIQUETAS;
  }
}


void PantallaCustom::mostrarDato(int x, int y, const char* etiqueta, float valor, const char* unidad) {
  // Se imprime la etiqueta
  tft.setCursor(x, y);
  tft.print(etiqueta);

  // Segundo renglón con valor + unidad
  // 🟥 Este bloque es el señalado en Issue #26 (desbordamiento horizontal)
  tft.setCursor(x, y + 30);
  tft.print(valor);
  tft.print(" ");
  tft.print(unidad);
}


void PantallaCustom::dibujar_imagen(uint8_t indice) {
  // Renderiza la imagen correspondiente desde Images.h
  tft.drawRGBBitmap(
    imagenes[indice].x_index,
    imagenes[indice].y_index,
    imagenes[indice].pixels,
    imagenes[indice].width,
    imagenes[indice].height
  );
}


double PantallaCustom::reducirDecimales(double numero, int decimales) {
    // Función auxiliar para controlar la cantidad de decimales mostrados
    double factor = pow(10, decimales);
    return round(numero * factor) / factor;
}

