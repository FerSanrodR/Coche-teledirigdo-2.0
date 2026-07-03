/*
 * ServoCtrl.h - Control de Servomotor para Dirección
 * Coche Radiocontrol ESP32 - Coche (Receptor)
 */

#ifndef SERVOCTRL_H
#define SERVOCTRL_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"

class ServoCtrl {
public:
  // Constructor
  ServoCtrl();
  
  // Inicializar el servomotor en su pin
  void begin();
  
  // Establecer el ángulo del servo (0-180)
  void setAngle(int16_t angle);
  
  // Centrar el servo (90 grados)
  void center();

private:
  Servo servo;
};

#endif // SERVOCTRL_H
