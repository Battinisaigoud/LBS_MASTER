// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef VARIANTS_H
#define VARIANTS_H

/*****************/
/*
In order to Enable Ethernet/WiFi,
* Edit this "virtual void begin(uint16_t port=0) =0;" for WIFI :C:\Users\Mr Cool\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\cores\esp32\Server.h
* Set proper flag in OCPP_ETH_WIFI file kept in arduinoWebsockets/src
*/

#define VERSION String("EVSE_BM_7.4KW_V1.2.6")
#define EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION String("126")
#define CHARGE_POINT_VENDOR String("EVRE")
#define CHARGE_POINT_MODEL String("7.4KW")
#define CHARGE_POINT_VENDOR_SIZE strlen("EVRE")
#define CHARGE_POINT_MODEL_SIZE strlen("7.4KW")

#define DEVICE_ID String("evse_001")

#define GSM_ENABLED 1
#define WIFI_ENABLED 1
#define ETHERNET_ENABLED 0

#define BLE_ENABLE 1

#define CP_ACTIVE 0

#define CP_A_ACTIVE 1

#define DISPLAY_ENABLED 0

#define DWIN_ENABLED 0

#define MASTERPIN_ENABLED 0

#define LED_ENABLED 1

#define LCD_ENABLED 1

#define GSM_PING 1

#define EARTH_DISCONNECT 1

#define EVSECEREBRO 1
#define STEVE 0
#define ALPR_ENABLED 0

#define V_charge_lite1_4 1

#define SIMCOM_A7672S 1

#define SIMCOM_A7670C 0

#define LOAD_CONTROL 1

#define EVSE_FOTA_ENABLE_4G (1)
#define EVSE_FOTA_ENABLE_WIFI (1)
#define POWER_RECYCLE (0)

#define TEST_OTA 1

/*****************/
#define DEBUG_OUT true
#define DEBUG_EXTRA false
#define DEBUG_OUT_M true

// #define OCPP_SERVER //comment out if this should be compiled as server <--- needs to be implemented again

typedef enum evse_boot_stat_t
{
    EVSE_BOOT_INITIATED = 0,
    EVSE_BOOT_SENT,
    EVSE_BOOT_ACCEPTED,
    EVSE_BOOT_REJECTED,
    EVSE_BOOT_DEFAULT,
};
extern evse_boot_stat_t evse_boot_state;

#endif
