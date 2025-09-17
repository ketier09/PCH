#pragma once
#include <Arduino.h>

// --------------------- Índices de datos ---------------------------
// Aquí creamos una lista con "nombres" que representan diferentes cosas
// (alturas y caudales en distintos puntos del sistema).
// Estos nombres se usan como atajos para acceder a los datos,
// en vez de tener que usar números (0, 1, 2, ...).
enum DataIndex {
    IDX_COTA_CAPTACION,     // Altura en el punto de captación
    IDX_COTA_RIO,           // Altura del río
    IDX_COTA_GARANTIA,      // Altura mínima de seguridad (garantía)
    IDX_COTA_ADUCCION,      // Altura en el canal de aducción

    IDX_CAUDAL_CAPTACION,   // Agua captada
    IDX_CAUDAL_GARANTIA,    // Agua mínima que debe respetarse
    IDX_CAUDAL_ADUCCION,    // Agua que pasa por el canal de aducción

    IDX_CAUDAL_INICIO,      // Agua al inicio del proceso
    IDX_CAUDAL_TURBINABLE,  // Agua que puede usarse en la turbina
    IDX_CAUDAL_FINAL,       // Agua al final del proceso

    IDX_GENERADORES_ACTIVOS // Número de generadores que están encendidos
};

// --------------------- Estructura común ---------------------------
// Aquí definimos una "cajita de información" llamada "datos".
// Cada "dato" tendrá:
// - Una etiqueta (nombre corto para mostrar).
// - Una etiqueta para Firebase (nombre usado si se envía a la nube).
// - La unidad de medida (ejemplo: "m", "L/s").
// - El valor (número real que representa la medición).
struct datos {
  const char* etiqueta;          // Nombre para mostrar (ejemplo: "Caudal captación")
  const char* etiquetaFirebase;  // Nombre usado para guardar en Firebase
  const char* unidad;            // Unidad de medida (ejemplo: "m", "L/s")
  float valor = 0.0;             // El valor numérico (inicialmente 0)
};
