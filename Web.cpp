#include "Web.h"

void web::syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Servidores de hora
  Serial.println("\nSincronizando hora...");
  struct tm timeinfo;
  
  int intentos = 0; 
  // Establece un máximo de 10 segundos de espera (20 intentos * 500ms)
  const int max_intentos = 20; 
  
  // Espera hasta que llegue la hora correcta o se agoten los intentos.
  while (!getLocalTime(&timeinfo) && intentos < max_intentos) {
    Serial.print(F("."));      // Muestra puntos para indicar que sigue intentando
    delay(500);             // Pausa medio segundo entre intentos
    intentos++;             // Incrementa el contador
  }

  if (intentos < max_intentos) {
    Serial.println(F("\nHora sincronizada."));
  } else {
    // El programa continuará sin la hora NTP.
    Serial.println(F("\nAviso: La sincronización de la hora falló o tardó demasiado. El programa continúa sin hora NTP."));
  }
}

void web::firebaseInit() {
  // Credenciales y configuración del proyecto
  config.api_key      = key;
  config.database_url = url;
  auth.user.email     = email;
  auth.user.password  = password;

  Firebase.reconnectWiFi(true);  // Si se corta el WiFi, intenta reconectar
  fbdo.setResponseSize(4096);    // Tamaño máximo de respuesta

  Firebase.begin(&config, &auth); // Empieza la conexión con Firebase

  Serial.println(F("Conectando a Firebase..."));
  unsigned long startTime = millis();
  // Espera hasta 15 segundos a que Firebase quede listo
  while (!Firebase.ready() && millis() - startTime < 15000) {
    Serial.print(F("."));
    delay(500);
  }

  if (Firebase.ready()) {
    Serial.println(F("\nConexión con Firebase establecida."));
    // Abre un "stream": una especie de escucha en vivo de un valor en la nube
    if (!Firebase.RTDB.beginStream(&stream, "/commands/valve1State")) {
      Serial.println(F("Error al iniciar el stream: ") + stream.errorReason());
    }
  } else {
    Serial.println(F("\nNo se pudo conectar con Firebase. Verifica API Key, URL y credenciales."));
  }
}

void web::set_up() {
  WiFiConfigManager.begin();        //Prepara la conexión al WiFi
  syncTime();        // Pide la hora correcta a Internet
  firebaseInit();    // Prepara la conexión con Firebase
}

void web::enviar(dato data[], int n) {
  // Si hay WiFi pero Firebase no está listo, intenta prepararlo de nuevo
  if (WiFiConfigManager.isConnected()) {
    if (!Firebase.ready()) {
      Serial.println(F("Reconectando a Firebase..."));
      firebaseInit();
    }
  }

  // Si aún no está listo, vuelve a intentar inicializar
  if (!Firebase.ready()) {
    firebaseInit();
  }

  if (Firebase.ready()) {
    // Recorre todas las mediciones y las envía una por una
    for (int i = 0; i < n; i++) {
      char path[64];
      // Construye la ruta donde se guardará este dato
      // Ejemplo: /sensorData/caudalCaptacion
      snprintf(path, sizeof(path), "/sensorData/%s", data[i].etiquetaFirebase);
      Firebase.RTDB.setFloat(&fbdo, path, data[i].valor);  // Envía el número
    }
    Serial.println(F("-> Datos de sensores enviados a Firebase."));
  } else {
    Serial.println(F("-> No se pudieron enviar datos a Firebase. Conexión no lista."));
  }
}





