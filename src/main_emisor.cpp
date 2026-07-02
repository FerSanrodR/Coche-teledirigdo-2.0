/*
 * main_emisor.cpp - Programa Principal del Mando (Emisor)
 * Coche Radiocontrol ESP32
 */

#include <Arduino.h>
#include "../include/config.h"
#include "../lib/Joystick/Joystick.h"
#include "../lib/Comunicacion/EspNowComm.h"

// ============================================================
// OBJETOS GLOBALES
// ============================================================
Joystick joystick;
EspNowComm comm;

// Dirección MAC del receptor (coche) - CAMBIAR POR LA MAC REAL
uint8_t receptorMacAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// ============================================================
// CALLBACK DE ENVÍO DE DATOS
// ============================================================
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Paquete enviado con éxito");
  } else {
    Serial.println("Error al enviar el paquete");
  }
}

// ============================================================
// CONFIGURACIÓN INICIAL
// ============================================================
void setup() {
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
  
  Serial.println("Emisor inicializado correctamente");
}

// ============================================================
// BUCLE PRINCIPAL
// ============================================================
void loop() {
  // Leer ambos joysticks
  PaqueteControl paquete = joystick.readAll();
  
  // Imprimir valores para depuración
  Serial.print("Aceleración: ");
  Serial.print(paquete.aceleracion);
  Serial.print(" | Dirección: ");
  Serial.println(paquete.direccion);
  
  // Enviar paquete al coche
  if (!comm.sendData(receptorMacAddress, (uint8_t*)&paquete, sizeof(PaqueteControl))) {
    Serial.println("Error al enviar datos ESP-NOW");
  }
  
  // Retraso para mantener baja latencia sin saturar el canal
  delay(EMISOR_DELAY_MS);
}
