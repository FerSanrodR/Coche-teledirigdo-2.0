/*
 * Emisor (Mando a Distancia) - Coche Radiocontrol
 * Microcontrolador: ESP32
 * Protocolo: ESP-NOW
 * Framework: Arduino para ESP32
 */

#include <esp_now.h>
#include <WiFi.h>

// ============================================================
// ASIGNACIÓN DE PINES (DEFINES)
// ============================================================
#define PIN_ACCELERATION 32  // Eje Y del joystick izquierdo (aceleración)
#define PIN_STEERING     33  // Eje X del joystick derecho (dirección)

// ============================================================
// CONFIGURACIÓN ADC
// ============================================================
// El ESP32 tiene un ADC de 12 bits (rango 0-4095)
// Configuramos la atenuación a 11dB para permitir lecturas hasta 3.3V
#define ADC_ATTENUATION ADC_ATTEN_DB_11
#define ADC_RESOLUTION 4095

// ============================================================
// ZONA MUERTA (DEADZONE)
// ============================================================
// Rango de lecturas en crudo donde se considera que el joystick está en reposo
#define DEADZONE_MIN 1900
#define DEADZONE_MAX 2200

// ============================================================
// RANGOS DE MAPEO
// ============================================================
// Aceleración: -255 (marcha atrás máxima) a 255 (marcha adelante máxima)
#define ACCEL_MIN -255
#define ACCEL_MAX 255

// Dirección: 0 (izquierda) a 180 (derecha), 90 es el centro
#define STEER_MIN 0
#define STEER_MAX 180
#define STEER_CENTER 90

// ============================================================
// ESTRUCTURA DE DATOS (PAYLOAD)
// ============================================================
// Paquete de datos a enviar mediante ESP-NOW
// Usamos int16_t para eficiencia (2 bytes por variable)
struct PaqueteControl {
  int16_t aceleracion;  // Rango: -255 a 255
  int16_t direccion;    // Rango: 0 a 180
};

// ============================================================
// VARIABLES GLOBALES
// ============================================================
// Dirección MAC del receptor (coche) - CAMBIAR POR LA MAC REAL DEL COCHE
uint8_t receptorMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Variable para almacenar el estado del peer
esp_now_peer_info_t peerInfo;

// Paquete de control a enviar
PaqueteControl paquete;

// ============================================================
// CONFIGURACIÓN INICIAL
// ============================================================
void setup() {
  // Inicializar monitor serie para depuración
  Serial.begin(115200);
  
  // Configurar los pines de los joysticks como entrada
  pinMode(PIN_ACCELERATION, INPUT);
  pinMode(PIN_STEERING, INPUT);
  
  // ============================================================
  // CONFIGURACIÓN DE LA ATENUACIÓN DEL ADC
  // ============================================================
  // ADC_ATTEN_DB_11 permite medir hasta 3.3V de entrada
  // Esto es necesario para obtener el rango completo del ADC (0-4095)
  analogReadResolution(12);  // Resolución de 12 bits
  analogSetAttenuation(ADC_ATTENUATION);
  
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
  
  // ============================================================
  // REGISTRO DEL PEER DE ESP-NOW
  // ============================================================
  
  // Copiar la dirección MAC al peer info
  memcpy(peerInfo.peer_addr, receptorMacAddress, 6);
  peerInfo.channel = 0;        // Canal 0 (auto-detección)
  peerInfo.encrypt = false;     // Sin encriptación
  
  // Registrar el peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error al registrar el peer");
    return;
  }
  
  // Registrar la función de callback para confirmación de envío
  esp_now_register_send_cb(OnDataSent);
  
  Serial.println("Emisor inicializado correctamente");
}

// ============================================================
// FUNCIÓN DE CALLBACK - CONFIRMACIÓN DE ENVÍO
// ============================================================
// Se ejecuta cuando se completa el envío de un paquete ESP-NOW
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Paquete enviado con éxito");
  } else {
    Serial.println("Error al enviar el paquete");
  }
}

// ============================================================
// FUNCIÓN DE LECTURA Y PROCESAMIENTO DE JOYSTICKS
// ============================================================
void leerJoysticks() {
  // Leer valores en crudo del ADC (0-4095)
  int16_t accelRaw = analogRead(PIN_ACCELERATION);
  int16_t steerRaw = analogRead(PIN_STEERING);
  
  // ============================================================
  // LÓGICA DE ZONA MUERTA (DEADZONE)
  // ============================================================
  // Si la lectura está en la zona muerta, forzar valores de reposo
  if (accelRaw >= DEADZONE_MIN && accelRaw <= DEADZONE_MAX) {
    paquete.aceleracion = 0;
  } else {
    // Mapear aceleración: 0-4095 -> -255 a 255
    // 0 -> -255, 2048 (centro) -> 0, 4095 -> 255
    paquete.aceleracion = map(accelRaw, 0, ADC_RESOLUTION, ACCEL_MIN, ACCEL_MAX);
  }
  
  if (steerRaw >= DEADZONE_MIN && steerRaw <= DEADZONE_MAX) {
    paquete.direccion = STEER_CENTER;
  } else {
    // Mapear dirección: 0-4095 -> 0 a 180
    // 0 -> 0, 2048 (centro) -> 90, 4095 -> 180
    paquete.direccion = map(steerRaw, 0, ADC_RESOLUTION, STEER_MIN, STEER_MAX);
  }
  
  // Limitar valores para asegurar que estén dentro del rango esperado
  paquete.aceleracion = constrain(paquete.aceleracion, ACCEL_MIN, ACCEL_MAX);
  paquete.direccion = constrain(paquete.direccion, STEER_MIN, STEER_MAX);
}

// ============================================================
// BUCLE PRINCIPAL
// ============================================================
void loop() {
  // Leer y procesar los joysticks
  leerJoysticks();
  
  // Imprimir valores para depuración (opcional)
  Serial.print("Aceleración: ");
  Serial.print(paquete.aceleracion);
  Serial.print(" | Dirección: ");
  Serial.println(paquete.direccion);
  
  // Enviar el paquete mediante ESP-NOW
  esp_err_t resultado = esp_now_send(
    receptorMacAddress,           // Dirección MAC del receptor
    (uint8_t *) &paquete,         // Puntero al paquete de datos
    sizeof(PaqueteControl)        // Tamaño del paquete en bytes
  );
  
  if (resultado != ESP_OK) {
    Serial.println("Error al enviar datos ESP-NOW");
  }
  
  // Pequeño retraso para mantener baja latencia sin saturar el canal de radio
  // 20-50ms es un buen compromiso entre respuesta y estabilidad
  delay(20);
}
