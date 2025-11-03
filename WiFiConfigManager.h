#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

// Definiciones de tamaño máximo para SSID y Contraseña
#define MAX_SSID_LEN 32  // Longitud máxima estándar para SSID
#define MAX_PASS_LEN 64  // Longitud máxima estándar para WPA2

class WiFiConfigManager {
  public:
    // El constructor ahora recibe una referencia a un objeto WebServer existente,
    // o se usa un constructor con un puerto por defecto.
    WiFiConfigManager(const char* path = "/wifi.json");
    void begin();
    void connect();
    bool isConnected();

  private:
    // Uso de arreglos de char en lugar de String para evitar fragmentación y sobrecarga.
    char ssid[MAX_SSID_LEN + 1];
    char password[MAX_PASS_LEN + 1];

    const char* filePath;
    // Se deja como miembro, pero se considera el impacto en la memoria de heap (alternativa: pasar por referencia en el constructor)
    WebServer server{80};

    // La función de guardar ahora usa const char* o referencias a String para evitar copias.
    bool loadCredentials();
    void saveCredentials(const char* ssid, const char* password); // Cambio de String a const char*
    void eraseCredentials();
    void startConfigPortal();
    void handleRoot();
    void handleSave();
    void handleNotFound(); // Añadir un handler para 404
};
