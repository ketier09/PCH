#include "WiFiConfigManager.h"
#include <ArduinoJson.h>
#include <string.h> // Para memset y strlcpy

// --- Página HTML guardada en memoria flash ---
const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Configuración WiFi</title>
  </head>
  <body>
    <div class="card">
      <h2>Configurar WiFi</h2>
      <form action="/save" method="POST">
        <input type="text" name="ssid" placeholder="SSID"><br>
        <input type="password" name="password" placeholder="Contraseña"><br>
        <button type="submit">Guardar</button>
      </form>
    </div>
  </body>
</html>
)rawliteral";

// --- Mensajes HTML también en flash ---
const char PAGE_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <body>
    <h3>✅ Credenciales guardadas correctamente.</h3>
    <p>El dispositivo se reiniciará...</p>
  </body>
</html>
)rawliteral";

const char PAGE_ERROR[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <body>
    <h3>❌ Datos inválidos.</h3>
    <p>Por favor, vuelva atrás e inténtelo de nuevo.</p>
  </body>
</html>
)rawliteral";

// Constructor optimizado para inicializar buffers
WiFiConfigManager::WiFiConfigManager(const char* path) : filePath(path) {
  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));
}

void WiFiConfigManager::begin() {
  if (!LittleFS.begin(true)) {
    Serial.println(F("❌ Error montando LittleFS."));
    return;
  }
  connect();
}

bool WiFiConfigManager::loadCredentials() {
  File file = LittleFS.open(filePath, "r");
  if (!file) return false;

  JsonDocument doc;  // v7 style (dynamic)
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error || doc["ssid"].isNull() || doc["password"].isNull()) {
    Serial.printf("❌ Error deserializando JSON: %s\n", error.c_str());
    return false;
  }

  // Copia segura
  strlcpy(ssid, doc["ssid"] | "", sizeof(ssid));
  strlcpy(password, doc["password"] | "", sizeof(password));

  return (ssid[0] != '\0' && password[0] != '\0');
}

void WiFiConfigManager::saveCredentials(const char* newSsid, const char* newPassword) {
  JsonDocument doc;  // v7 style (dynamic)
  doc["ssid"] = newSsid;
  doc["password"] = newPassword;

  // Copia segura a los miembros de la clase
  strlcpy(ssid, newSsid, sizeof(ssid));
  strlcpy(password, newPassword, sizeof(password));

  File file = LittleFS.open(filePath, "w");
  if (!file) return;

  if (serializeJson(doc, file) == 0) {
    Serial.println(F("❌ Error al serializar a LittleFS."));
  } else {
    Serial.println(F("✅ Credenciales guardadas en LittleFS."));
  }
  file.close();
}

void WiFiConfigManager::eraseCredentials() {
  if (LittleFS.exists(filePath)) {
    if (LittleFS.remove(filePath)) {
      Serial.println(F("🗑️ Credenciales eliminadas."));
      memset(ssid, 0, sizeof(ssid));
      memset(password, 0, sizeof(password));
    }
  }
}

void WiFiConfigManager::handleRoot() {
  server.send_P(200, "text/html", PAGE_INDEX);
}

void WiFiConfigManager::handleSave() {
  String newSSID_str = server.arg("ssid");
  String newPass_str = server.arg("password");

  if (newSSID_str.length() > 0 && newPass_str.length() > 0) {
    // Uso de c_str() para pasar a la función optimizada
    saveCredentials(newSSID_str.c_str(), newPass_str.c_str());
    server.send_P(200, "text/html", PAGE_SAVED);
    delay(1000); // 💡 OPTIMIZACIÓN: Reducción de delay de 2s a 1s
    ESP.restart();
  } else {
    server.send_P(400, "text/html", PAGE_ERROR);
    delay(500);
  }
}

void WiFiConfigManager::handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void WiFiConfigManager::startConfigPortal() {
  Serial.println(F("🌐 Iniciando modo AP para configuración..."));
  WiFi.mode(WIFI_AP);

  // 💡 OPTIMIZACIÓN: AP Único con Chip ID para evitar conflictos
  String apName = "ESP_Config-" + String(ESP.getChipId(), HEX);
  WiFi.softAP(apName.c_str());

  IPAddress ip = WiFi.softAPIP();
  Serial.printf("Conéctate a la red '%s' y entra a http://%s\n", apName.c_str(), ip.toString().c_str());

  server.on("/", HTTP_GET, std::bind(&WiFiConfigManager::handleRoot, this));
  server.on("/save", HTTP_POST, std::bind(&WiFiConfigManager::handleSave, this));
  server.onNotFound(std::bind(&WiFiConfigManager::handleNotFound, this));
  server.begin();

  while (true) {
    server.handleClient();
    delay(1); // 💡 OPTIMIZACIÓN: Reducción de delay(10) a delay(1)
  }
}

void WiFiConfigManager::connect() {
  if (!loadCredentials()) {
    startConfigPortal();
    return;
  }

  Serial.printf("📶 Conectando a %s...\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // Uso de char[] directo

  unsigned long start = millis();
  const unsigned long CONNECTION_TIMEOUT = 15000; // 💡 OPTIMIZACIÓN: Aumento a 15s
  
  while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECTION_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ Conectado a %s. IP: %s\n", ssid, WiFi.localIP().toString().c_str());
  } else {
    Serial.println(F("\n❌ No se pudo conectar. Eliminando credenciales e iniciando portal..."));
    WiFi.disconnect(true);
    eraseCredentials(); // Eliminar credenciales si la conexión falla
    startConfigPortal();
  }
}

bool WiFiConfigManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
