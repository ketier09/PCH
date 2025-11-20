#include "Ultrasonico.h"

void IRAM_ATTR ultrasonico::isrThunk(void *p) {
    ultrasonico *self = static_cast<ultrasonico*>(p);
    self->instance_isrThunk();
}

void IRAM_ATTR ultrasonico::instance_isrThunk() {
    const uint32_t tiempo_actual = micros();
  
    if (digitalRead(echo)) {           // ya estás dentro del objeto
        disparo = tiempo_actual;       // RISING
    } else {
        const uint32_t duracion_calculada = tiempo_actual - disparo; 
    
        portENTER_CRITICAL_ISR(&mux);
        duracion = duracion_calculada;
        portEXIT_CRITICAL_ISR(&mux);
    }
}

// 💡 OPTIMIZACIÓN: Aplicación consistente de ESCALA a las constantes
ultrasonico::ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe)
  : trig(t), echo(e), 
    techo(te * ESCALA + NIVEL_0 + (float)c), 
    piso(pi * ESCALA + NIVEL_0 + (float)c), 
    ancho(a * ESCALA), 
    raizCuadrada_pendiente(pe) {}
    
void ultrasonico::set_up() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW);
  attachInterruptArg(digitalPinToInterrupt(echo), ultrasonico::isrThunk, this, CHANGE);
}

void ultrasonico::disparar() {
  if (!digitalRead(echo)) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
  }
}

float ultrasonico::reading(uint32_t timeout_us) {
  uint32_t actual_duracion = 0;

  // Reset y disparo
  portENTER_CRITICAL(&mux);
  duracion = 0;
  portEXIT_CRITICAL(&mux);

  disparar();

  const uint32_t t0 = micros();
  while (true) {
    portENTER_CRITICAL(&mux);
    actual_duracion = duracion; // Leer la nueva duración
    portEXIT_CRITICAL(&mux);
    
    if (actual_duracion != 0) break;
    
    if ((uint32_t)(micros() - t0) > timeout_us) {
      // timeout sigue siendo un error real (no hay dato), se devuelve NaN
      if (advertencias) {
        Serial.println(F("[Ultrasonico] Advertencia: timeout esperando eco."));
      }
      return NAN;
    }
  }

  // Conversión fuera de los bloques críticos
  float distancia_mm = actual_duracion * MM_POR_US; // Uso de MM_POR_US (mm/us)
  float distancia_m  = distancia_mm * ESCALA;       // ESCALA: mm->m (o tu factor)

  // nivel = techo - distancia_m (m)
  nivel = techo - distancia_m;

  if (!isnan(nivel)) {
    const bool fuera_rango = (nivel < piso) || (nivel > techo);

    // ⚠️ Ahora NO anulamos la medición con NaN si está fuera de rango.
    //    En su lugar, avisamos y retornamos igualmente el valor.
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
      // Evitamos alimentar el filtro con un valor fuera de rango:
      // nivel_f se mantiene (si existe) con su último valor válido.
    } else {
      // Dentro de rango: actualizar filtro exponencial normalmente
      if (isnan(nivel_f)) nivel_f = nivel;
      nivel_f = suavizador * nivel + (1.0f - suavizador) * nivel_f;
    }
  }
  
  // Devolver SIEMPRE el valor calculado (dentro o fuera del rango).
  return nivel;
}

float ultrasonico::flujo() {
  float h = nivel_f - piso; 
  if (!(h > 0.0f)) return 0.0f; 

  float areaMojada = ancho * h;        
  float perimetroMojado = ancho + 2.0f * h; 
  float radioHidraulico = areaMojada / perimetroMojado;
  
  // 💡 OPTIMIZACIÓN: Uso de powf para mayor claridad de R^(2/3)
  float R23 = powf(radioHidraulico, 2.0f / 3.0f); 
  
  float velocidad = manningInverso * R23 * raizCuadrada_pendiente;
  return velocidad * areaMojada * kappa;
}

