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
  // Configurar la frecuencia PWM estándar de servos (50 Hz)
  servo.setPeriodHertz(SERVO_PWM_FREQUENCY);

  // Adjuntar el servo al pin configurado con un rango de pulsos compatible
  servo.attach(PIN_SERVO, SERVO_MIN_PULSE_WIDTH, SERVO_MAX_PULSE_WIDTH);

  // Pequeña espera para que el servo se estabilice
  delay(200);

  // Prueba inicial para comprobar que responde
  Serial.println("Probando movimiento del servo...");
  setAngle(STEER_MIN);
  delay(500);
  setAngle(STEER_MAX);
  delay(500);

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
