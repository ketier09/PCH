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
  
  unsigned long startTime = millis();
  while (!Firebase.ready() && millis() - startTime < FIREBASE_TIMEOUT_MS) {
    Serial.print(F("."));
    delay(500);
  }

  if (Firebase.ready()) {
    Serial.println(F("\n✅ Conexión con Firebase establecida."));
    delay(1000); // Dar un segundo extra para estabilización interna del token
    // Reintentar stream
    if (!Firebase.RTDB.beginStream(&stream, "/commands/valve1State")) {
      Serial.print(F("⚠ Stream falló al inicio: "));
      Serial.println(stream.errorReason().c_str());
    } else {
       Serial.println(F("✅ Stream de comandos iniciado."));
    }
    return true; // Éxito
  } else {
    Serial.println(F("\n❌ Fallo al conectar con Firebase."));
    return false; // Fracaso
  }
}

void web::set_up() {
  WiFiConfig.begin();
  syncTime();
  if (!firebaseInit()) {
    // Si la inicialización falla, podemos intentar reiniciar o entrar en un modo de error.
    Serial.println(F("Reiniciando en 10 segundos por fallo de Firebase..."));
    delay(10000);
    ESP.restart();
  }
}

bool web::ensureLogin() {
  if (!Firebase.ready()) {
    Serial.println(F("Firebase no está listo. Forzando reconexión..."));
    if (!firebaseInit()) {
        return false;
    }
  }
  return true;
}

void web::enviar(dato data[], int n) {
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("Aviso: WiFi no conectado. Omitiendo envío de datos."));
    return;
  }

  // Verificar conexión antes de intentar enviar
  if (!ensureLogin()) {
    Serial.println(F("-> ❌ No se pudo asegurar la conexión a Firebase. Saltando ciclo de envío."));
    return;
  }

  bool error_en_envio = false;
  for (int i = 0; i < n; ++i) {
    if (Firebase.RTDB.setFloat(&fbdo, String("sensorData/") + data[i].etiquetaFirebase, data[i].valor)) {
      // El envío fue exitoso
    } else {
      String errorReason = fbdo.errorReason();
      Serial.printf("❌ Error enviando %s: %s\n", data[i].etiqueta, errorReason.c_str());
      error_en_envio = true;
      // Si el error es por token, forzar reconexión para el próximo ciclo.
      if (errorReason.indexOf("token") != -1) {
        Serial.println(F("🔥 Token inválido detectado. Se forzará la reconexión en el próximo ciclo."));
        // Forzamos la reconexión, pero no reintentamos inmediatamente para evitar bucles.
        firebaseInit(); 
        break; // Salir del bucle for y esperar al siguiente llamado de enviar()
      }
    }
  }
  
  if (!error_en_envio) {
    Serial.println(F("-> ✅ Datos de sensores enviados a Firebase."));
  } else {
    Serial.println(F("-> ❌ Ocurrieron errores al enviar algunos datos."));
  }
}