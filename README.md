# 🌊⚡ PCH — Sistema de Monitoreo y Control con ESP32

> Monitorea **niveles** y **caudales** en una **pequeña central hidroeléctrica (PCH)**, decide cuántos **generadores** activar y publica los datos **localmente** y en la **nube (Firestore)**.

---

## 🚀 Resumen rápido

* **Sensores**: **2 ultrasónicos** (niveles) + **1 caudalímetro** (flujo).
* **Actuadores**: 1 servomotor para **compuerta** + **1 salida digital (relé)**.
* **Cada ~1 s**: lee → calcula → decide generadores → muestra/serial → envía a Firestore.
* **Conexión**: WiFi (hora NTP) + Google Firestore.
* **Serie**: 115200 baudios.

> Notas relacionadas:
> * La **estética del portal WiFi** se comenta en la **Issue #20 – “Estética de ingreso de credenciales”**.
> * La **conexión con Firestore** y posibles simplificaciones de código se discuten en la **Issue #21 – “¿Qué pasa en Firestore?”**.
> * El comportamiento del **envío de órdenes desde Firestore** hacia la compuerta se documenta en la **Issue #28 – “Firestore no envía órdenes”**.

---

## 🧩 Estructura del proyecto

```text
/src (o raíz del sketch)
│
├─ PCH-_.ino
│    ← Sketch principal
│
├─ Datos.h
├─ Datos.cpp
│    ← Definición de variables y arreglo data[]
│
├─ Conexiones.h
│    ← Definiciones de pines (enum PCH_Pines)
│
├─ Secrets.h
│    ← Credenciales de Firestore y WiFi (no se sube a Git)
│
├─ Caudalimetro.h
├─ Caudalimetro.cpp
│    ← Sensor de caudal por pulsos (con ISR)
│
├─ Ultrasonico.h
├─ Ultrasonico.cpp
│    ← Sensor de nivel + cálculo de caudal (Manning)
│
├─ Pulsador.h
├─ Pulsador.cpp
│    ← Entradas con anti-rebote y callbacks
│
├─ Actuador_digital.h
├─ Actuador_digital.cpp
│    ← Actuador digital (relé) para encender/apagar generadores
│       Ver Issue #27 – “El actuador digital es un relé”
│
├─ Motor.h
├─ Motor.cpp
│    ← Servomotor para la compuerta
│
├─ RGBLed.h
├─ RGBLed.cpp
│    ← LED RGB para indicar estados o número de generadores activos
│       Ver Issue #24 – “Colores del Led”
│
├─ Images.h
├─ Images.cpp
│    ← Imágenes para la pantalla TFT (uso no prioritario)
│       Ver Issue #25 – “Simulación”
│
├─ PantallaCustom.h
├─ PantallaCustom.cpp
│    ← Manejo de la pantalla ILI9341
│       Problemas de espacio de texto: Issue #26 – “El texto de la pantalla se corta”
│
├─ WiFiConfigManager.h
├─ WiFiConfigManager.cpp
│    ← Portal de configuración WiFi (AP)
│
├─ Web.h
├─ Web.cpp
│    ← Funciones de conexión a Firestore y hora NTP
│       Código objeto de limpieza: Issue #21 – “¿Qué pasa en Firestore?”
```
Además existe un archivo de simulación en Proteus que solo sirve como referencia visual de conexiones, comentado en la Issue #25 – “Simulación”; no es necesario para compilar ni ejecutar el proyecto real.
---
## 🌐 `WiFiConfigManager`: Gestión de Conexión WiFi en ESP32

El módulo `WiFiConfigManager` es la herramienta central que permite a nuestro sistema ESP32 conectarse de manera persistente a una red WiFi, sin necesidad de reprogramar las credenciales. Si no encuentra una red guardada o falla la conexión, automáticamente inicia un Portal de Configuración para que puedas introducir las credenciales de forma inalámbrica.

La versión actual utiliza páginas HTML muy sencillas; en la Issue #20 se deja constancia de que la estética de estas páginas puede mejorarse en el futuro (tipografía, estilos, mensajes al usuario), pero por ahora se prioriza la funcionalidad.

---

## 🚀 Flujo de Conexión

Al iniciar el dispositivo, el módulo sigue esta secuencia lógica:

1. **Carga de Credenciales (Modo STA):**
    * Intenta leer las credenciales (`SSID` y `Password`) guardadas en el sistema de archivos **LittleFS** (o SPIFFS) en el archivo `/wifi.json`.
    * Si las encuentra, intenta conectarse al WiFi con esas credenciales.
2. **Conexión Exitosa:**
    * El dispositivo se conecta (`WL_CONNECTED`) y continúa con la lógica principal (sincronización NTP, Firestore, etc.).
3. **Conexión Fallida / Sin Credenciales:**
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

1. **Desconecta** tu dispositivo (PC, móvil) de tu WiFi normal.
2. Busca y conéctate a la red WiFi llamada **`ESP_Config`**.
3. Abre un navegador y navega a la dirección **`http://192.168.4.1`**.
4. Aparecerá la página de configuración para ingresar el **SSID** y la **Contraseña** de tu red.
5. Pulsa **"Guardar"**.
6. El ESP32 guardará las credenciales y se **reiniciará** (`ESP.restart()`) para intentar conectarse con la nueva configuración.

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

Para más detalles revisa [el diagrama de conexiones](https://github.com/ketier09/PCH/blob/_/DiagramaDeConexiones.pdf)

### Sensores de caudal (entradas con interrupción)

| Sensor | Pin | Descripción |
|:-------|:----|:------------|
| CAUD-0 | **D34** | Caudal de **Generación** |

---

### Sensores ultrasónicos (TRIG salida, ECHO entrada)

| Sensor | TRIG | ECHO | Descripción |
|:-------|:-----|:-----|:------------|
| US-0 | **D18** | **D19** | Nivel de **Captación** |
| US-1 | **D25** | **D26** | Nivel de **Ingreso** |

---

### Actuadores

| Dispositivo | Pin | Descripción |
|:--------------------|:-----------|:---------------|
| Compuerta (servo) | **D13** | Compuerta |
| Actuador digital 0 | **D12** | Generador 1 / Motobomba |

La nomenclatura de este último componente se discute en detalle en la Issue #27, donde se propone llamar al módulo simplemente “Relé” en lugar de “Actuador digital”.

---

### Pulsadores

| Pulsador | Pin | Descripción |
|:-------- |:----|:---------------|
| P0 | **D39** | Forzar apagado de generadores |
| P1 | **D36** | Ciclar estados de la compuerta |

---

### SPI (pantalla local - ILI9341)

| Función | Pin |
|:------------------ |:-----|
| CS | **D5** |
| RST | **D4** |
| DC | **D2** |
| SCK | **D14** |
| SDI/MOSI | **D27** |

---

## 🧠 Lógica del sistema

1. **Lectura**: ultrasonidos → *nivel* y *caudal estimado* (Manning); caudalímetros → *flujo* por pulsos.
2. **Procesamiento**: se llenan las entradas del arreglo `data[]` (ver tabla más abajo).
3. **Decisión**: en función del **Caudal Turbinable**, se recomienda **1 generador activo** (la lógica en el código solo maneja un actuador digital).
4. **Salida**:

    * **Serial**: imprime etiquetas, valores y unidades.
    * **Pantalla TFT**: interfaz `PantallaCustom::actualizar(...)`.
    * **Firestore**: `web::enviar(...)` sube `data[]` a `/sensorData/...`.
    * **Actuación**: servo **compuerta** y **actuador digital**, además de 2 **pulsadores** con callbacks.

---

## 🗃️ Modelo de datos (Firestore + Serial)

Las mediciones se definen en `Datos.h` y se envían a Firestore bajo `/sensorData/<clave>`.

| ID (enum) | Etiqueta | Clave Firestore | Unidad |
| :--- | :--- | :--- | :--- |
| `caudalGeneracion` | Caudal Generación | `caudalGeneracion` | m³/s |
| `caudalIngreso` | Caudal Ingreso | `caudalIngreso` | m³/s |
| `caudalCaptacion` | Caudal Captación | `caudalCaptacion` | m³/s |
| `caudalGarantia` | Caudal Garantía | `caudalGarantia` | m³/s |
| `cotaGeneracion` | Cota Generación | `cotaGeneracion` | msnm |
| `cotaIngreso` | Cota Ingreso | `cotaIngreso` | msnm |
| `cotaCaptacion` | Cota Captación | `cotaCaptacion` | msnm |
| `cotaGarantia` | Cota Garantía | `cotaGarantia` | msnm |
| `cantidadGeneradoresActivos` | Generadores activos | `cantidadGeneradoresActivos` | |

> En serie, `GeneradoresActivos` se muestra como texto: **Apagados | 1 encendido | 2 encendidos | 2 a máxima capacidad | Error**.

---

## 🛠️ Requisitos e instalación

### Arduino IDE

1. **Placa**: instala *ESP32 by Espressif Systems* (Gestor de tarjetas).
2. **Librerías**:
    * `Firestore_ESP_Client` [Firestore Arduino Client Library for ESP8266 and ESP32 (autor: Mobizt)]
    * `ESP32Servo` [ESP32Servo (autor: Kevin Harrington, John K. Bennett)]
    * `Adafruit_GFX` [Adafruit GFX Library (autor: Adafruit)]
    * `Adafruit_ILI9341` [Adafruit ILI9341 (autor: Adafruit)]
    * `LittleFS` [LittleFS for ESP32 (autor: lorol)]
    * `ArduinoJson` [ArduinoJson (autor: Benoît Blanchon)]
    
4. **Abrir** el sketch principal (`PCH.ino`).
5. **Velocidad Serial**: 115200.
6. **Compila y sube**.

---

## 🔐 Configuración de credenciales

En el repo ya está `Secrets.h` con **declaraciones**. Debes crear **`Secrets.cpp`** con las **definiciones** reales.

```cpp
// Secrets.cpp
#include "Secrets.h"

// 🔑 Firestore Web API Key
const char key[]      = "TU_API_KEY_Firestore";

// 📧 Usuario autenticado
const char email[]    = "usuario@ejemplo.com";

// 🔒 Contraseña del usuario
const char password[] = "tu_password";

// 🚫 RTDB ya no se usa → pero la mantenemos por compatibilidad con tu código actual
// Puedes dejar el valor que ya tenías, pero es una buena práctica usar el Project ID si es para Firestore.
const char url[]      = ""; 

// 🌎 Firestore requiere el Project ID de Firestore
const char projectId[] = "TU_PROJECT_ID_Firestore";
```

Si eres "sanbartolomepch@gmail.com" puedes acceder a [las definiciones del Secrets.cpp oficiales](https://mail.google.com/mail/u/0/#inbox/FMfcgzQcqtjBxZdRWXcLgLrpCfnsTWxJ)
> Si no puedes acceder significa que no iniciaste seción de google como "sanbartolomepch@gmail.com"

---

## ⚖️ Calibración y parámetros

### Caudalímetros

* **ISR que cuenta pulsos**, período de cálculo: **1000 ms** (1 segundo).
* Factor de calibración en `Caudalimetro.h` (`FLOW_CALIBRATION_FACTOR`).

### Ultrasónicos

* **Constructor**:
    ```c++
    ultrasonico(byte trig, byte echo, int c, float techo, float piso, float ancho, float sqrtPendiente)
    ```
* `techo/piso`: límites de nivel (m).
* `ancho`: del canal (m).
* `sqrtPendiente`: `sqrt(S)` de Manning.

---

# 📐 Variables Empíricas (`κ`)

La Issue #23 – “Variables empíricas” discute por qué algunos parámetros (como κ) deben ajustarse experimentalmente para que estos datos coincidan con mediciones reales en maqueta.

Las variables **κ (kappa)** son factores de ajuste empíricos utilizados en las ecuaciones de los sensores.  
Su valor final debe ser determinado mediante **calibración en campo** para asegurar que las lecturas del dispositivo coincidan con mediciones de referencia.

---

## 1. Ultrasónico (`Ultrasonico.h`)

 **Nombre:** kappa
 
 **Unidades:** Adimensional
 
 **Uso:** Se aplica en el cálculo del **Flujo (Q)** mediante la fórmula de **Manning**:

$$
Q = \frac{\kappa}{n} A R^{2/3} \sqrt{S}
$$

---

## 2. Caudalímetro (`Caudalimetro.h`)

**Nombre:** kappa

**Unidades:** m³×min/L×s
 
 **Uso:** Se aplica en el escalamiento del valor obtenido del sensor, el cuál es muy pequeño.
 
---

## 3. Relé (`Actuador_digital.h`)

**Nombre:** kappa

**Unidades:** m³/s
 
 **Uso:** Se usa para reconocer el flujo en que es necesario activar la motobomba.
 
---

## 4. Principal (`PCH.ino`)

**Nombre:** kappa

**Unidades:** Adimensional
 
 **Uso:** Se usa para el cálculo de la cota a partir de **Flujo (Q)** mediante la fórmula de **Manning**.
 
---

## ▶️ Uso básico

1.  Conecta el **ESP32** y abre el **Monitor Serie** (`115200`).
2.  Verás los mensajes de **WiFi**, **hora** y **Firestore**.
3.  Cada **~1 s** se imprimirán cotas, caudales y recomendación de generadores.
4.  La **pantalla TFT** y **Firestore** reflejarán los mismos datos (salvo limitaciones indicadas en las issues #24, #26 y #28, todavía en revisión).

---

## 🧯 Solución de problemas

* **No compila**:
    * Asegúrate de tener `Secrets.cpp` y librerías instaladas.
    * Si ves errores con `datos`, confirma `using datos = dato;` en `Datos.h`.
* **Firestore no conecta**:
    * Verifica `key`, `projectId`, `email/password` y la hora **NTP** (que la sincronización sea exitosa).
    * La Issue #21 documenta partes del código que pueden limpiarse sin afectar el funcionamiento.
* **Firestore recibe datos pero no controla la compuerta**:
    * Caso descrito en la Issue #28. Actualmente solo se suben lecturas y el canal de órdenes está pendiente de pruebas adicionales.
* **Texto se corta en la pantalla**:
    * Revisa cableado y fuente de poder.
    * Ajusta `timeout_us` en `reading()`.
* **Firestore no conecta**:
    * Problema descrito en la Issue #26 cuando se imprimen valores grandes con unidades (msnm). Puede mitigarse acortando etiquetas o reduciendo decimales.
* **LED RGB no muestra todos los estados esperados**:
    * La Issue #24 recuerda que este componente fue el menos probado; se recomienda revisar conexiones y el tipo de LED (ánodo/cátodo común).
* **Posible daño en pines del ESP32**:
    * En la Issue #22 se documenta un caso de conexión accidental a +5V. Verificar siempre niveles lógicos y usar conversores de nivel cuando sea necesario.

---

## 🧭 Roadmap corto

* Implementar control remoto de actuadores vía **Firestore stream** (ya está listo el stream de comandos).
* Endpoint web/app para visualizar `/sensorData` en tiempo real.
* Hacer **pantalla TFT** más interactiva con gráficos y estados o animaciones.

---

## 📜 Licencia

Uso académico y demostrativo.

---

## 🤝 Contribuir

* Issues y PRs son bienvenidos.
* Al reportar un problema indica:
   * Hardware utilizado (placa, sensores, actuadores).
   * Extracto de logs de serie.
   * Si el problema ya está relacionado con alguna de las issues mencionadas (#20–#28).
