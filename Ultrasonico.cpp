#include "Ultrasonico.h"

void IRAM_ATTR ultrasonico::isrThunk(void* arg) {
  auto* self = static_cast<ultrasonico*>(arg);
  const uint32_t tiempo_actual = micros();
  
  if (digitalRead(self->echo)) {
    // Señal en ALTO (RISING): Inicia la medición
    self->disparo = tiempo_actual;
  } else {
    // Señal en BAJO (FALLING): Termina la medición
    const uint32_t duracion_calculada = tiempo_actual - self->disparo;
    
    // El bloque crítico SOLO se usa para actualizar la variable compartida (duracion)
    portENTER_CRITICAL_ISR(&self->mux);
    self->duracion = duracion_calculada;
    portEXIT_CRITICAL_ISR(&self->mux);
  }
}

// El constructor se simplifica ligeramente usando la nueva constante ESCALA
ultrasonico::ultrasonico(byte t, byte e, int c, float te, float pi, float a, float pe)
  : trig(t), echo(e), 
    techo(te * ESCALA + NIVEL_0 + (float)c * ESCALA), // Multiplicar c por ESCALA
    piso(pi * ESCALA + NIVEL_0 + (float)c * ESCALA),  // Multiplicar c por ESCALA
    ancho(a * ESCALA), 
    raizCuadrada_pendiente(pe) {}
// NOTA: Se asume que 'c' (corrección) está en cm, de ahí el * ESCALA. Si 'c' está en metros, se quita el * ESCALA.

void ultrasonico::set_up() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW);
  // Se mantiene la interrupción
  attachInterruptArg(echo, &ultrasonico::isrThunk, this, CHANGE);
}

// La función 'disparar' no necesita mucha optimización, es muy corta.
void ultrasonico::disparar() {
  // Se optimiza ligeramente la lógica: solo se verifica `echo` al inicio.
  if (!digitalRead(echo)) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);
  }
}

### 2. `reading()`: Lectura y Timeout No Bloqueante

Se mejora el manejo de la lectura de la duración y el bucle de espera, utilizando el valor de `duracion` de forma atómica para evitar el bloqueo constante en los bloques `portENTER_CRITICAL`.

```cpp
float ultrasonico::reading(uint32_t timeout_us) {
  // Leer la duración actual de forma atómica y resetearla a 0
  uint32_t actual_duracion;
  portENTER_CRITICAL(&mux);
  actual_duracion = duracion;
  duracion = 0; // Resetear la duración ANTES de disparar
  portEXIT_CRITICAL(&mux);

  disparar();

  const uint32_t t0 = micros();
  // Bucle de espera no bloqueante
  while (true) {
    portENTER_CRITICAL(&mux);
    actual_duracion = duracion; // Leer la nueva duración
    portEXIT_CRITICAL(&mux);
    
    if (actual_duracion != 0) break; // Éxito
    
    // Comprobar el Timeout
    if ((uint32_t)(micros() - t0) > timeout_us) {
      Serial.println(F("Timeout en ultrasonico::reading()"));
      return NAN; // Timeout
    }
    // Añadir un pequeño delay o `yield()` si se usa en un bucle principal muy ajustado
    // delay(1); // Se omite el delay si el bucle principal es rápido.
  }

  // Convertir a distancia: Mover el bloque crítico a la parte más corta posible
  // La conversión de microsegundos a centímetros es 1us = 1/58.0 cm
  
  // No es necesario usar el bloque crítico aquí si la duración ya fue leída y el bucle terminó
  float distancia_cm = actual_duracion * CM_POR_US; // Uso de la constante CM_POR_US

  // A metros y factor de escala (m/cm)
  float distancia_m = distancia_cm * ESCALA;

  nivel = techo - distancia_m;

  // --- Suavizado (Se mantiene la lógica, es una buena práctica) ---
  if (!isnan(nivel)) {
    // Recorte (Clamping)
    if (nivel < piso)  nivel = piso;
    if (nivel > techo) nivel = techo;
    
    // Inicialización del filtro
    if (isnan(nivel_f)) nivel_f = nivel;
    
    // Filtro IIR de primer orden
    nivel_f = suavizador * nivel + (1.0f - suavizador) * nivel_f;
  }
  
  return nivel;
}

float ultrasonico::flujo() {
  // m: Se usa nivel_f si está inicializado, sino nivel (se mantiene la lógica)
  float h = (isnan(nivel_f) ? nivel : nivel_f) - piso;
  
  // Validación de altura (se mejora la condición)
  if (h <= 0.0f) return 0.0f; 

  float areaMojada = ancho * h;         // m²
  float perimetroMojado = ancho + 2.0f * h; // m (Uso de 2.0f para asegurar float)
  float radioHidraulico = areaMojada / perimetroMojado; // m

  // Optimización: Uso de powf(x, 2.0f/3.0f) para mayor legibilidad de R^(2/3)
  float R23 = powf(radioHidraulico, 2.0f / 3.0f); // R^(2/3)
  
  // Velocidad de Manning: V = (1/n) * R^(2/3) * S^(1/2)
  float velocidad = manningInverso * R23 * raizCuadrada_pendiente; // m/s
  
  // Flujo Q = V * A * kappa
  return velocidad * areaMojada * kappa; // m³/s
}



