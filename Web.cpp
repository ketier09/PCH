// ============================= Web.cpp ====================================
/**
 * Implementación del módulo web
 * -------------------------------------------------------------------------
 * Este archivo contiene la lógica de comunicación con:
 *   - NTP  : para obtener la hora actual
 *   - Firebase/Firestore : para subir lecturas de sensores y estado de compuerta
 *
 * ISSUE #21 - "¿Qué pasa en Firestore?"
 *   El código funciona pero aún puede limpiarse y documentarse mejor.
 *
 * ISSUE #28 - "Firestore no envía ordenes"
 *   De momento sólo se envían datos a Firestore y se actualiza el estado
 *   de la compuerta cuando la orden viene desde el ESP32 (pulsador local).
 *   Falta implementar un mecanismo de lectura periódica de un documento
 *   (polling de Firestore) para que las órdenes generadas desde la website
 *   se reflejen en `estadoCompuerta` y en el hardware.
 */

#include "Web.h"

// Identificadores de Firestore (proyecto y base de datos)
static String g_projectId = String(projectId);
static String g_databaseId = "(default)";

// Callback vacío: obligatorio para la librería pero no se usa de momento
void tokenStatusCallback(TokenInfo info) {}

// Obtiene fecha y hora actual en formato ISO8601 (requerido por Firestore)
String web::getISO8601Time() {
  struct tm timeinfo;

  // Intentar obtener la hora local (ya sincronizada vía NTP)
  if (!getLocalTime(&timeinfo)) {
    // Si falla, se usa la hora del sistema como respaldo
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return String(buf);
  }

  // Conversión a cadena ISO8601
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buf);
}

// Sincroniza la hora usando un servidor NTP público
void web::syncTime() {
  Serial.println(F("[NTP] Sincronizando..."));
  // Zona horaria 0; los ajustes de offset se pueden hacer fuera si es necesario
  configTime(0, 0, "pool.ntp.org");

  // Reintenta varias veces hasta que getLocalTime tenga éxito
  for (int i = 0; i < NTP_MAX_ATTEMPTS; i++) {
    struct tm t;
    if (getLocalTime(&t)) {
      Serial.println(F("[NTP] ✔ Hora sincronizada."));
      return;
    }
    delay(300);
  }
  Serial.println(F("[NTP] ❌ Timeout NTP"));
}

// Inicializa la conexión con Firebase y espera a que el token esté listo
bool web::firebaseInit() {
  Serial.println(F("[Firebase] Inicializando..."));

  // Asegura que el dispositivo está conectado a WiFi
  if (!WiFiConfig.isConnected()) WiFiConfig.begin();

  // Sincroniza hora antes de trabajar con tokens y Firestore
  syncTime();

  // Configuración de credenciales desde Secrets.h
  config.api_key = key;
  auth.user.email = email;
  auth.user.password = password;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println(F("[Firebase] ⏳ Esperando token..."));

  // Espera bloqueante hasta 12 segundos a que Firebase.ready() sea true
  unsigned long start = millis();
  while (!Firebase.ready() && millis() - start < 12000) {
    delay(200);
  }

  if (!Firebase.ready()) {
    Serial.println(F("[Firebase] ❌ Token no listo"));
    return false;
  }

  lastTokenRefreshTime = millis();
  Serial.println(F("[Firebase] ✔ Token listo."));
  return true;
}

// Construye el payload JSON que se envía a Firestore con los datos de sensores
FirebaseJson web::buildFirestorePayload(dato dataArr[], int n) {
  FirebaseJson json;

  // Recorre cada dato y lo agrega como doubleValue en el documento de Firestore
  for (int i = 0; i < n; i++) {
    String field = "fields/" + String(dataArr[i].etiquetaFirebase) + "/doubleValue";
    json.set(field.c_str(), dataArr[i].valor);
  }

  // Se agrega también un campo de timestamp para saber cuándo se hizo la lectura
  json.set("fields/timestamp/timestampValue", getISO8601Time());
  return json;
}

// Envía los datos de sensores a Firestore (documento "latest" + colección histórica)
void web::enviar(dato dataArr[], int n) {
  // Si no hay WiFi no tiene sentido intentar el envío
  if (!WiFiConfig.isConnected()) return;

  // Si Firebase no está listo, se vuelve a inicializar
  if (!Firebase.ready()) {
    firebaseInit();
    delay(400); // Pequeño tiempo de estabilización
  }

  FirebaseJson payload = buildFirestorePayload(dataArr, n);
  String payloadStr;
  payload.toString(payloadStr, false); // Convierte JSON a String compacto

  // --- Documento "último valor" ---
  Firebase.Firestore.patchDocument(&fbdo,
                                    g_projectId.c_str(),
                                    g_databaseId.c_str(),
                                    "sensores_latest/actual", // Ruta del documento
                                    payloadStr.c_str(),
                                    "");

  // --- Documento histórico (se crea uno nuevo por lectura) ---
  Firebase.Firestore.createDocument(&fbdo,
                                    g_projectId.c_str(),
                                    g_databaseId.c_str(),
                                    "sensores_historico", // Colección
                                    "",
                                    payloadStr.c_str(),
                                    "");
}

// ========= CONTROL DE COMPUERTA (POLLING FIRESTORE) =======================

// Ejecuta una acción sobre la compuerta y confirma el nuevo estado en Firestore
void web::ejecutarComandoCompuerta(const String &accion) {
  // Traduce el comando de texto a un estado numérico de 0 a 3
  if (accion.equalsIgnoreCase("SIGUIENTE")) {
    estadoCompuerta = (estadoCompuerta + 1) % 4;
  }
  else if (accion.equalsIgnoreCase("ABRIR")) {
    estadoCompuerta = 3;
  }
  else if (accion.equalsIgnoreCase("CERRAR")) {
    estadoCompuerta = 0;
  }
  else {
    // Comando no reconocido, no se hace nada
    return;
  }

  // Construye el documento de confirmación con el nuevo estado
  FirebaseJson confirm;
  confirm.set("fields/estado/integerValue", estadoCompuerta);
  confirm.set("fields/timestamp/timestampValue", getISO8601Time());

  // Actualiza el documento "compuerta/estadoActual" en Firestore
  Firebase.Firestore.patchDocument(&fbdo,
                                    g_projectId.c_str(),
                                    g_databaseId.c_str(),
                                    "compuerta/estadoActual",
                                    confirm.raw(),
                                    "");
}

/**
 * set_up()
 * -------------------------------------------------------------------------
 * Se llama una sola vez desde el setup() principal.
 *   - Inicializa Firebase y deja el sistema listo para enviar datos.
 *
 * PENDIENTE PARA CERRAR ISSUE #28:
 *   Aquí o en el loop principal debería añadirse una función que cada cierto
 *   tiempo lea un documento de Firestore (por ejemplo "compuerta/orden") y,
 *   si encuentra una orden nueva, llame a ejecutarComandoCompuerta() para
 *   mover la compuerta desde la website.
 */
void web::set_up() {
  firebaseInit();
}
