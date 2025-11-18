// Web.cpp  -- Firestore integration (both latest + history) + commands handling
// Reemplaza completamente tu Web.cpp por este archivo.

#include "Web.h"
#include "Motor.h"
#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>

// Externs (desde tu proyecto)
extern WiFiConfigManager WiFiConfig;
extern motor mo_compuerta;
extern web pagina; // si declaraste extern web pagina en Web.h

// Firebase objects (local)
static FirebaseData fbdo;     // para requests normales (create/patch/get)
static FirebaseData stream;   // para el listener HTTP stream

// Project identifiers (desde Secrets.h / Secrets.cpp)
static String g_projectId = String(projectId);  // debe estar definido en Secrets.cpp
static String g_databaseId = "(default)";       // Firestore default

// Config / Auth (miembros de la clase web también existen en Web.h)
void tokenStatusCallback(TokenInfo info);

// ----------------- Helpers: time / ISO8601 -----------------
String web::getISO8601Time() {
  struct tm t;
  if (!getLocalTime(&t)) {
    time_t now = time(nullptr);
    char b[32];
    strftime(b, sizeof(b), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return String(b);
  }
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
  return String(buf);
}

void web::syncTime() {
  Serial.println(F("[NTP] ⏳ Sincronizando hora..."));
  // Ajusta zona si necesitas (ej UTC-5 pasar -5*3600)
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  for (int i = 0; i < NTP_MAX_ATTEMPTS; ++i) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.println(F("[NTP] ✅ Hora sincronizada."));
      return;
    }
    Serial.print(".");
    delay(300);
  }
  Serial.println(F("\n[NTP] ❌ No se pudo sincronizar la hora (usar fallback)."));
}

// ----------------- Token status callback -----------------
void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_ready) {
    Serial.println(F("[TOKEN] 🔐 Token listo"));
  } else if (info.status == token_status_error) {
    Serial.printf("[TOKEN] ❌ Token Error: %s\n", info.error.message.c_str());
  } else if (info.status == token_status_on_request) {
    Serial.println(F("[TOKEN] 📨 Solicitando token..."));
  } else {
    Serial.println(F("[TOKEN] 🔄 Token actualizándose..."));
  }
  // guardar tiempo de refresh (pagina es extern)
  pagina.lastTokenRefreshTime = millis();
}

// ----------------- Firebase / Firestore init -----------------
bool web::firebaseInit() {
  Serial.println(F("[Firebase] ⏳ Iniciando configuración Firestore..."));

  // 1) asegurar WiFi
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("[WiFi] No conectado -> intentando conectar..."));
    WiFiConfig.connect();
    unsigned long t0 = millis();
    while (!WiFiConfig.isConnected() && millis() - t0 < 10000) {
      Serial.print(".");
      delay(250);
    }
    Serial.println();
    if (!WiFiConfig.isConnected()) {
      Serial.println(F("[WiFi] ❌ No se pudo conectar al WiFi."));
      return false;
    }
  }
  Serial.println(F("[WiFi] ✅ Conectado."));

  // 2) NTP
  syncTime();

  // 3) Firebase config / auth
  config.api_key = key;
  config.database_url = ""; // no usado para Firestore, mantener vacío por compatibilidad
  auth.user.email = email;
  auth.user.password = password;
  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(8192);

  Serial.println(F("[Firebase] Llamando a Firebase.begin()..."));
  Firebase.begin(&config, &auth);

  // Esperar token listo (con timeout)
  Serial.print(F("[Firebase] Esperando token..."));
  unsigned long tstart = millis();
  while (!Firebase.ready() && millis() - tstart < 15000) {
    Serial.print(".");
    delay(200);
  }
  Serial.println();
  if (!Firebase.ready()) {
    Serial.println(F("[Firebase] ❌ Firebase no listo (token)."));
    return false;
  }
  Serial.println(F("[Firebase] ✅ Firebase listo (Firestore)."));

  // Iniciar listener en commands/compuerta (documento fijo) si se prefiere auto-start
  // Dejamos disponible la función pública listenCommands() para controlarlo desde setup()
  pagina.lastTokenRefreshTime = millis();
  return true;
}

// ----------------- Construir payload Firestore -----------------
// Firestore expects fields/<name>/<type>Value
FirebaseJson web::buildFirestorePayload(dato dataArr[], int n) {
  FirebaseJson json;
  for (int i = 0; i < n; ++i) {
    // asegurar etiqueta sin espacios y en formato de campo válido
    String field = String("fields/") + String(dataArr[i].etiquetaFirebase) + "/doubleValue";
    json.set(field.c_str(), String(dataArr[i].valor));
  }
  // timestamp
  json.set("fields/timestamp/timestampValue", getISO8601Time());
  return json;
}

// ----------------- Enviar datos (latest + historico) -----------------
void web::enviar(dato dataArr[], int n) 
{
  // protección WiFi
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("[Website] ⚠ WiFi no conectado. Omite envío."));
    return;
  }

  // protección Firebase ready
  if (!Firebase.ready()) {
    Serial.println(F("[Website] ⚠ Firebase no listo. Reintentando init..."));
    if (!firebaseInit()) {
      Serial.println(F("[Website] ❌ No se pudo inicializar Firebase -> omitir envío."));
      return;
    }
  }

  // esperar estabilización del token (evita token revoked al primer request)
  unsigned long since = millis() - lastTokenRefreshTime;
  if (since < TOKEN_STABILIZE_MS) {
    unsigned long wait = TOKEN_STABILIZE_MS - since;
    Serial.printf("[Website] ⏳ Esperando token %lums...\n", wait);
    delay(wait);
  }

  // construir payload
  FirebaseJson payload = buildFirestorePayload(dataArr, n);

  // 1) patchDocument -> sensorData/actual  (estado en tiempo real)
  String docLatest = "sensores/actual";
  bool okLatest = false;
  for (int attempt = 1; attempt <= FIRESTORE_RETRY_MAX && !okLatest; ++attempt) {
    Serial.printf("[Firestore] patchDocument %s (intento %d)\n", docLatest.c_str(), attempt);
    if (Firebase.Firestore.patchDocument(&fbdo, g_projectId.c_str(), g_databaseId.c_str(), docLatest.c_str(), &payload)) {
      Serial.println(F("[Firestore] ✅ patchDocument (actual) OK"));
      okLatest = true;
    } else {
      Serial.printf("[Firestore] ❌ patchDocument error: %s\n", fbdo.errorReason().c_str());
      delay(300 * attempt);
    }
  }

  // 2) createDocument -> sensores_historico (colección con auto-id)
  bool okHistory = false;
  for (int attempt = 1; attempt <= FIRESTORE_RETRY_MAX && !okHistory; ++attempt) {
    Serial.printf("[Firestore] createDocument sensores_historico (intento %d)\n", attempt);
    if (Firebase.Firestore.createDocument(&fbdo, g_projectId.c_str(), g_databaseId.c_str(), "sensores_historico", &payload)) {
      Serial.println(F("[Firestore] ✅ createDocument (historico) OK"));
      okHistory = true;
    } else {
      Serial.printf("[Firestore] ❌ createDocument error: %s\n", fbdo.errorReason().c_str());
      delay(300 * attempt);
    }
  }

  if (okLatest || okHistory) {
    Serial.println(F("-> ✅ Envío Firestore (actual + historico) finalizado."));
  } else {
    Serial.println(F("[Firestore] ❌ Ambos envíos fallaron."));
  }
}

// ----------------- Listener: iniciar escucha en documento fijo -----------------
// collection/document (ej. "commands/compuerta")
bool web::listenCommands(const String &collection, const String &docId) {
  String docPath = collection + "/" + docId;
  Serial.printf("[Firestore] Iniciando listener en %s\n", docPath.c_str());
  if (Firebase.Firestore.listen(&stream, g_projectId.c_str(), g_databaseId.c_str(), docPath.c_str())) {
    Serial.println(F("[Firestore] ✅ Listener iniciado."));
    return true;
  } else {
    Serial.printf("[Firestore] ❌ Error al iniciar listener: %s\n", stream.errorReason().c_str());
    return false;
  }
}

// ----------------- Procesar eventos del listener -----------------
// Llamar periódicamente (p. ej. pagina.handleStream() en TaskLenta)
void web::processFirestoreEvents() {
  // verificar que la conexión del stream esté activa
  if (!stream.httpConnected()) return;
  // si hay datos disponibles
  if (stream.available()) {
    String payload = stream.payload(); // payload completo del evento
    Serial.println(F("[Firestore] Evento payload:"));
    Serial.println(payload);

    // Extracción sencilla del campo "accion" desde estructura Firestore:
    // El evento contiene la representación JSON de documento; buscamos fields/accion/stringValue
    String accion = "";
    int idx = payload.indexOf("fields");
    if (idx >= 0) {
      // buscar "accion" y su stringValue después
      int posAcc = payload.indexOf("accion", idx);
      if (posAcc >= 0) {
        int posVal = payload.indexOf("stringValue", posAcc);
        if (posVal >= 0) {
          // buscar primer ":", luego la cadena entre comillas
          int q1 = payload.indexOf("\"", posVal);
          int q2 = payload.indexOf("\"", q1 + 1);
          if (q1 >= 0 && q2 > q1) {
            accion = payload.substring(q1 + 1, q2);
          } else {
            // fallback: buscar pattern '\"stringValue\":\"...\"'
            int p = payload.indexOf("stringValue", posAcc);
            int colon = payload.indexOf(":", p);
            if (colon >= 0) {
              int start = payload.indexOf("\"", colon);
              int end = payload.indexOf("\"", start + 1);
              if (start >= 0 && end > start) accion = payload.substring(start + 1, end);
            }
          }
        }
      }
    }

    if (accion.length() > 0) {
      Serial.printf("[Firestore] Accion detectada: %s\n", accion.c_str());

      // Ejecutar acción en motor
      ordenCompuerta(accion);

      // 1) Guardar confirmación en status/compuerta
      FirebaseJson confirm;
      confirm.set("fields/lastCommand/stringValue", accion);
      confirm.set("fields/lastAt/timestampValue", getISO8601Time());
      String statusDoc = "compuerta/estadoActual";
      if (Firebase.Firestore.patchDocument(&fbdo, g_projectId.c_str(), g_databaseId.c_str(), statusDoc.c_str(), &confirm)) {
        Serial.println(F("[Firestore] ✔ Estado compuerta actualizado (status)."));
      } else {
        Serial.printf("[Firestore] ❌ Error actualizando status: %s\n", fbdo.errorReason().c_str());
      }

      // 2) Guardar historial de comandos en commands_historico
      FirebaseJson hist;
      hist.set("fields/accion/stringValue", accion);
      hist.set("fields/timestamp/timestampValue", getISO8601Time());
      if (Firebase.Firestore.createDocument(&fbdo, g_projectId.c_str(), g_databaseId.c_str(), "commands_historico", &hist)) {
        Serial.println(F("[Firestore] ✔ Command historic stored."));
      } else {
        Serial.printf("[Firestore] ❌ Error creando historic command: %s\n", fbdo.errorReason().c_str());
      }
    } else {
      Serial.println(F("[Firestore] ⚠ No se detectó campo 'accion' en payload."));
    }
  }
}

// Wrapper público para ser llamado cada ciclo (TaskLenta)
void web::handleStream() {
  if (!stream.httpConnected()) return;
  processFirestoreEvents();
}

// ----------------- ordenCompuerta: wrapper que controla el motor -----------------
void web::ordenCompuerta(const String &accion) {
  Serial.printf("[Compuerta] Acción recibida -> %s\n", accion.c_str());
  uint8_t index = 0;

  if (accion.equalsIgnoreCase("toggle")) {
    index = mo_compuerta.siguiente_estado(); // devuelve index real (0..3)
    Serial.printf("[Compuerta] toggle -> index %u\n", index);
  } else if (accion.equalsIgnoreCase("abrir")) {
    index = 3;
    mo_compuerta.showState(index);
    Serial.println(F("[Compuerta] abrir -> estado 3"));
  } else if (accion.equalsIgnoreCase("cerrar")) {
    index = 0;
    mo_compuerta.showState(index);
    Serial.println(F("[Compuerta] cerrar -> estado 0"));
  } else {
    // Si viene un número (0..3) en texto, intentar parsearlo
    bool isNumber = true;
    for (size_t i = 0; i < accion.length(); ++i) if (!isDigit(accion.charAt(i))) { isNumber = false; break; }
    if (isNumber) {
      int v = accion.toInt();
      if (v >= 0 && v <= 3) {
        index = (uint8_t)v;
        mo_compuerta.showState(index);
        Serial.printf("[Compuerta] set -> estado %d\n", v);
      } else {
        Serial.println(F("[Compuerta] valor numerico fuera de rango (0-3)."));
        return;
      }
    } else {
      Serial.println(F("[Compuerta] comando desconocido."));
      return;
    }
  }

  // Actualizar documento compuerta/estadoActual con posicion y timestamp
  FirebaseJson doc;
  doc.set("fields/posicion/integerValue", (int)index);
  doc.set("fields/timestamp/timestampValue", getISO8601Time());
  String statusDoc = "compuerta/estadoActual";
  if (Firebase.Firestore.patchDocument(&fbdo, g_projectId.c_str(), g_databaseId.c_str(), statusDoc.c_str(), &doc)) {
    Serial.println(F("[Compuerta] Estado guardado en Firestore."));
  } else {
    Serial.printf("[Compuerta] ❌ Error guardando estado: %s\n", fbdo.errorReason().c_str());
  }
}

// ----------------- set_up() público -----------------
void web::set_up() {
  // Iniciar WiFi + Firebase (solo una vez)
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("[Web.set_up] WiFi no conectado. Intentando conectar..."));
    WiFiConfig.connect();
    unsigned long t0 = millis();
    while (!WiFiConfig.isConnected() && millis() - t0 < 10000) {
      Serial.print(".");
      delay(250);
    }
    Serial.println();
  }

  if (!firebaseInit()) {
    Serial.println(F("[Web.set_up] ⚠ FirebaseInit falló. De todos modos continúo."));
  } else {
    // Si deseas iniciar listener de comandos automático:
    // iniciar listener en commands/compuerta
    if (!listenCommands("commands", "compuerta")) {
      Serial.println(F("[Web.set_up] ⚠ No se pudo iniciar listener commands/compuerta."));
    }
  }
}
