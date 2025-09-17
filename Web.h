#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

#include "secrets.h" 
#include "Datos.h"

struct web {

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
