#pragma once
#include <Arduino.h>

// --------------------- Estructura común ---------------------------
#define DATOS_X \
X(caudalGeneracion, "Caudal Generación",  "caudalGeneracion",           "m³/s") \
X(caudalIngreso,    "Caudal Ingreso",     "caudalIngreso",              "m³/s") \
X(caudalCaptacion,  "Caudal Captación",   "caudalCaptacion",            "m³/s") \
X(caudalGarantia,   "Caudal Garantía",    "caudalGarantia",             "m³/s") \
X(cotaGeneracion,   "Cota Generación",    "cotaGeneracion",             "msnm") \
X(cotaIngreso,      "Cota Ingreso",       "cotaIngreso",                "msnm") \
X(cotaCaptacion,    "Cota Captación",     "cotaCaptacion",              "msnm") \
X(cotaGarantia,     "Cota Garantía",      "cotaGarantia", "msnm") \
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
