/*
 * Joystick.h - Control de Joysticks Analógicos
 * Coche Radiocontrol ESP32 - Mando (Emisor)
 */

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>
#include "../../include/config.h"

class Joystick {
public:
  // Constructor
  Joystick();
  
  // Inicializar los pines ADC
  void begin();
  
  // Leer y procesar el eje de aceleración
  // Devuelve valor mapeado entre ACCEL_MIN y ACCEL_MAX
  int16_t readAcceleration();
  
  // Leer y procesar el eje de dirección
  // Devuelve valor mapeado entre STEER_MIN y STEER_MAX
  int16_t readSteering();
  
  // Leer ambos ejes y devolver estructura PaqueteControl
  PaqueteControl readAll();

private:
  // Aplicar zona muerta a un valor en crudo
  int16_t applyDeadzone(int16_t rawValue, int16_t centerValue);
  
  // Mapear valor de aceleración
  int16_t mapAcceleration(int16_t rawValue);
  
  // Mapear valor de dirección
  int16_t mapSteering(int16_t rawValue);
};

#endif // JOYSTICK_H
