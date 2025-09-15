/*
  Sistema de monitoreo y control para PCH con ESP32
  -------------------------------------------------
  Arquitectura:
  - Sensores:
      * 3 caudalímetros por pulsos (inicio, turbinable, final)
      * 4 ultrasonidos (captación, río, garantía, aducción)
  - Actuadores:
      * Motor de compuerta (puente H, dos pines)
  - Interfaces:
      * Pantallas locales (pa_1, pa_2)
      * Envío a web/Firebase
      * Puerto serial para depuración
  - Temporización:
      * Bucle principal reporta cada 1000 ms
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
  // Caudalímetros (entradas por interrupción)
  PIN_CAUD_INI  = 13, // Caudal al inicio del sistema
  PIN_CAUD_TURB = 14, // Caudal turbinable (alimentará la lógica de generadores)
  PIN_CAUD_END  = 26, // Caudal al final del sistema

  // Ultrasonidos (cada uno: TRIG salida, ECHO entrada sólo-entrada)
  PIN_US_TRIG_C = 27, PIN_US_ECHO_C = 36, // Captación
  PIN_US_TRIG_R = 32, PIN_US_ECHO_R = 35, // Río
  PIN_US_TRIG_G = 33, PIN_US_ECHO_G = 34, // Canal garantía
  PIN_US_TRIG_A = 21, PIN_US_ECHO_A = 39, // Aducción

  // Motor compuerta (puente H: IN1/IN2)
  PIN_COMPUERTA_1 = 16,
  PIN_COMPUERTA_2 = 17,
};

//----------------- Prototipos de ISRs -----------------
// ISR = rutinas de servicio de interrupción. Deben ser rapidísimas.
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
// Cada caudalímetro recibe su pin de pulsos y la ISR que incrementa su contador interno.
caudalimetro ca_inicio    (PIN_CAUD_INI,  ISR_CAUD_INI);
caudalimetro ca_turbinable(PIN_CAUD_TURB, ISR_CAUD_TURB);
caudalimetro ca_final     (PIN_CAUD_END,  ISR_CAUD_END);

// ISRs de caudalímetros: sumar 1 pulso por flanco detectado.
// Importante: pulseCount debe ser `volatile` dentro de la clase para ser seguro en ISR.
void IRAM_ATTR ISR_CAUD_INI()  { ca_inicio.pulseCount++; }
void IRAM_ATTR ISR_CAUD_TURB() { ca_turbinable.pulseCount++; }
void IRAM_ATTR ISR_CAUD_END()  { ca_final.pulseCount++; }

//----------------- Objetos de ultrasonido -----------------
// Constructor: (TRIG, ECHO, ISR_disparo, ISR_regreso, cota_ref, offs, escala, filtro)
// Los parámetros TBD deben calibrarse en sitio (geometría del tanque/canal).
ultrasonico ut_captacion(PIN_US_TRIG_C, PIN_US_ECHO_C, ISR_ULTRA_DIS_CAP, ISR_ULTRA_REGR_CAP, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);
ultrasonico ut_rio      (PIN_US_TRIG_R, PIN_US_ECHO_R, ISR_ULTRA_DIS_RIO, ISR_ULTRA_REGR_RIO, 100.0f/*TBD*/, 0, 0, 0);
ultrasonico ut_garantia (PIN_US_TRIG_G, PIN_US_ECHO_G, ISR_ULTRA_DIS_GAR, ISR_ULTRA_REGR_GAR, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);
ultrasonico ut_aduccion (PIN_US_TRIG_A, PIN_US_ECHO_A, ISR_ULTRA_DIS_ADU, ISR_ULTRA_REGR_ADU, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);

// ISRs de ultrasonido: guardan marca de tiempo en envío y en eco.
// Nota: usar aritmética modular de uint32_t para tolerar overflow de micros().
static inline uint32_t diffMicros(uint32_t t1, uint32_t t0) { return t1 - t0; }

// Captación
void IRAM_ATTR ISR_ULTRA_DIS_CAP(){ ut_captacion.disparo = micros(); }
void IRAM_ATTR ISR_ULTRA_REGR_CAP(){
  const uint32_t regreso = micros();
  ut_captacion.duracion = diffMicros(regreso, ut_captacion.disparo);
}
// Río
void IRAM_ATTR ISR_ULTRA_DIS_RIO(){ ut_rio.disparo = micros(); }
void IRAM_ATTR ISR_ULTRA_REGR_RIO(){
  const uint32_t regreso = micros();
  ut_rio.duracion = diffMicros(regreso, ut_rio.disparo);
}
// Garantía
void IRAM_ATTR ISR_ULTRA_DIS_GAR(){ ut_garantia.disparo = micros(); }
void IRAM_ATTR ISR_ULTRA_REGR_GAR(){
  const uint32_t regreso = micros();
  ut_garantia.duracion = diffMicros(regreso, ut_garantia.disparo);
}
// Aducción
void IRAM_ATTR ISR_ULTRA_DIS_ADU(){ ut_aduccion.disparo = micros(); }
void IRAM_ATTR ISR_ULTRA_REGR_ADU(){
  const uint32_t regreso = micros();
  ut_aduccion.duracion = diffMicros(regreso, ut_aduccion.disparo);
}

//----------------- Actuador: motor compuerta -----------------
motor mo_compuerta(PIN_COMPUERTA_1, PIN_COMPUERTA_2);

//----------------- Pantallas (qué muestra cada una) ---------
pantalla pa_1(IDX_CAUDAL_INICIO,   IDX_CAUDAL_GARANTIA,   IDX_CAUDAL_CAPTACION);
pantalla pa_2(IDX_CAUDAL_ADUCCION, IDX_CAUDAL_TURBINABLE, IDX_CAUDAL_FINAL);

//----------------- Web / Firebase ---------------------------
// Rellena credenciales/URLs/Tokens en producción.
web pagina(""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/);

// -------------------- Lógica de generadores --------------------
// Traduce el caudal turbinable (L/s) a un estado simbólico (0..3).
int generadoresActivos() {
  const float flow = ca_turbinable.reading();  // Lectura rápida (L/s)
  if (flow <= 3.0f)   return 0;   // < 3 L/s: apagado
  if (flow <= 6.0f)   return 1;   // 3–6 L/s: 1 generador
  if (flow <  12.87f) return 2;   // 6–12.87 L/s: 2 generadores
  if (flow <= 13.0f)  return 3;   // ≈12.87–13 L/s: máxima PCH
  return -1;                      // >13 L/s o lectura inválida
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
// Imprime todas las variables con etiqueta, valor y unidad.
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
  // Serial a 115200 (frecuente en ESP32)
  Serial.begin(115200);

  // Inicialización de cada módulo/dispositivo (delegado a sus clases)
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

    // Tabla de variables a publicar/mostrar.
    // pero sus "valor" DEBEN actualizarse antes de enviar.
    static datos data[] = {
      {"Cota en captación",   "cotaCaptacion",   "msnm", 0},
      {"Cota del río",        "cotaRio",         "msnm", 0},
      {"Cota en garantía",    "cotaGarantia",    "msnm", 0},
      {"Cota en aducción",    "cotaAduccion",    "msnm", 0},

      {"Caudal en captación", "caudalCaptacion", "m³/s", 0},
      {"Caudal en garantía",  "caudalGarantia",  "m³/s", 0},
      {"Caudal en aducción",  "caudalAduccion",  "m³/s", 0},

      {"Caudal inicio",       "caudalInicio",     "L/s", 0},
      {"Caudal turbinable",   "caudalTurbinable", "L/s", 0},
      {"Caudal final",        "caudalFinal",      "L/s", 0},

      {"Generadores activos", "generadoresActivos","",   0},
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

    // Salidas
    serial_enviar(data, sizeof(data)/sizeof(data[0]));
    pagina.enviar(data, sizeof(data)/sizeof(data[0]));
    pa_1.enviar(data);
    pa_2.enviar(data);
  }
}
