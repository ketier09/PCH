// =======================================================================
// 📌 Archivo: Ultrasonico.cpp
// 📘 Descripción general
//
// Este módulo implementa el manejo completo de un sensor ultrasónico
// optimizado para ESP32, con:
//   - Lectura por interrupciones (precisión µs)
//   - Cálculo de distancia con filtros EMA
//   - Conversión de distancia → nivel hidráulico
//   - Cálculo de caudal usando fórmula de Manning
//
// 🔍 No existe issue asociada directamente a este archivo.
//
// =======================================================================

#include "Ultrasonico.h"

// -----------------------------------------------------------------------
// ISR puente estática → reenvía al método de instancia
// -----------------------------------------------------------------------
void IRAM_ATTR ultrasonico::isrThunk(void *p) {
  ultrasonico *self = static_cast<ultrasonico*>(p);
  self->instance_isrThunk();
}

// -----------------------------------------------------------------------
// ISR real: mide tiempos de subida y bajada de la señal ECHO
// -----------------------------------------------------------------------
void IRAM_ATTR ultrasonico::instance_isrThunk() {
  const uint32_t tiempo_actual = micros();

  if (digitalRead(echo)) {      
    // Flanco de subida → registro del inicio del pulso
    disparo = tiempo_actual;
  } else {
    // Flanco de bajada → calcula duración del pulso
    const uint32_t duracion_calculada = tiempo_actual - disparo;

    // Guarda valor en sección crítica (interrupción segura)
    portENTER_CRITICAL_ISR(&mux);
    duracion = duracion_calculada;
    portEXIT_CRITICAL_ISR(&mux);
  }
}

// -----------------------------------------------------------------------
// Constructor con conversión de parámetros según escala física
// -----------------------------------------------------------------------
ultrasonico::ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe)
  : trig(t), echo(e),
    // Conversión de unidades → metros y ajustes de referencia
    techo(te * ESCALA + NIVEL_0 + (float)c),
    piso(pi * ESCALA + NIVEL_0 + (float)c),
    ancho(a * ESCALA),
    raizCuadrada_pendiente(pe) {}

// -----------------------------------------------------------------------
// Configuración de pines e interrupción por cambio de estado
// -----------------------------------------------------------------------
void ultrasonico::set_up() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW);

  // Interrupción por cualquier cambio (RISING y FALLING)
  attachInterruptArg(
      digitalPinToInterrupt(echo),
      ultrasonico::isrThunk,
      this,
      CHANGE
  );
}

// -----------------------------------------------------------------------
// Rutina de disparo del pulso ultrasónico
// -----------------------------------------------------------------------
void ultrasonico::disparar() {
  if (!digitalRead(echo)) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
  }
}

// -----------------------------------------------------------------------
// Lectura del sensor con timeout y filtro EMA
// Devuelve: nivel de agua (m)
// -----------------------------------------------------------------------
float ultrasonico::reading(uint32_t timeout_us) {
  uint32_t actual_duracion = 0;

  // Limpia lectura previa
  portENTER_CRITICAL(&mux);
  duracion = 0;
  portEXIT_CRITICAL(&mux);

  disparar();

  // Espera la duración capturada por la ISR
  const uint32_t t0 = micros();
  while (true) {
    portENTER_CRITICAL(&mux);
    actual_duracion = duracion;
    portEXIT_CRITICAL(&mux);

    if (actual_duracion != 0) break;

    if ((uint32_t)(micros() - t0) > timeout_us) {
      if (advertencias) {
        Serial.println(F("[Ultrasonico] Advertencia: timeout esperando eco."));
      }
      return NAN;
    }
  }

  // --- Conversión a distancia (mm → m)
  float distancia_mm = actual_duracion * MM_POR_US;
  float distancia_m  = distancia_mm * ESCALA;

  // Cálculo del nivel respecto al techo físico
  nivel = techo - distancia_m;

  if (!isnan(nivel)) {
    const bool fuera_rango = (nivel < piso) || (nivel > techo);

    // Si está fuera de rango solo avisamos, pero no anulamos
    if (fuera_rango) {
      if (advertencias) {
        Serial.print(F("[Ultrasonico] Advertencia: medición fuera de rango. "));
        Serial.print(F("Valor="));
        Serial.print(nivel, 3);
        Serial.print(F(" (piso="));
        Serial.print(piso, 3);
        Serial.print(F(", techo="));
        Serial.print(techo, 3);
        Serial.println(F(")"));
      }
    } else {
      // Actualiza filtro EMA solo si el valor es válido
      if (isnan(nivel_f)) nivel_f = nivel;
      nivel_f = suavizador * nivel + (1.0f - suavizador) * nivel_f;
    }
  }

  return nivel; // devuelve SIEMPRE la medición directa
}

// -----------------------------------------------------------------------
// Cálculo de caudal por fórmula de Manning
// Devuelve: Q (m³/s)
// -----------------------------------------------------------------------
float ultrasonico::flujo() {
  float h = nivel_f - piso;
  if (!(h > 0.0f)) return 0.0f;

  float areaMojada = ancho * h;
  float perimetroMojado = ancho + 2.0f * h;
  float radioHidraulico = areaMojada / perimetroMojado;

  float R23 = powf(radioHidraulico, 2.0f / 3.0f);

  float velocidad = manningInverso * R23 * raizCuadrada_pendiente;
  return velocidad * areaMojada * kappa;
}
