#include "Web.h"
#include <addons/TokenHelper.h>

WiFiConfigManager WiFiConfig;

void web::syncTime() {
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("Aviso: WiFi no conectado, omitiendo sincronización de hora."));
    return;
  }
  
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); 
  Serial.println("\nSincronizando hora...");
  struct tm timeinfo;
  
  int intentos = 0; 
  const int max_intentos = NTP_MAX_ATTEMPTS; // Uso de constante de 15s
  
  while (!getLocalTime(&timeinfo) && intentos < max_intentos) {
    Serial.print(F("."));
    delay(500);
    intentos++;
  }

  if (intentos < max_intentos) {
    Serial.println(F("\n✅ Hora sincronizada."));
  } else {
    Serial.println(F("\n❌ La sincronización de la hora falló o tardó demasiado."));
  }
}

bool web::firebaseInit() {
  if (!WiFiConfig.isConnected()) {
      Serial.println(F("❌ WiFi no conectado. Fallo al iniciar Firebase."));
      return false;
  }
  
  config.api_key      = key;
  config.database_url = url;
  auth.user.email     = email;
  auth.user.password  = password;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.timeout.serverResponse = FIREBASE_TIMEOUT_MS;

  Serial.println(F("Conectando a Firebase..."));
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  
  unsigned long startTime = millis();
  while (!Firebase.ready() && millis() - startTime < FIREBASE_TIMEOUT_MS) {
    Serial.print(F("."));
    delay(500);
  }

  if (Firebase.ready()) {
    Serial.println(F("\n✅ Conexión con Firebase establecida."));
    delay(1000); 
    if (!Firebase.RTDB.beginStream(&stream, "/commands/valve1State")) {
      Serial.print(F("⚠️ Stream falló al inicio: "));
      Serial.println(stream.errorReason().c_str());
    } else {
       Serial.println(F("✅ Stream de comandos iniciado."));
    }
    return true;
  } else {
    Serial.println(F("\n❌ Fallo al conectar con Firebase."));
    return false;
  }
}

void web::set_up() {
  WiFiConfig.begin();
  syncTime();
  if (!firebaseInit()) {
    Serial.println(F("Reiniciando en 10 segundos por fallo de Firebase..."));
    delay(10000);
    ESP.restart();
  }
}

void web::enviar(dato data[], int n) {
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("Aviso: WiFi no conectado. Omitiendo envío de datos."));
    return;
  }

  // Si Firebase no está listo, intenta reconectar. Si falla, sal del método.
  if (!Firebase.ready()) {
    Serial.println(F("! Firebase no está listo. Intentando reconectar..."));
    if (!firebaseInit()) {
      Serial.println(F("! La reconexión falló. Se reintentará en el próximo ciclo."));
      return; // Salir y esperar al próximo ciclo de envío.
    }
  }

  bool error_general = false;
  for (int i = 0; i < n; ++i) {
    // Re-verificar antes de cada envío por si el token expiró entre envíos.
    if (Firebase.ready()) {
      if (!Firebase.RTDB.setFloat(&fbdo, String("sensorData/") + data[i].etiquetaFirebase, data[i].valor)) {
        String errorReason = fbdo.errorReason();
        Serial.printf("❌ Error enviando %s: %s\n", data[i].etiqueta, errorReason.c_str());
        error_general = true;
      }
    } else {
        Serial.printf("❌ Omitiendo envío de %s: Firebase no está listo.\n", data[i].etiqueta);
        error_general = true;
        // Si no está listo, no tiene sentido seguir en el bucle.
        break;
    }
  }
  
  if (!error_general) {
    Serial.println(F("-> ✅ Datos de sensores enviados a Firebase."));
  } else {
    Serial.println(F("-> ❌ Ocurrieron errores al enviar algunos datos."));
  }
}
