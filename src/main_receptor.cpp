/*
 * main_receptor.cpp - Programa Principal del Coche (Receptor)
 * Coche Radiocontrol ESP32
 */

#include <Arduino.h>
#include "config.h"
#include "Motor.h"
#include "ServoCtrl.h"
#include "EspNowComm.h"

// ============================================================
// OBJETOS GLOBALES
// ============================================================
Motor motor;
ServoCtrl servo;
EspNowComm comm;

const int LED_PIN = 2;

void blinkStartupIndicator() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
    delay(250);
  }
  digitalWrite(LED_PIN, HIGH);
}

// Variables para failsafe
unsigned long ultimoPaqueteRecibido = 0;
bool signalLost = false;

// ============================================================
// CALLBACK DE RECEPCIÓN DE DATOS
// ============================================================
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // Actualizar timestamp del último paquete
  ultimoPaqueteRecibido = millis();
  signalLost = false;
  
  // Obtener el paquete recibido
  PaqueteControl datos = comm.getLastReceivedPacket();
  
  // Imprimir datos para depuración
  Serial.print("[COCHE] Recibido Acel=");
  Serial.print(datos.aceleracion);
  Serial.print(" Dir=");
  Serial.println(datos.direccion);
}

// ============================================================
// VERIFICACIÓN DE FAILSAFE
// ============================================================
void verificarFailsafe() {
  // Verificar si ha pasado más tiempo del permitido sin recibir paquetes
  if (millis() - ultimoPaqueteRecibido > FAILSAFE_TIMEOUT) {
    if (!signalLost) {
      Serial.println("¡SEÑAL PERDIDA! Activando failsafe...");
      signalLost = true;
    }
    
    // Detener el motor DC
    motor.stop();
    
    // Centrar el servomotor
    servo.center();
    
  } else {
    if (signalLost) {
      Serial.println("Señal restaurada");
      signalLost = false;
    }
  }
}

// ============================================================
// CONFIGURACIÓN INICIAL
// ============================================================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Inicializar monitor serie
  Serial.begin(SERIAL_BAUDRATE);
  
  // Inicializar motor DC
  motor.begin();
  
  // Inicializar servomotor
  servo.begin();
  
  // Inicializar comunicación ESP-NOW como receptor
  if (!comm.begin(DEVICE_RECEPTOR)) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }
  
  // Establecer callback de recepción
  comm.setOnDataRecvCallback(OnDataRecv);
  
  // Imprimir identificación clara del receptor
  Serial.println("================================");
  Serial.println("DISPOSITIVO: COCHE (RECEPTOR)");
  Serial.println("================================");
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  blinkStartupIndicator();
  Serial.println("[COCHE] Receptor listo");
  Serial.println("[COCHE] Esperando paquetes...");
}

// ============================================================
// BUCLE PRINCIPAL
// ============================================================
void loop() {
  // Verificar failsafe (detección de pérdida de señal)
  verificarFailsafe();
  
  // Solo procesar datos si la señal está presente
  if (!signalLost && comm.hasNewData()) {
    // Obtener el paquete recibido
    PaqueteControl datos = comm.getLastReceivedPacket();
    
    // Controlar el motor DC con la aceleración
    motor.setSpeed(datos.aceleracion);
    
    // Controlar el servomotor con la dirección
    servo.setAngle(datos.direccion);

    Serial.print("[COCHE] Aplicando Acel=");
    Serial.print(datos.aceleracion);
    Serial.print(" Dir=");
    Serial.println(datos.direccion);
    
    // Marcar datos como leídos
    comm.clearNewDataFlag();
  }
  
  // Pequeño retardo para estabilidad
  delay(10);
}
