// ============================================================================
// 📌 Archivo: Caudalimetro.h
// 📘 Propósito general
// Clase encargada de medir caudal mediante un sensor basado en pulsos (YF-S201
// u otros similares). Se encarga de:
//    - Configurar el pin y la interrupción.
//    - Medir frecuencia de pulsos.
//    - Convertir frecuencia → caudal (L/min).
//    - Aplicar filtro exponencial (suavizado EMA).
//    - Entregar el caudal final en m³/s.
//
// 🔧 Issue relacionada (conceptual): mediciones inestables o ruidosas → 
//      se corrige mediante el suavizador EMA.
// ============================================================================

#pragma once
#include <Arduino.h>

class caudalimetro {
public:

  // Constructor: recibe el pin del sensor
  caudalimetro(byte p);

  // Configura interrupción + modo de entrada
  void set_up();

  // Devuelve lectura filtrada en m³/s
  float reading();

  // ISR estática obligatoria por attachInterruptArg()
  static void IRAM_ATTR isrThunk(void *p);

  // ISR real que incrementa el contador
  void IRAM_ATTR instance_isrThunk();

private:

  // Mutex usado para proteger variables dentro de la ISR
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

  // Tiempo entre mediciones (1 segundo = mayor estabilidad)
  static constexpr float periodo_de_las_mediciones = 1000.0f;

  // Pulsos por litro del sensor
  static constexpr float PULSES_PER_LITTER = 450.0f;

  // Conversión 450 pulsos / 60 seg = 7.5 Hz por L/min
  static constexpr float FLOW_CALIBRATION_FACTOR = PULSES_PER_LITTER / 60.0f;

  // Peso del filtro EMA (0.3 = suaviza sin volverse lento)
  static constexpr float suavizador = 0.3f;

  // Conversión final L/min → m³/s
  static constexpr float kappa = 1.0f;

  const byte pin;

  uint32_t lastMillis = 0;
  float    flowRate   = NAN;  // Caudal instantáneo
  float    flowRate_f = NAN;  // Caudal filtrado
  bool initialized    = false;

  // Variable escrita dentro de la ISR (pulsos por periodo)
  volatile uint32_t pulseCount = 0;
};
