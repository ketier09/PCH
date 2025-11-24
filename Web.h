// ============================= Web.h ======================================
/**
 * Módulo web
 * -------------------------------------------------------------------------
 * Encargado de:
 *   - Inicializar Firebase/Firestore
 *   - Sincronizar la hora mediante NTP
 *   - Enviar lecturas de sensores a Firestore (documento "latest" e histórico)
 *   - Gestionar el estado de la compuerta de forma local (variable estadoCompuerta)
 *
 * ISSUE RELACIONADA CON FIRESTORE (#28 - "Firestore no envía ordenes"):
 *   Actualmente el código permite:
 *     - ENVIAR datos a Firestore
 *     - Actualizar el documento "compuerta/estadoActual" cuando la orden
 *       viene desde el propio dispositivo (pulsador local).
 *   Pero NO se está leyendo de Firestore para ejecutar órdenes que vengan
 *   desde la website. El bucle de "polling" o escucha remota todavía no
 *   está implementado; ahí es donde hay que trabajar para cerrar la issue.
 */

#pragma once

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>

#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

// Callback de estado del token (requerido por la librería)
void tokenStatusCallback(TokenInfo info);

class web {
public:
  // Configura Firebase y realiza la primera inicialización
  void set_up();

  // Envía un arreglo de datos (sensores) a Firestore
  void enviar(dato dataArr[], int n);

  // Estado actual de la compuerta manejado localmente (4 posiciones posibles)
  volatile uint8_t estadoCompuerta = 0;

  // Ejecuta una acción sobre la compuerta (normalmente llamada desde un pulsador)
  void ejecutarComandoCompuerta(const String &accion);

  // Tiempo del último refresh del token de autenticación
  unsigned long lastTokenRefreshTime = 0;

private:
  // Tiempo de estabilización tras obtener un token nuevo
  static constexpr uint32_t TOKEN_STABILIZE_MS = 2000;

  // Intentos máximos para sincronizar hora por NTP
  static constexpr int NTP_MAX_ATTEMPTS = 20;

  // Intentos máximos para reintentar operaciones con Firestore (reservado para mejoras)
  static constexpr int FIRESTORE_RETRY_MAX = 5;

  // Inicializa Firebase y espera a que el token esté listo
  bool firebaseInit();

  // Sincroniza la hora del dispositivo mediante NTP
  void syncTime();

  // Devuelve fecha/hora actual en formato ISO8601 (para Firestore)
  String getISO8601Time();

  // Construye el JSON de Firestore con los campos de sensores
  FirebaseJson buildFirestorePayload(dato dataArr[], int n);

  // Objetos internos de Firebase
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;
};
