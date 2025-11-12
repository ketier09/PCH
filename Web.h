#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

#include <time.h>

#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

class web {
public:
  void set_up();
  void enviar(dato data[], int n);

private:
  FirebaseData fbdo;
  FirebaseData stream;
  FirebaseAuth auth;
  FirebaseConfig config;
  
  unsigned long lastTokenRefreshTime = 0;
  static constexpr int FIREBASE_TIMEOUT_MS = 30000;
  static constexpr int NTP_MAX_ATTEMPTS = 30;
  
  void syncTime();
  bool firebaseInit();
};
