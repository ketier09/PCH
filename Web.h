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
  // Mover los objetos de Firebase a la sección privada:
  // Solo la clase 'web' debe interactuar directamente con ellos.
  FirebaseData fbdo;
  FirebaseData stream;
  FirebaseAuth auth;
  FirebaseConfig config;
  
  // Constantes para tiempos de espera
  static constexpr int FIREBASE_TIMEOUT_MS = 15000;
  static constexpr int NTP_MAX_ATTEMPTS = 30; // 15 segundos (30 * 500ms)
  
  void syncTime();
  bool firebaseInit(); // Se cambia a bool para indicar éxito/fracaso
  void handleFirebaseConnection(); // Nueva función para manejar reconexión
};

// Se mantiene WiFiConfig como global/extern, asumiendo que debe ser accesible.
extern WiFiConfigManager WiFiConfig;
