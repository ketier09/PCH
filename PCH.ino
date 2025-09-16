/*
  ¿Qué es esto?
  Un sistema con un ESP32 que mide niveles de agua y caudales en varios puntos
  de una pequeña central hidroeléctrica (PCH), decide cuántos generadores
  deberían estar activos y muestra/envía esos datos cada segundo.

  ¿Qué mide?
  - Niveles de agua (4 sensores de distancia por ultrasonido).
  - Cantidad de agua que pasa (3 medidores de flujo).

  ¿Qué hace con esos datos?
  - Decide el estado de los generadores según el caudal disponible.
  - Muestra la información en 2 pantallas.
  - La envía al computador (cable) y a una página web.
  - Controla una compuerta (motor) cuando se necesite.

  ¿Cada cuánto?
  - Aproximadamente cada 1 segundo repite: leer → calcular → mostrar/enviar.

Glosario rápido:
   - Caudalímetro: medidor de cuánta agua pasa por un punto (flujo).
   - Ultrasonido: sensor que “lanza un eco” y calcula la distancia por el tiempo de regreso.
   - ISR: función que se ejecuta de inmediato cuando llega una señal del sensor.
   - Puente H: circuito que permite que un motor gire hacia un lado o hacia el otro.
   - msnm: metros sobre el nivel del mar (altura del agua respecto a una referencia).
   - Puerto serial: “cable de datos” para ver mensajes en el computador.
*/


#include "Datos.h"
#include "Caudalimetro.h"
#include "Ultrasonico.h"
#include "Pantalla.h"
#include "Motobomba.h"
#include "Motor.h"
#include "Web.h"

// --------------------- Mapeo de pines ---------------------------
// Usamos un enum anónimo para mantener los pines con nombre y tipo fijo (uint8_t).
enum : uint8_t {
  // ¿Dónde está conectado cada sensor/actuador en la tarjeta (ESP32)?
  // - 3 medidores de flujo: inicio (PIN 13), turbinable (14), final (26).
  // - 4 sensores de nivel (ultrasonido):
  //     Captación (TRIG 27, ECHO 36), Río (TRIG 32, ECHO 35),
  //     Garantía (TRIG 33, ECHO 34), Aducción (TRIG 21, ECHO 39).
  // - Motor de compuerta: dos cables de control (16 y 17).

  // Caudalímetros
  PIN_CAUD_INI  = 13,
  PIN_CAUD_TURB = 14,
  PIN_CAUD_END  = 26,

  // Ultrasonidos (cada uno: TRIG salida, ECHO entrada sólo-entrada)
  PIN_US_TRIG_C = 27, PIN_US_ECHO_C = 36,
  PIN_US_TRIG_R = 32, PIN_US_ECHO_R = 35,
  PIN_US_TRIG_G = 33, PIN_US_ECHO_G = 34,
  PIN_US_TRIG_A = 21, PIN_US_ECHO_A = 39,

  // Motor de la compuerta
  PIN_COMPUERTA_1 = 16,
  PIN_COMPUERTA_2 = 17,
};

//----------------- Prototipos de ISRs -----------------
// Funciones rápidas que se activan como reacción a eventos importantes en los sensores (Más abajo se explican sus algoritmos internos).
void IRAM_ATTR ISR_CAUD_INI();
void IRAM_ATTR ISR_CAUD_TURB();
void IRAM_ATTR ISR_CAUD_END();

void IRAM_ATTR ISR_ULTRA_DIS_CAP();
void IRAM_ATTR ISR_ULTRA_REGR_CAP();
void IRAM_ATTR ISR_ULTRA_DIS_RIO();
void IRAM_ATTR ISR_ULTRA_REGR_RIO();
void IRAM_ATTR ISR_ULTRA_DIS_GAR();
void IRAM_ATTR ISR_ULTRA_REGR_GAR();
void IRAM_ATTR ISR_ULTRA_DIS_ADU();
void IRAM_ATTR ISR_ULTRA_REGR_ADU();

//----------------- Objetos de caudalímetros -----------------
// Se crean los objetos que representan los caudalímetros del sistema
caudalimetro ca_inicio    (PIN_CAUD_INI,  ISR_CAUD_INI);
caudalimetro ca_turbinable(PIN_CAUD_TURB, ISR_CAUD_TURB);
caudalimetro ca_final     (PIN_CAUD_END,  ISR_CAUD_END);

// ISRs de caudalímetros: sumar 1 pulso por pulso detectado.
void IRAM_ATTR ISR_CAUD_INI()  { ca_inicio.pulseCount++; }
void IRAM_ATTR ISR_CAUD_TURB() { ca_turbinable.pulseCount++; }
void IRAM_ATTR ISR_CAUD_END()  { ca_final.pulseCount++; }

//----------------- Objetos de ultrasonido -----------------
// Se crean los objetos que representan los ultrasonidos del sistema
ultrasonico ut_captacion(PIN_US_TRIG_C, PIN_US_ECHO_C, ISR_ULTRA_DIS_CAP, ISR_ULTRA_REGR_CAP, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);
ultrasonico ut_rio      (PIN_US_TRIG_R, PIN_US_ECHO_R, ISR_ULTRA_DIS_RIO, ISR_ULTRA_REGR_RIO, 100.0f/*TBD*/, 0, 0, 0);
ultrasonico ut_garantia (PIN_US_TRIG_G, PIN_US_ECHO_G, ISR_ULTRA_DIS_GAR, ISR_ULTRA_REGR_GAR, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);
ultrasonico ut_aduccion (PIN_US_TRIG_A, PIN_US_ECHO_A, ISR_ULTRA_DIS_ADU, ISR_ULTRA_REGR_ADU, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);

// ISRs de ultrasonido: guardan marca de tiempo en envío y en eco.
static inline uint32_t diffMicros(uint32_t t1, uint32_t t0) { return t1 - t0; }

// Captación
// Captación (sensor de nivel por ultrasonido)
// Idea simpilficada: el sensor "hace un grito" (disparo) y espera el eco (regreso).
// Con el tiempo entre el grito y el eco sabemos qué tan lejos está el agua.
void IRAM_ATTR ISR_ULTRA_DIS_CAP(){ ut_captacion.disparo = micros(); }  // Guardamos el instante del "grito"
void IRAM_ATTR ISR_ULTRA_REGR_CAP(){
  const uint32_t regreso = micros();                                    // Guardamos el instante del "eco"
  ut_captacion.duracion = diffMicros(regreso, ut_captacion.disparo);    // Tiempo del viaje
}
// Río
void IRAM_ATTR ISR_ULTRA_DIS_RIO(){ ut_rio.disparo = micros(); }  // Guardamos el instante del "grito"
void IRAM_ATTR ISR_ULTRA_REGR_RIO(){
  const uint32_t regreso = micros();                              // Guardamos el instante del "eco"
  ut_rio.duracion = diffMicros(regreso, ut_rio.disparo);          // Tiempo del viaje
}
// Garantía
void IRAM_ATTR ISR_ULTRA_DIS_GAR(){ ut_garantia.disparo = micros(); }  // Guardamos el instante del "grito"
void IRAM_ATTR ISR_ULTRA_REGR_GAR(){
  const uint32_t regreso = micros();                                  // Guardamos el instante del "eco"
  ut_garantia.duracion = diffMicros(regreso, ut_garantia.disparo);    // Tiempo del viaje
}
// Aducción
void IRAM_ATTR ISR_ULTRA_DIS_ADU(){ ut_aduccion.disparo = micros(); }  // Guardamos el instante del "grito"
void IRAM_ATTR ISR_ULTRA_REGR_ADU(){
  const uint32_t regreso = micros();                                  // Guardamos el instante del "eco"
  ut_aduccion.duracion = diffMicros(regreso, ut_aduccion.disparo);    // Tiempo del viaje
}

//----------------- Motor de la compuerta -----------------
// Se crea el objeto que representa el motor de la compuerta
motor mo_compuerta(PIN_COMPUERTA_1, PIN_COMPUERTA_2);

//----------------- Pantallas ---------
// Se crean los objetos que representan las pantallas del sistema
pantalla pa_1(IDX_CAUDAL_INICIO,   IDX_CAUDAL_GARANTIA,   IDX_CAUDAL_CAPTACION); // La primera pantalla muestra los caudales, al inicio del río, en el canal de garantía, y en captación
pantalla pa_2(IDX_CAUDAL_ADUCCION, IDX_CAUDAL_TURBINABLE, IDX_CAUDAL_FINAL);  //La segunda pantalla muestra los caudales, en aduccion, turbinable, y al final del río

//----------------- Firebase ---------------------------
// Se crea el objeto que representa la página online
web pagina(""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/);

// -------------------- Lógica de generadores --------------------
// Decisión simple para los generadores:
// Según el caudal disponible (litros por segundo), elegimos 0, 1 o 2 generadores.
// La idea es no encender de más si el agua no alcanza, y aprovechar cuando sí.
int generadoresActivos() {
  const float flow = ca_turbinable.reading();  // Lectura rápida (L/s)
  if (flow <= 3.0f)   return 0;   // Muy poca agua: generadores apagados
  if (flow <= 6.0f)   return 1;   // Agua suficiente para 1 generador
  if (flow <  12.87f) return 2;   // Agua suficiente para 2 generadores
  if (flow <= 13.0f)  return 3;   // Muy cerca del máximo de la PCH
  return -1;                      // Lectura fuera de rango (revisar sensores)
}


// Texto amigable del estado simbólico:
const char* generadoresActivosExplicacion(int g) {
  switch (g) {
    case 0:  return "Apagados";
    case 1:  return "1 encendido";
    case 2:  return "2 encendidos";
    case 3:  return "2 a máxima capacidad";
    default: return "Error";
  }
}

//----------------- Salida por puerto serial -----------------
// Envía todas las variables, al computador, con etiqueta, valor y unidad.
void serial_enviar(datos data[], int n) {
  for (int i = 0; i < n-1; i++) {
    Serial.print(data[i].etiqueta);
    Serial.print(": ");
    Serial.print(data[i].valor, 2); // 2 decimales -> ±0.005
    Serial.print(" ");
    Serial.println(data[i].unidad);
  }
  // La última variable (estado de generadores) se imprime como texto legible.
  const int g = (int)data[IDX_GENERADORES_ACTIVOS].valor;
  Serial.print(data[IDX_GENERADORES_ACTIVOS].etiqueta);
  Serial.print(": ");
  Serial.println(generadoresActivosExplicacion(g));
}

// -------------------- Setup (una sola vez) --------------------
void setup() {
  // Se inicializa el pueto serial
  Serial.begin(115200);

  // Inicialización de cada módulo/dispositivo
  pagina.set_up();
  pa_1.set_up();
  pa_2.set_up();

  ca_inicio.set_up();
  ca_turbinable.set_up();
  ca_final.set_up();

  ut_captacion.set_up();
  ut_rio.set_up();
  ut_garantia.set_up();
  ut_aduccion.set_up();

  mo_compuerta.set_up();
}

// -------------------- Loop (cada ~1 s) --------------------
void loop() {
  static uint32_t lastPrint = 0;
  const uint32_t now = millis();
  if (now - lastPrint > 1000) {   // Periodicidad ≈1 s
    lastPrint = now;
    // Cada ~1 segundo:
    // 1) Leemos todos los sensores.
    // 2) Calculamos caudales y el estado recomendado de generadores.
    // 3) Enviamos/mostramos los resultados (pantallas, web y texto por cable).


    // Tabla de variables.
    static datos data[] = {

    //|       Nombre       |     Dirección     | Unidad |
    //|                    |    en Firebase    | Física |
      {"Cota en captación",   "cotaCaptacion",   "msnm"},
      {"Cota del río",        "cotaRio",         "msnm"},
      {"Cota en garantía",    "cotaGarantia",    "msnm"},
      {"Cota en aducción",    "cotaAduccion",    "msnm"},

      {"Caudal en captación", "caudalCaptacion", "m³/s"},
      {"Caudal en garantía",  "caudalGarantia",  "m³/s"},
      {"Caudal en aducción",  "caudalAduccion",  "m³/s"},

      {"Caudal inicio",       "caudalInicio",     "L/s"},
      {"Caudal turbinable",   "caudalTurbinable", "L/s"},
      {"Caudal final",        "caudalFinal",      "L/s"},

      {"Generadores activos", "generadoresActivos",""},
    };

    // *** ACTUALIZACIÓN DE VALORES EN CADA CICLO ***
    data[IDX_COTA_CAPTACION].valor   = ut_captacion.reading();
    data[IDX_COTA_RIO].valor         = ut_rio.reading();
    data[IDX_COTA_GARANTIA].valor    = ut_garantia.reading();
    data[IDX_COTA_ADUCCION].valor    = ut_aduccion.reading();

    data[IDX_CAUDAL_CAPTACION].valor = ut_captacion.flujo();
    data[IDX_CAUDAL_GARANTIA].valor  = ut_garantia.flujo();
    data[IDX_CAUDAL_ADUCCION].valor  = ut_aduccion.flujo();

    data[IDX_CAUDAL_INICIO].valor    = ca_inicio.reading();
    data[IDX_CAUDAL_TURBINABLE].valor= ca_turbinable.reading();
    data[IDX_CAUDAL_FINAL].valor     = ca_final.reading();

    data[IDX_GENERADORES_ACTIVOS].valor = (float)generadoresActivos();

    // Se envía la información al equipo conectado a la tarjeta, a la página en línea, y a las pantallas
    serial_enviar(data, sizeof(data)/sizeof(data[0]));
    pagina.enviar(data, sizeof(data)/sizeof(data[0]));
    pa_1.enviar(data);
    pa_2.enviar(data);
  }
}


