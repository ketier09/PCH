#pragma once

#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <time.h>

#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

void tokenStatusCallback(TokenInfo info); // callback ya manejado por Firebase

class web {
public:
    void set_up();
    void enviar(dato data[], int n);
    void handleStream();
    void ordenCompuerta(const String &accion);

private:
    bool firebaseInit();
    void syncTime();

    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;

    unsigned long lastTokenRefreshTime = 0;
    int estadoActualCompuerta = 0;
};

extern web pagina;
