# 🌊⚡ PCH — Sistema de Monitoreo y Control con ESP32

> Monitorea **niveles** y **caudales** en una **pequeña central hidroeléctrica (PCH)**, decide cuántos **generadores** activar y publica los datos **localmente** y en la **nube (Firebase)**.

---

## 🚀 Resumen rápido (TL;DR)

* **Sensores**: 3 ultrasónicos (niveles) + 3 caudalímetros (flujo).
* **Actuadores**: 1 servomotor para **compuerta** + 3 **salidas digitales**.
* **Cada ~1 s**: lee → calcula → decide generadores → muestra/serial → envía a Firebase.
* **Conexión**: WiFi (hora NTP) + Firebase Realtime Database.
* **Serie**: 115200 baudios.

---

## 🧩 Estructura del proyecto

```
/src (o raíz del sketch)
├─ PCH.ino (antes PCH.txt)  ← Sketch principal
├─ Datos.h                   ← Definición de variables y arreglo `data[]`
├─ Caudalimetro.h/.cpp       ← Sensor de caudal por pulsos (con ISR)
├─ Ultrasonico.h/.cpp        ← Sensor de nivel + cálculo de caudal (Manning)
├─ Pulsador.h/.cpp           ← Entradas con anti-rebote y callbacks
├─ Actuador_digital.h/.cpp   ← Salidas digitales ON/OFF
├─ Motor.h/.cpp              ← Servo que mueve la compuerta (posiciones)
├─ PantallaCustom.h/.cpp     ← Interfaz TFT para mostrar 3 datos por lado
├─ Web.h/.cpp                ← WiFi + NTP + Firebase
├─ Conexiones.h              ← Mapa de pines ESP32
├─ secrets.h                 ← Declaraciones de credenciales
└─ secrets.cpp               ← **Definiciones** de credenciales (lo creas tú)
```

---

## 🔌 Mapa de pines (ESP32)

### Sensores de caudal (entradas con interrupción)

| Sensor  |            Pin                  |
| ------- | -------------------             |
| CAUD-0  | **GPIO32** (`PCH_TOUCH9`)       |
| CAUD-1  | **GPIO33** (`PCH_TOUCH8`)       |
| CAUD-2  | **GPIO34** (`PCH_INPUT_ONLY_0`) |

### Sensores ultrasónicos (TRIG salida, ECHO entrada)

| Sensor |                    TRIG |       ECHO |
| ------ | ----------------------: | ---------: |
| US-0   | **GPIO25** (`PCH_DAC0`) | **GPIO35** (`PCH_INPUT_ONLY_1`)|
| US-1   | **GPIO26** (`PCH_DAC1`) | **GPIO36** (`PCH_INPUT_ONLY_2`)|
| US-2   | **GPIO16** (`PCH_UART2_RX`) | **GPIO39** (`PCH_INPUT_ONLY_3`)|

### Actuadores

| Dispositivo        |                       Pin |
| ------------------ | ------------------------: |
| Compuerta (servo)  | **GPIO13** (`PCH_TOUCH4`) |
| Actuador digital 0 | **GPIO12** (`PCH_TOUCH5`) |
| Actuador digital 1 | **GPIO14** (`PCH_TOUCH6`) |
| Actuador digital 2 | **GPIO17** (`PCH_UART2_TX`) |

### Pulsadores

| Pulsador |                       Pin |
| -------- | ------------------------: |
| P0       |  **GPIO4** (`PCH_TOUCH0`) |
| P1       |  **GPIO2** (`PCH_TOUCH2`) |
| P2       | **GPIO15** (`PCH_TOUCH3`) |
| P3       | **GPIO27** (`PCH_TOUCH7`) |

### SPI (pantalla local)

|Pin de la pantalla| Pin |
| ----- | ---------: |
| CS  | **GPIO23** (`PCH_VSPI_SS`) |
| DC  | **GPIO18** (`PCH_I2C_SDA`)|
| RST |  **GPIO5** (`PCH_I2C_SCL`)|

> ℹ️ El archivo define también aliases para buses UART/I2C y pines “input only”. Evita usar los pines de **Flash** (GPIO6–10).

---

## 🧠 Lógica del sistema

1. **Lectura**: ultrasonidos → *nivel* y *caudal estimado* (Manning); caudalímetros → *flujo* por pulsos.
2. **Procesamiento**: se llenan las entradas del arreglo `data[]` (ver tabla más abajo).
3. **Decisión**: en función del **Caudal Turbinable**, se recomiendan **0–3 generadores**.
4. **Salida**:

   * **Serial**: imprime etiquetas, valores y unidades.
   * **Pantalla TFT**: interfaz `PantallaCustom::actualizar(...)`.
   * **Firebase**: `Web::enviar(...)` sube `data[]` a `/sensorData/...`.
   * **Actuación**: servo **compuerta** y **actuadores digitales**, además de 4 **pulsadores** con callbacks.

---

## 🗃️ Modelo de datos (Firebase + Serial)

Las mediciones se definen en `Datos.h` y se envían a Firebase bajo `/sensorData/<clave>`.

| ID (enum)            | Etiqueta              | Clave Firebase       | Unidad  |
| -------------------- | --------------------- | -------------------- | ------- |
| `caudalRio`              | Caudal del río                 | `caudalRio`                | m³/s   |
| `caudalCaptacion`        | Caudal de captación            | `caudalCaptacion`          | m³/s   |
| `caudalNoCaptado`        | Caudal no captado              | `caudalNoCaptado`          | m³/s   |
| `caudalGarantíaAmbiental`| Caudal de garantía ambiental   | `caudalGarantíaAmbiental`  | m³/s   |
| `caudalAduccion`         | Caudal de aducción             | `caudalAduccion`           | m³/s   |
| `caudalTurbinable`       | Caudal turbinable              | `caudalTurbinable`         | m³/s   |
| `caudalDevuelto`         | Caudal devuelto                | `caudalDevuelto`           | m³/s   |
| `caudalRetorno`          | Caudal de retorno              | `caudalRetorno`            | m³/s   |
| `cotaCaptacion`          | Cota en captación              | `cotaCaptacion`            | msnm   |
| `cotaRio`                | Cota del río                   | `cotaRio`                  | msnm   |
| `cotaAduccion`           | Cota en aducción               | `cotaAduccion`             | msnm   |
| `cotaGarantíaAmbiental`  | Cota de garantía ambiental     | `cotaGarantíaAmbiental`    | msnm   |
| `generadoresActivos`     | Generadores activos            | `generadoresActivos`       |(texto) |

> En serie, `GeneradoresActivos` se muestra como texto: **Apagados | 1 encendido | 2 encendidos | 2 a máxima capacidad | Error**.

---

## 🛠️ Requisitos e instalación

### Arduino IDE

1. **Placa**: instala *ESP32 by Espressif Systems* (Gestor de tarjetas).
2. **Librerías**:

   * `Firebase_ESP_Client` [Firebase Arduino Client Library for ESP8266 and ESP32 (autor: Mobizt)](https://github.com/mobizt/Firebase-ESP-Client)
   * `ESP32Servo`
   * `Adafruit_GFX` [Adafruit GFX Library (autor: Adafruit)](https://github.com/adafruit/Adafruit-GFX-Library)
   * `Adafruit_ILI9341` [Adafruit ILI9341 (autor: Adafruit)](https://github.com/adafruit/Adafruit-GFX-Library)
3. **Abrir** el sketch principal (`PCH.ino`).
4. **Velocidad Serial**: 115200.
5. **Compila y sube**.

### PlatformIO (VS Code)

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
  mobizt/Firebase Arduino Client Library for ESP8266 and ESP32
  arduino-libraries/Servo
  adafruit/Adafruit GFX Library
  adafruit/Adafruit ILI9341
```

---

## 🔐 Configuración de credenciales

En el repo ya está `secrets.h` con **declaraciones**. Debes crear **`secrets.cpp`** con las **definiciones** reales.

```cpp
// secrets.cpp
#include "secrets.h"

const char WIFI_SSID[]     = "TU_SSID";
const char WIFI_PASSWORD[] = "TU_PASSWORD";
const char key[]           = "TU_API_KEY_FIREBASE";
const char url[]           = "https://TU_PROYECTO.firebaseio.com";
const char email[]         = "usuario@ejemplo.com";
const char password[]      = "tu_password";
```

---

## ⚖️ Calibración y parámetros

### Caudalímetros

* ISR que cuenta pulsos, período de cálculo: **2000 ms**.
* Factor de calibración en `Caudalimetro.h` (**`FLOW_CALIBRATION_FACTOR`**).

### Ultrasónicos

Constructor:
`ultrasonico(byte trig, byte echo, int c, float techo, float piso, float ancho, float sqrtPendiente)`

* **techo/piso**: límites de nivel (m).
* **ancho**: del canal (m).
* **sqrtPendiente**: `sqrt(S)` de Manning.

---

## ▶️ Uso básico

1. Conecta el ESP32 y abre el **Monitor Serie** (115200).
2. Verás los mensajes de **WiFi**, **hora** y **Firebase**.
3. Cada ~1 s se imprimirán cotas, caudales y recomendación de generadores.
4. La **pantalla TFT** y Firebase reflejarán los mismos datos.

---

## 🧯 Solución de problemas

* **No compila**:

  * Asegúrate de tener `secrets.cpp` y librerías.
  * Si ves errores con `datos`, confirma `using datos = dato;` en `Datos.h`.
* **Firebase no conecta**:

  * Verifica `key`, `url`, `email/password` y la hora NTP.
* **Lecturas ultrasónicas inestables**:

  * Revisa cableado y fuente de poder.
  * Ajusta `timeout_us` en `reading()`.
* **Pulsadores no responden**:

  * Confirma lógica **LOW** = presionado y `INPUT_PULLUP`.

---

## 🧭 Roadmap corto

* Implementar control remoto de actuadores vía **Firebase stream**.
* Endpoint web/app para visualizar `/sensorData` en tiempo real.
* Hacer **pantalla TFT** más interactiva con gráficos y estados o animaciones.

---

## 📌 Sugerencias de ChatGPT

* **Documentación viva**: mantener README con cambios reales de hardware/software.
* **Tests de campo**: comparar lecturas con mediciones manuales.
* **Escalabilidad**: pensar en MQTT además de Firebase.
* **UX**: añadir menús básicos en la pantalla TFT.
* **Mantenimiento**: dividir lógicas largas (ej. `loop`) en funciones pequeñas.

---

## 📜 Licencia

Uso académico y demostrativo. Úsalo con criterio y **bajo tu responsabilidad**.

---

## 🤝 Contribuir

* Issues y PRs son bienvenidos.
* Describe hardware (placa, sensores) y logs al reportar problemas.
