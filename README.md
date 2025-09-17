# PCH
# 🌊⚡ Sistema de Monitoreo y Control para PCH con ESP32

## 📖 ¿Qué es esto?
Un sistema basado en **ESP32** que mide **niveles de agua** y **caudales** en varios puntos de una **pequeña central hidroeléctrica (PCH)**.  
Cada segundo, el sistema:
1. **Lee los sensores** (agua y caudal).
2. **Calcula** cuántos generadores deberían estar activos.
3. **Muestra y envía** los resultados en tiempo real.

---

## 🔍 ¿Qué mide?
- **Niveles de agua** → con **4 sensores ultrasónicos** (calculan la distancia al agua usando un “eco” sonoro).
- **Caudales** (flujo de agua) → con **3 caudalímetros** (cuentan pulsos de agua que pasa).

---

## ⚙️ ¿Qué hace con esos datos?
- Decide cuántos **generadores** deben estar encendidos (0, 1, 2 o máximo).
- **Muestra** la información en **2 pantallas** locales.
- **Envía** los datos al computador por cable (puerto serial).
- **Publica** los datos en **Firebase** (nube → página web).
- **Controla un motor** que mueve una compuerta cuando es necesario.

---

## ⏱️ ¿Cada cuánto?
- Aproximadamente cada **1 segundo** repite el ciclo:  
  `leer → calcular → mostrar/enviar`.

---

## 📊 Glosario rápido
- **Caudalímetro** → mide cuánta agua pasa por un punto (flujo).  
- **Ultrasonido** → lanza un “grito” y mide el eco para calcular la distancia al agua.  
- **ISR** → función ultrarrápida que se ejecuta sola cuando llega una señal de un sensor.  
- **Puente H** → circuito que hace girar un motor en ambos sentidos.  
- **msnm** → metros sobre el nivel del mar.  
- **Puerto serial** → el “cable de datos” virtual entre la placa y el PC.  

---

## 🧩 Componentes principales
- **ESP32** → cerebro del sistema.  
- **4 sensores ultrasónicos** → niveles de agua (captación, río, garantía, aducción).  
- **3 caudalímetros** → caudal de inicio, turbinable y final.  
- **Pantallas** → muestran los caudales principales.  
- **Motor con puente H** → abre/cierra la compuerta.  
- **WiFi + Firebase** → conexión a la nube para monitoreo remoto.  

---

## 📐 Arquitectura (simplificada)
