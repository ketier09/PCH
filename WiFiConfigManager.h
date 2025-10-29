#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

class WiFiConfigManager {
  public:
    WiFiConfigManager(const char* path = "/wifi.json");
    void begin();
    void connect();
    bool isConnected();

  private:
    String ssid;
    String password;
    const char* filePath;
    WebServer server{80};

    bool loadCredentials();
    void saveCredentials(const String& ssid, const String& password);
    void eraseCredentials();
    void startConfigPortal();
    void handleRoot();
    void handleSave();
};
WiFiConfigManager WiFiConfig