#pragma once

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>

#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

void tokenStatusCallback(TokenInfo info);

// Control de compuerta → 4 estados
class web {
public:
    void set_up();
    void enviar(dato dataArr[], int n);
    void handleStream();

    uint8_t estadoCompuerta = 0; 

    // Procesar acción desde Firebase
    void ejecutarComandoCompuerta(const String &accion);

    unsigned long lastTokenRefreshTime = 0;

private:
    bool firebaseInit();
    void syncTime();
    String getISO8601Time();
    FirebaseJson buildFirestorePayload(dato dataArr[], int n);

    bool listenCommands(const String &collection, const String &docId);
    void processFirestoreEvents();

    int estadoActualCompuerta = 0;

    FirebaseData fbdo;
    FirebaseData stream;
    FirebaseAuth auth;
    FirebaseConfig config;
};

extern web pagina;

#define TOKEN_STABILIZE_MS 2000
#define NTP_MAX_ATTEMPTS   20
#define FIRESTORE_RETRY_MAX 5
