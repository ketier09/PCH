#pragma once
#include <Arduino.h>

// --------------------- Estructura común ---------------------------
#define DATOS_X \
X(caudalRio,                  "Caudal del río",                 "caudalRio",                  "m³/s") \
X(caudalCaptacion,            "Caudal de captación",            "caudalCaptacion",            "m³/s") \
X(caudalNoCaptado,            "Caudal no captado",              "caudalNoCaptado",            "m³/s") \
X(caudalGarantíaAmbiental,    "Caudal de garantía ambiental",   "caudalGarantíaAmbiental",    "m³/s") \
X(caudalAduccion,             "Caudal de aducción",             "caudalAduccion",             "m³/s") \
X(caudalTurbinable,           "Caudal turbinable",              "caudalTurbinable",           "m³/s") \
X(caudalDevuelto,             "Caudal devuelto",                "caudalDevuelto",             "m³/s") \
X(caudalRetorno,              "Caudal de retorno",              "caudalRetorno",              "m³/s") \
X(cotaCaptacion,              "Cota en captación",              "cotaCaptacion",              "msnm") \
X(cotaRio,                    "Cota del río",                   "cotaRio",                    "msnm") \
X(cotaAduccion,               "Cota en aducción",               "cotaAduccion",               "msnm") \
X(cotaGarantíaAmbiental,      "Cota de garantía ambiental",     "cotaGarantíaAmbiental",      "msnm") \
X(cantidadGeneradoresActivos, "Generadores activos",            "cantidadGeneradoresActivos", "    ") \



//--------------------- enum ---------------------------
enum Dato {
#define X(ID, NOMBRE, FIRE, UNI) ID,
    DATOS_X
#undef X
    DatoCount
};

//--------------------- estructura ---------------------------
class dato {
public:
  const char* etiqueta;
  const char* etiquetaFirebase;
  const char* unidad;
  float valor;
};

extern dato data[DatoCount];

using datos = dato;
