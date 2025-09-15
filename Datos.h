#pragma once
#include <Arduino.h>

// --------------------- Índices de datos ---------------------------
enum DataIndex {
    IDX_COTA_CAPTACION,
    IDX_COTA_RIO,
    IDX_COTA_GARANTIA,
    IDX_COTA_ADUCCION,

    IDX_CAUDAL_CAPTACION,
    IDX_CAUDAL_GARANTIA,
    IDX_CAUDAL_ADUCCION,

    IDX_CAUDAL_INICIO,
    IDX_CAUDAL_TURBINABLE,
    IDX_CAUDAL_FINAL,

    IDX_GENERADORES_ACTIVOS
};

// --------------------- Estructura común ---------------------------
struct datos {
  const char* etiqueta;
  const char* etiquetaFirebase;
  const char* unidad;
  float valor;
};

