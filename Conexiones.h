#pragma once
#include <Arduino.h>

enum PCH_Pines : uint8_t {
  // ---- GPIO ----
  GPIO0,   // TOUCH1, ADC11, UART0_TX, cuidado: pin de arranque
  GPIO1,   // UART0_TX (depuración serie)
  GPIO2,   // TOUCH2, ADC12
  GPIO3,   // UART0_RX (depuración serie)
  GPIO4,   // TOUCH0, ADC10
  GPIO5,   // VSPI_SS
  GPIO6,   // FLASH_CK
  GPIO7,   // FLASH_D0
  GPIO8,   // FLASH_D1
  GPIO9,   // UART1_RX, FLASH_D2
  GPIO10,  // UART1_TX, FLASH_D3
  GPIO12 = 12, // TOUCH5, ADC15
  GPIO13,  // TOUCH4, ADC14
  GPIO14,  // TOUCH6, ADC16
  GPIO15,  // TOUCH3, ADC13
  GPIO16,  // UART2_RX
  GPIO17,  // UART2_TX
  GPIO18,  // VSPI_SCK
  GPIO19,  // VSPI_MISO
  GPIO21 = 21, // I2C_SDA
  GPIO22,  // I2C_SCL
  GPIO23,  // VSPI_MOSI
  GPIO25 = 25, // ADC18, DAC1, usado como DAC1
  GPIO26,  // ADC19, DAC2, usado como DAC2
  GPIO27,  // TOUCH7, ADC17
  GPIO32 = 32, // TOUCH9, ADC4
  GPIO33,  // TOUCH8, ADC5
  GPIO34,  // INPUT ONLY, ADC6
  GPIO35,  // INPUT ONLY, ADC7
  GPIO36,  // INPUT ONLY, ADC0, VP
  GPIO39 = 39,  // INPUT ONLY, ADC3, VN

  // ---- INPUT ONLY ----
  PCH_INPUT_ONLY_0 = GPIO34,
  PCH_INPUT_ONLY_1 = GPIO35,
  PCH_INPUT_ONLY_2 = GPIO36,
  PCH_INPUT_ONLY_3 = GPIO39,

  // ---- DAC ----
  PCH_DAC0 = GPIO25,
  PCH_DAC1 = GPIO26,

  // ---- TOUCH ----
  PCH_TOUCH0 = GPIO4,  
  PCH_TOUCH1 = GPIO0,  
  PCH_TOUCH2 = GPIO2,  
  PCH_TOUCH3 = GPIO15, 
  PCH_TOUCH4 = GPIO13, 
  PCH_TOUCH5 = GPIO12, 
  PCH_TOUCH6 = GPIO14, 
  PCH_TOUCH7 = GPIO27, 
  PCH_TOUCH8 = GPIO33, 
  PCH_TOUCH9 = GPIO32, 

  // ---- UART ----
  PCH_UART0_TX = GPIO1,
  PCH_UART0_RX = GPIO3,
  PCH_UART1_TX = GPIO10,
  PCH_UART1_RX = GPIO9,
  PCH_UART2_TX = GPIO17,
  PCH_UART2_RX = GPIO16,

  // ---- I2C ----
  PCH_I2C_SDA = GPIO21,
  PCH_I2C_SCL = GPIO22,

  // ---- SPI (VSPI por defecto) ----
  PCH_VSPI_MOSI = GPIO23,
  PCH_VSPI_MISO = GPIO19,
  PCH_VSPI_SCK  = GPIO18,
  PCH_VSPI_SS   = GPIO5,

  // ---- Pines reservados para Flash (NO usar) ----
  PCH_FLASH_CK  = GPIO6, 
  PCH_FLASH_D0  = GPIO7, 
  PCH_FLASH_D1  = GPIO8, 
  PCH_FLASH_D2  = GPIO9, 
  PCH_FLASH_D3  = GPIO10
};

enum Devices_Pines: uint8_t {
  // --- Caudalímetros (entradas con interrupción) ---
  PCH_CAUD_0        = PCH_TOUCH9,
  PCH_CAUD_1        = PCH_TOUCH8,
  PCH_CAUD_2        = PCH_INPUT_ONLY_0,

  // --- Ultrasonidos: TRIG = salida, ECHO = entrada ---
  PCH_ULTRA_TRIG_0  = PCH_DAC0,
  PCH_ULTRA_ECHO_0  = PCH_INPUT_ONLY_1,

  PCH_ULTRA_TRIG_1  = PCH_DAC1,
  PCH_ULTRA_ECHO_1  = PCH_INPUT_ONLY_2,

  PCH_ULTRA_TRIG_2  = PCH_UART2_RX,
  PCH_ULTRA_ECHO_2  = PCH_INPUT_ONLY_3,

  // --- Actuadores (salidas) ---
  PCH_COMPUERTA     = PCH_TOUCH4,
  PCH_ACTUADOR_DIGITAL_0  = PCH_TOUCH5,
  PCH_ACTUADOR_DIGITAL_1  = PCH_TOUCH6,
  PCH_ACTUADOR_DIGITAL_2  = PCH_UART2_TX,

  // --- Pulsadores (puedes leerlos como digitales o usar touch) ---
  PCH_PULSADOR_0    = PCH_TOUCH0,
  PCH_PULSADOR_1    = PCH_TOUCH2,
  PCH_PULSADOR_2    = PCH_TOUCH3,
  PCH_PULSADOR_3    = PCH_TOUCH7,

  // --- Pantalla TFT ILI9341 (SPI) ---
  PCH_TFT_CS   = PCH_VSPI_SS,
  PCH_TFT_DC   = PCH_I2C_SDA,
  PCH_TFT_RST  = PCH_I2C_SCL
};
