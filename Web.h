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
  bool ensureLogin();

private:
  // 💡 OPTIMIZACIÓN: Mover a privado para encapsulamiento
  FirebaseData fbdo;
  FirebaseData stream;
  FirebaseAuth auth;
  FirebaseConfig config;
  
  // Constantes para tiempos de espera
  static constexpr int FIREBASE_TIMEOUT_MS = 30000;
  static constexpr int NTP_MAX_ATTEMPTS = 30; // 15 segundos (30 * 500ms)
  
  void syncTime();
  bool firebaseInit(); // 💡 OPTIMIZACIÓN: Cambiar a bool para control de flujo
};