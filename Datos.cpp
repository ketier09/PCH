#include "Datos.h"
dato data[DatoCount] = {
#define X(ID, NOMBRE, FIRE, UNI) { NOMBRE, FIRE, UNI, 0.0 },
    DATOS_X
#undef X
};
