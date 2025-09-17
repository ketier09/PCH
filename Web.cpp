#include "Web.h"

void web::syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nSincronizando hora...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nHora sincronizada.");
}

void web::firebaseInit() {
  config.api_key = key;
  config.database_url = url;
  auth.user.email = email;
  auth.user.password = password;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  
  Firebase.begin(&config, &auth);
  
  Serial.println("Conectando a Firebase...");
  unsigned long startTime = millis();
  while (!Firebase.ready() && millis() - startTime < 15000) {
      Serial.print(".");
      delay(500);
  }

  if (Firebase.ready()) {
    Serial.println("\nConexión con Firebase establecida.");
    if (!Firebase.RTDB.beginStream(&stream, "/commands/valve1State")) {
      Serial.println("Error al iniciar el stream: " + stream.errorReason());
    }
  } else {
    Serial.println("\nNo se pudo conectar con Firebase. Verifica API Key, URL y credenciales.");
  }
}

void web::set_up() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  syncTime();
  firebaseInit();
}

void web::enviar(datos data[], int n) {
  if (WiFi.status() == WL_CONNECTED) {
    if (!Firebase.ready()) {
      Serial.println("Reconectando a Firebase...");
      firebaseInit();
    }
  }

  if (!Firebase.ready()) {
    firebaseInit();
  }
    if (Firebase.ready()) {
      for (int i = 0; i < n; i++) {
        char path[64];
        snprintf(path, sizeof(path), "/sensorData/%s", data[i].etiquetaFirebase);
        Firebase.RTDB.setFloat(&fbdo, path, data[i].valor);
      }
      Serial.println("-> Datos de sensores enviados a Firebase.");
    } else {
      Serial.println("-> No se pudieron enviar datos a Firebase. Conexión no lista.");
    }
}
