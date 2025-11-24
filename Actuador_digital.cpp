/**
 * ----------------------------------------------------------------------------
 * WiFiConfigManager
 * ----------------------------------------------------------------------------
 * Este archivo contiene la declaración de la clase WiFiConfigManager, encargada 
 * de administrar:
 *   - Lectura y guardado de credenciales WiFi en LittleFS
 *   - Manejo de un portal cautivo de configuración (modo AP)
 *   - Conexión automática a redes guardadas
 *
 * Está diseñado para ESP8266/ESP32 y evita el uso excesivo de String para 
 * prevenir fragmentación de memoria.
 * ----------------------------------------------------------------------------
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
    // Constructor con ruta por defecto al archivo JSON
    WiFiConfigManager(const char* path = "/wifi.json");

    // Inicializa LittleFS y trata de conectar o abrir el portal si falla
    void begin();

    // Inicia conexión WiFi usando credenciales guardadas
    void connect();

    // Devuelve true si el dispositivo está conectado al WiFi
    bool isConnected();

  private:
    // Buffers estáticos para evitar fragmentación (mejor que usar String)
    char ssid[MAX_SSID_LEN + 1];
    char password[MAX_PASS_LEN + 1];

    const char* filePath;
    WebServer server{80}; // Servidor HTTP para el portal de configuración

    // Carga credenciales desde LittleFS (retorna false si no existen)
    bool loadCredentials();

    // Guarda nuevas credenciales en LittleFS de manera segura
    void saveCredentials(const char* ssid, const char* password);

    // Borra el archivo JSON con credenciales
    void eraseCredentials();

    // Inicia el portal en modo Access Point
    void startConfigPortal();

    // Manejadores HTTP para el portal
    void handleRoot();
    void handleSave();
    void handleNotFound();
};

// Instancia global para facilitar su uso en el código principal
extern WiFiConfigManager WiFiConfig;
