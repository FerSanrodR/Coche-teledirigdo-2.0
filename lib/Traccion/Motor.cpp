/*
 * Motor.cpp - Implementación del Control de Motor DC
 * Coche Radiocontrol ESP32 - Coche (Receptor)
 */

#include "Motor.h"

// Constructor
Motor::Motor() {
}

// Inicializar pines y PWM
void Motor::begin() {
  // Configurar pines como salida
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  
  // Configurar PWM
  setupPWM();
  
  // Inicializar motor detenido
  stop();
}

// Configurar PWM usando API moderna de ESP32
void Motor::setupPWM() {
  // Configurar el canal PWM con frecuencia y resolución
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  // Adjuntar el pin PWM al canal
  ledcAttachPin(PIN_MOTOR_PWM, PWM_CHANNEL);
}

// Establecer dirección de los pines del puente H
void Motor::setDirection(bool forward) {
  if (forward) {
    // Avanzar: IN1 HIGH, IN2 LOW
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
  } else {
    // Retroceder: IN1 LOW, IN2 HIGH
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
  }
}

// Avanzar con velocidad especificada (0-255)
void Motor::forward(int16_t speed) {
  // Limitar velocidad al rango válido
  speed = constrain(speed, 0, 255);
  
  // Configurar dirección para avanzar
  setDirection(true);
  
  // Aplicar velocidad PWM
  ledcWrite(PWM_CHANNEL, speed);
}

// Retroceder con velocidad especificada (0-255)
void Motor::backward(int16_t speed) {
  // Limitar velocidad al rango válido
  speed = constrain(speed, 0, 255);
  
  // Configurar dirección para retroceder
  setDirection(false);
  
  // Aplicar velocidad PWM
  ledcWrite(PWM_CHANNEL, speed);
}

// Detener el motor
void Motor::stop() {
  // Configurar ambos pines en LOW
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  
  // Aplicar PWM de 0
  ledcWrite(PWM_CHANNEL, 0);
}

// Controlar motor con valor de aceleración (-255 a 255)
void Motor::setSpeed(int16_t acceleration) {
  if (acceleration > 0) {
    // Aceleración positiva: avanzar
    forward(acceleration);
  } else if (acceleration < 0) {
    // Aceleración negativa: retroceder
    forward(abs(acceleration));
  } else {
    // Aceleración cero: detener
    stop();
  }
}
