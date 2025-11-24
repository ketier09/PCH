/**
 * ----------------------------------------------------------------------------
 * Datos.cpp
 * ----------------------------------------------------------------------------
 * Implementación del arreglo global `data`, que contiene todas las variables
 * definidas en DATOS_X (ver Datos.h).
 *
 * El uso de la macro X permite:
 *   - Mantener sincronizados el enum Dato y el arreglo `data[]`.
 *   - Evitar errores al añadir/eliminar variables.
 * ----------------------------------------------------------------------------
 */

#include "Datos.h"

// ---------------------------------------------------------------------------
// Definición del arreglo global de datos
// ---------------------------------------------------------------------------
// Cada entrada inicializa:
//   { etiqueta, etiquetaFirebase, unidad, valorInicial }
//
// El valor inicial se establece en 0.0; luego el resto del código actualiza
// `data[i].valor` con las mediciones reales o valores calculados.
// ---------------------------------------------------------------------------
dato data[DatoCount] = {
#define X(ID, NOMBRE, FIRE, UNI) { NOMBRE, FIRE, UNI, 0.0 },
    DATOS_X
#undef X
};
