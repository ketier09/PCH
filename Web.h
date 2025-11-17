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
    void handleStream(); // 👈 Aquí sí, dentro de la clase web

private:
    bool firebaseInit();
    void syncTime();

    FirebaseData fbdo;
    FirebaseAuth auth;
    FirebaseConfig config;
    FirebaseStream stream;

    const int NTP_MAX_ATTEMPTS = 10;
    unsigned long lastTokenRefreshTime = 0;
};
