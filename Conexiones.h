#pragma once
#include <Arduino.h>

enum PCH_Pines : uint8_t {
  TX0 = 1,
  D2,
  RX0,
  D4,
  D5,
  D12 = 12,
  D13,
  D14,
  D15,
  RX2,
  TX2,
  D18,
  D19,
  D21 = 21,
  D22,
  D23,
  D25 = 25,
  D26,
  D27,
  D32 = 32,
  D33,
  D34,
  D35,
  VP,
  VN = 39
  
};

enum Devices_Pines: uint8_t {
  // --- Caudalímetros (entradas con interrupción) ---
  CAUD_0              = D32,
  CAUD_1              = D33,
  CAUD_2              = D34,

  // --- Ultrasonidos: TRIG = salida, ECHO = entrada ---
  ULTRA_TRIG_0        = D25,
  ULTRA_ECHO_0        = D35,

  ULTRA_TRIG_1        = D26,
  ULTRA_ECHO_1        = VP,

  // --- Actuadores (salidas) ---
  COMPUERTA           = D13,
  ACTUADOR_DIGITAL_0  = D12,
  ACTUADOR_DIGITAL_1  = D14,

  // --- Pulsadores (puedes leerlos como digitales o usar touch) ---
  PULSADOR_0          = D27,

  // --- Pantalla TFT ILI9341 (SPI) ---
  TFT_MOSI            = D23,
  TFT_SCK             = D18,
  TFT_CS              = D5,
  TFT_RST             = D4,
  TFT_DC              = D2
};
