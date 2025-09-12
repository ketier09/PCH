#include "Ultrasonico.h"

ultrasonico::ultrasonico(byte t, byte e, void (*i1)(), void (*i2)(), float te, float pi, float a, float pe)
  : trig(t), echo(e), echoRising(i1), echoFalling(i2), techo(te), piso(pi), ancho(a), pendiente(pe) {}

void ultrasonico::set_up() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, LOW);
  attachInterrupt(digitalPinToInterrupt(echo), echoRising, FALLING);
  attachInterrupt(digitalPinToInterrupt(echo), echoFalling, RISING);
}

float ultrasonico::reading() {
  // trigger
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // echo
  portENTER_CRITICAL(&mux);
  float distancia_cm = duracion / 58.0f;
  portEXIT_CRITICAL(&mux);
  float distancia_real_metros = (distancia_cm * ESCALA) / 100.0f;
  nivel = techo - distancia_real_metros;

  if (!isnan(nivel)) {
    if (nivel < piso)  nivel = piso;
    if (nivel > techo) nivel = techo;
  }
  return nivel;
}

float ultrasonico::flujo() {
  float h = nivel - piso;
  if (h <= 0) return 0.0f;

  float areaMojada = ancho * h;
  float perimetroMojado = ancho + 2 * h;
  float radioHidraulico = areaMojada / perimetroMojado;

  float velocidadFlujo = manningInverso *
                         pow(radioHidraulico, 2.0f / 3.0f) *
                         raizCuadrada_pendiente;

  return velocidadFlujo * areaMojada;
}
