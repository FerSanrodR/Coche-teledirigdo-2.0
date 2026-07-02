/*
 * main_receptor.cpp - Programa Principal del Coche (Receptor)
 * Coche Radiocontrol ESP32
 */

#include <Arduino.h>
#include "../include/config.h"
#include "../lib/Traccion/Motor.h"
#include "../lib/Direccion/ServoCtrl.h"
#include "../lib/Comunicacion/EspNowComm.h"

// ============================================================
// OBJETOS GLOBALES
// ============================================================
Motor motor;
ServoCtrl servo;
EspNowComm comm;

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
  Serial.print("Recibido - Acel: ");
  Serial.print(datos.aceleracion);
  Serial.print(" | Dir: ");
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
  
  Serial.println("Receptor inicializado correctamente");
  Serial.println("Esperando paquetes del mando...");
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
    
    // Marcar datos como leídos
    comm.clearNewDataFlag();
  }
  
  // Pequeño retardo para estabilidad
  delay(10);
}
