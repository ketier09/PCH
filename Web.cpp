#include "Web.h"

WiFiConfigManager WiFiConfig;

void web::set_up() {
    WiFiConfig.begin();
    firebaseInit();
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
        WiFiConfig.begin();
    }

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

void web::handleStream() {
    if (!Firebase.RTDB.readStream(&stream)) return;

    if (stream.streamAvailable()) {
        String path = stream.dataPath();
        String value = stream.stringData();

        Serial.printf("[STREAM] Cambio → %s = %s\n",
                      path.c_str(), value.c_str());

        if (path.endsWith("/compuerta")) {

            if (value == "toggle")
                ordenCompuerta = (ordenCompuerta + 1) % 4;
            else if (value == "abrir"){}
            else if (value == "cerrar"){}

            Firebase.RTDB.setString(&fbdo, "/status/compuerta", value);
            Serial.println("[STREAM] ✔ Acción ejecutada");
        }
    }
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

