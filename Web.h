#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ExampleFunctions.h>
#include <FirebaseClient.h>
#include <time.h>

#include "secrets.h"
#include "Datos.h"

struct web {
  FirebaseData fbdo;
  FirebaseData stream;
  FirebaseAuth auth;
  FirebaseConfig config;

  void syncTime();
  void firebaseInit();
  void set_up();
  void enviar(dato data[], int n);
};
