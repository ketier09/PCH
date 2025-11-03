#include "Web.h"

// Mantener como global por la estructura actual del proyecto
WiFiConfigManager WiFiConfig;

void web::syncTime() {
  // Solo inicializar si hay conectividad WiFi (asumiendo que begin() ya fue llamado)
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("Aviso: WiFi no conectado, omitiendo sincronización de hora."));
    return;
  }
  
  // Usar una zona horaria más específica y un servidor NTP que responda rápido
  configTime(0, 0, "pool.ntp.org"); 
  Serial.println(F("\nSincronizando hora..."));
  struct tm timeinfo;
  
  int intentos = 0; 
  // Uso de la constante definida en el .h
  const int max_intentos = NTP_MAX_ATTEMPTS; 
  
  // Espera hasta que llegue la hora correcta o se agoten los intentos.
  while (!getLocalTime(&timeinfo) && intentos < max_intentos) {
    Serial.print(F("."));
    delay(500);
    intentos++;
  }

  if (intentos < max_intentos) {
    // Mejorar la salida imprimiendo la hora
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo);
    Serial.printf("\n✅ Hora sincronizada: %s\n", timeStr);
  } else {
    Serial.println(F("\n❌ La sincronización de la hora falló o tardó demasiado. El programa continúa."));
  }
}

// Se cambia a 'bool' para indicar si la conexión fue exitosa.
bool web::firebaseInit() {
  if (!WiFiConfig.isConnected()) {
    Serial.println(F("Aviso: WiFi no conectado, omitiendo inicialización de Firebase."));
    return false;
  }
  
  // --- Configuración y credenciales (mejorado) ---
  // Uso de las constantes definidas en Secrets.h
  config.api_key      = key;
  config.database_url = url;
  auth.user.email     = email;
  auth.user.password  = password;

  // Ajustes de conexión. Se mueve reconnectWiFi antes de Firebase.begin
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  
  // Usar la función de error/debug para la configuración
  config.timeout.serverResponse = FIREBASE_TIMEOUT_MS;
  // config.debug.setSerialEnabled(true); // Descomentar para debug

  Serial.println(F("Conectando a Firebase..."));
  Firebase.begin(&config, &auth);

  unsigned long startTime = millis();
  // Uso de la constante definida en el .h
  while (!Firebase.ready() && millis() - startTime < FIREBASE_TIMEOUT_MS) {
    Serial.print(F("."));
    delay(500);
  }
  
  if (Firebase.ready()) {
    Serial.println(F("\n✅ Conexión con Firebase establecida."));
    // Intentar iniciar el stream
    if (!Firebase.RTDB.beginStream(&stream, "/commands/valve1State")) {
      Serial.print(F("⚠️ Stream falló al inicio: "));
      Serial.println(stream.errorReason().c_str());
      // No retorna false aquí, ya que Firebase está "listo", solo el stream falló.
    } else {
      Serial.println(F("✅ Stream de comandos iniciado."));
    }
    return true;
  } else {
    Serial.printf("\n❌ Conexión a Firebase falló: %s\n", Firebase.errorReason().c_str());
    return false;
  }
}

void web::set_up() {
  Serial.println(F("--- Iniciando Configuración de Red y Servicios ---"));
  
  WiFiConfig.begin(); // Primero, asegura la conexión WiFi (puede entrar al portal AP)
  
  // Si hay conexión WiFi, procede con el resto de la inicialización
  if (WiFiConfig.isConnected()) {
    syncTime();      
    firebaseInit();
  } else {
    Serial.println(F("⚠️ WiFi no conectado al finalizar begin(), servicios NTP/Firebase omitidos."));
  }
}

// Nueva función de apoyo para manejar la reconexión de Firebase
void web::handleFirebaseConnection() {
  if (WiFiConfig.isConnected()) {
    if (!Firebase.ready()) {
      Serial.println(F("Reconectando a Firebase..."));
      // Si la inicialización falla, se detiene
      if (!firebaseInit()) { 
        Serial.println(F("⚠️ Re-inicialización de Firebase fallida. Omitiendo envío."));
      }
    }
  } else {
    Serial.println(F("❌ Sin conexión WiFi. Omitiendo reconexión de Firebase."));
  }
}

void web::enviar(dato data[], int n) {
  // Llama a la función unificada de manejo de conexión
  handleFirebaseConnection();

  if (Firebase.ready()) {
    // 💡 Optimización: Usar setValues en lugar de setFloat repetidamente
    // para reducir el número de peticiones HTTP, si la librería lo soporta.
    // El método actual es enviar una por una, lo que es menos eficiente.
    // Lo mantendremos como setFloat por simplicidad de la estructura 'dato'.

    for (int i = 0; i < n; i++) {
      char path[64];
      // Construye la ruta donde se guardará este dato
      snprintf(path, sizeof(path), "/sensorData/%s", data[i].etiquetaFirebase);
      
      // Se utiliza setFloat. Se podría usar setDouble para mayor precisión, o setInt si es el caso.
      if (!Firebase.RTDB.setFloat(&fbdo, path, data[i].valor)) {
        // Mejor manejo de errores por cada envío
        Serial.printf("❌ Error enviando %s: %s\n", data[i].etiquetaFirebase, fbdo.errorReason().c_str());
      }
    }
    Serial.println(F("-> ✅ Datos de sensores enviados a Firebase."));
  } else {
    Serial.println(F("-> ❌ No se pudieron enviar datos a Firebase. Conexión no lista."));
  }
}






