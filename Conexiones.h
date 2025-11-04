#pragma once
#incluPCH_De <Arduino.h>

enum PCH_Pines : uint8_t {
  PCH_TX0 = 1,
  PCH_D2,
  PCH_RX0,
  PCH_D4,
  PCH_D5,
  PCH_D12 = 12,
  PCH_D13,
  PCH_D14,
  PCH_D15,
  PCH_RX2,
  PCH_TX2,
  PCH_D18,
  PCH_D19,
  PCH_D21 = 21,
  PCH_D22,
  PCH_D23,
  PCH_D25 = 25,
  PCH_D26,
  PCH_D27,
  PCH_D32 = 32,
  PCH_D33,
  PCH_D34,
  PCH_D35,
  PCH_VP,
  PCH_VN = 39
  
};

enum PCH_Devices_Pines: uint8_t {
  // --- CauPCH_Dalímetros (entraPCH_Das con interrupción) ---
  CAUPCH_D_0              = PCH_D32,
  CAUPCH_D_1              = PCH_D33,
  CAUPCH_D_2              = PCH_D34,

  // --- UltrasoniPCH_Dos: TRIG = saliPCH_Da, ECHO = entraPCH_Da ---
  ULTRA_TRIG_0        = PCH_D25,
  ULTRA_ECHO_0        = PCH_D35,

  ULTRA_TRIG_1        = PCH_D26,
  ULTRA_ECHO_1        = PCH_VP,

  // --- ActuaPCH_Dores (saliPCH_Das) ---
  COMPUERTA                   = PCH_D13,
  ACTUAPCH_DOR_PCH_DIGITAL_0  = PCH_D12,
  ACTUAPCH_DOR_PCH_DIGITAL_1  = PCH_D14,

  // --- PulsaPCH_Dores (puePCH_Des leerlos como PCH_Digitales o usar touch) ---
  PULSAPCH_DOR_0          = PCH_D27,

  // --- Pantalla TFT ILI9341 (SPI) ---
  TFT_MOSI            = PCH_D23,
  TFT_SCK             = PCH_D18,
  TFT_CS              = PCH_D5,
  TFT_RST             = PCH_D4,
  TFT_PCH_DC          = PCH_D2
};
