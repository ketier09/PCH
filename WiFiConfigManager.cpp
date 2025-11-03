#include "WiFiConfigManager.h"
#include <ArduinoJson.h>
#include <functional> // Necesario para std::bind

// --- Las páginas HTML se mantienen en PROGMEM para ahorrar RAM ---
// ... (PAGE_INDEX, PAGE_SAVED, PAGE_ERROR se mantienen igual) ...

// Se incluye la definición de los macros para evitar un error de compilación
#ifndef MAX_SSID_LEN
#define MAX_SSID_LEN 32
#endif
#ifndef MAX_PASS_LEN
#define MAX_PASS_LEN 64
#endif


// El constructor inicializa el arreglo de caracteres
WiFiConfigManager::WiFiConfigManager(const char* path) : filePath(path) {
  // Asegurar que los buffers inicien vacíos y terminen con null
  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));
}

void WiFiConfigManager::begin() {
  // Mejorar el manejo de LittleFS: detener la ejecución si falla.
  if (!LittleFS.begin(true)) {
    Serial.println(F("❌ Error montando LittleFS. No se puede continuar."));
    // Considerar un 'while(true)' o un LED de error si no se quiere retornar.
    return;
  }
  // No llamar a connect() en begin(), ya que begin() debería solo preparar el sistema de archivos.
  // La conexión debería ser una acción explícita en el sketch principal.
  // Se deja 'connect()' para mantener la funcionalidad original, pero es menos flexible.
  connect(); 
}

// -------------------------------------------------------------------
// 💾 Gestión de Credenciales
// -------------------------------------------------------------------

bool WiFiConfigManager::loadCredentials() {
  File file = LittleFS.open(filePath, "r");
  if (!file) {
    Serial.println(F("Archivo de credenciales no encontrado."));
    return false;
  }

  // Se usa un tamaño un poco mayor y se verifica la des-serialización
  StaticJsonDocument<256> doc; // Mayor margen para el JSON (128 es muy justo)
  DeserializationError error = deserializeJson(doc, file);
  file.close(); // Siempre cerrar el archivo después de usarlo

  if (error) {
    Serial.printf("❌ Error deserializando JSON: %s\n", error.c_str());
    return false;
  }

  // Uso de strlcpy para evitar desbordamiento de búfer al copiar strings
  const char* jsonSsid = doc["ssid"];
  const char* jsonPass = doc["password"];

  if (!jsonSsid || !jsonPass) {
    return false; // Claves JSON no encontradas
  }

  size_t ssid_len = strlcpy(ssid, jsonSsid, sizeof(ssid));
  size_t pass_len = strlcpy(password, jsonPass, sizeof(password));

  // Verificar si se pudo copiar y si los campos no están vacíos
  return (ssid_len > 0 && pass_len > 0 && ssid_len < sizeof(ssid) && pass_len < sizeof(password));
}

// Se cambia a const char* para evitar la creación de objetos String temporales
void WiFiConfigManager::saveCredentials(const char* newSsid, const char* newPassword) {
  StaticJsonDocument<256> doc; // Usar el mismo tamaño que en loadCredentials
  doc["ssid"] = newSsid;
  doc["password"] = newPassword;

  File file = LittleFS.open(filePath, "w");
  if (!file) {
    Serial.println(F("❌ Error al abrir archivo para escribir."));
    return;
  }
  
  // Guardar credenciales en los miembros de la clase
  strlcpy(ssid, newSsid, sizeof(ssid));
  strlcpy(password, newPassword, sizeof(password));

  if (serializeJson(doc, file) == 0) {
    Serial.println(F("❌ Error al serializar a LittleFS."));
  } else {
    Serial.println(F("✅ Credenciales guardadas en LittleFS."));
  }
  file.close(); // Cierre de archivo explícito y temprano
}

void WiFiConfigManager::eraseCredentials() {
  if (LittleFS.exists(filePath)) {
    // Verificar si se pudo eliminar
    if (LittleFS.remove(filePath)) {
      Serial.println(F("🗑️ Credenciales eliminadas."));
      memset(ssid, 0, sizeof(ssid));
      memset(password, 0, sizeof(password));
    } else {
      Serial.println(F("❌ Error al eliminar el archivo."));
    }
  }
}

// -------------------------------------------------------------------
// 🌐 Portal de Configuración Web
// -------------------------------------------------------------------

void WiFiConfigManager::handleRoot() {
  server.send_P(200, "text/html", PAGE_INDEX);
}

void WiFiConfigManager::handleSave() {
  // Uso de server.arg(name).c_str() para obtener const char*, pero la variable String temporal sigue existiendo.
  // Es mejor usar String localmente si los argumentos son de tamaño variable.
  String newSSID_str = server.arg("ssid");
  String newPass_str = server.arg("password");

  // Validar longitud antes de proceder
  if (newSSID_str.length() > 0 && newPass_str.length() > 0) {
    // Usar la función saveCredentials optimizada
    saveCredentials(newSSID_str.c_str(), newPass_str.c_str());

    server.send_P(200, "text/html", PAGE_SAVED);
    
    // Mejorar el manejo del reinicio: desconectar y apagar AP antes de reiniciar.
    delay(1000); 
    WiFi.softAPdisconnect(true);
    Serial.println(F("Reiniciando ESP..."));
    ESP.restart();
  } else {
    server.send_P(400, "text/html", PAGE_ERROR);
    // Añadir un pequeño delay para que el cliente pueda ver el error
    delay(500);
  }
}

void WiFiConfigManager::handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void WiFiConfigManager::startConfigPortal() {
  Serial.println(F("🌐 Iniciando modo AP para configuración..."));
  // Configurar el modo WiFi antes de llamar a softAP
  WiFi.mode(WIFI_AP);
  
  // Usar una constante para el nombre del AP y usar el chip ID para hacerlo único
  String apName = "ESP_Config-" + String(ESP.getChipId(), HEX); 
  WiFi.softAP(apName.c_str()); 
  
  IPAddress ip = WiFi.softAPIP();
  Serial.print(F("Conéctate a la red '"));
  Serial.print(apName);
  Serial.print(F("' y entra a http://"));
  Serial.println(ip);

  // Uso de std::bind para enlazar funciones miembro
  server.on("/", HTTP_GET, std::bind(&WiFiConfigManager::handleRoot, this));
  server.on("/save", HTTP_POST, std::bind(&WiFiConfigManager::handleSave, this));
  server.onNotFound(std::bind(&WiFiConfigManager::handleNotFound, this)); // Mejorar: handler 404
  server.begin();

  // El bucle de configuración debe tener un timeout, no ser infinito.
  // Se usará una variable global o de clase para un control más limpio
  // de cuándo salir del portal (p. ej., si el usuario pulsa un botón).
  while (true) { // Se mantiene 'while(true)' para la estructura original
    server.handleClient();
    delay(10); // Reducir delay a 1 ms (o 0 si es un ESP32) para mayor reactividad
  }
}

// -------------------------------------------------------------------
// 📶 Conexión WiFi
// -------------------------------------------------------------------

void WiFiConfigManager::connect() {
  if (!loadCredentials()) {
    startConfigPortal();
    return;
  }

  // Uso directo de char[] en printf
  Serial.printf("📶 Conectando a %s...\n", ssid); 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // Uso de char[] directo

  unsigned long start = millis();
  const unsigned long CONNECTION_TIMEOUT = 15000; // Aumentar a 15s para mayor fiabilidad
  
  while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECTION_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Uso directo de char[] y WiFi.localIP().toString()
    Serial.printf("\n✅ Conectado a %s. IP: %s\n", ssid, WiFi.localIP().toString().c_str());
  } else {
    Serial.println(F("\n❌ No se pudo conectar. Eliminando credenciales e iniciando portal..."));
    // Al fallar la conexión, eliminamos credenciales para que no intente
    // continuamente con unas credenciales que no funcionan.
    WiFi.disconnect(true); // Limpiar cualquier intento previo
    eraseCredentials(); 
    startConfigPortal();
  }
}

bool WiFiConfigManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
