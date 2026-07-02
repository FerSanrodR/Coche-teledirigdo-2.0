# 🚗 Coche Radiocontrol ESP32 con ESP-NOW

Sistema de control remoto para coche radiocontrol basado en microcontroladores ESP32 utilizando el protocolo **ESP-NOW** para comunicación inalámbrica de baja latencia. El proyecto implementa una arquitectura modular y escalable con código estructurado en C++ para el framework Arduino.

## 📋 Descripción

Este proyecto consiste en dos nodos independientes que se comunican directamente mediante el protocolo ESP-NOW de Espressif:

- **Emisor (Mando):** Lee las posiciones de dos joysticks analógicos, procesa los datos con zona muerta y los envía inalámbricamente.
- **Receptor (Coche):** Recibe los paquetes de control, gestiona un motor DC para tracción y un servomotor para dirección, e incluye un sistema de failsafe para detener el vehículo en caso de pérdida de señal.

El protocolo ESP-NOW permite una comunicación MAC-to-MAC directa sin necesidad de un punto de acceso Wi-Fi, ofreciendo latencias inferiores a 10ms y un consumo de energía mínimo.

---

## 🏗️ Arquitectura del Sistema

### 🎮 Hardware del Mando (Emisor)

- **Microcontrolador:** ESP32
- **Joysticks:** 2 joysticks analógicos estándar
  - Eje Y del joystick izquierdo → Aceleración (GPIO 32)
  - Eje X del joystick derecho → Dirección (GPIO 33)
- **ADC:** 12 bits (rango 0-4095) con atenuación de 11dB
- **Zona Muerta (Deadzone):** Implementación por software para lecturas entre 1900-2200, evitando movimientos fantasma en reposo
- **Mapeo:**
  - Aceleración: 0-4095 → -255 a 255 (negativo = marcha atrás)
  - Dirección: 0-4095 → 0° a 180° (90° = centro)

### 🚗 Hardware del Coche (Receptor)

- **Microcontrolador:** ESP32
- **Motor DC:** Para tracción trasera
- **Puente H:** Controlador de motor (L298N o TB6612FNG)
  - Pin PWM (Enable): GPIO 14
  - Pin Dirección 1 (IN1): GPIO 27
  - Pin Dirección 2 (IN2): GPIO 26
- **Servomotor:** Para dirección de ruedas delanteras (GPIO 13)
- **PWM:** Configurado a 5000 Hz con resolución de 8 bits usando la API moderna de ESP32 (`ledcSetup`, `ledcAttachPin`, `ledcWrite`)
- **Failsafe:** Sistema de seguridad que detiene el motor y centra el servo si no se reciben paquetes durante más de **500ms**

---

## 📁 Estructura de Archivos

```
Coche-teledirigdo-2.0/
├── include/
│   └── config.h                    # Configuración global (pines, constantes, PaqueteControl)
├── lib/
│   ├── Joystick/
│   │   ├── Joystick.h              # Header del control de joysticks
│   │   └── Joystick.cpp            # Implementación con deadzone y mapeo ADC
│   ├── Traccion/
│   │   ├── Motor.h                 # Header del control de motor DC
│   │   └── Motor.cpp               # Implementación con PWM API moderna
│   ├── Direccion/
│   │   ├── ServoCtrl.h             # Header del control de servomotor
│   │   └── ServoCtrl.cpp           # Implementación con ESP32Servo
│   └── Comunicacion/
│       ├── EspNowComm.h            # Header de comunicación ESP-NOW
│       └── EspNowComm.cpp          # Implementación compartida emisor/receptor
├── src/
│   ├── main_emisor.cpp             # Programa principal del mando
│   └── main_receptor.cpp           # Programa principal del coche
├── emisor.ino                      # Versión monolítica del emisor (legacy)
├── receptor.ino                    # Versión monolítica del receptor (legacy)
└── README.md                       # Este archivo
```

---

## 🔄 Flujo de Funcionamiento

### 1. **Lectura Analógica (Mando)**
   - El ESP32 lee los valores de los joysticks mediante el ADC de 12 bits (rango 0-4095).
   - Se aplica la zona muerta: si el valor está entre 1900-2200, se fuerza al valor de reposo (0 para aceleración, 90 para dirección).
   - Los valores se mapean a rangos utilizables:
     - Aceleración: -255 a 255
     - Dirección: 0° a 180°

### 2. **Empaquetado de Datos**
   - Los valores mapeados se almacenan en la estructura `PaqueteControl`:
     ```cpp
     typedef struct {
       int16_t aceleracion;  // -255 a 255
       int16_t direccion;    // 0 a 180
     } PaqueteControl;
     ```
   - Esta estructura ocupa 4 bytes (2 bytes por variable `int16_t`).

### 3. **Envío ESP-NOW (MAC-to-MAC)**
   - El emisor inicializa Wi-Fi en modo estación (WIFI_STA).
   - ESP-NOW se inicializa y se registra el peer con la dirección MAC del coche.
   - El paquete se envía directamente a la dirección MAC del receptor sin pasar por un router.
   - El callback `OnDataSent` confirma si el envío fue exitoso.

### 4. **Recepción e Interrupción (Coche)**
   - El receptor inicializa ESP-NOW en modo estación.
   - El callback `OnDataRecv` se ejecuta automáticamente al recibir un paquete.
   - Los datos se copian a una variable global y se actualiza el timestamp del último paquete.
   - El bucle principal verifica si hay datos nuevos y aplica los comandos a los actuadores.

### 5. **Control de Actuadores**
   - **Motor DC:** Según el valor de aceleración:
     - Positivo: IN1 HIGH, IN2 LOW (avanzar)
     - Negativo: IN1 LOW, IN2 HIGH (retroceder)
     - Cero: Ambos LOW (detener)
   - **Servomotor:** Se aplica directamente el ángulo de dirección (0-180°).

### 6. **Failsafe**
   - En cada ciclo del bucle, se verifica si han pasado más de 500ms desde el último paquete.
   - Si se detecta pérdida de señal:
     - Motor DC se detiene (PWM = 0)
     - Servomotor se centra (90°)
   - Al restaurarse la señal, el control se reanuda automáticamente.

---

## 🛠️ Guía de Instalación

### Requisitos Previos

1. **Arduino IDE** (versión 1.8.x o 2.x)
2. **Plataforma ESP32 para Arduino IDE**
   - En Arduino IDE, ir a `File > Preferences > Additional Boards Manager URLs`
   - Añadir: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Instalar desde `Tools > Board > Boards Manager > esp32`
3. **Librería ESP32Servo**
   - Instalar desde `Tools > Manage Libraries > ESP32Servo`
   - Esta librería evita conflictos con los timers nativos del ESP32

### Emparejamiento de Dirección MAC

1. **Obtener la MAC del coche (receptor):**
   - Cargar el código del receptor en el ESP32 del coche.
   - Abrir el monitor serie (115200 baudios).
   - La dirección MAC se imprimirá al inicio (formato: `XX:XX:XX:XX:XX:XX`).

2. **Configurar el mando (emisor):**
   - Abrir `src/main_emisor.cpp`.
   - Localizar la línea:
     ```cpp
     uint8_t receptorMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
     ```
   - Reemplazar con la MAC real del coche, por ejemplo:
     ```cpp
     uint8_t receptorMacAddress[] = {0x24, 0x6F, 0x28, 0xA1, 0x2B, 0x3C};
     ```

### Compilación y Carga

#### Para el Mando (Emisor)

1. Seleccionar la placa: `Tools > Board > ESP32 Dev Module`
2. Seleccionar el puerto COM correspondiente
3. Abrir `src/main_emisor.cpp`
4. Compilar y cargar (Upload)
5. Abrir el monitor serie a 115200 baudios para ver el estado de los envíos

#### Para el Coche (Receptor)

1. Seleccionar la placa: `Tools > Board > ESP32 Dev Module`
2. Seleccionar el puerto COM correspondiente
3. Abrir `src/main_receptor.cpp`
4. Compilar y cargar (Upload)
5. Abrir el monitor serie a 115200 baudios para ver los paquetes recibidos

### Verificación de Funcionamiento

- Mover los joysticks del mando y observar que los valores se imprimen en el monitor serie del emisor.
- Verificar que el receptor imprime los paquetes recibidos.
- Comprobar que el motor y el servo responden correctamente a los comandos.
- Apagar el mando y verificar que el coche se detiene después de 500ms (failsafe activado).

---

## ⚡ Características Técnicas

- **Protocolo:** ESP-NOW (comunicación directa MAC-to-MAC)
- **Latencia:** < 10ms (dependiendo de la distancia y condiciones RF)
- **Frecuencia de envío:** 50Hz (20ms entre paquetes)
- **Resolución ADC:** 12 bits (0-4095)
- **Resolución PWM:** 8 bits (0-255)
- **Frecuencia PWM:** 5000 Hz
- **Failsafe timeout:** 500 ms
- **Consumo:** Bajo (ESP-NOW consume menos energía que Wi-Fi tradicional)

---

## 📝 Notas

- Asegúrese de que ambos ESP32 tengan suficiente alimentación (mínimo 5V, recomendado 5V-12V para el motor DC).
- El puente H requiere una fuente de alimentación externa para el motor (no alimentar el motor directamente desde el ESP32).
- La distancia máxima de comunicación depende de las condiciones ambientales, pero típicamente es de 50-100 metros en línea de vista.
- Para mejorar la robustez, se puede añadir redundancia en el envío de paquetes o implementar un protocolo de ACK/NACK personalizado.

---

## 📄 Licencia

Este proyecto es de código abierto y está disponible para uso educativo y personal.

---

**Desarrollado para el framework Arduino ESP32** 🎛️
