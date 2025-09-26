#pragma once
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>

class PantallaCustom {
public:
    PantallaCustom(uint8_t cs, uint8_t dc, uint8_t rst);

    void set_up();
    void actualizar();

private:
    Adafruit_ILI9341 tft;
    unsigned long tiempoAnterior;
    static constexpr unsigned long intervalo = 10000; // 10s

    // Datos principales
    int dato1, dato2, dato3;
    int dato4, dato5, dato6;

    // Variables secundarias
    int var1, var2, var3;
    int var4, var5, var6;

    void dibujarTitulos();
    void mostrarVariables();
};
