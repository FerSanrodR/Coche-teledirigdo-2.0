/*
 * main_emisor.cpp - Programa Principal del Mando (Emisor)
 * Coche Radiocontrol ESP32
 */

#include <Arduino.h>
#include "config.h"
#include "Joystick.h"
#include "EspNowComm.h"

// ============================================================
// OBJETOS GLOBALES
// ============================================================
Joystick joystick;
EspNowComm comm;

const int LED_PIN = 2;

void blinkStartupIndicator() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    delay(150);
  }
  digitalWrite(LED_PIN, HIGH);
}

// Dirección MAC del receptor (coche)
uint8_t receptorMacAddress[] = {0xD4, 0xE9, 0xF4, 0xB5, 0x78, 0xBC};

// ============================================================
// CALLBACK DE ENVÍO DE DATOS
// ============================================================
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("[MANDO] Paquete enviado");
  } else {
    Serial.println("[MANDO] Error al enviar");
  }
}

// ============================================================
// CONFIGURACIÓN INICIAL
// ============================================================
void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Inicializar monitor serie
  Serial.begin(SERIAL_BAUDRATE);
  
  // Inicializar joystick
  joystick.begin();
  
  // Inicializar comunicación ESP-NOW como emisor
  if (!comm.begin(DEVICE_EMITOR)) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }
  
  // Registrar el peer (coche)
  if (!comm.registerPeer(receptorMacAddress)) {
    Serial.println("Error al registrar el peer");
    return;
  }
  
  // Establecer callback de envío
  comm.setOnDataSentCallback(OnDataSent);
  
  // Imprimir identificación clara del emisor
  Serial.println("================================");
  Serial.println("DISPOSITIVO: MANDO (EMISOR)");
  Serial.println("================================");
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  
  blinkStartupIndicator();
  Serial.println("[MANDO] Emisor listo");
}

// ============================================================
// BUCLE PRINCIPAL
// ============================================================
void loop() {
  // Leer ambos joysticks
  PaqueteControl paquete = joystick.readAll();
  
  // Imprimir valores para depuración
  Serial.print("[MANDO] Acel=");
  Serial.print(paquete.aceleracion);
  Serial.print(" Dir=");
  Serial.println(paquete.direccion);
  
  // Enviar paquete al coche
  if (!comm.sendData(receptorMacAddress, (uint8_t*)&paquete, sizeof(PaqueteControl))) {
    Serial.println("Error al enviar datos ESP-NOW");
  }
  
  // Retraso para mantener baja latencia sin saturar el canal
  delay(EMISOR_DELAY_MS);
}
