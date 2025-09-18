#include "Ultrasonico.h"

// Cuando se crea un sensor ultrasónico, le decimos:
// - Qué pin usará para enviar la señal (trig).
// - Qué pin usará para recibir el eco (echo).
// - Qué funciones se llamarán cuando llegue o termine el eco.
// - Y también algunos parámetros físicos: nivel máximo (techo),
//   nivel mínimo (piso), ancho del canal, y la pendiente.
ultrasonico::ultrasonico(byte e, void (*i1)(), void (*i2)(), float te, float pi, float a, float pe)
  : echo(e), echoRising(i1), echoFalling(i2), techo(te), piso(pi), ancho(a), raizCuadrada_pendiente(pe) {}

// Prepara el sensor ultrasónico para empezar a trabajar.
void ultrasonico::set_up() {
  pinMode(trig, OUTPUT);   // El pin "trig" se usa para enviar un pulso.
  pinMode(echo, INPUT);    // El pin "echo" escucha el rebote de ese pulso.
  digitalWrite(trig, LOW); // Nos aseguramos de que empiece apagado.

  // Configuramos interrupciones: 
  // Se activan automáticamente cuando el pin "echo" cambia de estado.
  attachInterrupt(digitalPinToInterrupt(echo), echoRising, RISING);
  attachInterrupt(digitalPinToInterrupt(echo), echoFalling, FALLING);
}
void ultrasonico::disparo(){
  // Enviamos un pequeño pulso ultrasónico (el "disparo").
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
}

// Mide la distancia y calcula el nivel del agua.
float ultrasonico::reading() {
  disparo();
  // Calculamos el tiempo que tardó en regresar el eco
  // y lo convertimos en centímetros.
  portENTER_CRITICAL(&mux);           // Zona protegida para leer la variable
  float distancia_cm = duracion / 58.0f;
  portEXIT_CRITICAL(&mux);

  // Convertimos la distancia en metros y calculamos el nivel del agua.
  float distancia_real_metros = (distancia_cm * ESCALA) / 100.0f;
  nivel = techo - distancia_real_metros;

  // Corregimos valores fuera de rango:
  if (!isnan(nivel)) {
    if (nivel < piso)  nivel = piso;   // No puede bajar de este mínimo
    if (nivel > techo) nivel = techo;  // No puede superar el máximo
  }
  return nivel;
}

// Calcula el flujo de agua en el canal usando la fórmula de Manning.
float ultrasonico::flujo() {
  float h = nivel - piso;     // Profundidad real del agua
  if (h <= 0) return 0.0f;    // Si no hay agua, el flujo es 0

  // Área mojada (sección transversal del agua en el canal)
  float areaMojada = ancho * h;

  // Perímetro mojado (el contacto entre agua y paredes del canal)
  float perimetroMojado = ancho + 2 * h;

  // Radio hidráulico (relación entre área y perímetro mojado)
  float radioHidraulico = areaMojada / perimetroMojado;

  // Velocidad del agua según la fórmula de Manning
  float velocidadFlujo = manningInverso *
                         pow(radioHidraulico, 2.0f / 3.0f) *
                         raizCuadrada_pendiente;

  // Flujo total = velocidad * área
  return velocidadFlujo * areaMojada;
}



