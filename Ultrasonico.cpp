#include "Ultrasonico.h"

void IRAM_ATTR ultrasonico::isrThunk(void* arg) {
  auto* self = static_cast<ultrasonico*>(arg);
  if (digitalRead(self->echo)) {
    self->disparo = micros(); // RISING
  } else {
    const uint32_t regreso = micros(); // FALLING
    portENTER_CRITICAL_ISR(&self->mux);
    self->duracion = regreso - self->disparo;
    portEXIT_CRITICAL_ISR(&self->mux);
  }
}

ultrasonico::ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe)
  : trig(t), echo(e), techo(te*ESCALA + NIVEL_0 + (float)c), piso(pi*ESCALA + NIVEL_0 + (float)c), ancho(a*ESCALA), raizCuadrada_pendiente(pe) {}

void ultrasonico::set_up() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW);
  attachInterruptArg(echo, &ultrasonico::isrThunk, this, CHANGE);
}

void ultrasonico::disparar() {
  if (!digitalRead(echo)) {          // Evita disparar si hay eco en curso
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
  }
}

float ultrasonico::reading(uint32_t timeout_us) {
  portENTER_CRITICAL(&mux);
  duracion = 0;
  portEXIT_CRITICAL(&mux);

  disparar();

  const uint32_t t0 = micros();
  while (true) {
    portENTER_CRITICAL(&mux);
    uint32_t d = duracion;
    portEXIT_CRITICAL(&mux);
    if (d != 0) break;
    if ((uint32_t)(micros() - t0) > timeout_us) {
      // Timeout: sin eco → reporta NAN o último nivel conocido
      return NAN;
    }
  }

  // Convertir a centímetros
  portENTER_CRITICAL(&mux);
  float distancia_cm = duracion / 58.0f;
  portEXIT_CRITICAL(&mux);

  // A metros y factor de escala (m/cm)
  float distancia_m = distancia_cm * ESCALA;

  nivel = techo - distancia_m;

  if (!isnan(nivel)) {
    if (nivel < piso)  nivel = piso;
    if (nivel > techo) nivel = techo;
    
    if (isnan(nivel_f)) nivel_f = nivel;
    nivel_f = suavizador * nivel + (1.0f - suavizador) * nivel_f; //suavizado
  }
  
  return nivel;
}

float ultrasonico::flujo() {
  float h = (isnan(nivel_f) ? nivel : nivel_f) - piso;   // m
  if (!(h > 0)) return 0.0f;

  float areaMojada = ancho * h;        // m²
  float perimetroMojado = ancho + 2*h; // m
  float radioHidraulico = areaMojada / perimetroMojado; // m
  float R23 = cbrtf(radioHidraulico * radioHidraulico); //³√(m)²
  float velocidad = manningInverso * R23 * raizCuadrada_pendiente; // m/s
  return velocidad * areaMojada * kappa; // m³/s
}



