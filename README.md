# 🌊⚡ PCH — Sistema de Monitoreo y Control con ESP32

> Monitorea **niveles** y **caudales** en una **pequeña central hidroeléctrica (PCH)**, decide cuántos **generadores** activar y publica los datos **localmente** y en la **nube (Firebase)**.

![Build](https://github.com/ketier09/PCH/actions/workflows/build.yml/badge.svg)

---

## 🚀 Resumen rápido

* **Sensores**: 3 ultrasónicos (niveles) + 3 caudalímetros (flujo).
* **Actuadores**: 1 servomotor para **compuerta** + 3 **salidas digitales**.
* **Cada ~1 s**: lee → calcula → decide generadores → muestra/serial → envía a Firebase.
* **Conexión**: WiFi (hora NTP) + Firebase Realtime Database.
* **Serie**: 115200 baudios.

---

## 🧩 Estructura del proyecto

```
/src (o raíz del sketch)
├─ PCH-_.ino                 ← Sketch principal
├─ Datos.h                   ← Definición de variables y arreglo `data[]`
├─ Caudalimetro.h/.cpp       ← Sensor de caudal por pulsos (con ISR)
├─ Ultrasonico.h/.cpp        ← Sensor de nivel + cálculo de caudal (Manning)
├─ Pulsador.h/.cpp           ← Entradas con anti-rebote y callbacks
├─ Actuador_digital.h/.cpp   ← Salidas digitales ON/OFF
├─ Motor.h/.cpp              ← Servo que mueve la compuerta (posiciones)
├─ PantallaCustom.h/.cpp     ← Interfaz TFT para mostrar 3 datos por lado
├─ Images.h/.cpp             ← Imágenes que serán usadas en la pantalla
├─ Web.h/.cpp                ← NTP + Firebase
├─ WiFiConfigManager.h/.cpp  ← WiFi
├─ Conexiones.h              ← Mapa de pines ESP32
├─ Secrets.h                 ← Declaraciones de credenciales
└─ Secrets.cpp               ← **Definiciones** de credenciales (lo creas tú)
```

---
# 🌐 `WiFiConfigManager`: Gestión de Conexión WiFi en ESP32

El módulo `WiFiConfigManager` es la herramienta central que permite a nuestro sistema **ESP32** conectarse de manera persistente a una red WiFi, sin necesidad de reprogramar las credenciales. Si no encuentra una red guardada o falla la conexión, automáticamente inicia un **Portal de Configuración** para que puedas introducir las credenciales de forma inalámbrica.

---

## 🚀 Flujo de Conexión

Al iniciar el dispositivo, el módulo sigue esta secuencia lógica:

1.  **Carga de Credenciales (Modo STA):**
    * Intenta leer las credenciales (`SSID` y `Password`) guardadas en el sistema de archivos **LittleFS** (o SPIFFS) en el archivo `/wifi.json`.
    * Si las encuentra, intenta conectarse al WiFi con esas credenciales.
2.  **Conexión Exitosa:**
    * El dispositivo se conecta (`WL_CONNECTED`) y continúa con la lógica principal (sincronización NTP, Firebase, etc.).
3.  **Conexión Fallida / Sin Credenciales:**
    * Si falla la conexión después de 10 segundos, o si no hay credenciales, el dispositivo entra en **Modo Punto de Acceso (AP)**.
    * Se inicia el **Portal de Configuración** (`startConfigPortal()`).

---

## 🛠️ Modo de Configuración (Portal Web)

Si el ESP32 no logra conectarse, se convertirá en su propia red WiFi para que puedas configurarlo desde tu teléfono o computador:

| Parámetro | Valor |
| :--- | :--- |
| **Nombre de la Red (SSID)** | `ESP_Config` |
| **Contraseña de la Red** | *Sin contraseña* |
| **Dirección IP del Portal** | `http://192.168.4.1` (IP por defecto del ESP32 en modo AP) |

### Pasos para configurar el WiFi:

1.  **Desconecta** tu dispositivo (PC, móvil) de tu WiFi normal.
2.  Busca y conéctate a la red WiFi llamada **`ESP_Config`**.
3.  Abre un navegador y navega a la dirección **`http://192.168.4.1`**.
4.  Aparecerá la página de configuración para ingresar el **SSID** y la **Contraseña** de tu red.
5.  Pulsa **"Guardar"**.
6.  El ESP32 guardará las credenciales y se **reiniciará** (`ESP.restart()`) para intentar conectarse con la nueva configuración.

---

## 💾 Detalle Técnico (Para Desarrolladores)

* **Persistencia:** Utiliza la librería **`LittleFS`** (o SPIFFS) para guardar las credenciales, asegurando que persistan incluso después de reinicios.
* **Servidor Web:** Utiliza la librería **`WebServer`** para levantar el portal de configuración en el puerto 80.
* **Rutas de manejo:**
    * `/`: Muestra el formulario HTML (`handleRoot()`).
    * `/save`: Recibe las credenciales por método POST, las guarda en `wifi.json` y reinicia el dispositivo (`handleSave()`).
* **Desconexión:** Si el usuario presiona el botón de reinicio (físico o programado), se puede optar por borrar las credenciales para forzar el portal en el próximo inicio (`eraseCredentials()` no se usa en el flujo principal, pero está disponible).

---

## 🔌 Mapa de pines (ESP32)

### Sensores de caudal (entradas con interrupción)

| Sensor | Pin                        |
|:-------|:----------------------------|
| CAUD-0 | **GPIO32** (`PCH_TOUCH9`)   Entrada|
| CAUD-1 | **GPIO33** (`PCH_TOUCH8`)   Salida|
| CAUD-2 | **GPIO34** (`PCH_INPUT_ONLY_0`) Sara|

---

### Sensores ultrasónicos (TRIG salida, ECHO entrada)

| Sensor | TRIG                         | ECHO                         |
|:-------|:-----------------------------|:------------------------------|
| US-0   | **GPIO25** (`PCH_DAC0`)      | **GPIO35** (`PCH_INPUT_ONLY_1`) Captación|
| US-1   | **GPIO26** (`PCH_DAC1`)      | **GPIO36** (`PCH_VP`) Garantía|

---

### Actuadores

| Dispositivo         | Pin                        |
|:--------------------|:---------------------------|
| Compuerta (servo)   | **GPIO13** (`PCH_TOUCH4`)  Compuerta|
| Actuador digital 0  | **GPIO12** (`PCH_TOUCH5`)  Motobomba sara|
| Actuador digital 1  | **GPIO14** (`PCH_TOUCH6`)  el led|

---

### Pulsadores

| Pulsador | Pin                      |
|:----------|:-------------------------|
| P0        | **GPIO27**  (`PCH_TOUCH0`) |

---

### SPI (pantalla local)

| Pin de la pantalla | Pin                     |
|:--------------------|:------------------------|
| SDI/MOSI   | **GPIO23** (`PCH_SDI`) |
| SCK   | **GPIO18** (`PCH_SCK`) |
| CS       | **GPIO5** |
 |     RST      | GPIO4|
| DC  | **GPIO2**  (``) |


> ℹ️ Evita usar los pines de **Flash** (GPIO6–10).

---

## 🧠 Lógica del sistema

1. **Lectura**: ultrasonidos → *nivel* y *caudal estimado* (Manning); caudalímetros → *flujo* por pulsos.
2. **Procesamiento**: se llenan las entradas del arreglo `data[]` (ver tabla más abajo).
3. **Decisión**: en función del **Caudal Turbinable**, se recomiendan **0–3 generadores**.
4. **Salida**:

   * **Serial**: imprime etiquetas, valores y unidades.
   * **Pantalla TFT**: interfaz `PantallaCustom::actualizar(...)`.
   * **Firebase**: `web::enviar(...)` sube `data[]` a `/sensorData/...`.
   * **Actuación**: servo **compuerta** y **actuadores digitales**, además de 4 **pulsadores** con callbacks.

---

## 🗃️ Modelo de datos (Firebase + Serial)

Las mediciones se definen en `Datos.h` y se envían a Firebase bajo `/sensorData/<clave>`.

| ID (enum)            | Etiqueta              | Clave Firebase       | Unidad  |
| -------------------- | --------------------- | -------------------- | ------- |
| `caudalRio`                      | Caudal del río                 | `caudalRio`                        | m³/s   |
| `caudalCaptacion`                | Caudal de captación            | `caudalCaptacion`                  | m³/s   |
| `caudalNoCaptado`                | Caudal no captado              | `caudalNoCaptado`                  | m³/s   |
| `caudalGarantíaAmbiental`        | Caudal de garantía ambiental   | `caudalGarantíaAmbiental`          | m³/s   |
| `caudalAduccion`                 | Caudal de aducción             | `caudalAduccion`                   | m³/s   |
| `caudalTurbinable`               | Caudal turbinable              | `caudalTurbinable`                 | m³/s   |
| `caudalDevuelto`                 | Caudal devuelto                | `caudalDevuelto`                   | m³/s   |
| `caudalRetorno`                  | Caudal de retorno              | `caudalRetorno`                    | m³/s   |
| `cotaCaptacion`                  | Cota en captación              | `cotaCaptacion`                    | msnm   |
| `cotaRio`                        | Cota del río                   | `cotaRio`                          | msnm   |
| `cotaAduccion`                   | Cota en aducción               | `cotaAduccion`                     | msnm   |
| `cotaGarantíaAmbiental`          | Cota de garantía ambiental     | `cotaGarantíaAmbiental`            | msnm   |
| `cantidadGeneradoresActivos`     | Generadores activos            | `cantidadGeneradoresActivos`       | texto  |

> En serie, `GeneradoresActivos` se muestra como texto: **Apagados | 1 encendido | 2 encendidos | 2 a máxima capacidad | Error**.

---

## 🛠️ Requisitos e instalación

### Arduino IDE

1. **Placa**: instala *ESP32 by Espressif Systems* (Gestor de tarjetas).
2. **Librerías**:

  * `Firebase_ESP_Client` [Firebase Arduino Client Library for ESP8266 and ESP32 (autor: Mobizt)](https://github.com/mobizt/Firebase-ESP-Client)
  * `ESP32Servo` [ESP32Servo (autor: Kevin Harrington, John K. Bennett)](https://github.com/madhephaestus/ESP32Servo)
  * `Adafruit_GFX` [Adafruit GFX Library (autor: Adafruit)](https://github.com/adafruit/Adafruit-GFX-Library)
  * `Adafruit_ILI9341` [Adafruit ILI9341 (autor: Adafruit)](https://github.com/adafruit/Adafruit_ILI9341)
  * `LittleFS` [LittleFS for ESP32 (autor: lorol)](https://github.com/lorol/LITTLEFS)
  * `ArduinoJson` [ArduinoJson (autor: Benoît Blanchon)](https://github.com/bblanchon/ArduinoJson)
   
4. **Abrir** el sketch principal (`PCH.ino`).
5. **Velocidad Serial**: 115200.
6. **Compila y sube**.

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

En el repo ya está `Secrets.h` con **declaraciones**. Debes crear **`Secrets.cpp`** con las **definiciones** reales.

```cpp
// Secrets.cpp
#include "Secrets.h"

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

  * Asegúrate de tener `Secrets.cpp` y librerías.
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
