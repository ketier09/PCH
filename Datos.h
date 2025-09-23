#pragma once
#include <Arduino.h>

// --------------------- Índices de datos ---------------------------
// Aquí creamos una lista con "nombres" que representan diferentes cosas
// (alturas y caudales en distintos puntos del sistema).
// Estos nombres se usan como atajos para acceder a los datos,
// en vez de tener que usar números (0, 1, 2, ...).

// --------------------- Estructura común ---------------------------
// Aquí definimos una "cajita de información" llamada "datos".
// Cada "dato" tendrá:
// - Una etiqueta (nombre corto para mostrar).
// - Una etiqueta para Firebase (nombre usado si se envía a la nube).
// - La unidad de medida (ejemplo: "m", "L/s").
// - El valor (número real que representa la medición).
#define DATOS_X \
X(CotaCaptacion,     "Cota en captación",    "cotaCaptacion",     "msnm") \
X(CotaRio,           "Cota del río",         "cotaRio",           "msnm") \
X(CotaDesarenador,   "Cota en desarenador",  "cotaDesarenador",   "msnm") \
X(CaudalCaptacion,   "Caudal en captación",  "caudalCaptacion",   "m³/s") \
X(CaudalDesarenador, "Caudal en desarenador","caudalDesarenador", "m³/s") \
X(CaudalInicio,      "Caudal inicio",        "caudalInicio",      "m³/s") \
X(CaudalTurbinable,  "Caudal turbinable",    "caudalTurbinable",  "m³/s") \
X(CaudalFinal,       "Caudal final",         "caudalFinal",       "m³/s") \
X(GeneradoresActivos,"Generadores activos",  "generadoresActivos","    ")

enum Dato {
#define X(ID, NOMBRE, FIRE, UNI) ID,
    DATOS_X
#undef X
    DatoCount
};

struct dato {
  const char* etiqueta;
  const char* etiquetaFirebase;
  const char* unidad;
  float valor;
};

static dato data[DatoCount] = {
#define X(ID, NOMBRE, FIRE, UNI) { NOMBRE, FIRE, UNI, 0.0 },
    DATOS_X
#undef X
};


