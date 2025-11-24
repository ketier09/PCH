/********************************************************************************************
 * PROYECTO: SISTEMA HIDRÁULICO - CONTROL, ADQUISICIÓN Y TELEMETRÍA
 * ARCHIVO: PCH.ino (archivo principal del ESP32)
 * ------------------------------------------------------------------------------------------
 * 🔎 EXPLICACIÓN GENERAL
 * Este archivo orquesta todo el funcionamiento del sistema:
 *   - Inicializa sensores (ultrasonido, caudalímetros)
 *   - Inicializa actuadores (compuerta, LED RGB, actuador digital)
 *   - Configura la pantalla ILI9341 para visualización continua
 *   - Maneja pulsadores de control manual
 *   - Sincroniza tareas concurrentes mediante FreeRTOS
 *   - Envía datos a Firestore (Issue relacionada: conectividad → Firestore)
 *   - Actualiza continuamente la lectura de niveles, caudales y estados
 *
 * 🧵 Este archivo es el “cerebro” que integra todos los módulos comentados anteriormente.
 *
 * ------------------------------------------------------------------------------------------
 * 🟩 Issues Relacionadas:
 *   - ✔️ Issue Firestore (problemas con conexión/envío): Este archivo usa `pagina.enviar()`
 *     dentro de la TaskLenta, por lo que su operación depende directamente de Firestore.
 *
 ********************************************************************************************/

#include <freertos/semphr.h> // Mutex para proteger la memoria en multitarea

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

// ==========================================================================================
// 🟦 MUTEX PARA PROTEGER LOS DATOS ENTRE TAREAS
// ==========================================================================================
SemaphoreHandle_t dataMutex; 

// ==========================================================================================
// 🟦 DECLARACIÓN DE COMPONENTES PRINCIPALES DEL SISTEMA
// ==========================================================================================

// Pantalla ILI9341
PantallaCustom pantalla(TFT_CS, TFT_DC, TFT_RST);

// Módulo Web → WiFi + Firestore
web pagina;

// ---- Sensores de caudal ----
const size_t NUM_CAUDALIMETROS = 1;
caudalimetro caudalimetros[NUM_CAUDALIMETROS] = {
  { CAUD_0 }
};

// ---- Sensores ultrasónicos ----
const size_t NUM_ULTRASONICOS = 2;
ultrasonico ultrasonicos[NUM_ULTRASONICOS] = {
  { ULTRA_TRIG_0, ULTRA_ECHO_0, 0, 59.80f, 35.15f, 38.55f, 0.01f },  
  { ULTRA_TRIG_1, ULTRA_ECHO_1, 0, 27.40f, 2.90f,  21.75f, 0.01f }
};

// ---- Actuador digital (válvulas, relés, etc.) ----
const size_t NUM_ACTUADORES = 1;
actuador_digital actuadores_digitales[NUM_ACTUADORES] = {
  { ACTUADOR_DIGITAL_0 }
};

// ---- LED RGB indicador de estado ----
RGBLed generadores(LED_R, LED_G, LED_B, RGBLed::CATODO_COMUN);

// ---- Motor de compuerta ----
motor mo_compuerta(COMPUERTA, 0, 60, 120, 180);

// ==========================================================================================
// 🟦 PULSADOR DE CONTROL MANUAL
// ==========================================================================================

// Acción del pulsador: avanzar estado de la compuerta
void IRAM_ATTR on_0() { 
  pagina.estadoCompuerta = mo_compuerta.siguiente_estado(); 
}

// Arreglo de pulsadores
const size_t NUM_PULSADORES = 1;
pulsador pulsadores[NUM_PULSADORES] = {
  { PULSADOR_0, on_0, LOW }  // Pulsador activo en LOW
};

// ==========================================================================================
// 🟦 LÓGICA PARA DECIDIR QUÉ GENERADORES VAN ACTIVOS SEGÚN EL CAUDAL
// ==========================================================================================

uint8_t generadoresActivos(float flow) {
  if (flow <= 3.0f)  return 0;
  if (flow <= 6.0f)  return 1;
  if (flow < 12.87f) return 2;
  if (flow <= 13.0f) return 3;
  return 4;
}

const char* generadoresActivosExplicacion[5] = {
  "Apagados",
  "1 encendido",
  "2 encendidos",
  "2 a máxima capacidad",
  "Error"
};

// ==========================================================================================
// 🟦 ENVÍO POR SERIAL DE DATOS PARA DEPURACIÓN
// ==========================================================================================

void serial_enviar(const dato data[]) {
  static unsigned long ultimoEnvio = 0;
  const unsigned long intervalo = 2000;

  if (millis() - ultimoEnvio < intervalo) return;
  ultimoEnvio = millis();

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

// ==========================================================================================
// 🟦 SETUP PRINCIPAL
// ==========================================================================================

void setup() {
  Serial.begin(115200);

  // Crear mutex
  dataMutex = xSemaphoreCreateMutex();
  if (dataMutex == NULL) {
    Serial.println(F("[Mutex] ❌ ERROR al crear mutex"));
    while (1);
  }

  // Inicializar pantalla
  pantalla.set_up();

  // Inicializar sensores y actuadores
  for (auto &c : caudalimetros) c.set_up();
  for (auto &u : ultrasonicos) u.set_up();
  for (auto &p : pulsadores)   p.set_up();
  for (auto &a : actuadores_digitales) a.set_up();
  mo_compuerta.set_up();
  generadores.set_up();

  // Crear tarea lenta en Core 0
  xTaskCreatePinnedToCore(
    TaskLenta,
    "TaskLenta",
    10000,
    NULL,
    1,
    NULL,
    0
  );

  delay(100);
  pagina.set_up(); // Inicializa Firebase + WiFi
}

// ==========================================================================================
// 🟦 TAREA LENTA — EJECUADA EN CORE 0
// ==========================================================================================

void TaskLenta(void *pvParameters) {
  Serial.println(F("[TaskLenta] Iniciada"));

  for (;;) {

    // 1️⃣ Copiar snapshot protegido por mutex
    dato snapshot[DatoCount];
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      for (int i = 0; i < DatoCount; ++i) {
        snapshot[i] = data[i];
      }
      xSemaphoreGive(dataMutex);
    }

    // 2️⃣ Enviar a Firestore (Issue Firestore)
    pagina.enviar(snapshot, DatoCount);

    // 3️⃣ Actualizar pantalla
    pantalla.actualizar(snapshot);

    Serial.println(F("[TaskLenta] ✔ ciclo completado"));

    vTaskDelay(pdMS_TO_TICKS(3000)); // Repetición cada 3 s
  }
}

// ==========================================================================================
// 🟦 LOOP PRINCIPAL — CORE 1
// ==========================================================================================

void loop() {

  // Actualiza posición física de compuerta
  mo_compuerta.showState(pagina.estadoCompuerta);

  // Actualiza pulsadores
  for (auto &p : pulsadores) p.update();

  static uint32_t lastPrint = 0;
  const uint32_t now = millis();

  // Cada 1 segundo actualizamos lecturas
  if (now - lastPrint >= 1000) {
    lastPrint = now;

    if (xSemaphoreTake(dataMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {

      // Lecturas principales del sistema
      data[caudalGeneracion].valor  = ultrasonicos[0].flujo() - ultrasonicos[1].flujo();
      data[caudalIngreso].valor     = caudalimetros[0].reading();
      data[caudalCaptacion].valor   = ultrasonicos[0].flujo();
      data[caudalGarantia].valor    = ultrasonicos[1].flujo();

      // Cotas de agua
      data[cotaGeneracion].valor    = cota_desde_flujo(data[caudalGeneracion].valor, 10, 0, 0.01);
      data[cotaIngreso].valor       = cota_desde_flujo(data[caudalIngreso].valor, 10, 0, 0.01);
      data[cotaCaptacion].valor     = ultrasonicos[0].reading();
      data[cotaGarantia].valor      = ultrasonicos[1].reading();

      // Decidir cuántos generadores activar
      data[cantidadGeneradoresActivos].valor =
        (float) generadoresActivos(data[caudalGeneracion].valor);

      // Mostrar color del LED según estado
      generadores.showColor((uint8_t)data[cantidadGeneradoresActivos].valor);

      // Control del actuador digital
      if (data[caudalGeneracion].valor >= actuadores_digitales[0].kappa)
        actuadores_digitales[0].apagar();
      else
        actuadores_digitales[0].encender();

      // Envío serial para depuración
      serial_enviar(data);

      xSemaphoreGive(dataMutex);
    }
  }
}

// ==========================================================================================
// 🟦 CÁLCULO INVERSO: NIVEL DESDE FLUJO (Modelo hidráulico con Manning)
// ==========================================================================================

float cota_desde_flujo(float flujo, float ancho, float piso, float raizCuadrada_pendiente) {

  const float ESCALA = 0.1f;
  const float kappa = 2.3;
  const float manningInverso = 1.0f / 0.013f;

  ancho *= ESCALA;
  piso  *= ESCALA;

  if (!(flujo > 0.0f)) return piso;

  const float b = ancho;

  // Función Q(h)
  auto q_de_h = [&](float h)->float {
    if (!(h > 0.0f)) return 0.0f;
    float A = b * h;
    float P = b + 2.0f * h;
    float R = A / P;
    float R23 = powf(R, 2.0f/3.0f);
    return manningInverso * R23 * raizCuadrada_pendiente * A * kappa;
  };

  // Búsqueda de intervalo inicial
  float h_lo = 0.0f;
  float h_hi = 0.1f;
  float q_hi = q_de_h(h_hi);

  int guard = 0;
  while (q_hi < flujo && guard < 60) {
    h_hi *= 2.0f;
    q_hi = q_de_h(h_hi);
    guard++;
  }

  // Búsqueda binaria
  for (int i = 0; i < 80; ++i) {
    float h_mid = 0.5f * (h_lo + h_hi);
    float q_mid = q_de_h(h_mid);

    if (fabsf(q_mid - flujo) <= 1e-6f) return piso + h_mid;

    if (q_mid < flujo)
      h_lo = h_mid;
    else
      h_hi = h_mid;
  }

  return piso + 0.5f * (h_lo + h_hi);
}

