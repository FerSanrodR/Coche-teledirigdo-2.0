/*
 * EspNowComm.h - Comunicación ESP-NOW
 * Coche Radiocontrol ESP32 - Compartido (Emisor y Receptor)
 */

#ifndef ESPNOWCOMM_H
#define ESPNOWCOMM_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "config.h"

// Tipo de dispositivo: EMISOR o RECEPTOR
enum DeviceType {
  DEVICE_EMITOR,
  DEVICE_RECEPTOR
};

// Callbacks personalizados para el usuario
typedef void (*DataSentCallback)(const uint8_t *mac_addr, esp_now_send_status_t status);
typedef void (*DataRecvCallback)(const uint8_t *mac, const uint8_t *incomingData, int len);

class EspNowComm {
public:
  // Constructor
  EspNowComm();
  
  // Inicializar ESP-NOW como emisor o receptor
  bool begin(DeviceType type);
  
  // Registrar peer (para emisor)
  bool registerPeer(const uint8_t *macAddress);
  
  // Enviar datos (para emisor)
  bool sendData(const uint8_t *macAddress, const uint8_t *data, size_t len);
  
  // Establecer callback personalizado para envío (emisor)
  void setOnDataSentCallback(DataSentCallback callback);
  
  // Establecer callback personalizado para recepción (receptor)
  void setOnDataRecvCallback(DataRecvCallback callback);
  
  // Obtener el último paquete recibido (receptor)
  PaqueteControl getLastReceivedPacket();
  
  // Verificar si hay datos nuevos disponibles (receptor)
  bool hasNewData();
  
  // Marcar datos como leídos (receptor)
  void clearNewDataFlag();

private:
  DeviceType deviceType;
  DataSentCallback userOnDataSent;
  DataRecvCallback userOnDataRecv;
  
  PaqueteControl lastPacket;
  bool newDataAvailable;
  
  // Callbacks internos de ESP-NOW
  static void internalOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
  static void internalOnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
  
  // Puntero estático a la instancia para callbacks
  static EspNowComm* instance;
};

#endif // ESPNOWCOMM_H
