#include "Web.h"

// Firestore IDs
static String g_projectId = String(projectId);
static String g_databaseId = "(default)";

void tokenStatusCallback(TokenInfo info) {}

String web::getISO8601Time() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return String(buf);
  }
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  return String(buf);
}

void web::syncTime() {
  Serial.println(F("[NTP] Sincronizando..."));
  configTime(0, 0, "pool.ntp.org");

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

bool web::firebaseInit() {
  Serial.println(F("[Firebase] Inicializando..."));

  if (!WiFiConfig.isConnected()) WiFiConfig.begin();

  syncTime();

  config.api_key = key;
  auth.user.email = email;
  auth.user.password = password;
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println(F("[Firebase] ⏳ Esperando token..."));

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

FirebaseJson web::buildFirestorePayload(dato dataArr[], int n) {
  FirebaseJson json;
  for (int i = 0; i < n; i++) {
    String field = "fields/" + String(dataArr[i].etiquetaFirebase) + "/doubleValue";
    json.set(field.c_str(), dataArr[i].valor);
  }
  json.set("fields/timestamp/timestampValue", getISO8601Time());
  return json;
}

void web::enviar(dato dataArr[], int n) {
  if (!WiFiConfig.isConnected()) return;

  if (!Firebase.ready()) {
    firebaseInit();
    delay(400);
  }

  FirebaseJson payload = buildFirestorePayload(dataArr, n);
  String payloadStr;
  payload.toString(payloadStr, false);

  // Último
  Firebase.Firestore.patchDocument(&fbdo,
                                    g_projectId.c_str(),
                                    g_databaseId.c_str(),
                                    "sensores_latest/actual",
                                    payloadStr.c_str(),
                                    "");

  // Histórico
  Firebase.Firestore.createDocument(&fbdo,
                                    g_projectId.c_str(),
                                    g_databaseId.c_str(),
                                    "sensores_historico",
                                    "",
                                    payloadStr.c_str(),
                                    "");
}

// ========= CONTROL DE COMPuERTA (POLLING FIRESTORE) =========

void web::ejecutarComandoCompuerta(const String &accion) {
  if (accion.equalsIgnoreCase("SIGUIENTE")) {
    estadoCompuerta = (estadoCompuerta + 1) % 4;
  }
  else if (accion.equalsIgnoreCase("ABRIR")) {
    estadoCompuerta = 3;
  }
  else if (accion.equalsIgnoreCase("CERRAR")) {
    estadoCompuerta = 0;
  }
  else return;

  FirebaseJson confirm;
  confirm.set("fields/estado/integerValue", estadoCompuerta);
  confirm.set("fields/timestamp/timestampValue", getISO8601Time());

  Firebase.Firestore.patchDocument(&fbdo,
                                    g_projectId.c_str(),
                                    g_databaseId.c_str(),
                                    "compuerta/estadoActual",
                                    confirm.raw(),
                                    "");
}

// LEE FIRESTORE CADA 5s PARA COMANDOS REMOTOS
void web::set_up() {
  firebaseInit();
}

