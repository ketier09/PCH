// ======================= WiFiConfigManager.h ===============================
/**
 * WiFiConfigManager
 * -------------------------------------------------------------------------
 * Clase encargada de toda la lógica de red local:
 *   - Leer y guardar credenciales WiFi en LittleFS (archivo JSON /wifi.json)
 *   - Levantar un portal de configuración en modo AP cuando no hay credenciales
 *   - Intentar la conexión automática a la red guardada
 *
 * NOTA RELACIONADA CON ISSUE #20:
 *   La estética de la página HTML del portal de configuración es muy básica
 *   (inputs sin estilos). Cualquier mejora visual se hace modificando las
 *   plantillas HTML definidas en WiFiConfigManager.cpp (PAGE_INDEX, etc.).
 */
#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <functional> // Necesario para std::bind

// --- Tamaños máximos para evitar desbordamientos ---
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

class WiFiConfigManager {
  public:
    // Constructor con ruta por defecto al archivo JSON donde se guardan las credenciales
    WiFiConfigManager(const char* path = "/wifi.json");

    // Inicializa LittleFS y trata de conectar; si falla, abre el portal de configuración
    void begin();

    // Inicia conexión WiFi usando credenciales guardadas (si existen)
    void connect();

    // Devuelve true si el dispositivo está conectado al WiFi
    bool isConnected();

  private:
    // Buffers estáticos para SSID y contraseña (evitan fragmentación por String)
    char ssid[MAX_SSID_LEN + 1];
    char password[MAX_PASS_LEN + 1];

    // Ruta al archivo de credenciales en LittleFS
    const char* filePath;

    // Servidor HTTP que atiende el portal de configuración en modo AP
    WebServer server{80};

    // Carga credenciales desde LittleFS (retorna false si no existen o hay error)
    bool loadCredentials();

    // Guarda nuevas credenciales en LittleFS de manera segura
    void saveCredentials(const char* ssid, const char* password);

    // Borra el archivo JSON con credenciales y limpia los buffers en RAM
    void eraseCredentials();

    // Inicia el portal en modo Access Point para configurar nuevas credenciales
    void startConfigPortal();

    // Manejadores HTTP para el portal
    void handleRoot();      // Página principal con el formulario
    void handleSave();      // Procesa el POST del formulario
    void handleNotFound();  // Respuesta 404
};

// Instancia global para facilitar su uso en el código principal
extern WiFiConfigManager WiFiConfig;
