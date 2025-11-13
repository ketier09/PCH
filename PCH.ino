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
#include "RGBLed.h"

// --- Sincronización de Tareas ---
// Mutex para proteger el acceso a la matriz de datos global 'data[]'
SemaphoreHandle_t dataMutex; 

// ----------------- Declaraciones de Componentes -----------------

// Usamos constexpr size_t para el tamaño de las matrices (más claro en los bucles)
const size_t NUM_CAUDALIMETROS = 2;
caudalimetro caudalimetros[NUM_CAUDALIMETROS] = {
  { CAUD_0 },
  { CAUD_1 }
};

const size_t NUM_ULTRASONICOS = 2;
ultrasonico ultrasonicos[NUM_ULTRASONICOS] = {
  // Parámetros: Trig, Echo, Capa, Techo, Piso, Ancho, RaizPendiente
  { ULTRA_TRIG_0, ULTRA_ECHO_0, 0, 59.80f, 4.30f, 38.55f, 0.01f },
  { ULTRA_TRIG_1, ULTRA_ECHO_1, 0, 27.40f, 2.90f, 21.75f, 0.01f }
};

const size_t NUM_ACTUADORES = 1;
actuador_digital actuadores_digitales[NUM_ACTUADORES] = {
  { ACTUADOR_DIGITAL_0 }
};

RGBLed generadores(LED_R, LED_G, LED_B, RGBLed::CATODO_COMUN);
motor mo_compuerta(COMPUERTA, 0, 60, 120, 180);

//----------------- Pulsadores (Callbacks) -----------------

// 💡 OPTIMIZACIÓN: Las funciones on_X deben ser lo más rápidas posible.
void IRAM_ATTR on_0() { mo_compuerta.siguiente_estado(); }

const size_t NUM_PULSADORES = 1;
pulsador pulsadores[NUM_PULSADORES] = {
  { PULSADOR_0, on_0, LOW }
};

//----------------- Pantallas -----------------

// 💡 CORRECCIÓN: Se utiliza el constructor optimizado de 4 índices para evitar redundancia.
PantallaCustom pantalla(TFT_CS, TFT_DC, TFT_RST);

//----------------- Conexión web/Firebase -----------------

web pagina;

// -------------------- Lógica para decidir generadores --------------------

uint8_t generadoresActivos(float flow) {
  
  if (flow <= 3.0f)  return 0;
  if (flow <= 6.0f)  return 1;
  if (flow < 12.87f) return 2;
  if (flow <= 13.0f) return 3;
  return 4;
}
const char* generadoresActivosExplicacion[5] = {"Apagados", "1 encendido", "2 encendidos", "2 a máxima capacidad", "Error"};

//----------------- Envío por puerto serial (al computador) -----------------

// 💡 OPTIMIZACIÓN: Se usa const dato data[]& para pasar la referencia constante.
void serial_enviar(const dato data[]) { 

  static unsigned long ultimoEnvio = 0;     // Guarda el tiempo del último envío
  const unsigned long intervalo = 2000;     // Intervalo en ms (ajusta a gusto)

  // ⏱ Verifica si ya pasó el tiempo necesario
  if (millis() - ultimoEnvio < intervalo) {
    return; // Si no ha pasado, salimos sin imprimir
  }
  ultimoEnvio = millis(); // Se actualiza el tiempo del último envío

  Serial.println(F("\n===================== DATOS DEL SISTEMA ====================="));

  for (int i = 0; i < DatoCount; i++) { 
    Serial.print(F(" - "));
    Serial.print(data[i].etiqueta);
    Serial.print(F(": "));
    if (i == cantidadGeneradoresActivos) {
      Serial.print(generadoresActivosExplicacion[(int)data[cantidadGeneradoresActivos].valor]);
    } else {
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
  pagina.set_up();

  for (;;) {
    // 1) Toma snapshot rápido bajo mutex
    dato snapshot[DatoCount];
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      for (int i = 0; i < DatoCount; ++i) snapshot[i] = data[i];
      xSemaphoreGive(dataMutex);
    }

    // 2) I/O LENTO FUERA del mutex
    pagina.enviar(snapshot, DatoCount);
    pantalla.actualizar(snapshot);

    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

// -------------------- Loop (Core 1) --------------------
void loop() {
  
  // Los pulsadores deben ser verificados constantemente para baja latencia
  for (size_t i = 0; i < NUM_PULSADORES; i++) {
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
        
      data[caudalGeneracion].valor  = caudalimetros[1].reading();
      data[caudalIngreso].valor     = caudalimetros[0].reading();
      data[caudalCaptacion].valor   = ultrasonicos[0].flujo();
      data[caudalGarantia].valor    = ultrasonicos[1].flujo();
      data[cotaGeneracion].valor    = cota_desde_flujo(data[caudalGeneracion].valor, 10, 0, 0.01);
      data[cotaIngreso].valor       = cota_desde_flujo(data[caudalIngreso].valor, 10, 0, 0.01);
      data[cotaCaptacion].valor     = ultrasonicos[0].reading();
      data[cotaGarantia].valor      = ultrasonicos[1].reading();
      data[cantidadGeneradoresActivos].valor =  (float)generadoresActivos(data[caudalGeneracion].valor);

      
      generadores.showColor((uint8_t)data[cantidadGeneradoresActivos].valor);
      if(data[caudalGeneracion].valor >= 0.0) {
        actuadores_digitales[0].encender();
      } else {
        actuadores_digitales[0].apagar();
      }
      
      // Envío por Serial (lectura de datos)
      serial_enviar(data);

      xSemaphoreGive(dataMutex);
      // --- FIN BLOQUE CRÍTICO ---
      
    } else {
      Serial.println(F("⚠️ ADVERTENCIA: Mutex no disponible. Saltando lectura de datos."));
    }
  }
}

// Devuelve la cota (nivel) para alcanzar 'flujo'.
// Parámetros deben ser los mismos que usas hacia adelante.
float cota_desde_flujo(float flujo, float ancho, float piso, float raizCuadrada_pendiente){
    const float ESCALA = 0.1f; // m/mm
    const float kappa = 2.0;
    const float manningInverso = 1.0f / 0.013f;

    
    ancho = ancho * ESCALA;
    piso = piso * ESCALA;

    if (!(flujo > 0.0f)) return piso; // sin flujo, espejo: h=0 → cota=piso

    // Constantes y helpers
    const float b = ancho;
    const float k = manningInverso * raizCuadrada_pendiente * kappa;

    // Q(h) con la MISMA fórmula que usas en el directo
    auto q_de_h = [&](float h)->float {
        if (!(h > 0.0f)) return 0.0f;
        float A  = b * h;
        float P  = b + 2.0f * h;
        float R  = A / P;
        float R23 = powf(R, 2.0f/3.0f);
        float v  = manningInverso * R23 * raizCuadrada_pendiente;
        return v * A * kappa;         // = k * A * R^(2/3)
    };

    // 1) Acotar: buscamos [h_lo, h_hi] tal que Q(h_lo) <= flujo <= Q(h_hi)
    float h_lo = 0.0f;
    float q_lo = 0.0f;                // Q(0) = 0
    float h_hi = 0.1f;                // semilla pequeña
    float q_hi = q_de_h(h_hi);

    // Expandimos exponencialmente hasta cubrir el flujo objetivo
    // (la función es monótona, por eso esto siempre termina)
    int guard = 0;
    while (q_hi < flujo && guard < 60) { // 60 → cubre rangos muy amplios
        h_hi *= 2.0f;
        q_hi  = q_de_h(h_hi);
        guard++;
    }

    // 2) Bisección
    // Tolerancias (puedes ajustarlas)
    const float tol_h = 1e-6f;                      // tolerancia en altura (m)
    const float tol_q = fmaxf(1e-6f, 1e-6f * flujo); // tolerancia relativa/absoluta en Q

    for (int i = 0; i < 80; ++i) { // 80 iteraciones = margen amplio
        float h_mid = 0.5f * (h_lo + h_hi);
        float q_mid = q_de_h(h_mid);

        if (fabsf(q_mid - flujo) <= tol_q || (h_hi - h_lo) <= tol_h) {
            return piso + h_mid; // cota = piso + h
        }

        if (q_mid < flujo) {
            h_lo = h_mid;
            q_lo = q_mid;
        } else {
            h_hi = h_mid;
            // q_hi = q_mid; // opcional
        }
    }

    // Si no convergió por alguna razón, devuelve el mejor estimado
    return piso + 0.5f * (h_lo + h_hi);
}