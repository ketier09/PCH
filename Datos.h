#pragma once
#include <Arduino.h>

// --------------------- Estructura común ---------------------------
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

//--------------------- enum ---------------------------
enum Dato {
#define X(ID, NOMBRE, FIRE, UNI) ID,
    DATOS_X
#undef X
    DatoCount
};

//--------------------- estructura ---------------------------
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

using datos = dato;
