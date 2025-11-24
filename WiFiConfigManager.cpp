// ======================= WiFiConfigManager.cpp ============================
/**
 * Implementación de WiFiConfigManager
 * -------------------------------------------------------------------------
 * Este módulo implementa:
 *   - Lectura y escritura de credenciales WiFi en LittleFS (JSON)
 *   - Portal local para configurar credenciales en modo AP
 *   - Conexión automática con manejo de errores y recuperación
 *
 * ISSUE RELACIONADA:
 *   #20 - "Estética de ingreso de credenciales"
 *   La estructura del formulario HTML está definida en PAGE_INDEX. Para mejorar
 *   la apariencia (colores, CSS, mensajes, etc.) basta con editar esa plantilla.
 */

#include "WiFiConfigManager.h"
#include <ArduinoJson.h>
#include <string.h> // Para memset y strlcpy

// Instancia global accesible desde otros módulos
WiFiConfigManager WiFiConfig;

// --- Página HTML mostrada en el portal de configuración ---
//  Actualmente es una página mínima sin estilos avanzados (ver issue #20).
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

// Mensaje al guardar credenciales correctamente
const char PAGE_SAVED[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <body>
    <h3>✅ Credenciales guardadas correctamente.</h3>
    <p>El dispositivo se reiniciara...</p>
  </body>
</html>
)rawliteral";

// Mensaje en caso de error en los datos recibidos
const char PAGE_ERROR[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <body>
    <h3>❌ Datos inválidos.</h3>
    <p>Por favor, vuelva atras e intentelo de nuevo.</p>
  </body>
</html>
)rawliteral";

// Constructor: limpia buffers y guarda ruta del archivo
WiFiConfigManager::WiFiConfigManager(const char* path) : filePath(path) {
  memset(ssid, 0, sizeof(ssid));
  memset(password, 0, sizeof(password));
}

void WiFiConfigManager::begin() {
  // Monta LittleFS (si falla, se rehace el sistema de archivos con format = true)
  if (!LittleFS.begin(true)) {
    Serial.println(F("\n[WiFi] ❌ Error montando LittleFS."));
    return;
  }
  // Tras montar el sistema de archivos, intenta conectar
  connect();
}

bool WiFiConfigManager::loadCredentials() {
  // Abre el archivo JSON en modo lectura
  File file = LittleFS.open(filePath, "r");
  if (!file) return false;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  // Validaciones de formato y contenido
  if (error || doc["ssid"].isNull() || doc["password"].isNull()) {
    Serial.printf("\n[WiFi] ❌ Error deserializando JSON: %s\n", error.c_str());
    return false;
  }

  // Copia segura a buffers locales (se asegura el \0 final)
  strlcpy(ssid, doc["ssid"] | "", sizeof(ssid));
  strlcpy(password, doc["password"] | "", sizeof(password));

  // Retorna true sólo si ambos campos no están vacíos
  return (ssid[0] != '\0' && password[0] != '\0');
}

void WiFiConfigManager::saveCredentials(const char* newSsid, const char* newPassword) {
  // Construye el objeto JSON a guardar
  JsonDocument doc;
  doc["ssid"] = newSsid;
  doc["password"] = newPassword;

  // Copia también al buffer interno para poder conectar sin recargar archivo
  strlcpy(ssid, newSsid, sizeof(ssid));
  strlcpy(password, newPassword, sizeof(password));

  // Abre/crea el archivo en modo escritura (se sobreescribe si ya existe)
  File file = LittleFS.open(filePath, "w");
  if (!file) return;

  // Serializa y escribe JSON en LittleFS
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("\n[WiFi] ❌ Error al serializar a LittleFS."));
  } else {
    Serial.println(F("\n✅ Credenciales guardadas en LittleFS."));
  }
  file.close();
}

// Elimina archivo con credenciales y limpia los buffers en RAM
void WiFiConfigManager::eraseCredentials() {
  if (LittleFS.exists(filePath)) {
    if (LittleFS.remove(filePath)) {
      Serial.println(F("\n🗑️ Credenciales eliminadas."));
      memset(ssid, 0, sizeof(ssid));
      memset(password, 0, sizeof(password));
    }
  }
}

// Manejador para la ruta raíz del portal de configuración
void WiFiConfigManager::handleRoot() {
  // Devuelve la página principal del portal
  server.send_P(200, "text/html", PAGE_INDEX);
}

// Procesa el formulario enviado desde la página de configuración
void WiFiConfigManager::handleSave() {
  // Obtiene ssid y contraseña desde el formulario HTML
  String newSSID_str = server.arg("ssid");
  String newPass_str = server.arg("password");

  // Validación básica: no aceptar campos vacíos
  if (newSSID_str.length() > 0 && newPass_str.length() > 0) {
    saveCredentials(newSSID_str.c_str(), newPass_str.c_str());
    server.send_P(200, "text/html", PAGE_SAVED);
    delay(1000);
    ESP.restart(); // Reinicia para conectar automáticamente con las nuevas credenciales
  } else {
    server.send_P(400, "text/html", PAGE_ERROR);
    Serial.println(F("\n[WiFi] ❌ SSID o contraseña vacíos."));
    delay(500);
  }
}

// Manejador por defecto cuando la ruta no existe
void WiFiConfigManager::handleNotFound() {
  server.send(404, "text/plain", "404: Not Found");
}

// Inicia el Access Point y el servidor web para la configuración
void WiFiConfigManager::startConfigPortal() {
  Serial.println(F("\n🌐📵 Iniciando modo AP para configuración..."));
  WiFi.mode(WIFI_AP);

  // Genera nombre único para la red AP usando el identificador del chip
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
  // Fallback para otros dispositivos
  String macStr = WiFi.softAPmacAddress();
  macStr.replace(":", "");
  String tail = macStr.substring(macStr.length() - 6);
  tail.toUpperCase();
  apName = "ESP_Config-" + tail;
#endif

  apName.toUpperCase();
  WiFi.softAP(apName.c_str());

  IPAddress ip = WiFi.softAPIP();
  Serial.printf("\nConéctate a la red '%s' y entra a http://%s\n",
                apName.c_str(), ip.toString().c_str());

  // Registro de rutas del servidor
  server.on("/", HTTP_GET, std::bind(&WiFiConfigManager::handleRoot, this));
  server.on("/save", HTTP_POST, std::bind(&WiFiConfigManager::handleSave, this));
  server.onNotFound(std::bind(&WiFiConfigManager::handleNotFound, this));
  server.begin();

  // Bucle del servidor (bloqueante durante el proceso de configuración)
  while (true) {
    server.handleClient();
    delay(1);
  }
}

void WiFiConfigManager::connect() {
  // Si no hay credenciales válidas, forzar apertura del portal de configuración
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

  // Intento de conexión con timeout
  while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECTION_TIMEOUT) {
    delay(500);
    Serial.print(F("....."));
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n✅ Conectado a %s. IP: %s\n", ssid, WiFi.localIP().toString().c_str());
  } else {
    // Si no se logra conectar, se borran las credenciales y se abre el portal
    Serial.println(F("\n[WiFi] ❌ No se pudo conectar. Eliminando credenciales e iniciando portal..."));
    eraseCredentials();
    startConfigPortal();
  }
}

// Comprueba si la interfaz WiFi está asociada y con IP
bool WiFiConfigManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
