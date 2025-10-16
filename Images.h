#pragma once
#include <stdint.h>
#include <Arduino.h>   // para PROGMEM

// Dimensiones del logo
constexpr int SANBART_W = 70;
constexpr int SANBART_H = 70;

// Declaración del arreglo en PROGMEM
extern const uint16_t sanBartLogo[] PROGMEM;
