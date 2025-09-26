#include "Pantalla.h"

PantallaCustom::PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst)
    : tft(cs, dc, rst) {}

void PantallaCustom::set_up() {
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(2);
}

void PantallaCustom::dibujarTitulos() {
    // Lado izquierdo
    tft.setCursor(10, 20);
    tft.setTextColor(ILI9341_RED);
    tft.print("Total");

    tft.setCursor(10, 60);
    tft.setTextColor(ILI9341_GREEN);
    tft.print("Captacion");

    tft.setCursor(10, 100);
    tft.setTextColor(ILI9341_BLUE);
    tft.print("Azud");

    // Lado derecho
    tft.setCursor(160, 20);
    tft.setTextColor(ILI9341_YELLOW);
    tft.print("Vertederos");

    tft.setCursor(160, 60);
    tft.setTextColor(ILI9341_CYAN);
    tft.print("Turbina");

    tft.setCursor(160, 100);
    tft.setTextColor(ILI9341_MAGENTA);
    tft.print("Desarenador");
}

void PantallaCustom::mostrarVariables() {
    // Borra rectángulos
    tft.fillRect(10, 40, 140, 20, ILI9341_BLACK);
    tft.fillRect(10, 80, 140, 20, ILI9341_BLACK);
    tft.fillRect(10, 120, 140, 20, ILI9341_BLACK);

    tft.fillRect(160, 40, 140, 20, ILI9341_BLACK);
    tft.fillRect(160, 80, 140, 20, ILI9341_BLACK);
    tft.fillRect(160, 120, 140, 20, ILI9341_BLACK);

    // Izquierda
    tft.setCursor(10, 40);
    tft.setTextColor(ILI9341_RED);
    tft.printf("Var 1: %d", var1);

    tft.setCursor(10, 80);
    tft.setTextColor(ILI9341_GREEN);
    tft.printf("Var 2: %d", var2);

    tft.setCursor(10, 120);
    tft.setTextColor(ILI9341_BLUE);
    tft.printf("Var 3: %d", var3);

    // Derecha
    tft.setCursor(160, 40);
    tft.setTextColor(ILI9341_YELLOW);
    tft.printf("Var 4: %d", var4);

    tft.setCursor(160, 80);
    tft.setTextColor(ILI9341_CYAN);
    tft.printf("Var 5: %d", var5);

    tft.setCursor(160, 120);
    tft.setTextColor(ILI9341_MAGENTA);
    tft.printf("Var 6: %d", var6);
}

void PantallaCustom::actualizar() {
    unsigned long tiempoActual = millis();

    // Actualizar datos principales cada 10s
    if (tiempoActual - tiempoAnterior >= intervalo) {
        tiempoAnterior = tiempoActual;

        // Números aleatorios de ejemplo
        dato1 = random(0, 100);
        dato2 = random(0, 100);
        dato3 = random(0, 100);
        dato4 = random(0, 100);
        dato5 = random(0, 100);
        dato6 = random(0, 100);

        tft.fillScreen(ILI9341_BLACK);
        dibujarTitulos();
    }

    // Variables secundarias aleatorias
    var1 = random(0, 100);
    var2 = random(0, 100);
    var3 = random(0, 100);
    var4 = random(0, 100);
    var5 = random(0, 100);
    var6 = random(0, 100);

    mostrarVariables();

    delay(300); // Evita parpadeo excesivo
}

