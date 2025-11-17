#include "Web.h"

extern WiFiConfigManager WiFiConfig;

void web::set_up() {
  syncTime();

  if (!firebaseInit()) {
    Serial.println(F("[Website] ⚠ Firebase no listo aún, pero continuo ejecución."));
  }
}

void web::syncTime() {
  Serial.println(F("[NTP] ⏳ Sincronizando hora..."));
  configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov"); // UTC-5 Colombia

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

  Serial.println(F("⏳ Esperando autenticación completa..."));
  while (!Firebase.ready()) {
    delay(200);
    Serial.print(".");
  }

  Serial.println("\n🔑 Token listo para lectura y escritura.");

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

void web::enviar(dato data[], int n) {

  if (!WiFiConfig.isConnected()) {
    Serial.println(F("[Website] ⚠ WiFi no conectado, se omiten datos."));
    return;
  }

  unsigned long tiempoDesdeRefresh = millis() - lastTokenRefreshTime;
  if (tiempoDesdeRefresh < 3000) {
    Serial.println(F("[Website] ⏳ Esperando estabilización del token..."));
    delay(3000);
  }

  if (!Firebase.ready()) {
    Serial.println(F("[Website] ⚠ Firebase no listo, reintentando inicialización..."));
    if (!firebaseInit()) {
      Serial.println(F("[Website] ❌ Firebase sigue no listo → No se envían datos."));
      return;
    }
  }

  Serial.println(F("[Website] 🔄 Enviando datos a Firebase..."));

  bool error_general = false;

  for (int i = 0; i < n; ++i) {
    String path = "/sensorData/" + String(data[i].etiquetaFirebase);
    Serial.printf("[Website] 📌 Enviando a: %s → %.3f\n", path.c_str(), data[i].valor);

    if (!Firebase.RTDB.setFloat(&fbdo, path.c_str(), data[i].valor)) {
      Serial.printf("[Website] ❌ Error enviando %s: %s\n",
                    data[i].etiqueta,
                    fbdo.errorReason().c_str());
      error_general = true;
    }
  }

  if (!error_general) {
    Serial.println(F("-> ✅ Datos enviados correctamente a Firebase."));
  }
}
