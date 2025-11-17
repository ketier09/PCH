#include "Web.h"
#include "Motor.h"  // 👈 Asegura acceso al motor

extern WiFiConfigManager WiFiConfig;
extern Motor mo_compuerta;  // 👈 Motor global

// --------------------------------------------------------
// PROTOTIPO DE CALLBACK DEL TOKEN
// --------------------------------------------------------
void tokenStatusCallback(TokenInfo info);

void web::set_up() {
    syncTime();

    if (!firebaseInit()) {
        Serial.println(F("[Website] ⚠ Firebase no listo aún, continuo ejecución."));
    }
}

void web::syncTime() {
    Serial.println(F("[NTP] ⏳ Sincronizando hora..."));
    configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // UTC-5

    for (int i = 0; i < NTP_MAX_ATTEMPTS; i++) {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            Serial.println(F("✅ Hora sincronizada."));
            return;
        }
        Serial.print(".");
        delay(200);
    }

    Serial.println(F("\n[NTP] ❌ No se pudo sincronizar la hora."));
}

// --------------------------------------------------------
// CONFIGURACIÓN DE FIREBASE
// --------------------------------------------------------
bool web::firebaseInit() {
    Serial.println(F("[Firebase] ⏳ Iniciando configuración..."));

    config.api_key = key;
    config.database_url = url;

    auth.user.email = email;
    auth.user.password = password;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Serial.println(F("[Firebase] ✓ Firebase iniciado."));

    Serial.print(F("⏳ Esperando autenticación..."));
    while (!Firebase.ready()) {
        delay(200);
        Serial.print(".");
    }
    Serial.println("\n🔑 Token listo.");

    if (Firebase.RTDB.beginStream(&stream, "/commands")) {
        Serial.println(F("✓ Stream de comandos iniciado."));
    } else {
        Serial.printf("❌ Error iniciando stream: %s\n", stream.errorReason().c_str());
        return false;
    }

    lastTokenRefreshTime = millis();
    Serial.println(F("[Firebase] 🚀 Configuración completa."));
    return true;
}

// --------------------------------------------------------
// ESCUCHAR CAMBIOS DESDE FIREBASE (CONTROL SERVO)
// --------------------------------------------------------
void web::handleStream() {
    if (!stream.available()) return;

    FirebaseStream msg = stream.readStream();
    String path = msg.dataPath();
    String value = msg.stringData();

    Serial.printf("[STREAM] Cambio → %s = %s\n",
                  path.c_str(), value.c_str());

    // === CONTROL DE COMPUERTA ===
    if (path.endsWith("/compuerta")) {

        if (value == "toggle") mo_compuerta.siguiente_estado();
        else if (value == "abrir") mo_compuerta.abrir();
        else if (value == "cerrar") mo_compuerta.cerrar();

        Firebase.RTDB.setString(&fbdo, "/status/compuerta", value);
        Serial.println("[STREAM] ✔ Acción ejecutada");
    }
}

// --------------------------------------------------------
// ENVÍO DE SENSORES A FIREBASE
// --------------------------------------------------------
void web::enviar(dato data[], int n) {

    if (!WiFiConfig.isConnected()) {
        Serial.println(F("[Website] ⚠ Sin WiFi."));
        return;
    }

    unsigned long diff = millis() - lastTokenRefreshTime;
    if (diff < 2500) {
        Serial.println(F("[Website] ⏳ Esperando estabilizar token..."));
        delay(2500);
    }

    if (!Firebase.ready()) {
        Serial.println(F("[Website] ⚠ Firebase no listo, reintentando..."));
        if (!firebaseInit()) return;
    }

    Serial.println(F("[Website] 🔄 Enviando datos..."));

    for (int i = 0; i < n; i++) {
        String path = "/sensorData/" + String(data[i].etiquetaFirebase);

        if (!Firebase.RTDB.setFloat(&fbdo, path, data[i].valor)) {
            Serial.printf("[❌] %s: %s\n",
                          data[i].etiqueta,
                          fbdo.errorReason().c_str());
        }
    }

    Serial.println(F("-> ✅ Datos enviados correctamente."));
}

// --------------------------------------------------------
// CALLBACK: ESTADO DEL TOKEN
// --------------------------------------------------------
void tokenStatusCallback(TokenInfo info) {
    switch (info.status) {
        case token_status_ready:
            Serial.println("[TOKEN] 🔐 Token listo");
            break;
        case token_status_error:
            Serial.printf("[TOKEN] ❌ Error: %s\n",
                          info.error.message.c_str());
            break;
        default:
            Serial.println("[TOKEN] 🔄 Actualizando token...");
            break;
    }
}
