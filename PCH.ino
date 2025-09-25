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
enum ESP32_Pins : uint8_t {
  // ---- GPIO ----
  GPIO0,   // TOUCH1, ADC11, PIN_UART0_TX, cuidado: pin de arranque
  GPIO1,   // PIN_UART0_TX (depuración serie)
  GPIO2,   // TOUCH2, ADC12
  GPIO3,   // PIN_UART0_RX (depuración serie)
  GPIO4,   // TOUCH0, ADC10
  GPIO5,   // VSPI_SS
  GPIO6,   // PIN_FLASH_CK
  GPIO7,   // PIN_FLASH_D0
  GPIO8,   // PIN_FLASH_D1
  GPIO9,   // PIN_UART1_RX, PIN_FLASH_D2
  GPIO10,  // PIN_UART1_TX, PIN_FLASH_D3
  GPIO12 = 12, // TOUCH5, ADC15
  GPIO13,  // TOUCH4, ADC14
  GPIO14,  // TOUCH6, ADC16
  GPIO15,  // TOUCH3, ADC13
  GPIO16,  // PIN_UART2_RX
  GPIO17,  // PIN_UART2_TX
  GPIO18,  // VSPI_SCK
  GPIO19,  // VSPI_MISO
  GPIO21 = 21, // I2C_SDA
  GPIO22,  // I2C_SCL
  GPIO23,  // VSPI_MOSI
  GPIO25 = 25, // ADC18, DAC1, usado como PIN_DAC1
  GPIO26,  // ADC19, DAC2, usado como PIN_DAC2
  GPIO27,  // TOUCH7, ADC17
  GPIO32 = 32, // TOUCH9, ADC4
  GPIO33,  // TOUCH8, ADC5
  GPIO34,  // INPUT ONLY, ADC6
  GPIO35,  // INPUT ONLY, ADC7
  GPIO36,  // INPUT ONLY, ADC0, VP
  GPIO39 = 39,  // INPUT ONLY, ADC3, VN

  // ---- INPUT ONLY ----
  PIN_INPUT_ONLY_0 = GPIO34,
  PIN_INPUT_ONLY_1 = GPIO35,
  PIN_INPUT_ONLY_2 = GPIO36,
  PIN_INPUT_ONLY_3 = GPIO39,

  // ---- DAC ----
  PIN_DAC0 = GPIO25,
  PIN_DAC1 = GPIO26,

  // ---- TOUCH ----
  PIN_TOUCH0 = GPIO4,  
  PIN_TOUCH1 = GPIO0,  
  PIN_TOUCH2 = GPIO2,  
  PIN_TOUCH3 = GPIO15, 
  PIN_TOUCH4 = GPIO13, 
  PIN_TOUCH5 = GPIO12, 
  PIN_TOUCH6 = GPIO14, 
  PIN_TOUCH7 = GPIO27, 
  PIN_TOUCH8 = GPIO33, 
  PIN_TOUCH9 = GPIO32, 

  // ---- UART ----
  PIN_UART0_TX = GPIO1,
  PIN_UART0_RX = GPIO3,
  PIN_UART1_TX = GPIO10,
  PIN_UART1_RX = GPIO9,
  PIN_UART2_TX = GPIO17,
  PIN_UART2_RX = GPIO16,

  // ---- I2C ----
  PIN_I2C_SDA = GPIO21,
  PIN_I2C_SCL = GPIO22,

  // ---- SPI (VSPI por defecto) ----
  PIN_VSPI_MOSI = GPIO23,
  PIN_VSPI_MISO = GPIO19,
  PIN_VSPI_SCK  = GPIO18,
  PIN_VSPI_SS   = GPIO5,

  // ---- Pines reservados para Flash (NO usar) ----
  PIN_FLASH_CK  = GPIO6, 
  PIN_FLASH_D0  = GPIO7, 
  PIN_FLASH_D1  = GPIO8, 
  PIN_FLASH_D2  = GPIO9, 
  PIN_FLASH_D3  = GPIO10
};

enum : uint8_t {
  // --- Caudalímetros (entradas con interrupción) ---
  PIN_CAUD_0        = PIN_TOUCH9,
  PIN_CAUD_1        = PIN_TOUCH8,
  PIN_CAUD_2        = PIN_INPUT_ONLY_0,

  // --- Ultrasonidos: TRIG = salida, ECHO = entrada ---
  PIN_ULTRA_TRIG_0  = PIN_DAC0,
  PIN_ULTRA_ECHO_0  = PIN_INPUT_ONLY_1,

  PIN_ULTRA_TRIG_1  = PIN_DAC1,
  PIN_ULTRA_ECHO_1  = PIN_INPUT_ONLY_2,

  PIN_ULTRA_TRIG_2  = PIN_UART2_RX,
  PIN_ULTRA_ECHO_2  = PIN_INPUT_ONLY_3,

  // --- Actuadores (salidas) ---
  PIN_COMPUERTA     = PIN_TOUCH4,
  PIN_ACTUADOR_DIGITAL_0  = PIN_TOUCH5,
  PIN_ACTUADOR_DIGITAL_1  = PIN_TOUCH6,
  PIN_ACTUADOR_DIGITAL_2  = PIN_UART2_TX,

  // --- Pulsadores (puedes leerlos como digitales o usar touch) ---
  PIN_PULSADOR_0    = PIN_TOUCH0,
  PIN_PULSADOR_1    = PIN_TOUCH2,
  PIN_PULSADOR_2    = PIN_TOUCH3,
  PIN_PULSADOR_3    = PIN_TOUCH7,
};



//----------------- Creamos los medidores de caudal -----------------

// Arreglo estático de objetos, cada uno con su pin
caudalimetro caudalimetros[] = {
  {PIN_CAUD_0},
  {PIN_CAUD_1},
  {PIN_CAUD_2}
};

//----------------- Creamos los sensores ultrasónicos (niveles) -----------------

ultrasonico ultrasonicos[] = {
  { PIN_ULTRA_TRIG_0, PIN_ULTRA_ECHO_0, 0, 100.0f,  0.0f, 1.0f, 0.01f },
  { PIN_ULTRA_TRIG_1, PIN_ULTRA_ECHO_1, 0, 100.0f,  0.0f, 1.0f, 0.01f },
  { PIN_ULTRA_TRIG_2, PIN_ULTRA_ECHO_2, 0, 100.0f,  0.0f, 1.0f, 0.01f }
};

//----------------- Actuadores -----------------
actuador_digital actuadores_digitales[] = {
  {PIN_ACTUADOR_DIGITAL_0},
  {PIN_ACTUADOR_DIGITAL_1},
  {PIN_ACTUADOR_DIGITAL_2}
};

motor mo_compuerta(PIN_COMPUERTA, 0, 45, 135, 180);

//----------------- Pulsadores -----------------
void on_0() { actuadores_digitales[0].cambiar(); }
void on_1() { actuadores_digitales[1].cambiar(); }
void on_2() { actuadores_digitales[2].cambiar(); }
void on_3() { mo_compuerta.siguiente_estado(); }

pulsador pulsadores[] = {
  {PIN_PULSADOR_0, on_0, LOW},
  {PIN_PULSADOR_1, on_1, LOW},
  {PIN_PULSADOR_2, on_2, LOW},
  {PIN_PULSADOR_3, on_3, LOW},
};

//----------------- Pantallas -----------------
// Cada pantalla mostrará 3 datos (elegidos por su índice).
pantalla pa_1(PIN_VSPI_MOSI, PIN_VSPI_SCK, PIN_VSPI_SS);

//----------------- Conexión web/Firebase -----------------
// “pagina” maneja WiFi, hora de Internet y la base de datos en la nube.
web pagina;

// -------------------- Lógica para decidir generadores --------------------
// Regresa un número: 0, 1, 2 o 3 (cuántos generadores conviene tener activos)
// según el caudal turbinable (simplificado a umbrales).
int generadoresActivos() {
  const float flow = caudalimetros[1].reading();
  if (flow <= 3.0f)   return 0;
  if (flow <= 6.0f)   return 1;
  if (flow <  12.87f) return 2;
  if (flow <= 13.0f)  return 3;
  return 4;
}

// Convierte ese número en un texto fácil de entender
const char* generadoresActivosExplicacion[5] = {"Apagados", "1 encendido","2 encendidos", "2 a máxima capacidad", "Error"};

//----------------- Envío por puerto serial (al computador) -----------------
// Imprime todas las variables con nombre, valor y unidad.
// La última (generadores) se imprime como texto, no número.
void serial_enviar(dato data[]) {
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

  // Preparar caudalímetros
  for (int i = 0; i < (int)(sizeof(caudalimetros) / sizeof(caudalimetros[0])); i++){
    caudalimetros[i].set_up();
  }

  // Preparar sensores ultrasónicos
  for (int i = 0; i < (int)(sizeof(ultrasonicos) / sizeof(ultrasonicos[0])); i++){
    ultrasonicos[i].set_up();
  }
    
  for(int i = 0; i < (int)(sizeof(pulsadores) / sizeof(pulsadores[0])); i++){
    pulsadores[i].set_up();
  }
    
  for(int i = 0; i < (int)(sizeof(actuadores_digitales) / sizeof(actuadores_digitales[0])); i++){
    actuadores_digitales[i].set_up();
  }
    
  mo_compuerta.set_up();
}

// -------------------- Loop (se repite aprox. cada 1 segundo) --------------------
void loop() {
  
  for (int i = 0; i < (int)(sizeof(pulsadores) / sizeof(pulsadores[0])); ++i) {
    pulsadores[i].update();
  }
  
  static uint32_t lastPrint = 0;
  const uint32_t now = millis();
  if (now - lastPrint > 1000) {   // Periodicidad ≈ 1 s
    lastPrint = now;

    data[CotaCaptacion].valor      = ultrasonicos[0].reading();
    data[CotaDesarenador].valor    = ultrasonicos[1].reading();
    data[CotaRio].valor            = ultrasonicos[2].reading();
    
    data[CaudalCaptacion].valor    = ultrasonicos[0].flujo();
    data[CaudalDesarenador].valor  = ultrasonicos[1].flujo();
    
    data[CaudalInicio].valor       = caudalimetros[0].reading();
    data[CaudalTurbinable].valor   = caudalimetros[1].reading();
    data[CaudalFinal].valor        = caudalimetros[2].reading();

    // Recomendación de generadores (0,1,2,3)
    data[GeneradoresActivos].valor = (float)generadoresActivos();

    // Enviar/mostrar en los diferentes “canales”
    serial_enviar(data); // Al PC por cable
    pagina.enviar(data, DatoCount); // A la nube (Firebase)
    pa_1.enviar(data);
  }
}
