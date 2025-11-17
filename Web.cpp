#include "Web.h"
#include <addons/TokenHelper.h>

WiFiConfigManager WiFiConfig;

void web::syncTime() {
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("[Website] Aviso: WiFi no conectado, omitiendo sincronización de hora."));
    return;
  }
  
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); 
  Serial.println("\nSincronizando hora...");
  struct tm timeinfo;
  
  int intentos = 0; 
  const int max_intentos = NTP_MAX_ATTEMPTS;
  
  while (!getLocalTime(&timeinfo) && intentos < max_intentos) {
    Serial.print(F("....."));
    delay(500);
    intentos++;
  }

  if (intentos < max_intentos) {
    Serial.println(F("\n✅ Hora sincronizada."));
  } else {
    Serial.println(F("[Website] ❌ La sincronización de la hora falló o tardó demasiado."));
  }
}

bool web::firebaseInit() {

  Serial.println(F("[Firebase] ⏳ Iniciando configuración..."));

  // Asignar credenciales
  config.api_key = key;
  config.database_url = url;
  
  auth.user.email = email;
  auth.user.password = password;

 // Reintentos y buffers
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Mostrar estado del token en serial
  config.token_status_callback = tokenStatusCallback;

  // --- Iniciar Firebase ---
  Firebase.begin(&config, &auth);
  Serial.println(F("[Firebase] ✓ Firebase iniciado."));

  // Esperar a que el token esté funcional (lectura y escritura)
  Serial.println(F("⏳ Esperando autenticación completa..."));
  while (!Firebase.ready()) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n🔑 Token listo para lectura y escritura.");

  // --- Iniciar Stream de comandos ---
  if (Firebase.RTDB.beginStream(&stream, "/commands")) {
    Serial.println(F("✓ Stream de comandos iniciado."));
  } else {
    Serial.printf("❌ Error iniciando stream: %s\n",
                  stream.errorReason().c_str());
    return false;
  }

  Serial.println(F("[Firebase] 🚀 Configuración completa."));
  return true;
}

void web::set_up() {
  WiFiConfig.begin();
  syncTime();
  if (!firebaseInit()) {
    Serial.println(F("[Website] Reiniciando en 10 segundos por fallo de Firebase..."));
    delay(10000);
    ESP.restart();
  }
}

void web::enviar(dato data[], int n) {
  
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("[Website] ⚠ WiFi no conectado, no se envían datos."));
    return;
  }

  if (!Firebase.ready()) {
    Serial.println(F("[Website] ⏳ Token no listo, reintentando inicialización..."));

    // Intentar inicializar Firebase de nuevo SOLO si no está listo
    if (!firebaseInit()) {
      Serial.println(F("[Website] ❌ Firebase sigue no listo. No se envían datos."));
      return;
    }

    // Tiempo para permitir que el token se estabilice
    delay(1500);
  }

  Serial.println(F("[Website] 🔄 Enviando datos a Firebase..."));

  bool error_general = false;

  for (int i = 0; i < n; ++i) {
    if (!Firebase.RTDB.setFloat(&fbdo,
        String("sensorData/") + data[i].etiquetaFirebase,
        data[i].valor)) {

      Serial.printf("[Website] ❌ Error enviando %s: %s\n",
        data[i].etiqueta,
        fbdo.errorReason().c_str());
      error_general = true;
    }
  }

  if (!error_general) {
    Serial.println(F("-> ✅ Datos enviados a Firebase correctamente."));
  }
}
 