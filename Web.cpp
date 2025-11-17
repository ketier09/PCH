#include "Web.h"
#include "Motor.h"  // Para controlar mo_compuerta

extern WiFiConfigManager WiFiConfig;
extern motor mo_compuerta; // 👈 motor en minúscula (tu clase real)

// Prototipo del callback del token
void tokenStatusCallback(TokenInfo info);

void web::set_up() {
    syncTime();
    firebaseInit();
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

    if (!Firebase.RTDB.beginStream(&stream, "/commands")) {
        Serial.printf("❌ Error iniciando stream: %s\n",
                      stream.errorReason().c_str());
        return false;
    }

    Serial.println(F("✓ Stream escuchando comandos..."));

    lastTokenRefreshTime = millis();
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
                mo_compuerta.siguiente_estado();
            else if (value == "abrir")
                mo_compuerta.abrir();
            else if (value == "cerrar")
                mo_compuerta.cerrar();

            Firebase.RTDB.setString(&fbdo, "/status/compuerta", value);
            Serial.println("[STREAM] ✔ Acción ejecutada");
        }
    }
}

void web::enviar(dato data[], int n) {

    if (!WiFiConfig.isConnected()) {
        Serial.println(F("[Website] ⚠ Sin WiFi."));
        return;
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

void tokenStatusCallback(TokenInfo info) {
    if (info.status == token_status_ready) {
        Serial.println("[TOKEN] 🔐 Token listo");
    } else if (info.status == token_status_error) {
        Serial.printf("[TOKEN] ❌ Error: %s\n",
                      info.error.message.c_str());
    } else {
        Serial.println("[TOKEN] 🔄 Actualizando token...");
    }
}
