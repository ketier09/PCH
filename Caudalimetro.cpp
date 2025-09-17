#include "Caudalimetro.h"   // Traemos la definición del caudalímetro desde otro archivo

// Cuando creamos un caudalímetro, tenemos que decirle:
// 1. En qué cable del microcontrolador está conectado el sensor.
// 2. Qué función debe ejecutar cada vez que el sensor detecta que pasó agua.
caudalimetro::caudalimetro(byte p, void (*i)())
  : pin(p), isr(i) {}

// Esta función prepara todo para que el sensor funcione.
void caudalimetro::set_up() {
  pinMode(pin, INPUT_PULLUP);   // Configura el cable como "entrada", listo para escuchar al sensor.
  attachInterrupt(digitalPinToInterrupt(pin), isr, FALLING);  
  // Aquí le decimos al microcontrolador:
  // "Cada vez que el sensor mande una señal porque pasó agua,
  // llama a la función que te indiqué al principio".
  // Esa señal es como un pequeño "clic" que el sensor hace.
}

// Esta función devuelve cuánta agua está pasando por el sensor.
float caudalimetro::reading() {
  // Solo calculamos cada cierto tiempo (2 segundos por defecto).
  if (millis() - lastMillis >= periodo_de_las_mediciones) {
    lastMillis = millis();   // Guardamos el momento en que hicimos esta medición.
    
    uint32_t pulses;         // Aquí vamos a guardar cuántos "clics" hizo el sensor.

    // Zona protegida: copiamos el número de clics sin que nadie lo interrumpa.
    portENTER_CRITICAL(&mux);
    pulses = pulseCount;     // Guardamos el número de clics acumulados.
    pulseCount = 0;          // Reiniciamos el contador para volver a empezar.
    portEXIT_CRITICAL(&mux);

    // Calculamos el caudal: cada "clic" equivale a una cierta cantidad de agua.
    flowRate = pulses / FLOW_CALIBRATION_FACTOR;
  }

  // Devolvemos el resultado, ajustado por el factor de escala.
  return flowRate * ESCALA;
}
