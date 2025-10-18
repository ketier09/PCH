#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

#include "Secrets.h"
#include "Datos.h"

struct web {
  FirebaseData fbdo;
  FirebaseData stream;
  FirebaseAuth auth;
  FirebaseConfig config;

  void wifiInit();
  void syncTime();
  void firebaseInit();
  void set_up();
  void enviar(dato data[], int n);
};
