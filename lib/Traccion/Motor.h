/*
 * Motor.h - Control de Motor DC con Puente H
 * Coche Radiocontrol ESP32 - Coche (Receptor)
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <Arduino.h>
#include "config.h"

class Motor {
public:
  // Constructor
  Motor();
  
  // Inicializar pines y PWM
  void begin();
  
  // Avanzar con velocidad especificada (0-255)
  void forward(int16_t speed);
  
  // Retroceder con velocidad especificada (0-255)
  void backward(int16_t speed);
  
  // Detener el motor
  void stop();
  
  // Controlar motor con valor de aceleración (-255 a 255)
  // Positivo: avanzar, Negativo: retroceder, 0: detener
  void setSpeed(int16_t acceleration);

private:
  // Configurar PWM usando API moderna de ESP32
  void setupPWM();
  
  // Establecer dirección de los pines del puente H
  void setDirection(bool forward);
};

#endif // MOTOR_H
