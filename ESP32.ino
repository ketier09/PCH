/*
  ¿Qué es este programa?
  Un sistema con un ESP32 que:
  - Mide niveles de agua y caudales (flujo) en varios puntos de una pequeña central hidroeléctrica (PCH).
  - Decide cuántos generadores deberían estar encendidos.
  - Muestra y envía esos datos cada segundo.

  ¿Qué sensores usa?
  - 4 sensores ultrasónicos (miden distancia → nivel de agua).
  - 3 medidores de caudal (cuentan "clics" → cantidad de agua que pasa).

  ¿Qué hace con las mediciones?
  - Calcula caudales y niveles.
  - Decide cuántos generadores conviene encender.
  - Muestra en 2 pantallas.
  - Envía por cable al computador y a una página web (Firebase).
  - Puede mover una compuerta (motor) si es necesario.

  ¿Cada cuánto?
  - Aproximadamente cada 1 segundo repite: leer → calcular → mostrar/enviar.

  Glosario rápido:
   - Caudalímetro: mide cuánta agua pasa por un punto.
   - Ultrasonido: lanza un “grito” y mide el tiempo del eco para saber la distancia.
   - ISR: función muy rápida que se ejecuta automáticamente cuando llega una señal.
   - Puente H: circuito que hace girar un motor hacia un lado o hacia el otro.
   - msnm: metros sobre el nivel del mar.
   - Puerto serial: “cable de datos” virtual para ver mensajes en el computador.
*/

#include "secrets.h" 
#include "Datos.h"
#include "Caudalimetro.h"
#include "Ultrasonico.h"
#include "Pantalla.h"
#include "Motor.h"
#include "Web.h"
#include "Actuador_digital.h"

// --------------------- Mapeo de pines (dónde se conecta cada cosa) ---------------------------
// Usamos nombres claros para no tener que memorizar números de pines.
enum ESP32_Pins {
    // ---- INPUT ONLY ----
    PIN_INPUT_ONLY_1 = 34,
    PIN_INPUT_ONLY_2 = 35,
    PIN_INPUT_ONLY_3 = 36, // ADC0, VP
    PIN_INPUT_ONLY_4 = 39, // ADC3, VN

    // ---- ADC1 ----
    PIN_ADC0  = 36, // VP, INPUT_ONLY_3
    PIN_ADC3  = 39, // VN, INPUT_ONLY_4
    PIN_ADC4  = 32, // TOUCH9
    PIN_ADC5  = 33, // TOUCH8
    PIN_ADC6  = 34, // INPUT_ONLY_1
    PIN_ADC7  = 35, // INPUT_ONLY_2
    PIN_ADC10 = 4,  // TOUCH0
    PIN_ADC11 = 0,  // TOUCH1
    PIN_ADC12 = 2,  // TOUCH2
    PIN_ADC13 = 15, // TOUCH3
    PIN_ADC14 = 13, // TOUCH4
    PIN_ADC15 = 12, // TOUCH5
    PIN_ADC16 = 14, // TOUCH6
    PIN_ADC17 = 27, // TOUCH7
    PIN_ADC18 = 25, // DAC1
    PIN_ADC19 = 26, // DAC2

    // ---- DAC ----
    PIN_DAC1 = 25,  // ADC18
    PIN_DAC2 = 26,  // ADC19

    // ---- TOUCH ----
    PIN_TOUCH0 = 4,  
    PIN_TOUCH1 = 0,  
    PIN_TOUCH2 = 2,  
    PIN_TOUCH3 = 15, 
    PIN_TOUCH4 = 13, 
    PIN_TOUCH5 = 12, 
    PIN_TOUCH6 = 14, 
    PIN_TOUCH7 = 27, 
    PIN_TOUCH8 = 33, 
    PIN_TOUCH9 = 32, 

    // ---- UART ----
    PIN_UART0_TX = 1,
    PIN_UART0_RX = 3,
    PIN_UART1_TX = 10, // FLASH_D3
    PIN_UART1_RX = 9,  // FLASH_D2
    PIN_UART2_TX = 17,
    PIN_UART2_RX = 16,

    // ---- I2C ----
    PIN_I2C_SDA = 21,
    PIN_I2C_SCL = 22,

    // ---- SPI (VSPI por defecto) ----
    PIN_VSPI_MOSI = 23,
    PIN_VSPI_MISO = 19,
    PIN_VSPI_SCK  = 18,
    PIN_VSPI_SS   = 5,

    // ---- Pines reservados para Flash (NO usar) ----
    PIN_FLASH_CK  = 6, 
    PIN_FLASH_D0  = 7, 
    PIN_FLASH_D1  = 8, 
    PIN_FLASH_D2  = 9, 
    PIN_FLASH_D3  = 10
};

constexpr byte PIN_CAUD[] = {};
constexpr byte PIN_ULTRA_TRIG[] = {};
constexpr byte PIN_ULTRA_ECHO[] = {};

//----------------- Creamos los medidores de caudal -----------------

// Arreglo estático de objetos, cada uno con su pin
caudalimetro ca_inicio(PIN_CAUD[0]);
caudalimetro ca_turbinable(PIN_CAUD[1]);
caudalimetro ca_final(PIN_CAUD[2]);
caudalimetro* caudalimetros[] = { &ca_inicio, &ca_turbinable, &ca_final };

//----------------- Creamos los sensores ultrasónicos (niveles) -----------------
// Entre paréntesis: pines TRIG y ECHO, funciones de inicio/fin del eco, y parámetros físicos del canal.
void IRAM_ATTR ISR_ULTRA_CAP();
void IRAM_ATTR ISR_ULTRA_RIO();
void IRAM_ATTR ISR_ULTRA_DES();

const byte ultrasonico::trig = PIN_US_TRIG;
ultrasonico ut_captacion    (PIN_US_ECHO_C, ISR_ULTRA_CAP, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);
ultrasonico ut_rio          (PIN_US_ECHO_R, ISR_ULTRA_RIO, 100.0f/*TBD*/, 0, 0, 0);
ultrasonico ut_desarenador  (PIN_US_ECHO_D, ISR_ULTRA_DES, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);

// Función auxiliar: diferencia de tiempos en microsegundos
static inline uint32_t diffMicros(uint32_t t1, uint32_t t0) { return t1 - t0; }

// Guardamos el instante del “grito” (disparo) y luego el del “eco” (regreso).
// Captación
void IRAM_ATTR ISR_ULTRA_CAP() {
  if (digitalRead(PIN_US_ECHO_C)) {
    ut_captacion.disparo = micros();           // flanco RISING
  } else {
    const uint32_t regreso = micros();         // flanco FALLING
    // Protege la escritura compartida (ISR-safe)
    portENTER_CRITICAL_ISR(&ut_captacion.mux);
    ut_captacion.duracion = regreso - ut_captacion.disparo;
    portEXIT_CRITICAL_ISR(&ut_captacion.mux);
  }
}
// Río
void IRAM_ATTR ISR_ULTRA_RIO() {
  if (digitalRead(PIN_US_ECHO_R)) {
    ut_rio.disparo = micros();
  } else {
    const uint32_t regreso = micros();
    portENTER_CRITICAL_ISR(&ut_rio.mux);
    ut_rio.duracion = regreso - ut_rio.disparo;
    portEXIT_CRITICAL_ISR(&ut_rio.mux);
  }
}
// Desarenador
void IRAM_ATTR ISR_ULTRA_DES() {
  if (digitalRead(PIN_US_ECHO_D)) {
    ut_desarenador.disparo = micros();  // Cambiar ut_garantia por ut_desarenador
  } else {
    const uint32_t regreso = micros();
    portENTER_CRITICAL_ISR(&ut_desarenador.mux);
    ut_desarenador.duracion = regreso - ut_desarenador.disparo;
    portEXIT_CRITICAL_ISR(&ut_desarenador.mux);
  }
}

//----------------- Actuadores -----------------
motor mo_compuerta(PIN_COMPUERTA, 0/*TBD*/, 45/*TBD*/, 90/*TBD*/, 135/*TBD*/);
actuador_digital dig_valvula(PIN_VALVE);
actuador_digital dig_motobombaPrincipal(PIN_NACIMIENTO);
actuador_digital dig_motobombaSecundaria(PIN_IMPULSADOR);

//----------------- Pulsadores -----------------
pulsador puls_compuerta            (PIN_PULS_COMPUERTA,  mo_compuerta.siguiente_estado(),   LOW);
pulsador puls_valvula              (PIN_PULS_VALVE,      dig_valvula.cambiar(),             LOW);
pulsador puls_motobombaPrincipal   (PIN_PULS_NACIMIENTO, dig_motobombaPrincipal.cambiar(),  LOW);
pulsador puls_motobombaSecundaria  (PIN_PULS_IMPULSADOR, dig_motobombaSecundaria.cambiar(), LOW);

//----------------- Pantallas -----------------
// Cada pantalla mostrará 3 datos (elegidos por su índice).
pantalla pa_1(IDX_CAUDAL_INICIO,      IDX_CAUDAL_CAPTACION,  IDX_CAUDAL_CAPTACION);
pantalla pa_2(IDX_CAUDAL_DESARENADOR, IDX_CAUDAL_TURBINABLE, IDX_CAUDAL_FINAL);

//----------------- Conexión web/Firebase -----------------
// “pagina” maneja WiFi, hora de Internet y la base de datos en la nube.
web pagina;

// -------------------- Lógica para decidir generadores --------------------
// Regresa un número: 0, 1, 2 o 3 (cuántos generadores conviene tener activos)
// según el caudal turbinable (simplificado a umbrales).
int generadoresActivos() {
  const float flow = ca_turbinable.reading();  // Lectura rápida (L/s)
  if (flow <= 3.0f)   return 0;   // Muy poca agua
  if (flow <= 6.0f)   return 1;   // Agua para 1 generador
  if (flow <  12.87f) return 2;   // Agua para 2 generadores
  if (flow <= 13.0f)  return 3;   // Muy cerca del máximo
  return -1;                      // Fuera de rango (revisar sensor)
}

// Convierte ese número en un texto fácil de entender
const char* generadoresActivosExplicacion[5] = {"Apagados", "1 encendido","2 encendidos", "2 a máxima capacidad", "Error"};

//----------------- Envío por puerto serial (al computador) -----------------
// Imprime todas las variables con nombre, valor y unidad.
// La última (generadores) se imprime como texto, no número.
void serial_enviar(datos data[]) {
  for (int i = 0; i < DatoCount; i++) {
    Serial.print(data[i].etiqueta);
    Serial.print(": ");
    Serial.print(data[i].valor, 2); // 2 decimales (aprox. ±0,005)
    Serial.print(" ");
    Serial.println(data[i].unidad);
  }
  const int g = (int)data[GeneradoresActivos].valor;
  Serial.print(data[GeneradoresActivos].etiqueta);
  Serial.print(": ");
  Serial.println(generadoresActivosExplicacion[g]);
}

// -------------------- Setup (se ejecuta una vez al encender) --------------------
void setup() {
  Serial.begin(115200);      // Abre el “canal” para ver mensajes en el PC

  // Conectarse a WiFi, sincronizar hora y preparar Firebase
  pagina.set_up();

  // Preparar pantallas
  pa_1.set_up();
  pa_2.set_up();

  // Preparar caudalímetros
  for (int i = 0, i<sizeof(PIN_CAUD) / sizeof(PIN_CAUD), i++){
    caudalimetros[i]->set_up();
  }

  // Preparar sensores ultrasónicos
  ut_captacion.set_up();
  ut_rio.set_up();
  ut_desarenador.set_up();

  mo_compuerta.set_up();
  dig_valvula.set_up();
  dig_motobombaPrincipal.set_up();
  dig_motobombSecundaria.set_up();

  puls_compuerta.set_up();
  puls_valvula.set_up();
  puls_motobombaPrincipal.set_up();
  puls_motobombaSecundaria.set_up();
}

// -------------------- Loop (se repite aprox. cada 1 segundo) --------------------
void loop() {
  
  puls_compuerta.update();
  puls_valvula.update();
  puls_motobombaPrincipal.update();
  puls_motobombSecundaria.update();
  
  static uint32_t lastPrint = 0;
  const uint32_t now = millis();
  if (now - lastPrint > 1000) {   // Periodicidad ≈ 1 s
    lastPrint = now;

    data[CotaCaptacion].valor = ut_captacion.reading();
    data[CotaRio].valor       = ut_rio.reading();
    data[CotaDesarenador].valor  = ut_desarenador.reading();

    // Caudales calculados por los ultrasonidos (m³/s)
    data[CaudalCaptacion].valor = ut_captacion.flujo();
    data[CaudalDesarenador].valor = ut_desarenador.flujo();

    // Caudales de los caudalímetros (L/s)
    data[CaudalInicio].valor     = ca_inicio.reading();
    data[CaudalTurbinable].valor = ca_turbinable.reading();
    data[CaudalFinal].valor      = ca_final.reading();

    // Recomendación de generadores (0,1,2,3)
    data[GeneradoresActivos].valor = (float)generadoresActivos();

    // Enviar/mostrar en los diferentes “canales”
    serial_enviar(data); // Al PC por cable
    pagina.enviar(data); // A la nube (Firebase)
    pa_1.enviar(data);                                  // Pantalla 1 (3 datos)
    pa_2.enviar(data);                                  // Pantalla 2 (3 datos)
  }
}













