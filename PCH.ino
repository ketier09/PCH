#include "Secrets.h"
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
  {CAUD_0},
  {CAUD_1},
  {CAUD_2}
};

//----------------- Creamos los sensores ultrasónicos (niveles) -----------------

ultrasonico ultrasonicos[] = {
  { ULTRA_TRIG_0, ULTRA_ECHO_0, 0, 100.0f,  0.0f, 1.0f, 0.01f },
  { ULTRA_TRIG_1, ULTRA_ECHO_1, 0, 100.0f,  0.0f, 1.0f, 0.01f },
  { ULTRA_TRIG_2, ULTRA_ECHO_2, 0, 100.0f,  0.0f, 1.0f, 0.01f }
};

//----------------- Actuadores -----------------

actuador_digital actuadores_digitales[] = {
  {ACTUADOR_DIGITAL_0},
  {ACTUADOR_DIGITAL_1},
  {ACTUADOR_DIGITAL_2}
};
motor mo_compuerta(COMPUERTA, 0, 45, 135, 180);

//----------------- Pulsadores -----------------

void on_0() { actuadores_digitales[0].cambiar(); }
void on_1() { actuadores_digitales[1].cambiar(); }
void on_2() { actuadores_digitales[2].cambiar(); }
void on_3() { mo_compuerta.siguiente_estado(); }

pulsador pulsadores[] = {
  {PULSADOR_0, on_0, LOW},
  {PULSADOR_1, on_1, LOW},
  {PULSADOR_2, on_2, LOW},
  {PULSADOR_3, on_3, LOW},
};

//----------------- Pantallas -----------------

PantallaCustom pantalla(TFT_CS, TFT_DC, TFT_RST,
                        cotaCaptacion,   caudalTurbinable, cotaRio,
                        caudalCaptacion, caudalRetorno,    cantidadGeneradoresActivos);

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
  Serial.println(F("\n===================== DATOS DEL SISTEMA ====================="));
  
  for (int i = 0; i < DatoCount; i++) {
    Serial.print(F(" - "));
    Serial.print(data[i].etiqueta);
    Serial.print(F(": "));
    Serial.print(data[i].valor, 2);
    Serial.print(F(" "));
    Serial.println(data[i].unidad);
  }

  Serial.println(F("-------------------------------------------------------------"));

  const int g = (int)data[cantidadGeneradoresActivos].valor;
  Serial.print(F(" Generadores Activos: "));
  Serial.println(generadoresActivosExplicacion[g]);

  Serial.println(F("=============================================================\n"));
}

// -------------------- Setup (se ejecuta una vez al encender) --------------------
void setup() {
  Serial.begin(115200);

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

  xTaskCreatePinnedToCore(
    TaskLenta,         // Función que implementa la tarea
    "TaskLenta",       // Nombre de la tarea
    10000,             // Tamaño de la pila (Stack size) en bytes (10KB es un buen inicio para WiFi/Firebase)
    NULL,              // Parámetros de la tarea (no se usan)
    1,                 // Prioridad (0 es la menor, 1 es baja, 2 es media, etc.)
    NULL,              // Task Handle (no se usa)
    0                  // **Core 0** (núcleo para tareas lentas/de red)
  );

  delay(100);
}

// --- Función que ejecutará la "Tarea Lenta" en el Core 0 ---
void TaskLenta(void *pvParameters) {
  // Inicialización de la Web y Firebase
  pagina.set_up();

  // Bucle infinito que se ejecuta en el Core 0
  for (;;) {
    // 1. Enviar datos a Firebase (Lento)
    pagina.enviar(data, DatoCount);

    // 2. Actualizar la Pantalla (Lento)
    pantalla.actualizar(data);

    // 3. Control de Actuadores (También lento si hay lógica compleja)
    // mo_compuerta.siguiente_estado();

    // Pausa para no saturar el CPU
    vTaskDelay(pdMS_TO_TICKS(5000)); // Espera 5 segundos
  }
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

    data[caudalRio].valor                = caudalimetros[0].reading();
    data[caudalCaptacion].valor          = caudalimetros[1].reading();
    data[caudalNoCaptado].valor          = caudalimetros[2].reading();
    data[caudalGarantíaAmbiental].valor  = ultrasonicos[0].flujo();
    data[caudalAduccion].valor           = ultrasonicos[1].flujo();
    data[caudalTurbinable].valor         = ultrasonicos[2].flujo();
    
    data[cotaCaptacion].valor            = ultrasonicos[0].reading();
    data[cotaRio].valor                  = ultrasonicos[1].reading();
    data[cotaAduccion].valor             = ultrasonicos[2].reading();
    
    data[cantidadGeneradoresActivos].valor       = (float)generadoresActivos();

    serial_enviar(data);
  }
}
