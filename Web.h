#include <Firebase_ESP_Client.h>
#include <time.h>

#include "Secrets.h"
#include "Datos.h"
#include "WiFiConfigManager.h"

// Callback del token Firebase
void tokenStatusCallback(TokenInfo info);
class web {
public:
    void set_up();
    void enviar(dato data[], int n);
    void handleStream(); // control remoto desde Firebase

private:
    bool firebaseInit();
    void syncTime();

    FirebaseData fbdo;
    FirebaseData stream; // 👈 CORREGIDO: ya no FirebaseStream
    FirebaseAuth auth;
    FirebaseConfig config;

    const int NTP_MAX_ATTEMPTS = 10;
    unsigned long lastTokenRefreshTime = 0;
};

