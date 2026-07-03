/*
 * EspNowComm.cpp - Implementación de Comunicación ESP-NOW
 * Coche Radiocontrol ESP32 - Compartido (Emisor y Receptor)
 */

#include "EspNowComm.h"
#include <esp_wifi.h>

// Inicializar puntero estático
EspNowComm* EspNowComm::instance = nullptr;

// Constructor
EspNowComm::EspNowComm() {
  deviceType = DEVICE_EMITOR;
  userOnDataSent = nullptr;
  userOnDataRecv = nullptr;
  newDataAvailable = false;
  instance = this;
}

// Inicializar ESP-NOW como emisor o receptor
bool EspNowComm::begin(DeviceType type) {
  deviceType = type;
  
  // Limpiar cualquier estado previo de Wi-Fi/ESP-NOW
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);

  // Fijar un canal Wi-Fi explícito para que ambos ESP32 se encuentren
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  
  // Inicializar ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return false;
  }
  
  // Registrar callbacks según el tipo de dispositivo
  if (deviceType == DEVICE_EMITOR) {
    esp_now_register_send_cb(internalOnDataSent);
  } else {
    esp_now_register_recv_cb(internalOnDataRecv);
  }
  
  return true;
}

// Registrar peer (para emisor)
bool EspNowComm::registerPeer(const uint8_t *macAddress) {
  esp_now_peer_info_t peerInfo;
  
  // Copiar dirección MAC
  memcpy(peerInfo.peer_addr, macAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;
  
  // Registrar peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error al registrar el peer");
    return false;
  }
  
  return true;
}

// Enviar datos (para emisor)
bool EspNowComm::sendData(const uint8_t *macAddress, const uint8_t *data, size_t len) {
  esp_err_t result = esp_now_send(macAddress, data, len);
  return (result == ESP_OK);
}

// Establecer callback personalizado para envío (emisor)
void EspNowComm::setOnDataSentCallback(DataSentCallback callback) {
  userOnDataSent = callback;
}

// Establecer callback personalizado para recepción (receptor)
void EspNowComm::setOnDataRecvCallback(DataRecvCallback callback) {
  userOnDataRecv = callback;
}

// Obtener el último paquete recibido (receptor)
PaqueteControl EspNowComm::getLastReceivedPacket() {
  return lastPacket;
}

// Verificar si hay datos nuevos disponibles (receptor)
bool EspNowComm::hasNewData() {
  return newDataAvailable;
}

// Marcar datos como leídos (receptor)
void EspNowComm::clearNewDataFlag() {
  newDataAvailable = false;
}

// Callback interno para envío de datos
void EspNowComm::internalOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (instance != nullptr && instance->userOnDataSent != nullptr) {
    instance->userOnDataSent(mac_addr, status);
  }
}

// Callback interno para recepción de datos
void EspNowComm::internalOnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  if (instance != nullptr) {
    // Verificar tamaño del paquete
    if (len == sizeof(PaqueteControl)) {
      // Copiar datos al paquete
      memcpy(&instance->lastPacket, incomingData, sizeof(PaqueteControl));
      instance->newDataAvailable = true;
      
      // Llamar al callback del usuario si está configurado
      if (instance->userOnDataRecv != nullptr) {
        instance->userOnDataRecv(mac, incomingData, len);
      }
    }
  }
}
