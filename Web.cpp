#include "Web.h"
#include "Motor.h"
#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>

// Externs de tu proyecto
extern WiFiConfigManager WiFiConfig;
extern motor mo_compuerta;
extern web pagina;

// Identificadores Firestore
static String g_projectId = String(projectId);
static String g_databaseId = "(default)";

// Constantes
#define TOKEN_STABILIZE_MS 2000
#define FIRESTORE_RETRY_MAX 5
#define NTP_MAX_ATTEMPTS 20

// Prototipo
void tokenStatusCallback(TokenInfo info);

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

    if (!WiFiConfig.isConnected()) {
        Serial.println(F("[WiFi] ❌ No conectado"));
        return false;
    }

    syncTime();

    config.api_key = key;
    auth.user.email = email;
    auth.user.password = password;
    config.token_status_callback = tokenStatusCallback;

    // FIX ✔ Firebase.begin ya no se chequea como booleano
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Serial.println(F("[Firebase] ✔ begin() llamado"));

    unsigned long start = millis();
    while (!Firebase.ready() && millis() - start < 15000) {
        delay(200);
    }

    if (!Firebase.ready()) {
        Serial.println(F("[Firebase] ❌ Token no listo"));
        return false;
    }

    Serial.println(F("[Firebase] ✔ Token listo"));
    lastTokenRefreshTime = millis();
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
        delay(500);
    }

    FirebaseJson payload = buildFirestorePayload(dataArr, n);
    String payloadStr;
    payload.toString(payloadStr, false);

    // Último estado
    String pathLatest = "sensores_latest/estado";
    Firebase.Firestore.patchDocument(&fbdo,
                                     g_projectId.c_str(),
                                     g_databaseId.c_str(),
                                     pathLatest.c_str(),
                                     payloadStr.c_str(),
                                     "");

    // Histórico con ID automático — FIX ✔ CreateDocument sin ambigüedad
    String collection = "sensores_historico";
    String documentId = "";

    Firebase.Firestore.createDocument(&fbdo,
                                      g_projectId.c_str(),
                                      g_databaseId.c_str(),
                                      collection.c_str(),
                                      documentId.c_str(),
                                      payloadStr.c_str(),
                                      "");
}

bool web::listenCommands(const String &collection, const String &docId) {
    String docPath = collection + "/" + docId;

    // Listener correcto para esta versión — FIX ✔
    if (!Firebase.Firestore.listenDocument(&stream,
                                           g_projectId.c_str(),
                                           g_databaseId.c_str(),
                                           docPath.c_str())) {
        Serial.println(F("[Commands] ❌ Listener fail"));
        return false;
    }

    Serial.println(F("[Commands] ✔ escuchando Firestore"));
    return true;
}

void web::processFirestoreEvents() {
    if (!stream.httpConnected()) return;
    if (!stream.dataAvailable()) return;

    FirebaseJson json = stream.to<FirebaseJson>();

    // FIX ✔ Usar FirebaseJsonData
    FirebaseJsonData actionData;
    if (!json.get(actionData, "fields/accion/stringValue")) return;

    String accion = actionData.stringValue;
    Serial.println("[Comando] " + accion);

    ejecutarComandoCompuerta(accion);

    FirebaseJson status;
    status.set("fields/accion/stringValue", accion);
    status.set("fields/timestamp/timestampValue", getISO8601Time());

    Firebase.Firestore.patchDocument(&fbdo,
                                     g_projectId.c_str(),
                                     g_databaseId.c_str(),
                                     "comandos_estado/actual",
                                     status.raw(),
                                     "");
}

void web::handleStream() {
    processFirestoreEvents();
}

void web::ejecutarComandoCompuerta(const String &accion) {
    if (accion.equalsIgnoreCase("SIGUIENTE")) {
        estadoCompuerta = mo_compuerta.siguiente_estado();
    } else if (accion.equalsIgnoreCase("ABRIR")) {
        estadoCompuerta = 3;
        mo_compuerta.showState(3);
    } else if (accion.equalsIgnoreCase("CERRAR")) {
        estadoCompuerta = 0;
        mo_compuerta.showState(0);
    } else {
        return;
    }

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

void web::set_up() {
    if (!firebaseInit()) {
        Serial.println(F("[Web] FirebaseInit falló"));
        return;
    }

    listenCommands("commands", "compuerta");
}
