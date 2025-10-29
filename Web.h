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

  FirebaseData fbdo;
  FirebaseData stream;
  FirebaseAuth auth;
  FirebaseConfig config;
  
  void set_up();
  void enviar(dato data[], int n);

private:

  void syncTime();
  void firebaseInit();
};
