#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

#include "Datos.h"

struct web {
  const char* WIFI_SSID;
  const char* WIFI_PASSWORD;
  const char* key;
  const char* url;
  const char* email;
  const char* password;
    //No hay necesidad de poner esta información

  // Constructor
  web(const char* w, const char* p, const char* k,
      const char* u, const char* e, const char* pass);

  // Objetos Firebase
  FirebaseData fbdo;
  FirebaseData stream;
  FirebaseAuth auth;
  FirebaseConfig config;

  // Métodos
  void syncTime();
  void firebaseInit();
  void set_up();
  void enviar(datos data[], int n);
};
