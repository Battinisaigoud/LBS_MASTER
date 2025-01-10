#ifndef EVSE_WIFI_H
#define EVSE_WIFI_H

#include <WiFi.h>
#include "esp_wifi.h"

#define WIFI_FIRMWARE_DRIVER_VERSION  String("1.0.1")

void wifi_init(void);
void wifi_deinit(void);

extern uint8_t isWifiConnected;
extern int32_t rssi;

#endif