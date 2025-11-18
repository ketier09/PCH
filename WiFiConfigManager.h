#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <functional> // Necesario para std::bind

// --- Definiciones de tamaño para control de memoria ---
#define MAX_SSID_LEN 32
#define MAX_PASS_LEN 64

class WiFiConfigManager {
  public:
    WiFiConfigManager(const char* path = "/wifi.json");
    void begin();
    void connect();
    bool isConnected();

  private:
    // 💡 OPTIMIZACIÓN: Reemplazo de String por char[] para evitar fragmentación.
    char ssid[MAX_SSID_LEN + 1];
    char password[MAX_PASS_LEN + 1];

    const char* filePath;
    WebServer server{80};

    // Actualización de firmas para usar const char*
    bool loadCredentials();
    // 💡 OPTIMIZACIÓN: Acepta const char* para ser independiente de String
    void saveCredentials(const char* ssid, const char* password);
    void eraseCredentials();
    void startConfigPortal();
    void handleRoot();
    void handleSave();
    void handleNotFound(); // Añadido para mejor manejo de errores HTTP
};
WiFiConfigManager WiFiConfig;
