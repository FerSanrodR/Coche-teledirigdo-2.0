/*
 * Joystick.cpp - Implementación del Control de Joysticks
 * Coche Radiocontrol ESP32 - Mando (Emisor)
 */

#include "Joystick.h"

// Constructor
Joystick::Joystick() {
}

// Inicializar los pines ADC
void Joystick::begin() {
  pinMode(PIN_ACCELERATION, INPUT);
  pinMode(PIN_STEERING, INPUT);

  // Configuración simple para lectura analógica estable
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

// Leer y procesar el eje de aceleración
int16_t Joystick::readAcceleration() {
  int16_t rawValue = analogRead(PIN_ACCELERATION);
  return mapAcceleration(rawValue);
}

// Leer y procesar el eje de dirección
int16_t Joystick::readSteering() {
  int16_t rawValue = analogRead(PIN_STEERING);
  return mapSteering(rawValue);
}

// Leer ambos ejes y devolver estructura PaqueteControl
PaqueteControl Joystick::readAll() {
  PaqueteControl packet;
  packet.aceleracion = readAcceleration();
  packet.direccion = readSteering();
  return packet;
}

// Aplicar zona muerta a un valor en crudo
int16_t Joystick::applyDeadzone(int16_t rawValue, int16_t centerValue) {
  // Si el valor está en la zona muerta, devolver el valor central
  if (rawValue >= DEADZONE_MIN && rawValue <= DEADZONE_MAX) {
    return centerValue;
  }
  return rawValue;
}

// Mapear valor de aceleración
int16_t Joystick::mapAcceleration(int16_t rawValue) {
  // Aplicar zona muerta (centro es 0 para aceleración)
  int16_t processedValue = applyDeadzone(rawValue, 0);
  
  // Si está en zona muerta, devolver 0
  if (processedValue == 0) {
    return 0;
  }
  
  // Mapear de 0-4095 a -255 a 255
  int16_t mappedValue = map(rawValue, 0, ADC_RESOLUTION, ACCEL_MIN, ACCEL_MAX);
  
  // Limitar al rango válido
  return constrain(mappedValue, ACCEL_MIN, ACCEL_MAX);
}

// Mapear valor de dirección
int16_t Joystick::mapSteering(int16_t rawValue) {
  // Aplicar zona muerta: si está en el centro, devolver 90
  int16_t processedValue = applyDeadzone(rawValue, STEER_CENTER);
  if (processedValue == STEER_CENTER) {
    return STEER_CENTER;
  }

  // Mapear de 0-4095 a 0-180, con el centro en 90
  int16_t mappedValue = map(rawValue, 0, ADC_RESOLUTION, STEER_MIN, STEER_MAX);

  // Limitar al rango válido y mantener el centro en 90
  return constrain(mappedValue, STEER_MIN, STEER_MAX);
}
