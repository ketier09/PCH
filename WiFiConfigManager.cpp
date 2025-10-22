#include "WiFiConfigManager.h"
#include <ArduinoJson.h>

// --- Página HTML guardada en memoria flash ---
const char PAGE_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Configuración WiFi</title>
    <style>
      body { font-family: sans-serif; text-align: center; margin-top: 60px; background-color: #fafafa; }
      input { padding: 6px; width: 200px; margin: 5px; }
      button { padding: 8px 16px; background-color: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; }
      button:hover { background-color: #0056b3; }
      .card { display:inline-block; padding: 20px; border-radius: 12px; background-color: #fff; box-shadow: 0 2px 5px rgba(0,0,0,0.15);}
    </style>
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
  <body style="font-family:sans-serif; text-align:center; margin-top:50px;">
    <h3>✅ Credenciales guardadas correctamente.</h3>
    <p>El dispositivo se reiniciará...</p>
  </body>
</html>
)rawliteral";

const char PAGE_ERROR[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <body style="font-family:sans-serif; text-align:center; margin-top:50px;">
    <h3>❌ Datos inválidos.</h3>
    <p>Por favor, vuelva atrás e inténtelo de nuevo.</p>
  </body>
</html>
)rawliteral";

WiFiConfigManager::WiFiConfigManager(const char* path) : filePath(path) {}

void WiFiConfigManager::begin() {
  if (!LittleFS.begin(true)) {
    Serial.println(F("❌ Error montando LittleFS"));
    return;
  }
  connect();
}

bool WiFiConfigManager::loadCredentials() {
  File file = LittleFS.open(filePath, "r");
  if (!file) return false;

  StaticJsonDocument<128> doc;
  if (deserializeJson(doc, file)) {
    file.close();
    return false;
  }
  file.close();

  ssid = doc["ssid"].as<String>();
  password = doc["password"].as<String>();
  return !(ssid.isEmpty() || password.isEmpty());
}

void WiFiConfigManager::saveCredentials(const String& ssid, const String& password) {
  StaticJsonDocument<128> doc;
  doc["ssid"] = ssid;
  doc["password"] = password;

  File file = LittleFS.open(filePath, "w");
  if (!file) return;
  serializeJson(doc, file);
  file.close();
  Serial.println(F("✅ Credenciales guardadas en LittleFS."));
}

void WiFiConfigManager::eraseCredentials() {
  if (LittleFS.exists(filePath)) {
    LittleFS.remove(filePath);
    Serial.println(F("🗑️ Credenciales eliminadas."));
  }
}

void WiFiConfigManager::handleRoot() {
  server.send_P(200, "text/html", PAGE_INDEX);
}

void WiFiConfigManager::handleSave() {
  String newSSID = server.arg("ssid");
  String newPass = server.arg("password");

  if (newSSID.length() > 0 && newPass.length() > 0) {
    saveCredentials(newSSID, newPass);
    server.send_P(200, "text/html", PAGE_SAVED);
    delay(2000);
    ESP.restart();
  } else {
    server.send_P(400, "text/html", PAGE_ERROR);
  }
}

void WiFiConfigManager::startConfigPortal() {
  Serial.println(F("🌐 Iniciando modo AP para configuración..."));
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP_Config");
  IPAddress ip = WiFi.softAPIP();
  Serial.print(F("Conéctate a la red 'ESP_Config' y entra a http://"));
  Serial.println(ip);

  server.on("/", HTTP_GET, std::bind(&WiFiConfigManager::handleRoot, this));
  server.on("/save", HTTP_POST, std::bind(&WiFiConfigManager::handleSave, this));
  server.begin();

  while (true) {
    server.handleClient();
    delay(10);
  }
}

void WiFiConfigManager::connect() {
  if (!loadCredentials()) {
    startConfigPortal();
    return;
  }

  Serial.printf("📶 Conectando a %s...\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ Conectado a %s. IP: %s\n", ssid.c_str(), WiFi.localIP().toString().c_str());
  } else {
    Serial.println(F("\n❌ No se pudo conectar. Iniciando portal de configuración..."));
    eraseCredentials();
    startConfigPortal();
  }
}

bool WiFiConfigManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
