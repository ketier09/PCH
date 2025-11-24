/**
 * ----------------------------------------------------------------------------
 * Datos.h
 * ----------------------------------------------------------------------------
 * Punto central donde se definen todas las variables medidas/calculadas
 * del sistema:
 *   - Nombre legible para mostrar en pantalla
 *   - Nombre de campo en Firestore
 *   - Unidades físicas
 *   - Valor numérico actual
 *
 * Este archivo se usa en:
 *   - Envío de datos a Firestore (Web.cpp)
 *   - Actualización de la pantalla (PantallaCustom.cpp)
 *   - Lógica general de la maqueta
 * ----------------------------------------------------------------------------
 */

#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// Lista de todas las variables del sistema
// ---------------------------------------------------------------------------
// La macro DATOS_X se usa para declarar:
//   - El enum Dato (índices)
//   - El arreglo global `data[]` en Datos.cpp
//
// Cada entrada tiene la forma:
//   X(ID, "Nombre legible", "nombreEnFirestore", "unidad")
//
// Si se quiere agregar una nueva variable:
//   1. Añadir una línea aquí.
//   2. Recompilar. El enum, el arreglo y cualquier `switch`/uso por índice
//      quedarán actualizados automáticamente.
// ---------------------------------------------------------------------------
#define DATOS_X \
X(caudalGeneracion,   "Caudal Generación",   "caudalGeneracion",           "m³/s") \
X(caudalIngreso,      "Caudal Ingreso",      "caudalIngreso",              "m³/s") \
X(caudalCaptacion,    "Caudal Captación",    "caudalCaptacion",            "m³/s") \
X(caudalGarantia,     "Caudal Garantía",     "caudalGarantia",             "m³/s") \
X(cotaGeneracion,     "Cota Generación",     "cotaGeneracion",             "msnm") \
X(cotaIngreso,        "Cota Ingreso",        "cotaIngreso",                "msnm") \
X(cotaCaptacion,      "Cota Captación",      "cotaCaptacion",              "msnm") \
X(cotaGarantia,       "Cota Garantía",       "cotaGarantia",               "msnm") \
X(cantidadGeneradoresActivos, "Generadores activos", "cantidadGeneradoresActivos", "    ") \


//--------------------- enum ---------------------------
// Enum con un identificador por cada dato definido en DATOS_X.
// Permite acceder a `data[ID]` de forma segura y legible.
enum Dato {
#define X(ID, NOMBRE, FIRE, UNI) ID,
    DATOS_X
#undef X
    DatoCount   // Número total de variables (tamaño del arreglo data[])
};

//--------------------- estructura ---------------------------
// Estructura que almacena la información asociada a cada variable.
class dato {
public:
  const char* etiqueta;         // Nombre legible para mostrar en pantalla
  const char* etiquetaFirebase; // Nombre del campo en Firestore
  const char* unidad;           // Unidades físicas (m³/s, msnm, etc.)
  float valor;                  // Valor numérico actual
};

// Arreglo global definido en Datos.cpp con una entrada por cada Dato.
extern dato data[DatoCount];

// Alias opcional para quien prefiera usar "datos" en lugar de "dato".
using datos = dato;
