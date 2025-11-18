#pragma once

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>

#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

void tokenStatusCallback(TokenInfo info);

class web {
public:
    void set_up();
    void enviar(dato dataArr[], int n);

    // Estado actual de la compuerta manejado localmente (4 posiciones)
    uint8_t estadoCompuerta = 0;

    // Acciones de control local (desde pulsador)
    void ejecutarComandoCompuerta(const String &accion);

    // Tiempo del último refresh del token
    unsigned long lastTokenRefreshTime = 0;

private:
    bool firebaseInit();
    void syncTime();
    String getISO8601Time();
    FirebaseJson buildFirestorePayload(dato dataArr[], int n);

    // Firebase internos del objeto
    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
};

// Instancia global
extern web pagina;

// Constantes
#define TOKEN_STABILIZE_MS    2000
#define NTP_MAX_ATTEMPTS      20
#define FIRESTORE_RETRY_MAX   5
