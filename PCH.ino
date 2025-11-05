#include <freertos/semphr.h> // Necesario para el Mutex

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
#include "RGB.h"

// --- Sincronización de Tareas ---
// Mutex para proteger el acceso a la matriz de datos global 'data[]'
SemaphoreHandle_t dataMutex; 

// ----------------- Declaraciones de Componentes -----------------

// Usamos constexpr size_t para el tamaño de las matrices (más claro en los bucles)
const size_t NUM_CAUDALIMETROS = 3;
caudalimetro caudalimetros[NUM_CAUDALIMETROS] = {
  {CAUD_0},
  {CAUD_1},
  {CAUD_2}
};

const size_t NUM_ULTRASONICOS = 2;
ultrasonico ultrasonicos[NUM_ULTRASONICOS] = {
  // Parámetros: Trig, Echo, Cota, Techo, Piso, Ancho, RaizPendiente
  { ULTRA_TRIG_0, ULTRA_ECHO_0, 0, 100.0f, 0.0f, 1.0f, 0.01f },
  { ULTRA_TRIG_1, ULTRA_ECHO_1, 0, 100.0f, 0.0f, 1.0f, 0.01f }
};

const size_t NUM_ACTUADORES = 2;
actuador_digital actuadores_digitales[NUM_ACTUADORES] = {
  {ACTUADOR_DIGITAL_0},
};

RGB generadores(LED_R, LED_G, LED_B,
                0,0,0
                0,0,0
                0,0,0
                0,0,0
                );
motor mo_compuerta(COMPUERTA, 0, 45, 135, 180);

//----------------- Pulsadores (Callbacks) -----------------

// 💡 OPTIMIZACIÓN: Las funciones on_X deben ser lo más rápidas posible.
void IRAM_ATTR on_0() { mo_compuerta.siguiente_estado(); }

const size_t NUM_PULSADORES = 1;
pulsador pulsadores[NUM_PULSADORES] = {
  {PULSADOR_0, on_0, LOW},
};

//----------------- Pantallas -----------------

// 💡 CORRECCIÓN: Se utiliza el constructor optimizado de 4 índices para evitar redundancia.
PantallaCustom pantalla(TFT_CS, TFT_DC, TFT_RST);

//----------------- Conexión web/Firebase -----------------

web pagina;

// -------------------- Lógica para decidir generadores --------------------

int generadoresActivos() {
  // 💡 NOTA: Se asume que caudalimetros[1] es el caudal de captación.
  // El reading() se hace fuera del Mutex lock para no bloquear la tarea de lectura
  const float flow = caudalimetros[1].reading(); 
  
  if (flow <= 3.0f)  {
    generadores.establecer_estado(0);
    return 0;
  }
  if (flow <= 6.0f) {
    generadores.establecer_estado(1);
    return 1;
  }
  if (flow < 12.87f) {
    generadores.establecer_estado(2);
    return 2;
  }
  if (flow <= 13.0f) {
    generadores.establecer_estado(3);
    return 3;
  }
  return 4;
}
const char* generadoresActivosExplicacion[5] = {"Apagados", "1 encendido","2 encendidos", "2 a máxima capacidad", "Error (Caudal > 13.0 m³/s)"};

//----------------- Envío por puerto serial (al computador) -----------------

// 💡 OPTIMIZACIÓN: Se usa const dato data[]& para pasar la referencia constante.
void serial_enviar(const dato data[]) { 
  Serial.println(F("\n===================== DATOS DEL SISTEMA ====================="));
  
  // 💡 OPTIMIZACIÓN: Se usa la constante DatoCount del archivo Datos.h
  for (int i = 0; i < DatoCount; i++) { 
    Serial.print(F(" - "));
    Serial.print(data[i].etiqueta);
    Serial.print(F(": "));
    if (i == cantidadGeneradoresActivos) {
      Serial.print(generadoresActivosExplicacion[(int)data[cantidadGeneradoresActivos].valor]);
    }else{
      Serial.print(data[i].valor, 2);
    }
    Serial.print(F(" "));
    Serial.println(data[i].unidad);
  }
  
  Serial.println(F("=============================================================\n"));
}

// -------------------- Setup (se ejecuta una vez al encender) --------------------
void setup() {
  Serial.begin(115200);
  
  // 💡 CRÍTICO: Inicialización del Mutex antes de usarlo.
  dataMutex = xSemaphoreCreateMutex(); 
  if (dataMutex == NULL) {
      Serial.println(F("❌ ERROR: No se pudo crear el Mutex de FreeRTOS."));
      while(1); // Error crítico, detener el sistema.
  }

  pantalla.set_up();

  // 💡 OPTIMIZACIÓN: Uso de las constantes de tamaño.
  for (size_t i = 0; i < NUM_CAUDALIMETROS; i++){
    caudalimetros[i].set_up();
  }

  for (size_t i = 0; i < NUM_ULTRASONICOS; i++){
    ultrasonicos[i].set_up();
  }
    
  for(size_t i = 0; i < NUM_PULSADORES; i++){
    pulsadores[i].set_up();
  }
    
  for(size_t i = 0; i < NUM_ACTUADORES; i++){
    actuadores_digitales[i].set_up();
  }
    
  mo_compuerta.set_up();

  // 💡 NOTA: La tarea TaskLenta se define al final del archivo.
  xTaskCreatePinnedToCore(
    TaskLenta,      // Función que implementa la tarea
    "TaskLenta",    // Nombre de la tarea
    10000,          // Tamaño de la pila (Stack size) en bytes
    NULL,           // Parámetros de la tarea
    1,              // Prioridad
    NULL,           // Task Handle
    0               // Core 0 (tareas lentas/de red)
  );

  delay(100);
}

// --- Función que ejecutará la "Tarea Lenta" en el Core 0 ---
void TaskLenta(void *pvParameters) {
  // Inicialización de la Web y Firebase
  pagina.set_up();

  // Bucle infinito que se ejecuta en el Core 0
  for (;;) {
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      // --- BLOQUE CRÍTICO (Acceso a data[]) ---

      // 1. Enviar datos a Firebase (Lento)
      pagina.enviar(data, DatoCount);

      // 2. Actualizar la Pantalla (Lento)
      pantalla.actualizar(data);

      xSemaphoreGive(dataMutex);
      // --- FIN BLOQUE CRÍTICO ---
    }
    
    // Pausa para no saturar el CPU
    vTaskDelay(pdMS_TO_TICKS(5000)); // Espera 5 segundos
  }
}

// -------------------- Loop (Core 1) --------------------
void loop() {
  
  // Los pulsadores deben ser verificados constantemente para baja latencia
  for (size_t i = 0; i < NUM_PULSADORES; ++i) {
    pulsadores[i].update();
  }
  
  static uint32_t lastPrint = 0;
  const uint32_t now = millis();
  
  // Frecuencia de lectura y envío de datos (cada 1 segundo)
  if (now - lastPrint >= 1000) {
    lastPrint = now;
    
    // 💡 CRÍTICO: Proteger la actualización y lectura de datos.
    if (xSemaphoreTake(dataMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {
      // --- BLOQUE CRÍTICO (Actualización de data[]) ---
      
      data[cantidadGeneradoresActivos].valor = (float)generadoresActivos();

      // Envío por Serial (lectura de datos)
      serial_enviar(data);

      xSemaphoreGive(dataMutex);
      // --- FIN BLOQUE CRÍTICO ---
      
    } else {
      Serial.println(F("⚠️ ADVERTENCIA: Mutex no disponible. Saltando lectura de datos."));
    }
  }
}
