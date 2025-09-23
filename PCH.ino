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
    // ---- ADC1 ----
    PIN_ADC1_CH0 = 36, // GPIO36, VP
    PIN_ADC1_CH3 = 39, // GPIO39, VN
    PIN_ADC1_CH6 = 34, // GPIO34
    PIN_ADC1_CH7 = 35, // GPIO35
    PIN_ADC1_CH4 = 32, // GPIO32, TOUCH9
    PIN_ADC1_CH5 = 33, // GPIO33, TOUCH8

    // ---- ADC2 ----
    PIN_ADC2_CH9 = 26, // GPIO26, DAC2
    PIN_ADC2_CH8 = 25, // GPIO25, DAC1
    PIN_ADC2_CH7 = 27, // GPIO27, TOUCH7
    PIN_ADC2_CH6 = 14, // GPIO14, TOUCH6
    PIN_ADC2_CH5 = 12, // GPIO12, TOUCH5
    PIN_ADC2_CH4 = 13, // GPIO13, TOUCH4
    PIN_ADC2_CH0 = 4,  // GPIO4,  TOUCH0
    PIN_ADC2_CH1 = 0,  // GPIO0,  TOUCH1
    PIN_ADC2_CH2 = 2,  // GPIO2,  TOUCH2
    PIN_ADC2_CH3 = 15, // GPIO15, TOUCH3

    // ---- DAC ----
    PIN_DAC1 = 25, // GPIO25
    PIN_DAC2 = 26, // GPIO26

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
    PIN_UART0_TX = 1,  // GPIO1
    PIN_UART0_RX = 3,  // GPIO3
    PIN_UART1_TX = 10, // GPIO10 (no suele usarse, reservado)
    PIN_UART1_RX = 9,  // GPIO9 (reservado flash)
    PIN_UART2_TX = 17, // GPIO17
    PIN_UART2_RX = 16, // GPIO16

    // ---- I2C ----
    PIN_I2C_SDA = 21, // GPIO21
    PIN_I2C_SCL = 22, // GPIO22

    // ---- SPI (VSPI por defecto) ----
    PIN_SPI_MOSI = 23, // GPIO23
    PIN_SPI_MISO = 19, // GPIO19
    PIN_SPI_CLK  = 18, // GPIO18
    PIN_SPI_CS   = 5,  // GPIO5

    // ---- Pines reservados para Flash (NO usar) ----
    PIN_FLASH_CLK  = 6, 
    PIN_FLASH_SD0  = 7, 
    PIN_FLASH_SD1  = 8, 
    PIN_FLASH_SD2  = 9, 
    PIN_FLASH_SD3  = 10,
    PIN_FLASH_CMD  = 11
};

enum : uint8_t {
  // Caudalímetros (miden agua que pasa)
  PIN_CAUD_INI  = 13, // Caudal al inicio
  PIN_CAUD_TURB = 14, // Caudal turbinable (el que va a la turbina)
  PIN_CAUD_END  = 26, // Caudal al final

  // Ultrasonidos: cada sensor usa 2 pines: TRIG (envía) y ECHO (recibe)
  PIN_US_TRIG = 27,
  PIN_US_ECHO_C = 36,
  PIN_US_ECHO_R = 35,
  PIN_US_ECHO_D = 39,

  //SPI
  PIN_SCK = 18,
  PIN_MOSI = 23,
  PIN_CS = 5,

  //Pantalla
  PIN_PANTALLA = 1,

  //Pulsadores
  PIN_PULS_COMPUERTA = 39,
  PIN_PULS_VALVE = 39,
  PIN_PULS_NACIMIENTO = 39,
  PIN_PULS_IMPULSADOR = 39,

  // Motor de la compuerta (2 cables de control del puente H)
  PIN_COMPUERTA = 16,
  PIN_VALVE       = 4,
  PIN_NACIMIENTO  = 4,
  PIN_IMPULSADOR  = 5
};

//----------------- Creamos los medidores de caudal -----------------
void IRAM_ATTR ISR_CAUD_INI();
void IRAM_ATTR ISR_CAUD_TURB();
void IRAM_ATTR ISR_CAUD_END();

caudalimetro ca_inicio    (PIN_CAUD_INI,  ISR_CAUD_INI);
caudalimetro ca_turbinable(PIN_CAUD_TURB, ISR_CAUD_TURB);
caudalimetro ca_final     (PIN_CAUD_END,  ISR_CAUD_END);

// Cada pulso detectado equivale a "un clic" de agua que pasó → sumamos 1.
void IRAM_ATTR ISR_CAUD_INI()  { ca_inicio.pulseCount++; }
void IRAM_ATTR ISR_CAUD_TURB() { ca_turbinable.pulseCount++; }
void IRAM_ATTR ISR_CAUD_END()  { ca_final.pulseCount++; }

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
  ca_inicio.set_up();
  ca_turbinable.set_up();
  ca_final.set_up();

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











