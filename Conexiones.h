#pragma once
#include <Arduino.h>
#include <stdint.h>

// GPIO34–39 son SOLO ENTRADA en ESP32.
// TX0=GPIO1, RX0=GPIO3, RX2=GPIO16, TX2=GPIO17.

enum PCH_Pines : uint8_t {
  // UART0
  PCH_TX0 = 1,     // GPIO1
  PCH_RX0 = 3,     // GPIO3

  // Pines "D" típicos (alias a GPIO)
  PCH_D2  = 2,
  PCH_D4  = 4,
  PCH_D5  = 5,
  PCH_D12 = 12,
  PCH_D13 = 13,
  PCH_D14 = 14,
  PCH_D15 = 15,

  // UART2
  PCH_RX2 = 16,    // GPIO16
  PCH_TX2 = 17,    // GPIO17

  // Más GPIO
  PCH_D18 = 18,
  PCH_D19 = 19,
  PCH_D21 = 21,
  PCH_D22 = 22,
  PCH_D23 = 23,
  PCH_D25 = 25,
  PCH_D26 = 26,
  PCH_D27 = 27,
  PCH_D32 = 32,
  PCH_D33 = 33,
  PCH_D34 = 34,    // entrada-solo
  PCH_D35 = 35,    // entrada-solo
  PCH_VP  = 36,    // entrada-solo (a.k.a. VP)
  PCH_VN  = 39     // entrada-solo (a.k.a. VN)
};

enum PCH_Devices_Pines : uint8_t {
  // --- Caudalímetros (entradas con interrupción) ---
  CAUD_0       = PCH_D33,

  // --- Ultrasonidos: TRIG = salida, ECHO = entrada ---
  ULTRA_TRIG_0 = PCH_D25,
  ULTRA_ECHO_0 = PCH_D35,  // entrada-solo
  ULTRA_TRIG_1 = PCH_D26,
  ULTRA_ECHO_1 = PCH_D15,

  // --- Actuadores (salidas) ---
  COMPUERTA            = PCH_D13,
  ACTUADOR_DIGITAL_0   = PCH_D12,

  // --- Pulsadores ---
  PULSADOR_0     = PCH_D27,

  // --- Led ---
  LED_R = PCH_D19,
  LED_G = PCH_D21,
  LED_B = PCH_D22,

  // --- Pantalla TFT ILI9341 (SPI) ---
  TFT_MOSI       = PCH_D23,
  TFT_SCK        = PCH_D18,
  TFT_CS         = PCH_D5,
  TFT_RST        = PCH_D4,
  TFT_DC         = PCH_D2   // <- renombrado (antes: TFT_PCH_DC)
};
