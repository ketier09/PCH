#include "WiFiConfigManager.h"
#include <ArduinoJson.h>
#include <string.h> // Para memset y strlcpy

// --- Página HTML guardada en memoria flash ---
const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Configuracion WiFi</title>
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
    <p>El dispositivo se reiniciara...</p>
  </body>
</html>
)rawliteral";

const char PAGE_ERROR[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <body>
    <h3>❌ Datos inválidos.</h3>
    <p>Por favor, vuelva atras e intentelo de nuevo.</p>
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
    Serial.println(F("\n[WiFi] ❌ Error montando LittleFS."));
    return;
  }
  connect();
}

bool WiFiConfigManager::loadCredentials() {
  File file = LittleFS.open(filePath, "r");
  if (!file) return false;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error || doc["ssid"].isNull() || doc["password"].isNull()) {
    Serial.printf("\n[WiFi] ❌ Error deserializando JSON: %s\n", error.c_str());
    return false;
  }

  strlcpy(ssid, doc["ssid"] | "", sizeof(ssid));
  strlcpy(password, doc["password"] | "", sizeof(password));

  return (ssid[0] != '\0' && password[0] != '\0');
}

void WiFiConfigManager::saveCredentials(const char* newSsid, const char* newPassword) {
  JsonDocument doc;
  doc["ssid"] = newSsid;
  doc["password"] = newPassword;

  strlcpy(ssid, newSsid, sizeof(ssid));
  strlcpy(password, newPassword, sizeof(password));

  File file = LittleFS.open(filePath, "w");
  if (!file) return;

  if (serializeJson(doc, file) == 0) {
    Serial.println(F("\n[WiFi] ❌ Error al serializar a LittleFS."));
  } else {
    Serial.println(F("\n✅ Credenciales guardadas en LittleFS."));
  }
  file.close();
}

void WiFiConfigManager::eraseCredentials() {
  if (LittleFS.exists(filePath)) {
    if (LittleFS.remove(filePath)) {
      Serial.println(F("\n🗑️ Credenciales eliminadas."));
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
    saveCredentials(newSSID_str.c_str(), newPass_str.c_str());
    server.send_P(200, "text/html", PAGE_SAVED);
    delay(1000);
    ESP.restart();
  } else {
    server.send_P(400, "text/html", PAGE_ERROR);
    Serial.println(F("\n[WiFi] ❌ SSID o contraseña vacíos."));
    delay(500);
  }
}

void WiFiConfigManager::handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

void WiFiConfigManager::startConfigPortal() {
  Serial.println(F("\n🌐📵 Iniciando modo AP para configuración..."));
  WiFi.mode(WIFI_AP);

  String apName;

#if defined(ESP8266)
  apName = "ESP_Config-" + String(ESP.getChipId(), HEX);
#elif defined(ESP32)
  uint64_t mac = ESP.getEfuseMac();
  uint32_t chip = (uint32_t)(mac & 0xFFFFFF);

  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%06X", chip);
  apName = String("ESP_Config-") + suffix;
#else
  String macStr = WiFi.softAPmacAddress();
  macStr.replace(":", "");
  String tail = macStr.length() >= 6 ? macStr.substring(macStr.length() - 6) : macStr;
  tail.toUpperCase();
  apName = "ESP_Config-" + tail;
#endif

  apName.toUpperCase();
  WiFi.softAP(apName.c_str());

  IPAddress ip = WiFi.softAPIP();
  Serial.printf("\nConéctate a la red '%s' y entra a http://%s\n",
                apName.c_str(), ip.toString().c_str());

  server.on("/", HTTP_GET, std::bind(&WiFiConfigManager::handleRoot, this));
  server.on("/save", HTTP_POST, std::bind(&WiFiConfigManager::handleSave, this));
  server.onNotFound(std::bind(&WiFiConfigManager::handleNotFound, this));
  server.begin();

  while (true) {
    server.handleClient();
    delay(1);
  }
}

void WiFiConfigManager::connect() {
  if (!loadCredentials()) {
    Serial.println(F("\n[WiFi] ❌ No hay credenciales guardadas."));
    startConfigPortal();
    return;
  }

  Serial.printf("\n📶 Conectando a %s...\n", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  const unsigned long CONNECTION_TIMEOUT = 15000;

  while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECTION_TIMEOUT) {
    delay(500);
    Serial.print(F("....."));
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ Conectado a %s. IP: %s\n", ssid, WiFi.localIP().toString().c_str());
  } else {
    Serial.println(F("\n[WiFi] ❌ No se pudo conectar. Eliminando credenciales e iniciando portal..."));
    eraseCredentials();
    startConfigPortal();
  }
}

bool WiFiConfigManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
