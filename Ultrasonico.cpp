#include "Ultrasonico.h"

void IRAM_ATTR ultrasonico::isrThunk(void* arg) {
  auto* self = static_cast<ultrasonico*>(arg);
  const uint32_t tiempo_actual = micros();
  
  if (digitalRead(self->echo)) {
    self->disparo = tiempo_actual; // RISING
  } else {
    // 💡 OPTIMIZACIÓN: Cálculo fuera del bloque crítico ISR para mayor eficiencia
    const uint32_t duracion_calculada = tiempo_actual - self->disparo; 
    
    portENTER_CRITICAL_ISR(&self->mux);
    self->duracion = duracion_calculada;
    portEXIT_CRITICAL_ISR(&self->mux);
  }
}

// 💡 OPTIMIZACIÓN: Aplicación consistente de ESCALA a las constantes
ultrasonico::ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe)
  : trig(t), echo(e), 
    techo(te * ESCALA + NIVEL_0 + (float)c * ESCALA), 
    piso(pi * ESCALA + NIVEL_0 + (float)c * ESCALA), 
    ancho(a * ESCALA), 
    raizCuadrada_pendiente(pe) {}
    
void ultrasonico::set_up() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW);
  attachInterruptArg(echo, &ultrasonico::isrThunk, this, CHANGE);
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
      return NAN;
    }
  }

  // Conversión fuera de los bloques críticos
  float distancia_cm = actual_duracion * CM_POR_US; // Uso de CM_POR_US
  float distancia_m = distancia_cm * ESCALA;

  nivel = techo - distancia_m;

  if (!isnan(nivel)) {
    if (nivel < piso)  nivel = piso;
    if (nivel > techo) nivel = techo;
    
    if (isnan(nivel_f)) nivel_f = nivel;
    nivel_f = suavizador * nivel + (1.0f - suavizador) * nivel_f;
  }
  
  return nivel;
}

float ultrasonico::flujo() {
  float h = (isnan(nivel_f) ? nivel : nivel_f) - piso; 
  if (!(h > 0.0f)) return 0.0f; 

  float areaMojada = ancho * h;        
  float perimetroMojado = ancho + 2.0f * h; 
  float radioHidraulico = areaMojada / perimetroMojado;
  
  // 💡 OPTIMIZACIÓN: Uso de powf para mayor claridad de R^(2/3)
  float R23 = powf(radioHidraulico, 2.0f / 3.0f); 
  
  float velocidad = manningInverso * R23 * raizCuadrada_pendiente;
  return velocidad * areaMojada * kappa;
}