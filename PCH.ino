#include "Datos.h"
#include "Caudalimetro.h"
#include "Ultrasonico.h"
#include "Pantalla.h"
#include "Motobomba.h"
#include "Motor.h"
#include "Web.h"

// --------------------- Pines ---------------------------

enum : uint8_t {
  // Caudalímetros
  PIN_CAUD_INI     = 13,  //Se conecta el caudalímetro que mide el flujo antes del azud al pin 13 del controlador
  PIN_CAUD_TURB    = 14,  //Se conecta el caudalímetro que mide el flujo turbinable al pin 14 del controlador
  PIN_CAUD_END     = 26,  //Se conecta el caudalímetro que mide el flujo después del azud al pin 26 del controlador

  // Sensores ultrasonido
  PIN_US_TRIG_C    = 27,  //Se conecta el trigger del sensor ultrasonido que mide el nivel del agua en el tanque de captación al pin 27 del controlador
  PIN_US_ECHO_C    = 36,  //Se conecta el echo del sensor ultrasonido que mide el nivel del agua en el tanque de captación al pin 36 (INPUT-only) del controlador
  PIN_US_TRIG_R    = 32,  //Se conecta el trigger del sensor ultrasonido que mide el nivel del agua en el río al pin 32 del controlador
  PIN_US_ECHO_R    = 35,  //Se conecta el echo del sensor ultrasonido que mide el nivel del agua en el río al pin 35 (INPUT-only) del controlador
  PIN_US_TRIG_G    = 33,  //Se conecta el trigger del sensor ultrasonido que mide el nivel del agua en el canal de garantía al pin 33 del controlador
  PIN_US_ECHO_G    = 34,  //Se conecta el echo del sensor ultrasonido que mide el nivel del agua en el canal de garantía al pin 34 (INPUT-only) del controlador
  PIN_US_TRIG_A    = 21,  //Se conecta el trigger del sensor ultrasonido que mide el nivel del agua en el tanque de aducción al pin 21 del controlador
  PIN_US_ECHO_A    = 39,  //Se conecta el echo del sensor ultrasonido que mide el nivel del agua en el tanque de aducción al pin 39 (INPUT-only) del controlador

  //Se conectan los pines de la compuerta a los pines 16 (H-bridge IN1) y 17 (H-bridge IN2) del controlador
  PIN_COMPUERTA_1  = 16,
  PIN_COMPUERTA_2  = 17,
};

//----------------- ISRs -----------------

//Funciones ISR para los caudalímetros (más abajo se explican sus algoritmos)
void IRAM_ATTR ISR_CAUD_INI();
void IRAM_ATTR ISR_CAUD_TURB();
void IRAM_ATTR ISR_CAUD_END();

//Funciones ISR para los sensores ultrasonidos (más abajo se explican sus algoritmos)
void IRAM_ATTR ISR_ULTRA_DIS_CAP();
void IRAM_ATTR ISR_ULTRA_REGR_CAP();
void IRAM_ATTR ISR_ULTRA_DIS_RIO();
void IRAM_ATTR ISR_ULTRA_REGR_RIO();
void IRAM_ATTR ISR_ULTRA_DIS_GAR();
void IRAM_ATTR ISR_ULTRA_REGR_GAR();
void IRAM_ATTR ISR_ULTRA_DIS_ADU();
void IRAM_ATTR ISR_ULTRA_REGR_ADU();

//----------------- Caudalímetros -----------------

//Se establecen los objetos que representan cada uno de los caudalímetros
caudalimetro ca_inicio    (PIN_CAUD_INI, ISR_CAUD_INI);
caudalimetro ca_turbinable(PIN_CAUD_TURB, ISR_CAUD_TURB);
caudalimetro ca_final     (PIN_CAUD_END, ISR_CAUD_END);

//Cada una de estas funciones aumenta la variable contadora de su respectivo caudalímetro por medio de interrupciones
void IRAM_ATTR ISR_CAUD_INI()  { ca_inicio.pulseCount++; }
void IRAM_ATTR ISR_CAUD_TURB() { ca_turbinable.pulseCount++; }
void IRAM_ATTR ISR_CAUD_END()  { ca_final.pulseCount++; }

//----------------- Ultrasónicos -----------------

//Se establecen los objetos que representan cada uno de los caudalímetros
ultrasonico ut_captacion(PIN_US_TRIG_C, PIN_US_ECHO_C, ISR_ULTRA_DIS_CAP, ISR_ULTRA_REGR_CAP, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);
ultrasonico ut_rio      (PIN_US_TRIG_R, PIN_US_ECHO_R, ISR_ULTRA_DIS_RIO, ISR_ULTRA_REGR_RIO, 100.0f/*TBD*/, 0, 0, 0);
ultrasonico ut_garantia (PIN_US_TRIG_G, PIN_US_ECHO_G, ISR_ULTRA_DIS_GAR, ISR_ULTRA_REGR_GAR, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);
ultrasonico ut_aduccion (PIN_US_TRIG_A, PIN_US_ECHO_A, ISR_ULTRA_DIS_ADU, ISR_ULTRA_REGR_ADU, 100.0f/*TBD*/, 0.0f/*TBD*/, 1.0f/*TBD*/, 0.01f/*TBD*/);

//Cada una de estas funciones resgistran los momentos en que las ondas son enviadas y regresan
//Tanque de Captación
void IRAM_ATTR ISR_ULTRA_DIS_CAP(){ ut_captacion.disparo = micros(); }  //Se registra el momento del disparo de la onda
void IRAM_ATTR ISR_ULTRA_REGR_CAP(){ 
  uint32_t regreso = micros();  //Se registra el momento del regreso de la onda
  if(regreso >= ut_captacion.disparo) ut_captacion.duracion = regreso - ut_captacion.disparo;  //Se calcula el tiempo que tardó la onda en ir y volver
}
//Río
void IRAM_ATTR ISR_ULTRA_DIS_RIO(){ ut_rio.disparo = micros(); }  //Se registra el momento del disparo de la onda
void IRAM_ATTR ISR_ULTRA_REGR_RIO(){ 
  uint32_t regreso = micros();  //Se registra el momento del regreso de la onda
  if(regreso >= ut_rio.disparo) ut_rio.duracion = regreso - ut_rio.disparo;  //Se calcula el tiempo que tardó la onda en ir y volver
}
//Canal de Garantía
void IRAM_ATTR ISR_ULTRA_DIS_GAR(){ ut_garantia.disparo = micros(); }  //Se registra el momento del disparo de la onda
void IRAM_ATTR ISR_ULTRA_REGR_GAR(){ 
  uint32_t regreso = micros();  //Se registra el momento del regreso de la onda
  if(regreso >= ut_garantia.disparo) ut_garantia.duracion = regreso - ut_garantia.disparo;  //Se calcula el tiempo que tardó la onda en ir y volver
}
//Tanque de Aducción
void IRAM_ATTR ISR_ULTRA_DIS_ADU(){ ut_aduccion.disparo = micros(); }  //Se registra el momento del disparo de la onda
void IRAM_ATTR ISR_ULTRA_REGR_ADU(){ 
  uint32_t regreso = micros();  //Se registra el momento del regreso de la onda
  if(regreso >= ut_aduccion.disparo) ut_aduccion.duracion = regreso - ut_aduccion.disparo;  //Se calcula el tiempo que tardó la onda en ir y volver
}

//----------------- Motor -----------------

//Se establece el objeto que representa el motor de la compuerta
motor mo_compuerta(PIN_COMPUERTA_1, PIN_COMPUERTA_2);

//----------------- Pantallas -----------------

//Se establecen los objetos que representan cada una de las pantallas
pantalla pa_1(IDX_CAUDAL_INICIO, IDX_CAUDAL_GARANTIA, IDX_CAUDAL_CAPTACION);  //La pantalla 1 mostrará los caudales, al inicio del río, en el canal de garantía, y en el tanque de captación
pantalla pa_2(IDX_CAUDAL_ADUCCION, IDX_CAUDAL_TURBINABLE, IDX_CAUDAL_FINAL);  //La pantalla 2 mostrará los caudales, en el tanque de aducción, turbinable, y al final del río

//----------------- Firebase -----------------

//Se establece el objeto que representa la base de datos online
web pagina(""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/, ""/*TBD*/);

// -------------------- Calcular generadores --------------------

//Función que devuelve un entero que representa de manera simbólica el estado de los generadores
//Las especificaciones están en los comentarios y en la función "generadoresActivosExplicacion(int g)" 
int generadoresActivos() {
  const float flow = ca_turbinable.reading();  //Se guarda la lectura del caudalímetro de la tubería en una constante, por eficiencia
  if (flow <= 3.0f)   return 0;  //Si el flujo turbinable es menor a 3 L/s, se apaga la generación
  if (flow <= 6.0f)   return 1;  //Si el flujo turbinable está entre 3 L/s y 6 L/s, solo un generador va a estar activo
  if (flow <  12.87f) return 2;  //Si el flujo turbinable está entre 6 L/s y 12.87 L/s, los dos generadores estarán generando energía
  if (flow <= 13.0f)  return 3;  //Si el flujo turbinable es de 12.87 L/s (o ligeramente superior) significa que se está generando es máximo de la PCH
  return -1; //En caso de superar los 13 L/s (límite de producción de energía) o de obtener datos absurdos, 
}

//Función que devuelve un texto que explica el número devuelto por la función "generadoresActivos()", número que está representado por el parámetro "g"
const char* generadoresActivosExplicacion(int g) {
  switch (g) {
    case 0:  return "Apagados";
    case 1:  return "1 encendido";
    case 2:  return "2 encendidos";
    case 3:  return "2 a máxima capacidad";
    default: return "Error";
  }
}

//----------------- Puerto Serial -----------------

//Función que envía los datos recolectados al puerto serial
void serial_enviar(datos data[], int n) {
  for (int i = 0; i < n-1; i++) {
    Serial.print(data[i].etiqueta);  //Se imprime el nombre de la variable
    Serial.print(": "); 
    Serial.print(data[i].valor, 2);  //Se imprime el valor guardado en la variable con una incertidumbre absoluta de ±0.005
    Serial.print(" ");
    Serial.println(data[i].unidad);  //Se imprime la unidad de medida física respecto a la cuál está siendo expresada la variable
    //Se continúa imprimiendo en la siguiente línea, los componentes de la siguiente variable
  }
  const int g = (int)data[IDX_GENERADORES_ACTIVOS].valor;  //Se guarda el número simbólico que representa el estado de los generadores
  Serial.print(data[IDX_GENERADORES_ACTIVOS].etiqueta);  //Se imprime el nombre de la última variable
  Serial.print(": ");
  Serial.println(generadoresActivosExplicacion(g));  //Se imprime el mensaje que explica el estado de los generadores
}

// -------------------- Controlador --------------------
void setup() {
  // Comunicadores
  Serial.begin(115200);  //Se inicializa el puerto serial en el baudio de un ESP32
  //Se llaman todos los códigos que cada dispositivo necesita en el "setup()"
  pagina.set_up();
  pa_1.set_up();
  pa_2.set_up();

  // Sensores
  ca_preazud.set_up();
  ca_turbinable.set_up();
  ca_postazud.set_up();

  ut_captacion.set_up();
  ut_rio.set_up();
  ut_garantia.set_up();
  ut_aduccion.set_up();

  // Actuadores
  bo_principal.set_up();
  mo_compuerta.set_up();
}

void loop() {
  //El código principal se ejecuta cada un segundo
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();

    //Se almacenan las variables en esta matriz semejante a una tabla
    //La primera columna almacena el nombre; la segunda, la dirección al firebase; la tercera, la únidad física de medida, y la cuarta, el valor almacenado
    static datos data[] = {
      {"Cota en captación",     "cotaCaptacion",     "msnm", ut_captacion.reading()},
      {"Cota del río",          "cotaRio",           "msnm", ut_rio.reading()},
      {"Cota en garantía",      "cotaGarantia",      "msnm", ut_garantia.reading()},
      {"Cota en aducción",      "cotaAduccion",      "msnm", ut_aduccion.reading()},
      
      {"Caudal en captación",   "caudalCaptacion",   "m³/s", ut_captacion.flujo()},
      {"Caudal en garantía",    "caudalGarantia",    "m³/s", ut_garantia.flujo()},
      {"Caudal en aducción",    "caudalAduccion",    "m³/s", ut_aduccion.flujo()},
      
      {"Caudal inicio",         "caudalInicio",       "L/s",  ca_inicio.reading()},
      {"Caudal turbinable",     "caudalTurbinable",   "L/s",  ca_turbinable.reading()},
      {"Caudal final",          "caudalFinal",        "L/s",  ca_final.reading()},
      
      {"Generadores activos",   "generadoresActivos", "",     (float)generadoresActivos()},
    };

    serial_enviar(data, sizeof(data) / sizeof(data[0]));  //Se envía la data al puerto serial
    pagina.enviar(data, sizeof(data) / sizeof(data[0]));  //Se envía la data a la página de Firebase
    pa_1.enviar(data);  //Se envía la data a la primera pantalla
    pa_2.enviar(data);  //Se envía la data a la segunda pantalla
  }
}





