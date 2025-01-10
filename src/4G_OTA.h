
#ifndef _4G_OTA_H_
#define _4G_OTA_H_

#include <ArduinoJson.h>
#include "libraries/arduinoWebSockets-master/src/WebSocketsClient.h"
#include "OcppEngine.h"
#include "EVSE_A.h"
#include "Variants.h"
#include "internet.h"
#include "Master.h"
#include "CustomGsm.h"
#include "Preferences.h"
#include <HTTPClient.h>
#include <Update.h>
#include "FFat.h"
#include <TinyGsmClient.h>
#include <CRC32.h>
#include "FFat.h"
#include "FS.h"
#include "SPIFFS.h"
#include "esp32-hal-cpu.h"
#include "ESP32Time.h"
#include "Esp.h"
#include "core_version.h"

void OTA_4G_setup_4G_OTA_get(void);
void OTA_4G_setup_4G_OTA(void);
void OTA_4G_setup4G(void);
uint8_t OTA_4G_waitForResp(uint8_t timeout);
void OTA_4G_printPercent(uint32_t readLength, uint32_t contentLength);
void OTA_4G_pu(const char* path);



#endif
