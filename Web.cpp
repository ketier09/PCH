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

// 💡 OPTIMIZACIÓN: Cambiar a bool para indicar éxito/fracaso
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
  config.timeout.serverResponse = FIREBASE_TIMEOUT_MS; // Uso de constante

  Serial.println(F("Conectando a Firebase..."));
  config.token_status_callback = tokenStatusCallback; // logs de estado del token
  Firebase.begin(&config, &auth);
  ensureLogin();

  unsigned long startTime = millis();
  while (!Firebase.ready() && millis() - startTime < FIREBASE_TIMEOUT_MS) {
    Serial.print(F("."));
    delay(500);
  }

  if (Firebase.ready()) {
    Serial.println(F("\n✅ Conexión con Firebase establecida."));
    // Reintentar stream
    if (!Firebase.RTDB.beginStream(&stream, "/commands/valve1State")) {
      Serial.print(F("⚠️ Stream falló al inicio: "));
      Serial.println(stream.errorReason().c_str());
    } else {
       Serial.println(F("✅ Stream de comandos iniciado."));
    }
    return true; // Éxito
  } else {
    // 💡 OPTIMIZACIÓN: Imprimir el error de la conexión, no del stream
    Serial.printf("\n❌ Conexión a Firebase falló: %s\n", fbdo.errorReason().c_str());
    return false; // Fallo
  }
}

void web::set_up() {
  WiFiConfig.begin();
  // 💡 OPTIMIZACIÓN: Ejecutar servicios solo si hay conexión WiFi
  if (WiFiConfig.isConnected()) { 
    syncTime();
    firebaseInit();
  } else {
    Serial.println(F("⚠️ WiFi no conectado, servicios NTP/Firebase omitidos en set_up."));
  }
}

void web::enviar(dato data[], int n) {
  // 💡 OPTIMIZACIÓN: Lógica de reconexión centralizada y limpia
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("❌ Sin conexión WiFi. Omitiendo envío a Firebase."));
    return;
  }

  // Intenta re-inicializar si Firebase no está listo
  if (!Firebase.ready()) {
    Serial.println(F("Reconectando a Firebase..."));
    if (!firebaseInit()) { // Intenta re-inicializar. Si falla, sale.
      Serial.println(F("⚠️ Re-inicialización de Firebase fallida. Omitiendo envío."));
      return;
    }
  }
  
  if (Firebase.ready()) {
    for (int i = 0; i < n; i++) {
      char path[64];
      snprintf(path, sizeof(path), "/sensorData/%s", data[i].etiquetaFirebase);
      
      // 💡 OPTIMIZACIÓN: Verificar el resultado de la operación de envío
      if (!Firebase.RTDB.setFloat(&fbdo, path, data[i].valor)) {
        String er = fbdo.errorReason();
        Serial.printf("❌ Error enviando %s: %s\n", data[i].etiquetaFirebase, er.c_str());

        // Si es problema de token, reautentica una vez y reintenta
        if (er.indexOf("token is not ready") >= 0 || fbdo.httpCode() == 401 || fbdo.httpCode() == 403) {
          if (ensureLogin()) {
            if (!Firebase.RTDB.setFloat(&fbdo, path, data[i].valor)) {
              Serial.printf("❌ Reintento falló %s: %s\n", data[i].etiquetaFirebase, fbdo.errorReason().c_str());
            }
          }
        }
      }

    }
    Serial.println(F("-> ✅ Datos de sensores enviados a Firebase."));
  }

}

bool web::ensureLogin() {

  // Si ya está listo, nada que hacer
  if (Firebase.ready()) return true;

  // Mantén el mismo chequeo de WiFi que usas en el resto de la clase
  if (!WiFiConfig.isConnected()) return false;

  // Cargar credenciales desde Secrets.h (ya incluidas por Web.h)
  // y asegurar que config/auth tengan valores
  if (config.api_key.length() == 0) {
    config.api_key      = key;     // <-- antes: FIREBASE_API_KEY
    config.database_url = url;     // <-- antes: FIREBASE_DATABASE_URL
    auth.user.email     = email;   // <-- antes: USER_EMAIL
    auth.user.password  = password;// <-- antes: USER_PASSWORD

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);
    config.timeout.serverResponse = FIREBASE_TIMEOUT_MS;
    config.token_status_callback = tokenStatusCallback;
  }

  // Intentar (re)inicializar
  Firebase.begin(&config, &auth);

  unsigned long t0 = millis();
  while (!Firebase.ready() && millis() - t0 < FIREBASE_TIMEOUT_MS) {
    delay(100);
  }

  if (!Firebase.ready()) {
    Serial.printf("Auth falló: %s\n",
                  config.signer.tokens.error.message.c_str());
    return false;
  }

  return true;
}



