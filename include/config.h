/*
 * config.h - Configuración Global del Proyecto
 * Coche Radiocontrol ESP32
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================
// CONFIGURACIÓN SERIAL
// ============================================================
#define SERIAL_BAUDRATE 115200

// ============================================================
// CONFIGURACIÓN ADC (MANDO)
// ============================================================
#define ADC_ATTENUATION ADC_11db
#define ADC_RESOLUTION 4095

// ============================================================
// ZONA MUERTA (DEADZONE)
// ============================================================
// Se amplía ligeramente para que el mando responda mejor en placas
// o joysticks con el centro un poco desplazado.
#define DEADZONE_MIN 1500
#define DEADZONE_MAX 2500

// ============================================================
// RANGOS DE MAPEO
// ============================================================
// Aceleración: -255 (marcha atrás) a 255 (marcha adelante)
#define ACCEL_MIN -255
#define ACCEL_MAX 255

// Dirección: 0 (izquierda) a 180 (derecha), 90 es centro
#define STEER_MIN 0
#define STEER_MAX 180
#define STEER_CENTER 90

// ============================================================
// PINES DEL MANDO (EMISOR)
// ============================================================
// Se usan pines ADC que funcionan bien con ESP32 en módulos básicos.
#define PIN_ACCELERATION 32  // Eje Y del joystick izquierdo
#define PIN_STEERING     33  // Eje X del joystick derecho

// ============================================================
// PINES DEL COCHE (RECEPTOR)
// ============================================================
// Servomotor para dirección
#define PIN_SERVO 13

// Motor DC - Puente H
#define PIN_MOTOR_PWM 14   // Pin PWM (Enable)
#define PIN_MOTOR_IN1 27   // Pin Dirección 1
#define PIN_MOTOR_IN2 26   // Pin Dirección 2

// ============================================================
// CONFIGURACIÓN PWM (MOTOR DC)
// ============================================================
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8
#define PWM_CHANNEL 0

// ============================================================
// CONFIGURACIÓN SERVOMOTOR
// ============================================================
#define SERVO_PWM_FREQUENCY 50
#define SERVO_MIN_PULSE_WIDTH 500
#define SERVO_MAX_PULSE_WIDTH 2400

// ============================================================
// CONFIGURACIÓN FAILSAFE
// ============================================================
#define FAILSAFE_TIMEOUT 500  // ms

// ============================================================
// TIMING DEL EMISOR
// ============================================================
#define EMISOR_DELAY_MS 20  // Retraso entre envíos

// ============================================================
// ESTRUCTURA DE DATOS COMPARTIDA (PAYLOAD)
// ============================================================
typedef struct {
  int16_t aceleracion;  // Rango: -255 a 255
  int16_t direccion;    // Rango: 0 a 180 (90 es centro)
} PaqueteControl;

#endif // CONFIG_H
