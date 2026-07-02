/*
 * Receptor (Coche) - Coche Radiocontrol
 * Microcontrolador: ESP32
 * Protocolo: ESP-NOW
 * Framework: Arduino para ESP32
 */

#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

// ============================================================
// ASIGNACIÓN DE PINES (DEFINES)
// ============================================================
// Servomotor para dirección
#define PIN_SERVO 13

// Motor DC - Puente H
#define PIN_MOTOR_PWM 14   // Pin PWM (Enable) para velocidad
#define PIN_MOTOR_IN1 27   // Pin Dirección 1
#define PIN_MOTOR_IN2 26   // Pin Dirección 2

// ============================================================
// CONFIGURACIÓN PWM (LED CONTROL)
// ============================================================
// Frecuencia PWM para el motor DC (5000 Hz es estándar para motores)
#define PWM_FREQ 5000
// Resolución PWM (8 bits = 0-255)
#define PWM_RESOLUTION 8
// Canal PWM (0-15, usamos el canal 0)
#define PWM_CHANNEL 0

// ============================================================
// CONFIGURACIÓN FAILSAFE
// ============================================================
// Tiempo máximo sin recibir paquetes antes de activar el failsafe (ms)
#define FAILSAFE_TIMEOUT 500

// ============================================================
// ESTRUCTURA DE DATOS (PAYLOAD)
// ============================================================
// Debe ser idéntica a la del emisor
typedef struct {
  int16_t aceleracion;  // Rango: -255 a 255
  int16_t direccion;    // Rango: 0 a 180 (90 es centro)
} PaqueteControl;

// Variable global para almacenar los datos recibidos
PaqueteControl datosRecibidos;

// ============================================================
// VARIABLES GLOBALES
// ============================================================
// Objeto servo para controlar la dirección
Servo servoDireccion;

// Variable para controlar el failsafe
unsigned long ultimoPaqueteRecibido = 0;
bool signalLost = false;

// ============================================================
// CONFIGURACIÓN INICIAL
// ============================================================
void setup() {
  // Inicializar monitor serie para depuración
  Serial.begin(115200);
  
  // ============================================================
  // CONFIGURACIÓN DE PINES DEL MOTOR DC
// ============================================================
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_IN1, OUTPUT);
  pinMode(PIN_MOTOR_IN2, OUTPUT);
  
  // ============================================================
  // CONFIGURACIÓN PWM USANDO API MODERNA DE ESP32
  // ============================================================
  // Configurar el canal PWM con frecuencia y resolución
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  // Adjuntar el pin PWM al canal
  ledcAttachPin(PIN_MOTOR_PWM, PWM_CHANNEL);
  
  // Inicializar motor detenido
  detenerMotor();
  
  // ============================================================
  // CONFIGURACIÓN DEL SERVOMOTOR
  // ============================================================
  // Inicializar el servo en el pin asignado
  servoDireccion.attach(PIN_SERVO);
  // Posicionar el servo en el centro inicialmente
  servoDireccion.write(90);
  
  // ============================================================
  // INICIALIZACIÓN DE WI-FI Y ESP-NOW
  // ============================================================
  
  // Inicializar Wi-Fi en modo estación (WIFI_STA)
  WiFi.mode(WIFI_STA);
  
  // Inicializar el protocolo ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }
  
  // Registrar la función de callback para recepción de datos
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("Receptor inicializado correctamente");
  Serial.println("Esperando paquetes del mando...");
}

// ============================================================
// FUNCIÓN DE CALLBACK - RECEPCIÓN DE DATOS
// ============================================================
// Se ejecuta cuando se recibe un paquete ESP-NOW
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  // Verificar que el tamaño del paquete sea correcto
  if (len == sizeof(PaqueteControl)) {
    // Copiar los datos recibidos a la variable global
    memcpy(&datosRecibidos, incomingData, sizeof(PaqueteControl));
    
    // Actualizar el timestamp del último paquete recibido
    ultimoPaqueteRecibido = millis();
    signalLost = false;
    
    // Imprimir datos recibidos para depuración (opcional)
    Serial.print("Recibido - Acel: ");
    Serial.print(datosRecibidos.aceleracion);
    Serial.print(" | Dir: ");
    Serial.println(datosRecibidos.direccion);
  }
}

// ============================================================
// LÓGICA DEL MOTOR DC (TRACCIÓN)
// ============================================================
void controlarMotor(int16_t aceleracion) {
  // ============================================================
  // INTERPRETACIÓN DEL VALOR DE ACELERACIÓN
  // ============================================================
  
  if (aceleracion > 0) {
    // ============================================================
    // ACELERACIÓN POSITIVA: AVANZAR
    // Configurar IN1 HIGH, IN2 LOW para avanzar
    // Aplicar el valor absoluto al PWM
    // ============================================================
    digitalWrite(PIN_MOTOR_IN1, HIGH);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    ledcWrite(PWM_CHANNEL, abs(aceleracion));
    
  } else if (aceleracion < 0) {
    // ============================================================
    // ACELERACIÓN NEGATIVA: RETROCEDER
    // Configurar IN1 LOW, IN2 HIGH para retroceder
    // Aplicar el valor absoluto al PWM
    // ============================================================
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, HIGH);
    ledcWrite(PWM_CHANNEL, abs(aceleracion));
    
  } else {
    // ============================================================
    // ACELERACIÓN CERO: DETENER MOTOR
    // Configurar ambos pines IN1 e IN2 en LOW
    // Aplicar PWM de 0
    // ============================================================
    digitalWrite(PIN_MOTOR_IN1, LOW);
    digitalWrite(PIN_MOTOR_IN2, LOW);
    ledcWrite(PWM_CHANNEL, 0);
  }
}

// ============================================================
// FUNCIÓN AUXILIAR - DETENER MOTOR
// ============================================================
void detenerMotor() {
  digitalWrite(PIN_MOTOR_IN1, LOW);
  digitalWrite(PIN_MOTOR_IN2, LOW);
  ledcWrite(PWM_CHANNEL, 0);
}

// ============================================================
// LÓGICA DE DIRECCIÓN (SERVOMOTOR)
// ============================================================
void controlarServo(int16_t direccion) {
  // Aplicar directamente el valor recibido (0-180) al servo
  servoDireccion.write(direccion);
}

// ============================================================
// IMPLEMENTACIÓN DEL FAILSAFE
// ============================================================
void verificarFailsafe() {
  // Verificar si ha pasado más tiempo del permitido sin recibir paquetes
  if (millis() - ultimoPaqueteRecibido > FAILSAFE_TIMEOUT) {
    if (!signalLost) {
      Serial.println("¡SEÑAL PERDIDA! Activando failsafe...");
      signalLost = true;
    }
    
    // Detener el motor DC (PWM a 0)
    detenerMotor();
    
    // Centrar el servomotor (90 grados)
    servoDireccion.write(90);
    
  } else {
    if (signalLost) {
      Serial.println("Señal restaurada");
      signalLost = false;
    }
  }
}

// ============================================================
// BUCLE PRINCIPAL
// ============================================================
void loop() {
  // ============================================================
  // VERIFICACIÓN DE FAILSAFE
  // ============================================================
  // Comprobar si se ha perdido la señal del mando
  verificarFailsafe();
  
  // Solo procesar los datos si la señal está presente
  if (!signalLost) {
    // Controlar el motor DC con la aceleración recibida
    controlarMotor(datosRecibidos.aceleracion);
    
    // Controlar el servomotor con la dirección recibida
    controlarServo(datosRecibidos.direccion);
  }
  
  // Pequeño retraso para estabilidad
  delay(10);
}
