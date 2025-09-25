# 🌊⚡ PCH — Sistema de Monitoreo y Control con ESP32

> Monitorea **niveles** y **caudales** en una **pequeña central hidroeléctrica (PCH)**, decide cuántos **generadores** activar y publica los datos **localmente** y en la **nube (Firebase)**.

---

## 🚀 Resumen rápido (TL;DR)
- **Sensores**: 3 ultrasónicos (niveles) + 3 caudalímetros (flujo).
- **Actuadores**: 1 servomotor para **compuerta** + 3 **salidas digitales**.
- **Cada ~1 s**: lee → calcula → decide generadores → muestra/serial → envía a Firebase.
- **Conexión**: WiFi (hora NTP) + Firebase Realtime Database.
- **Serie**: 115200 baudios.

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
├─ Pantalla.h/.cpp           ← Interfaz para mostrar 3 datos (stub)
├─ Web.h/.cpp                ← WiFi + NTP + Firebase
├─ secrets.h                 ← Declaraciones de credenciales
└─ secrets.cpp               ← **Definiciones** de credenciales (lo creas tú)
```

> ✅ El tipo `dato` es el registro de cada medición. Existe alias `using datos = dato;` para compatibilidad.

---

## 🔌 Mapa de pines (ESP32)

### Sensores de caudal (entradas con interrupción)
| Nombre lógico | Pin | Comentario |
|---|---:|---|
| `PIN_CAUD_0` | **GPIO32** | `PIN_TOUCH9` |
| `PIN_CAUD_1` | **GPIO33** | `PIN_TOUCH8` |
| `PIN_CAUD_2` | **GPIO34** | `INPUT ONLY` |

### Sensores ultrasónicos (TRIG salida, ECHO entrada)
| Sensor | TRIG | ECHO |
|---|---:|---:|
| US-0 | **GPIO25** (`PIN_DAC0`) | **GPIO35** |
| US-1 | **GPIO26** (`PIN_DAC1`) | **GPIO36** |
| US-2 | **GPIO16** (`UART2_RX`) | **GPIO39** |

### Actuadores
| Dispositivo | Pin |
|---|---:|
| Compuerta (servo) | **GPIO13** (`PIN_TOUCH4`) |
| Actuador digital 0 | **GPIO12** (`PIN_TOUCH5`) |
| Actuador digital 1 | **GPIO14** (`PIN_TOUCH6`) |
| Actuador digital 2 | **GPIO17** (`UART2_TX`) |

### Pulsadores
| Pulsador | Pin |
|---|---:|
| P0 | **GPIO4** (`PIN_TOUCH0`) |
| P1 | **GPIO2** (`PIN_TOUCH2`) |
| P2 | **GPIO15** (`PIN_TOUCH3`) |
| P3 | **GPIO27** (`PIN_TOUCH7`) |

### SPI (pantalla local)
| Señal | Pin |
|---|---:|
| MOSI | **GPIO23** |
| SCK  | **GPIO18** |
| SS   | **GPIO5**  |

> ℹ️ El archivo define también aliases para buses UART/I2C y pines “input only”. Evita usar los pines de **Flash** (GPIO6–10).

---

## 🧠 Lógica del sistema
1. **Lectura**: ultrasonidos → *nivel* y *caudal estimado* (Manning); caudalímetros → *flujo* por pulsos.
2. **Procesamiento**: se llenan las entradas del arreglo `data[]` (ver tabla más abajo).
3. **Decisión**: en función del **Caudal Turbinable**, se recomiendan **0–3 generadores**.
4. **Salida**:
   - **Serial**: imprime etiquetas, valores y unidades.
   - **Pantalla**: interfaz `Pantalla::enviar(...)` (listo para implementar).
   - **Firebase**: `Web::enviar(...)` sube `data[]` a `/sensorData/...`.
   - **Actuación**: servo **compuerta** y **actuadores digitales**, además de 4 **pulsadores** con callbacks.

---

## 🗃️ Modelo de datos (Firebase + Serial)
Las mediciones se definen en `Datos.h` y se envían a Firebase bajo `/sensorData/<clave>`.  

| ID (enum) | Etiqueta | Clave Firebase | Unidad |
|---|---|---|---|
| `CotaCaptacion` | Cota en captación | `cotaCaptacion` | msnm |
| `CotaRio` | Cota del río | `cotaRio` | msnm |
| `CotaDesarenador` | Cota en desarenador | `cotaDesarenador` | msnm |
| `CaudalCaptacion` | Caudal en captación | `caudalCaptacion` | m³/s |
| `CaudalDesarenador` | Caudal en desarenador | `caudalDesarenador` | m³/s |
| `CaudalInicio` | Caudal inicio | `caudalInicio` | m³/s |
| `CaudalTurbinable` | Caudal turbinable | `caudalTurbinable` | m³/s |
| `CaudalFinal` | Caudal final | `caudalFinal` | m³/s |
| `GeneradoresActivos` | Generadores activos | `generadoresActivos` | (texto) |

> En serie, `GeneradoresActivos` se muestra como texto: **Apagados | 1 encendido | 2 encendidos | 2 a máxima capacidad | Error**.

---

## 🛠️ Requisitos e instalación

### Arduino IDE
1. **Placa**: instala *ESP32 by Espressif Systems* (Gestor de tarjetas) y selecciona tu módulo.
2. **Librerías**:
   - `Firebase_ESP_Client`
   - `Servo`
3. **Abrir** el sketch principal (`PCH.ino`). Si tu archivo se llama `PCH.txt`, **renómbralo a `PCH.ino`**.
4. **Velocidad Serial**: 115200.
5. **Compila y sube**.

### PlatformIO (VS Code)
- `platformio.ini` típico:
  ```ini
  [env:esp32dev]
  platform = espressif32
  board = esp32dev
  framework = arduino
  monitor_speed = 115200
  lib_deps =
    mobizt/Firebase Arduino Client Library for ESP8266 and ESP32
    arduino-libraries/Servo
  ```

---

## 🔐 Configuración de credenciales
En el repo ya está `secrets.h` con **declaraciones**. Debes crear **`secrets.cpp`** con las **definiciones** reales:

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

> La primera conexión **sincroniza hora NTP** y luego inicializa Firebase. Si falla, el código reintenta.

---

## ⚖️ Calibración y parámetros

### Caudalímetros
- Definidos con una **ISR** que cuenta pulsos. Período de cálculo por defecto: **2000 ms**.
- Factor de calibración (pulsos ↔ volumen) ajustable en `Caudalimetro.h` (**`FLOW_CALIBRATION_FACTOR`**).

### Ultrasónicos
Constructor:  
`ultrasonico(byte trig, byte echo, int c, float techo, float piso, float ancho, float sqrtPendiente)`

- **techo/piso**: límites de nivel en metros.
- **ancho**: del canal (m).
- **sqrtPendiente**: `sqrt(S)` para fórmula de Manning (ajustar a tu canal).
- La función `flujo()` usa **Manning** -> `Q = v·A`, con `v ∝ R^{2/3}·sqrt(S)`.

> Empieza con valores seguros y **ajusta en campo** midiendo contra referencias (regla en canal, medidor patrón, etc.).

---

## ▶️ Uso básico
1. Alimenta el ESP32 y **abre el Monitor Serie** (115200).
2. Verás los mensajes de **WiFi**, **hora** y **Firebase**.
3. Cada ~1 s se imprimirán las **cotas**, **caudales** y **recomendación de generadores**.
4. Conecta la **pantalla** y/o lee la **base de datos** en `/sensorData/...` para visualizar remotamente.

---

## 🧯 Solución de problemas
- **No compila**:
  - Asegúrate de tener `secrets.cpp` y las librerías listadas.
  - Si ves errores con el tipo `datos`, confirma que en `Datos.h` exista `using datos = dato;`.
- **Firebase no conecta**:
  - Verifica `key`, `url`, `email/password` y la **hora NTP** (la sincroniza `Web::syncTime()`).
- **Lecturas inestables (ultrasónico)**:
  - Revisa alimentación y cableado.
  - Ajusta `timeout_us` en `reading()` si trabajas con distancias mayores.
- **Pulsadores no responden**:
  - Confirma lógica **LOW** como “presionado” y el `INPUT_PULLUP` en `Pulsador::set_up()`.

---

## 🧭 Roadmap corto
- Implementar `Pantalla::enviar(...)` para mostrar 3 métricas clave.
- Añadir stream/listeners Firebase para **control remoto** de actuadores.
- Endpoint web/app sencilla que lea `/sensorData` en tiempo real.

---

## 📜 Licencia
Uso académico y demostrativo. Úsalo con criterio y **bajo tu responsabilidad** en instalaciones reales.

---

## 🤝 Contribuir
- Issues y PRs son bienvenidos.
- Por favor, describe hardware (placa, sensores) y logs de serie al reportar problemas.

---

¡Listo! Si quieres, puedo **adaptar** este README a tu formato de repositorio (Badges, CI, capturas de pantalla) o **generar una guía rápida de despliegue en campo**.
