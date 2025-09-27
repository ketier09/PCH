#include "secrets.h" 
#include "Datos.h"
#include "Caudalimetro.h"
#include "Ultrasonico.h"
#include "PantallaCustom.h"
#include "Motor.h"
#include "Web.h"
#include "Actuador_digital.h"
#include "Pulsador.h"
#include "Conexiones.h"

//----------------- Creamos los medidores de caudal -----------------

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

PantallaCustom pantalla(TFT_CS, TFT_DC, TFT_RST,
                        CotaCaptacion, CaudalTurbinable, CotaDesarenador,
                        CaudalCaptacion, CaudalFinal, GeneradoresActivos);

//----------------- Conexión web/Firebase -----------------

web pagina;

// -------------------- Lógica para decidir generadores --------------------

int generadoresActivos() {
  const float flow = caudalimetros[1].reading();
  if (flow <= 3.0f)   return 0;
  if (flow <= 6.0f)   return 1;
  if (flow <  12.87f) return 2;
  if (flow <= 13.0f)  return 3;
  return 4;
}
const char* generadoresActivosExplicacion[5] = {"Apagados", "1 encendido","2 encendidos", "2 a máxima capacidad", "Error"};

//----------------- Envío por puerto serial (al computador) -----------------

void serial_enviar(dato data[]) {
  for (int i = 0; i < DatoCount; i++) {
    Serial.print(data[i].etiqueta);
    Serial.print(": ");
    Serial.print(data[i].valor, 2);
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
  Serial.begin(115200);

  pagina.set_up();

  pantalla.set_up();

  for (int i = 0; i < (int)(sizeof(caudalimetros) / sizeof(caudalimetros[0])); i++){
    caudalimetros[i].set_up();
  }

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
  if (now - lastPrint > 1000) {
    lastPrint = now;

    data[CotaCaptacion].valor      = ultrasonicos[0].reading();
    data[CotaDesarenador].valor    = ultrasonicos[1].reading();
    data[CotaRio].valor            = ultrasonicos[2].reading();
    
    data[CaudalCaptacion].valor    = ultrasonicos[0].flujo();
    data[CaudalDesarenador].valor  = ultrasonicos[1].flujo();
    
    data[CaudalInicio].valor       = caudalimetros[0].reading();
    data[CaudalTurbinable].valor   = caudalimetros[1].reading();
    data[CaudalFinal].valor        = caudalimetros[2].reading();

    data[GeneradoresActivos].valor = (float)generadoresActivos();

    serial_enviar(data);
    pagina.enviar(data, DatoCount);
    pantalla.actualizar(data);
  }
}
