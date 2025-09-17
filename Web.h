#pragma once
#include <Arduino.h>
#include <WiFi.h>                 // Permite conectarse a Internet por WiFi
#include <Firebase_ESP_Client.h>  // Librería para hablar con Firebase (la nube)
#include <time.h>                 // Sirve para manejar fecha y hora

#include "secrets.h"  // Aquí guardas tus claves y contraseñas (WiFi, Firebase)
#include "Datos.h"    // Estructura donde vienen los datos de sensores

// "web" es como un asistente que:
// 1) se conecta al WiFi,
// 2) sincroniza la hora correcta desde Internet,
// 3) se conecta a Firebase,
// 4) y sube los datos de los sensores a la nube.
struct web {

  // -------- Objetos de Firebase --------
  FirebaseData fbdo;    // Canal para enviar/recibir datos (peticiones normales)
  FirebaseData stream;  // Canal "en vivo" para escuchar cambios (comandos, etc.)
  FirebaseAuth auth;    // Usuario/contraseña para entrar a tu proyecto
  FirebaseConfig config;// Dirección del proyecto y llave (API key)

  // -------- Acciones que puede hacer --------

  // Pide la hora correcta a servidores de Internet (NTP),
  // así el dispositivo sabe fecha y hora reales.
  void syncTime();

  // Configura e inicia la conexión con Firebase usando las credenciales
  // (API key, URL del proyecto, email y contraseña).
  void firebaseInit();

  // Paso a paso: conecta a WiFi, muestra la IP, sincroniza la hora
  // y prepara Firebase para trabajar.
  void set_up();

  // Sube a Firebase una lista de lecturas de sensores.
  // 'data' es el arreglo con las mediciones y 'n' es cuántas son.
  void enviar(datos data[], int n);
};
