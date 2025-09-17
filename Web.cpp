#include "Web.h"

// -------------------------------------------
// syncTime()
// Pide la hora correcta a servidores de Internet (NTP)
// para que el dispositivo sepa la hora y fecha actuales.
// -------------------------------------------
void web::syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Servidores de hora
  Serial.println("\nSincronizando hora...");
  struct tm timeinfo;
  // Espera hasta que llegue la hora correcta.
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");      // Muestra puntos para indicar que sigue intentando
    delay(500);             // Pausa medio segundo entre intentos
  }
  Serial.println("\nHora sincronizada.");
}

// -------------------------------------------
// firebaseInit()
// Inicia la conexión con Firebase (la base de datos en la nube).
// Coloca las llaves (API key), la dirección del proyecto y el usuario,
// y luego intenta conectarse y abrir un "canal" para leer comandos.
// -------------------------------------------
void web::firebaseInit() {
  // Credenciales y configuración del proyecto
  config.api_key = key;
  config.database_url = url;
  auth.user.email = email;
  auth.user.password = password;

  Firebase.reconnectWiFi(true);  // Si se corta el WiFi, intenta reconectar
  fbdo.setResponseSize(4096);    // Tamaño máximo de respuesta

  Firebase.begin(&config, &auth); // Empieza la conexión con Firebase

  Serial.println("Conectando a Firebase...");
  unsigned long startTime = millis();
  // Espera hasta 15 segundos a que Firebase quede listo
  while (!Firebase.ready() && millis() - startTime < 15000) {
      Serial.print(".");
      delay(500);
  }

  if (Firebase.ready()) {
    Serial.println("\nConexión con Firebase establecida.");
    // Abre un "stream": una especie de escucha en vivo de un valor en la nube
    if (!Firebase.RTDB.beginStream(&stream, "/commands/valve1State")) {
      Serial.println("Error al iniciar el stream: " + stream.errorReason());
    }
  } else {
    Serial.println("\nNo se pudo conectar con Firebase. Verifica API Key, URL y credenciales.");
  }
}

// -------------------------------------------
// set_up()
// Conecta el dispositivo a la red WiFi, muestra su IP,
// sincroniza la hora y prepara Firebase.
// -------------------------------------------
void web::set_up() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);  // Intento de conexión con tu red
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");                   // Muestra progreso
    delay(500);
  }

  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());        // Muestra la dirección IP asignada

  syncTime();        // Pide la hora correcta a Internet
  firebaseInit();    // Prepara la conexión con Firebase
}

// -------------------------------------------
// enviar()
// Sube a Firebase un conjunto de mediciones (datos de sensores).
// 'data' es la lista de mediciones, 'n' es cuántas son.
// Para cada medición, envía su valor a una ruta de la base de datos.
// -------------------------------------------
void web::enviar(datos data[], int n) {
  // Si hay WiFi pero Firebase no está listo, intenta prepararlo de nuevo
  if (WiFi.status() == WL_CONNECTED) {
    if (!Firebase.ready()) {
      Serial.println("Reconectando a Firebase...");
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
    Serial.println("-> Datos de sensores enviados a Firebase.");
  } else {
    Serial.println("-> No se pudieron enviar datos a Firebase. Conexión no lista.");
  }
}
