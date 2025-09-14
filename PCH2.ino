#include "Valvula_y_motobomba.h"

enum : uint8_t {
  PIN_VALVE,
  PIN_IMPULSADOR,
};

valvula_motobomba valvula(PIN_VALVE, PIN_IMPULSADOR);

void setup() {
  Serial.begin(115200);
  valvula.set_up();
}

void loop() {
  valvula.leer_orden();
}
