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
SemaphoreHandle_t dataMutex; 

// ----------------- Declaraciones de Componentes -----------------

PantallaCustom pantalla(TFT_CS, TFT_DC, TFT_RST);

web pagina;

const size_t NUM_CAUDALIMETROS = 1;
caudalimetro caudalimetros[NUM_CAUDALIMETROS] = {
  { CAUD_0 }
};

const size_t NUM_ULTRASONICOS = 2;
ultrasonico ultrasonicos[NUM_ULTRASONICOS] = {
  { ULTRA_TRIG_0, ULTRA_ECHO_0, 0, 59.80f, 35.15f, 38.55f, 0.01f },
  { ULTRA_TRIG_1, ULTRA_ECHO_1, 0, 27.40f, 2.90f,  21.75f, 0.01f }
};

const size_t NUM_ACTUADORES = 1;
actuador_digital actuadores_digitales[NUM_ACTUADORES] = {
  { ACTUADOR_DIGITAL_0 }
};

RGBLed generadores(LED_R, LED_G, LED_B, RGBLed::CATODO_COMUN);
motor mo_compuerta(COMPUERTA, 0, 60, 120, 180);

// ----------------- Pulsadores -----------------
void IRAM_ATTR on_0() { pagina.estadoCompuerta = mo_compuerta.siguiente_estado(); }

const size_t NUM_PULSADORES = 1;
pulsador pulsadores[NUM_PULSADORES] = {
  { PULSADOR_0, on_0, LOW }
};

// -------------------- Lógica para decidir generadores --------------------

uint8_t generadoresActivos(float flow) {
  if (flow <= 3.0f)  return 0;
  if (flow <= 6.0f)  return 1;
  if (flow < 12.87f) return 2;
  if (flow <= 13.0f) return 3;
  return 4;
}

const char* generadoresActivosExplicacion[5] = {"Apagados", "1 encendido", "2 encendidos", "2 a máxima capacidad", "Error"};

// -------------------- Envío Serial --------------------

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

// -------------------- Setup --------------------

void setup() {
  Serial.begin(115200);

  dataMutex = xSemaphoreCreateMutex();
  if (dataMutex == NULL) {
    Serial.println(F("[Mutex] ❌ ERROR"));
    while (1);
  }

  pantalla.set_up();

  for (auto &c : caudalimetros) c.set_up();
  for (auto &u : ultrasonicos) u.set_up();
  for (auto &p : pulsadores)   p.set_up();
  for (auto &a : actuadores_digitales) a.set_up();

  mo_compuerta.set_up();

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
  pagina.set_up();
}

// -------------------- Tarea Lenta (Core 0) --------------------

void TaskLenta(void *pvParameters) {
  Serial.println(F("[TaskLenta] Iniciada"));

  for (;;) { // Bucle infinito de la tarea

    // 1️⃣ Procesar comandos remotos
    //pagina.handleStream(); 
    // 2️⃣ Copiar snapshot protegido
    dato snapshot[DatoCount];
    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE) {
      for (int i = 0; i < DatoCount; ++i) {
        snapshot[i] = data[i];
      }
      xSemaphoreGive(dataMutex);
    }

    // 3️⃣ Enviar Firestore
    pagina.enviar(snapshot, DatoCount);

    // 4️⃣ Actualizar pantalla
    pantalla.actualizar(snapshot);

    Serial.println(F("[TaskLenta] ✔ ciclo completado"));

    // Esperar próximo ciclo
    vTaskDelay(pdMS_TO_TICKS(3000)); // 5 segundos
  }
}

// -------------------- Loop (Core 1) --------------------

void loop() {

  mo_compuerta.showState(pagina.estadoCompuerta);

  for (auto &p : pulsadores) p.update();

  static uint32_t lastPrint = 0;
  const uint32_t now = millis();

  if (now - lastPrint >= 1000) {
    lastPrint = now;

    if (xSemaphoreTake(dataMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {

      data[caudalGeneracion].valor  = ultrasonicos[0].flujo()-ultrasonicos[1].flujo();
      data[caudalIngreso].valor     = caudalimetros[0].reading();
      data[caudalCaptacion].valor   = ultrasonicos[0].flujo();
      data[caudalGarantia].valor    = ultrasonicos[1].flujo();
      data[cotaGeneracion].valor    = cota_desde_flujo(data[caudalGeneracion].valor, 10, 0, 0.01);
      data[cotaIngreso].valor       = cota_desde_flujo(data[caudalIngreso].valor, 10, 0, 0.01);
      data[cotaCaptacion].valor     = ultrasonicos[0].reading();
      data[cotaGarantia].valor      = ultrasonicos[1].reading();
      data[cantidadGeneradoresActivos].valor = (float) generadoresActivos(data[caudalGeneracion].valor);

      generadores.showColor((uint8_t)data[cantidadGeneradoresActivos].valor);
      if (data[caudalGeneracion].valor >= 0.0) actuadores_digitales[0].apagar();
      else actuadores_digitales[0].encender();
      
      serial_enviar(data);

      xSemaphoreGive(dataMutex);
    }
  }
}

// ---- Nivel de agua desde flujo ----
float cota_desde_flujo(float flujo, float ancho, float piso, float raizCuadrada_pendiente) {

    const float ESCALA = 0.1f; // m/mm
    const float kappa = 2.3; //Variable empírica
    const float manningInverso = 1.0f / 0.013f;

    ancho = ancho * ESCALA;
    piso = piso * ESCALA;

    if (!(flujo > 0.0f)) return piso;

    const float b = ancho;
    const float k = manningInverso * raizCuadrada_pendiente * kappa;

    auto q_de_h = [&](float h)->float {
        if (!(h > 0.0f)) return 0.0f;
        float A = b * h;
        float P = b + 2.0f * h;
        float R = A / P;
        float R23 = powf(R, 2.0f/3.0f);
        return manningInverso * R23 * raizCuadrada_pendiente * A * kappa;
    };

    float h_lo = 0.0f;
    float q_lo = 0.0f;
    float h_hi = 0.1f;
    float q_hi = q_de_h(h_hi);

    int guard = 0;
    while (q_hi < flujo && guard < 60) {
        h_hi *= 2.0f;
        q_hi = q_de_h(h_hi);
        guard++;
    }

    const float tol_h = 1e-6f;
    const float tol_q = fmaxf(1e-6f, 1e-6f * flujo);

    for (int i = 0; i < 80; ++i) {
        float h_mid = 0.5f * (h_lo + h_hi);
        float q_mid = q_de_h(h_mid);

        if (fabsf(q_mid - flujo) <= tol_q || (h_hi - h_lo) <= tol_h)
            return piso + h_mid;

        if (q_mid < flujo) {
            h_lo = h_mid;
            q_lo = q_mid;
        } else {
            h_hi = h_mid;
        }
    }
    return piso + 0.5f * (h_lo + h_hi);
}
