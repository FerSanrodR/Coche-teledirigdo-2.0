/*
 * ServoCtrl.cpp - Implementación del Control de Servomotor
 * Coche Radiocontrol ESP32 - Coche (Receptor)
 */

#include "ServoCtrl.h"

// Constructor
ServoCtrl::ServoCtrl() {
}

// Inicializar el servomotor en su pin
void ServoCtrl::begin() {
  // Adjuntar el servo al pin configurado
  servo.attach(PIN_SERVO);
  
  // Posicionar el servo en el centro inicialmente
  center();
}

// Establecer el ángulo del servo (0-180)
void ServoCtrl::setAngle(int16_t angle) {
  // Limitar el ángulo al rango válido
  angle = constrain(angle, STEER_MIN, STEER_MAX);
  
  // Aplicar el ángulo al servo
  servo.write(angle);
}

// Centrar el servo (90 grados)
void ServoCtrl::center() {
  setAngle(STEER_CENTER);
}
