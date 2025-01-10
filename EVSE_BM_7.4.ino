/*
   INFINITY 7.4kw

   Developed by G. Raja Sumant

   With modified metervalues - string - 13/04/2022
   With modified remote start - string - 13/04/2022
   With modified 4G - 27/04/2022

   Copyright 2022 raja <raja@raja-IdeaPad-Gaming-3-15IMH05>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.



*Added Master-Slave files
*Added EnergyMeter Fix
*10KW Connector
*/

#include <Arduino.h>
#include "src/libraries/arduinoWebSockets-master/src/WebSocketsClient.h"
// #include <ArduinoJson.h>
#include "src/Peripherals.h"

#if WIFI_ENABLED
#include <WiFi.h>
// #define SSIDW   "Amplify Mobility_PD"
// #define PSSWD   "Amplify5"
#endif

// OCPP Message Generation Class
#include "src/OcppEngine.h"
// #include "src/SmartChargingService.h"
#include "src/ChargePointStatusService.h"
#include "src/MeteringService.h"
#include "src/GetConfiguration.h"
#include "src/TimeHelper.h"
#include "src/SimpleOcppOperationFactory.h"
#include "src/EVSE_A.h"
#include "src/EVSE_B.h"
#include "src/EVSE_C.h"
#include "src/DataTransfer.h"
#include "src/ReserveNow.h"
#include "src/EVSE_A_Offline.h"
#include <nvs_flash.h>
#include "src/4G_OTA.h"
#include "src/evse_wifi.h"
#if ETHERNET_ENABLED
#include "src/CustomEthernet.h"
#endif

// Master Class
#include "src/Master.h"

// Power Cycle
#include "src/OTA.h"
#include "src/internet.h"
#include "FFat.h"
#include "FS.h"
#include "SPIFFS.h"
#include "esp32-hal-cpu.h"
#include "ESP32Time.h"
#include "Esp.h"
#include "core_version.h"
#include <TinyGsmClient.h>
#include <CRC32.h>
#include <Update.h> //ADDED
#include "Wire.h"

// added..........
bool flag_masterPin_Disp = true;
int masterpin = 0;
int masterPin_m;
int masterpinDisp;
bool flag_btnPressedDisp = false;
bool ButtonInp;
ulong timer1;
bool flag_stopOnlineCommunication = false;

// void OTA_4G_setup4G(void);
// void OTA_4G_setup_4G_OTA_get(void);
// void OTA_4G_setup_4G_OTA(void);

// ADDED............
void connectToWebsocket();

#define TINY_GSM_MODEM_SIM7600
// #define TINY_GSM_MODEM_SIM800

#if CP_A_ACTIVE
// Control Pilot files
#include "src/ControlPilot.h"
extern EVSE_states_enum EVSE_state;
#endif

// Gsm Files

#include "src/CustomGsm.h"
extern TinyGsmClient client;

#include "src/display_meterValues.h"

// #include "esp_wifi.h"

// 20x4 lcd display
#if ETHERNET_ENABLED
#include "src/CustomEthernet.h"
// C:\Users\Mr Cool\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\cores\esp32/Server.h edited this file @wamique
//  Enter a MAC address for your controller below.
// https://github.com/arduino-libraries/Ethernet/issues/88

// sudo nano /home/raja/.arduino15/packages/esp32/hardware/esp32/1.0.6/libraries/WiFi/src/WiFiServer.h
// virtual void begin() =0;

uint8_t counter_ether = 0;
#endif

#if LCD_ENABLED
#include "src/LCD_I2C.h"

LCD_I2C lcd(0x27, 20, 4); // Default address of most PCF8574 modules, change according
#endif

#include "src/urlparse.h"

WebSocketsClient webSocket;

extern uint8_t currentDisplay;
#if LOAD_CONTROL
float powerToSend = 3.3; // Example power value

#endif

ulong TimerEmcy = 0;
bool EMGCY_GFCI_Fault_Occ;
extern String reserve_currentIdTag;
// SmartChargingService *smartChargingService;
ChargePointStatusService* chargePointStatusService_A;
ChargePointStatusService* chargePointStatusService_B;
// ChargePointStatusService *chargePointStatusService_C;

// Mertering Service declarations
MeteringService* meteringService;
ATM90E36 eic(5);
#define SS_EIC 5 // GPIO 5 chip_select pin
SPIClass* hspi = NULL;

#if 0
void WiFiStationDisconnected(arduino_event_id_t event, arduino_event_info_t info);
#endif

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
bool webSocketConncted = false;

// RFID declarations
#define MFRC_RST 22
#define MFRC_SS 15
MFRC522 mfrc522(MFRC_SS, MFRC_RST); // Create MFRC522 instance
SPIClass* hspiRfid = NULL;

bool flag_ping_sent = false;

uint8_t reasonForStop_A = 3;
uint8_t flag_ed_A = 0;

bool wifi_reconnected_flag = false;

uint8_t gu8_websocket_begin_once = 0xff;

#if DWIN_ENABLED
#include "src/dwin.h"
int8_t button = 0;
extern unsigned char ct[22];        // connected
extern unsigned char nct[22];       // not connected
extern unsigned char et[22];        // ethernet
extern unsigned char wi[22];        // wifi
extern unsigned char tr[22];        // tap rfid
extern unsigned char utr[22];       // rfid unavailable
extern unsigned char g[22];         // 4g
extern unsigned char clu[22];       // connected
extern unsigned char clun[22];      // not connected
extern unsigned char avail[22];     // available
extern unsigned char not_avail[22]; // not available
extern unsigned char change_page[10];
extern unsigned char tap_rfid[30];
extern unsigned char clear_tap_rfid[30];
extern unsigned char CONN_UNAVAIL[30];
extern unsigned char clear_avail[28];
extern unsigned char select_connector[30];
extern unsigned char clear_tap_rfid[30];

extern bool flag_faultOccured_A;
extern bool flag_faultOccured_B;
extern bool flag_faultOccured_C;

extern unsigned char v1[8]; // 2];//not connected
extern unsigned char v2[8];
extern unsigned char v3[8];
extern unsigned char i1[8];
extern unsigned char i2[8];
extern unsigned char i3[8];
extern unsigned char e1[8];
extern unsigned char e2[8];
extern unsigned char e3[8];
extern unsigned char charging[28];
extern unsigned char cid1[8];
extern unsigned char cid2[8];
extern unsigned char cid3[8];
extern unsigned char unavail[30];
extern unsigned char preparing[28];
extern unsigned char fault_emgy[28];
extern unsigned char GFCI_66[20];
extern unsigned char GFCI_55[20];
void display_avail();
// int8_t dwin_input();
#endif

extern int client_reconnect_flag;

String currentIdTag;
extern bool flag_AuthorizeRemoteTxRequests;
extern bool flag_evseIsBooted_A;
// Bluetooth
#include "src/bluetoothConfig.h"
#define TIMEOUT_BLE 60000
extern BluetoothSerial SerialBT;
bool isInternetConnected = true;

bool flagswitchoffBLE = false;
int startBLETime = 0;
String ws_url_prefix_m = "";
String host_m = "";
int port_m = 0;
// String protocol_m = "";
String protocol = "ocpp1.6";
String key_m = "";
String ssid_m = "";
String path_m;

bool StopTxnViaRfid(String& readIdTag);

extern Preferences preferences;
Preferences change_config;
String url_m = "";
Preferences resumeTxn_A;
String idTagData_A = "";
bool ongoingTxn_A = false;

// Led timers
ulong timercloudconnect = 0;
void wifi_Loop();

#define NUM_OF_CONNECTORS 2

// internet
bool wifi_enable = false;
bool gsm_enable = false;
bool ethernet_enable = true;
uint8_t counter_websock = 0;
bool wifi_connect = false;
bool gsm_connect = false;
bool ethernet_connect = false;
bool offline_connect = false;
uint8_t gu8_fault_occured = 0;

volatile bool online_to_offline_flag = false;
uint8_t wifi_connection_available = 0;
extern uint8_t Boot_Accepted;

extern bool offline_charging_A;
// extern bool offline_charging_B;
// extern bool offline_charging_C;
extern int16_t counter_ethconnect;
extern uint8_t reasonForStop;

extern unsigned int meterSampleInterval;
extern unsigned int heartbeatInterval;

extern bool reservation_start_flag;

extern uint8_t flag_nopower;

extern int8_t fault_code_A;

extern bool reservation_start_flag_A;
extern bool reservation_start_flag_B;
extern bool reservation_start_flag_C;

extern bool notFaulty_A;
extern bool notFaulty_B;
extern bool notFaulty_C;

extern bool EMGCY_FaultOccured_A;
extern bool EMGCY_FaultOccured_B;
extern bool EMGCY_FaultOccured_C;

bool evse_A_unavail = false;
bool evse_B_unavail = false;
bool evse_C_unavail = false;

uint8_t gu8_online_flag = 0;

bool internet = false;
int counter_wifiNotConnected = 0;
int counter_gsmNotConnected = 0;

extern uint8_t gu8_ota_update_available;
bool otaenable_1 = false;
extern bool fota_available;
extern Preferences evse_preferences_fota;

uint8_t gu8_check_online_count = 60;
uint8_t gu8_check_online_count2 = 60;

uint8_t gu8_OTA_update_flag = 0;
size_t E_reason;
extern String CP_Id_m;
uint32_t ble_lastTime = 0;
uint32_t ble_startTime = 0;
uint32_t ble_timerDelay = 30000;
uint8_t gu8_rfid_tapped_A = 0xff;
extern uint8_t evse_WifiConnected;
extern uint8_t isWifiConnected;
uint8_t event_count = 0;
extern uint8_t Wifi_event;
extern bool wifi_deinit_flag;

uint8_t gu8_i2cnotfound_flag = 0;
byte address = 39;

// #define EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION      String("0.0.0\r\n")
// #define DEVICE_ID                                   String("evse_001")

void WiFiEvent(WiFiEvent_t event);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
// void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void check_connectivity(void);
void setup_WIFI_OTA_get_1(void);
void performUpdate_WiFi_1(WiFiClient& updateSource, size_t updateSize);
void setup_WIFI_OTA_1(void);
void printPercent_1(uint32_t readLength, uint32_t contentLength);
void setup_WIFI_OTA_getconfig_1(void);

void deleteFlash()
{
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init();  // initialize the NVS partition.
}
volatile int gsm_interruptCounter = 0; // for counting interrupt
int gu8_gsm_setup_counter = 10;
#if 0
void WiFiStationDisconnected(arduino_event_id_t event, arduino_event_info_t info) {
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println(info.wifi_ap_stadisconnected.aid);
  // Serial.println("Trying to Reconnect");
   //server.stop();
  // WiFi.disconnect();
#if WIFI
  webSocket.disconnect();
#endif
}
#endif
hw_timer_t* timer = NULL; // H/W timer defining (Pointer to the Structure)
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer()
{ // Defining Inerrupt function with IRAM_ATTR for faster access
  portENTER_CRITICAL_ISR(&timerMux);
  gsm_interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void setup()
{
  // Test LED
  //   pinMode(16,OUTPUT);
  Serial.begin(115200);
  // deleteFlash();
  Master_setup();
  // https://arduino-esp8266.readthedocs.io/en/latest/Troubleshooting/debugging.html
  Serial.setDebugOutput(true);
  pinMode(GFCI_PIN, INPUT);
  if (DEBUG_OUT)
    Serial.println();
  if (DEBUG_OUT)
    Serial.println();
  if (DEBUG_OUT)
    Serial.println();

#if LCD_ENABLED
  lcd.begin(true, 26, 27); // If you are using more I2C devices using the Wire library use lcd.begin(false)
  // this stop the library(LCD_I2C) from calling Wire.begin()
  lcd.backlight();
#endif

#if LCD_ENABLED
  // lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
  // lcd.print("***EVRE-INFINITY***"); // You can make spaces using well... spaces
  // lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
  // lcd.print("####################");
  // lcd.setCursor(4, 2);
  // lcd.print("**7.4 KW**");
  // lcd.setCursor(4, 3);
  // lcd.print("**TYPE 2***");
  // lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
  // lcd.print("***EVRE-INFINITY***"); // You can make spaces using well... spaces
  lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
  lcd.print("CHARGER INITIALIZING"); // You can make spaces using well... spaces
#endif
  requestLed(BLINKYWHITE, START, 1);

#if DWIN_ENABLED
  uint8_t err = 0;
  dwin_setup();
  change_page[9] = 0;
  err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0
  delay(10);
  err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0])); // status not available
  delay(10);
  err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0])); // status not available
  delay(10);
  err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0])); // cloud: not connected
  delay(10);
  err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0])); // cloud: not connected
  delay(10);
  CONN_UNAVAIL[4] = 0X66;
  err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
  CONN_UNAVAIL[4] = 0X71;
  err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
  CONN_UNAVAIL[4] = 0X7B;
  err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
  // err = DWIN_SET(clear_tap_rfid, sizeof(clear_tap_rfid) / sizeof(clear_tap_rfid[0]));
  // delay(50);
#endif

  for (uint8_t t = 4; t > 0; t--)
  {
    if (DEBUG_OUT)
      Serial.println("[SETUP] BOOT WAIT... " + VERSION);
    Serial.flush();
    delay(500);
  }


  requestForRelay(STOP, 1);
  requestforCP_OUT(STOP);

#if LCD_ENABLED & 0
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("PREPARING");
  lcd.setCursor(5, 2);
  lcd.print("PLEASE WAIT");
#endif
  Serial.println(F("*** EVSE Booting ......!***"));
  Serial.print(F("EVSE FIRMWARE VERSION:  "));
  Serial.println(VERSION);
  Serial.print("WIFI DRIVER Version: ");
  Serial.println(WIFI_FIRMWARE_DRIVER_VERSION);
  Serial.print(F("WEBSOCKET DRIVER VERSION:  "));
  Serial.println(WEBSOCKETS_VERSION);

  evse_preferences_fota.begin("fota_url", false);
  String evse_fota_uri = evse_preferences_fota.getString("fota_uri", "");
  uint32_t evse_fota_retries = evse_preferences_fota.getUInt("fota_retries", 0);
  String evse_fota_date = evse_preferences_fota.getString("fota_date", "");
  bool evse_fota_avail = evse_preferences_fota.getBool("fota_avial", false);
  evse_preferences_fota.end();

  Serial.println("FOTA UPDATE states ");
  Serial.println("evse_fota_uri :  " + String(evse_fota_uri));
  Serial.println("evse_fota_retries :  " + String(evse_fota_retries));
  Serial.println("evse_fota_date :  " + String(evse_fota_date));
  Serial.println("evse_fota_avail :  " + String(evse_fota_avail));

#if 0
#if BLE_ENABLE
  startingBTConfig();
#endif
#endif
  /************************Preferences***********************************************/
  /* preferences.begin("credentials",false);

   ws_url_prefix_m = preferences.getString("ws_url_prefix",""); //characters
   if(ws_url_prefix_m.length() > 0){
     Serial.println("Fetched WS URL success: " + String(ws_url_prefix_m));
   }else{
     Serial.println("Unable to Fetch WS URL / Empty");
   }
   delay(100);

   host_m = preferences.getString("host","");
   if(host_m.length() > 0){
     Serial.println("Fetched host data success: "+String(host_m));
   }else{
     Serial.println("Unable to Fetch host data / Empty");
   }
   delay(100);

   port_m = preferences.getInt("port",0);
   if(port_m>0){
     Serial.println("Fetched port data success: "+String(port_m));
   }else{
     Serial.println("Unable to Fetch port Or port is 0000");
   }
   delay(100);

   protocol_m = preferences.getString("protocol","");
   if(protocol_m.length() > 0){
     Serial.println("Fetched protocol data success: "+String(protocol_m));
   }else{
     Serial.println("Unable to Fetch protocol");
   }*/
  urlparser();
  // Serial.println(F("**** debug point ***** failed after url parser"));
  //  Added this not to break.
  preferences.begin("credentials", false);

  ssid_m = preferences.getString("ssid", "");
  if (ssid_m.length() > 0)
  {
    Serial.println("Fetched SSID: " + String(ssid_m));
  }
  else
  {
    Serial.println("Unable to Fetch SSID");
  }

  key_m = preferences.getString("key", "");
  if (key_m.length() > 0)
  {
    Serial.println("Fetched Key: " + String(key_m));
  }
  else
  {
    Serial.println("Unable to Fetch key");
  }

  wifi_enable = preferences.getBool("wifi", 0);
  Serial.println("Fetched Wifi data: " + String(wifi_enable));

  gsm_enable = preferences.getBool("gsm", 0);
  Serial.println("Fetched Gsm data: " + String(gsm_enable));

  ethernet_enable = preferences.getBool("ethernet", 0);
  Serial.println("Fetched ethernet data: " + String(ethernet_enable));

  otaenable_1 = preferences.getBool("otaenable", 0);
  Serial.println("otaenable: " + String(otaenable_1));

  preferences.end();
  if (ethernet_enable)
  {
    wifi_enable = false;
  }

  /******************************************************************************/
  /*             EVSE Offline Functionality  is Enabled                         */

  if ((wifi_enable == false) && (gsm_enable == false) && !ethernet_enable)
  {
    offline_connect = true;
    Serial.println("Both WiFi and GSM is Disabled....!");
    Serial.println("EVSE Offline Functionality is Enabled ....!");
    requestLed(GREEN, START, 1);
  }
  /******************************************************************************/

  /*
   * @brief change_config: Feature added by G. Raja Sumant
   * 09/07/2022
   * The values stored using change configuration must
   * be restored after a reboot as well.
   */

  change_config.begin("configurations", false);
  meterSampleInterval = change_config.getInt("meterSampleInterval", 0);
  heartbeatInterval = change_config.getInt("heartbeatInterval", 50);
  flag_AuthorizeRemoteTxRequests = change_config.getBool("authRemoteStart", false); // skip auth by default
  change_config.end();

  // WiFi
  wifi_connect = wifi_enable;
  gsm_connect = gsm_enable;
  ethernet_connect = ethernet_enable;
  // bool internet = false;
  // int counter_wifiNotConnected = 0;
  // int counter_gsmNotConnected = 0;
  if (wifi_enable == true)
  {
    WiFi.begin(ssid_m.c_str(), key_m.c_str());
#if 0
    WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
#endif
    // }

    if (otaenable_1 == 1 || evse_fota_avail == 1) // checks for OTA from configuration based or DEVICE ID based @Abhigna
    {
      for (uint8_t otawaittime = 0; otawaittime <= 5; otawaittime++)
      {
        Serial.println("OTA WAIT");
      }
#if EVSE_FOTA_ENABLE_WIFI
      // ptr_leds->Yellow_led();
      setup_WIFI_OTA_get_1();
      requestLed(ORANGE, START, 1);

#if LCD_DISPLAY_ENABLED
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("CHARGER UPDATING");
#endif
      preferences.begin("credentials", false);
      preferences.putBool("otaenable", false); // making otaenable false, once checked for update @Abhigna
      otaenable_1 = preferences.getBool("otaenable", 0);
      Serial.println("OTA_ENABLE:" + String(otaenable_1));
      preferences.end();
      evse_preferences_fota.begin("fota_url", false);
      evse_preferences_fota.putString("fota_uri", "");
      evse_preferences_fota.putUInt("fota_retries", 0);
      evse_preferences_fota.putString("fota_date", "");
      evse_preferences_fota.putBool("fota_avial", false);
      evse_preferences_fota.end();

      switch (gu8_OTA_update_flag)
      {
      case 2:
        Serial.println("OTA update available, In Switch ...!");
        setup_WIFI_OTA_1();
        break;
      case 3:
        Serial.println("No OTA update available, In Switch ...!");
        break;
      default:
        Serial.println("default case OTA update, In Switch ...! \r\n gu8_OTA_update_flag :" + String(gu8_OTA_update_flag));
        break;
      }
#endif
    }
  }
  else if (gsm_enable == true)
  {
    if (!FFat.begin(true))
    {
      Serial.println("Mount Failed");
    }
    else
    {
      Serial.println("File system mounted");
      if (otaenable_1 == 1 || evse_fota_avail == 1) // checks for OTA from configuration based & every power on @Abhigna
      {
#if EVSE_FOTA_ENABLE_4G
        Serial.println("File system mounted");
        OTA_4G_setup4G();
        Serial.println("******checking for OTA******");
        requestLed(ORANGE, START, 1);

#if LCD_DISPLAY_ENABLED
        lcd.clear();
        lcd.setCursor(1, 1);
        lcd.print("CHARGER UPDATING");
#endif
        preferences.begin("credentials", false);
        preferences.putBool("otaenable", false);
        otaenable_1 = preferences.getBool("otaenable", 0);
        Serial.println("OTA_ENABLE:" + String(otaenable_1));
        preferences.end();

        OTA_4G_setup_4G_OTA_get();
        Serial.println("******OTA check done******");
        evse_preferences_fota.begin("fota_url", false);
        evse_preferences_fota.putString("fota_uri", "");
        evse_preferences_fota.putUInt("fota_retries", 0);
        evse_preferences_fota.putString("fota_date", "");
        evse_preferences_fota.putBool("fota_avial", false);
        evse_preferences_fota.end();
        delay(1000);
        if (gu8_ota_update_available == 1)
        {
          OTA_4G_setup_4G_OTA();
          Serial.println("******OTA update done******");
        }
      }
#endif
    }
  }

#if BLE_ENABLE // Time for Initialising and configuring bluetooth for 45 sec @Abhigna
  lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
  lcd.print("CHARGER INITIALIZING");
  Serial.println("BLE Configuration...");
  // startingBTConfig();
  setupBLT();
  ble_startTime = millis();
  ble_lastTime = millis();
  while ((ble_lastTime - ble_startTime) <= ble_timerDelay)
  {
    ble_lastTime = millis();
    bluetooth_Loop();
  }
  Serial.println("Free Heap.....before Bluetooth");
  Serial.println(ESP.getFreeHeap());
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();
  Serial.println("Free Heap.....after Bluetooth");
  Serial.println(ESP.getFreeHeap());
#endif
  // DEVICE INITIALIZING.................. STOPS FOR 15 SEC - In order to avoid disconnections with PulseCMS//@Abhigna
  // int8_t gu8_DeviceInit = 30;
  // while (gu8_DeviceInit > 0)
  // {
  //   gu8_DeviceInit--;
  //   Serial.println("DEVICE INITIALIZING......" + String(gu8_DeviceInit));
  //   delay(1000);
  // }

  if (!offline_connect)
  {
#if LCD_ENABLED
    lcd.clear();
    lcd.setCursor(1, 1);
    if (wifi_enable)
    {
      wifi_init(); // Changed wifi functionality
      delay(2000);
      connectToWebsocket();
      lcd.print("CONNECTING TO WIFI");
      delay(1000);
    }
    else if (gsm_enable)
    {
      lcd.print("CONNECTING TO 4G");
    }
#endif
    // WiFi
    // while ((internet == false) && (gu8_check_online_count))
    while ((internet == false) && (online_to_offline_flag == false) && (gu8_check_online_count))
    {
      Serial.println("Internet loop");

      Serial.println("gu8_check_online_count" + String(gu8_check_online_count));
      gu8_check_online_count--;

#if DWIN_ENABLED
      err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
      delay(50);
      err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
      delay(50);
      err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
      delay(50);
      err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
      delay(50);
#endif
      // bluetooth_Loop();
#if 0
      if (wifi_enable == true && wifi_connect == true)
      {
        Serial.println("Waiting for WiFi Connction...");

        if (WiFi.status() == WL_CONNECTED)
        {
          wifi_reconnected_flag = true;
          internet = true;
          gsm_connect = false;
          Serial.println("Connected via WiFi");
#if LCD_ENABLED & 0
          lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
          lcd.print("                    ");
          lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
          lcd.print("STATUS: AVAILABLE");
          lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
          lcd.print("                    "); // Clear the line
          lcd.setCursor(0, 3);
          lcd.print("CLOUD: WIFI. TAP RFID");
#endif
#if LCD_ENABLED
          lcd.clear();
          lcd.setCursor(0, 2);
          lcd.print("CONNECTED VIA WIFI");
#endif

#if DWIN_ENABLED
          // Cloud : WiFi
          err = DWIN_SET(wi, sizeof(wi) / sizeof(wi[0]));
          delay(50);
          err = DWIN_SET(wi, sizeof(wi) / sizeof(wi[0]));
          delay(50);
          // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          // delay(50);
          // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          // delay(50);
#endif

          delay(100);
          connectToWebsocket();
        }
        else if (WiFi.status() != WL_CONNECTED)
        {
          Serial.print(".");
          delay(1000);
          // bluetooth_Loop();
          wifi_Loop();
          Serial.println("Wifi Not Connected: " + String(counter_wifiNotConnected));
#if DWIN_ENABLED
          err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
          delay(50);
          err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
          delay(50);
#endif
          if (counter_wifiNotConnected++ > 50)
          {
            counter_wifiNotConnected = 0;
            online_to_offline_flag = true;
            if (gsm_enable == true)
            {
              WiFi.disconnect();
              wifi_connect = false;
              gsm_connect = true;
            }
          }
        }
      }
#endif

      // changed wifi functionality @Abhigna
      if (wifi_enable == true)
      {
        Serial.println("Waiting for WiFi Connction...");
        if (evse_WifiConnected)
        {
          wifi_reconnected_flag = true;
          internet = true;
          gsm_connect = false;
          Serial.println("Connected via WiFi");
#if LCD_DISPLAY_ENABLED
          lcd.clear();
          lcd.setCursor(1, 1);
          // lcd.print("STATUS: WIFI");
          lcd.print("CONNECTED VIA WIFI");
#endif
#if DISPLAY_ENABLED
          cloudConnect_Disp(3);
          checkForResponse_Disp();
#endif
#if DWIN_ENABLED
          // Cloud : WiFi
          err = DWIN_SET(wi, sizeof(wi) / sizeof(wi[0]));
          delay(50);
          err = DWIN_SET(wi, sizeof(wi) / sizeof(wi[0]));
          delay(50);
          // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          // delay(50);
          // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          // delay(50);
#endif
          // connectToWebsocket();
        }
        else
        {
          online_to_offline_flag = true;
        }
      }
      else if (ethernet_enable && ethernet_connect)
      {
#if ETHERNET_ENABLED
        // SPI.begin();
        ethernetSetup();
        while (Ethernet.linkStatus() != LinkON)
        {
          ethernetLoop();
          delay(200);
          Serial.println(".");
          if (counter_ether++ > 10)
          {
            break;
          }
        }
        if (counter_ether <= 10)
        {
          internet = true;
          gsm_connect = false;
          ethernet_connect = true;
          Serial.println("[Eth] Connected to Internet");

#if LCD_ENABLED
          lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
          lcd.print("                    ");
          lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
          lcd.print("STATUS: AVAILABLE");
          lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
          lcd.print("                    "); // Clear the line
          lcd.setCursor(0, 3);
          lcd.print("CLOUD: ETH. TAP RFID");
#endif
#if DWIN_ENABLED
          err = DWIN_SET(et, sizeof(et) / sizeof(et[0]));
          delay(10);
          err = DWIN_SET(et, sizeof(et) / sizeof(et[0]));
          delay(10);
#endif
          connectToWebsocket();
        }
        else
        {
          internet = false;
          if (gsm_enable == true)
          {
            ethernet_connect = false;
            gsm_connect = true;
#if DWIN_ENABLED
            err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
            delay(50);
            err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
            delay(50);
#endif
          }
        }
#endif
      }

      else if (gsm_enable == true && gsm_connect == true)
      {
        // SetupGsm();                                     //redundant @optimise
        // ConnectToServer();
        if (!client.connected())
        {
          gsm_Loop();
          // bluetooth_Loop();
          Serial.println("4G not Connected: " + String(counter_gsmNotConnected));
#if DWIN_ENABLED
          err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
          delay(50);
          err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
          delay(50);
#endif
          if (counter_gsmNotConnected++ > 1)
          { // 2 == 5min
            counter_gsmNotConnected = 0;

            if (wifi_enable == true)
            {
              wifi_connect = true;
              gsm_connect = false;
            }
            if (ethernet_enable)
            {
              ethernet_connect = true;
              gsm_connect = false;
            }
          }
        }
        else if (client.connected())
        {
          internet = true;
          wifi_connect = false;
          Serial.println("connected via 4G");
#if LCD_ENABLED & 0
          lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
          lcd.print("                    ");
          lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
          lcd.print("STATUS: AVAILABLE");
          lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
          lcd.print("                    "); // Clear the line
          lcd.setCursor(0, 3);
          lcd.print("CLOUD: 4G. TAP RFID");
#endif

#if LCD_ENABLED
          lcd.clear();

          lcd.setCursor(0, 2);
          lcd.print("CONNECTED VIA 4G");
#endif

#if DWIN_ENABLED
          // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          // delay(50);
          // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          // delay(50);
          err = DWIN_SET(g, sizeof(g) / sizeof(g[0]));
          delay(50);
          err = DWIN_SET(g, sizeof(g) / sizeof(g[0]));
          delay(50);
#endif
        }
      }
      else if (offline_connect == true && gsm_connect == false && wifi_connect == false && ethernet_connect == false)
      { // redundous but for sake of safe coding testing all flags
        Serial.println("[SETUP] ******OFFLINE loop enabled!*****");
        break;
      }
    }
  }
  // SPI Enable for Energy Meter Read
  hspi = new SPIClass(HSPI); // Init SPI bus
  hspi->begin();
  pinMode(SS_EIC, OUTPUT); // HSPI SS Pin

  // SPI Enable for RFID
  hspiRfid = new SPIClass(HSPI);
  hspiRfid->begin();
  mfrc522.PCD_Init(); // Init MFRC522

  // Serial.println("closing preferences");
  //  preferences.end();
  ocppEngine_initialize(&webSocket, 4096); // default JSON document size = 2048

  chargePointStatusService_A = new ChargePointStatusService(&webSocket);
  // chargePointStatusService_B = new ChargePointStatusService(&webSocket);
  // chargePointStatusService_C = new ChargePointStatusService(&webSocket);

  getChargePointStatusService_A()->setConnectorId(1);
  // getChargePointStatusService_B()->setConnectorId(2);
  // getChargePointStatusService_C()->setConnectorId(3);
  //  getChargePointStatusService_A()->authorize();

  EVSE_A_setup();
  // EVSE_B_setup();
  //  EVSE_C_setup();

  meteringService = new MeteringService(&webSocket);

  // set system time to default value; will be without effect as soon as the BootNotification conf arrives
  setTimeFromJsonDateString("2021-22-12T11:59:55.123Z"); // use if needed for debugging
  if (!offline_connect)
  {
    if (DEBUG_OUT)
      Serial.println("Web Socket Connction...");
    while ((!webSocketConncted && wifi_connect == true) && (gu8_check_online_count2))
    {
      Serial.print("*");
      delay(50); // bit**
      if (isWifiConnected)
      {
        webSocket.loop();
      }
      // bluetooth_Loop();
      Serial.println("gu8_check_online_count2:" + String(gu8_check_online_count2));
      gu8_check_online_count2--;
    }
  }
  // EVSE_B_initialize();
  EVSE_A_initialize();
  // EVSE_C_initialize();

  if (ethernet_enable)
  {
    if (DEBUG_OUT)
      Serial.println("Waiting for Web Socket Connction...");
    while ((!webSocketConncted) && (gu8_check_online_count2))
    {
      Serial.print("*");
      delay(50); // bit**
      if (isWifiConnected)
      {
        webSocket.loop();
      }
      // bluetooth_Loop();
      Serial.println("gu8_check_online_count2:" + String(gu8_check_online_count2));
      gu8_check_online_count2--;
      if (counter_websock++ > 50)
      {
        break;
      }
    }
  }

  gu8_check_online_count2 = 10;

  timer = timerBegin(0, 80, true);             // timer 0, prescalar: 80, UP counting
  timerAttachInterrupt(timer, &onTimer, true); // Attach interrupt
  timerAlarmWrite(timer, 1000000, true);       // Match value= 1000000 for 1 sec. delay.
  timerAlarmEnable(timer);

  Serial.println("End of Setup");
#if DWIN_ENABLED
  // err = DWIN_SET(clear_tap_rfid, sizeof(clear_tap_rfid) / sizeof(clear_tap_rfid[0]));
  delay(50);
  avail[4] = 0x55;
  err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
  delay(50);

  // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
  // delay(50);
#endif
  startBLETime = millis();
}

void loop()
{

  Serial.println("*****************S****************");

  if (evse_ChargePointStatus == Charging)
  {
#if LOAD_CONTROL
    if (sendPower(powerToSend))
    {
      Serial.println("Power sent successfully.");
    }
    // else
    // {
    //   Serial.println("Failed to send power.");
    // }
#endif
  }

  Wire.beginTransmission(address);
  byte error = Wire.endTransmission();

  if (error == 0)
  {
    if (gu8_i2cnotfound_flag == 1)
    {
      // lenum_evse_ChargePointStatus = NOT_SET;
      // dip_EvseChargePointErrorCode_OtherError = EVSE_NO_Other_Error;
      evse_ChargePointStatus = NOT_SET;
      // evse_lcd_init();
      lcd.begin(true, 26, 27); // If you are using more I2C devices using the Wire library use lcd.begin(false)
      // this stop the library(LCD_I2C) from calling Wire.begin()
      lcd.backlight();

      gu8_i2cnotfound_flag = 0;
    }
  }
  else
  {
    gu8_i2cnotfound_flag = 1;
  }
#if BLE_ENABLE & 0
  if (millis() - startBLETime < TIMEOUT_BLE)
  {
    bluetooth_Loop();
    flagswitchoffBLE = true;
  }
  else
  {
    if (flagswitchoffBLE == true)
    {
      flagswitchoffBLE = false;
      Serial.println("Disconnecting BT");
      // SerialBT.println("Wifi Connected");
      SerialBT.println("Disconnecting BT");
      delay(100);
      SerialBT.flush();
      SerialBT.disconnect();
      SerialBT.end();
      Serial.println(ESP.getFreeHeap());
    }
  }
  /*
  #if DWIN_ENABLED
  display_avail();
  #endif
  */
#endif
  if (webSocketConncted == 0 && gsm_enable == true)
  {
    if (gsm_interruptCounter > 0)
    {
      portENTER_CRITICAL(&timerMux);
      gsm_interruptCounter--;
      portEXIT_CRITICAL(&timerMux);
      gu8_gsm_setup_counter++; // counting total interrupt
      Serial.print("An interrupt as occurred. Total number: ");
      Serial.println(gu8_gsm_setup_counter);
    }
  }

  /*
   * @brief : Run this only if offline_connect is true.
   */
  if (offline_connect)
  {
    /*
     * @brief : Check GFCI/EMGY irrespective of online/offline
     */
    checkEmgcy_Gfci();
    emergencyRelayClose_Loop_A_Offl();
    /*
     * @brief : Read RFID irrespective of online/offline
     */
    EVSE_ReadInput(&mfrc522);
    EVSE_A_LED_loop();
    if (offline_charging_A)
    {
      EVSE_A_offline_Loop();
    }
  }
  // else run the regular online functions.
  else
  {

    if (webSocketConncted == 1)
    {
      ocppEngine_loop();
    }
    /*
     * @brief : Check GFCI/EMGY irrespective of online/offline
     */
    checkEmgcy_Gfci();

    if (getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Unavailable)
    {
      emergencyRelayClose_Loop_A();
    }

    /*
     * @brief : Read RFID irrespective of online/offline
     */
    EVSE_ReadInput(&mfrc522);
    /*
     * @brief : Reservation will be enabled
     */
    EVSE_Reservation_loop();

    EVSE_A_loop();
    EVSE_Led_loop();

#if ALPR_ENABLED
    alprRead_loop();
#endif
    check_connectivity();
    internetLoop();
    cloudConnectivityLed_Loop();
    ota_Loop();
    if (isWifiConnected)
    {
      webSocket.loop();
    }
  }

  if ((webSocketConncted == 0))
  {
    Serial.println("session_ongoing_flag Internet loop 1");
    if (gu8_websocket_begin_once == 1)
    {
      Serial.println("gu8_websocket_begin_once 1");
      gu8_websocket_begin_once = 0;
      Serial.println("host_m Name:" + String(host_m));
      Serial.println("port_m Name:" + String(port_m));
      Serial.println("path_m Name:" + String(path_m));
      Serial.println("protocol Name:" + String(protocol));
      // webSocket.begin(host_m, port_m, path_m, protocol);
      // connectToWebsocket();
    }
    internetLoop();
    if (isWifiConnected)
    {
      webSocket.loop();
    }
  }

  // These will take place irrespective of others.
  if (Boot_Accepted)
  {
    getChargePointStatusService_A()->loop();
  }
  meteringService->loop();

#if CP_ACTIVE
  ControlP_loop();
#endif
#if LCD_ENABLED
  stateTimer();
  disp_lcd_meter();

#endif

#if DWIN_ENABLED
  stateTimer();
  disp_dwin_meter();
#endif

  if (evse_ChargePointStatus == Available)
  {
    Serial.println("CHARGER available");
    if (fota_available == 1)
    {
      Serial.println("Fota available");
      fota_available = 0;
      ESP.restart();
    }
  }

#if 1
  if (webSocketConncted == 0 && isWifiConnected == 0)
  {
    event_count++;
    if (event_count >= 30)
    {
      if (Wifi_event == 1)
      {
        event_count = 0;
        Wifi_event = 0;
        Serial.println("Wifi_event 1");
      }
      else
      {
        event_count = 0;
        Serial.println("Wifi_event 0");
        wifi_deinit_flag = 1;
        delay(1024);
        wifi_deinit();
        delay(1000);
        wifi_init();
        // delay(500);
      }
    }
  }
#endif

  Serial.println("Websocket: " + String(webSocketConncted));
  Serial.println("FREE HEAP");
  Serial.println(ESP.getFreeHeap());
  Serial.println("\n**************E***************");
}

String readIdTag = "";
bool EVSE_ReadInput(MFRC522* mfrc522)
{ // this funtion should be called only if there is Internet
  readIdTag = "";
  int readConnectorVal = 0;
  bool status = false;
  int startTimer = 0;
  readIdTag = readRfidTag(true, mfrc522);
  if (readIdTag.equals("") == false)
  {
    /*
     * @brief : Modified this to accomodate stop txn via rfid in offline mode as well.
     * G. Raja Sumant 11/07/2022
     */
    bool val = false;
    if (offline_connect)
    {
      val = StopTxnViaRfid_offline(readIdTag);
    }
    else
    {
      val = StopTxnViaRfid(readIdTag);
    }
    if (val)
    {
      return val;
    }

    startTimer = millis();
    while (millis() - startTimer < 15000)
    {
      ControlP_loop();
      if (EVSE_state == STATE_B)
      {
        gu8_rfid_tapped_A = 1;
        // if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available)
        if (evse_ChargePointStatus == Preparing || evse_ChargePointStatus == Finishing)
        {
          getChargePointStatusService_A()->authorize(readIdTag, 1);
          if (offline_connect)
          {
            startCPTransaction();
            offline_charging_A = true;
          }
          // getChargePointStatusService_B()->setUnavailabilityStatus(true);

#if DWIN_ENABLED
          uint8_t err = 0;
          err = DWIN_SET(clear_tap_rfid, sizeof(clear_tap_rfid) / sizeof(clear_tap_rfid[0]));
#endif

#if LCD_ENABLED
          lcd.clear();

          // lcd.setCursor(0, 1);
          // lcd.print("SCAN DONE.");
          lcd.setCursor(0, 2);
          lcd.print("RFID TAPPED");
#endif

          status = true;
          Serial.println("Starting Transaction at Connector 1");
          return true;
        }
        else if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Reserved)
        {
          Serial.println("[MAIN] It is now reserved!");
          if (readIdTag.equals(reserve_currentIdTag) == true)
          {
            getChargePointStatusService_A()->authorize(readIdTag, 1);
            return true;
          }
          else
          {
            return false;
          }
        }
      }
    }

    /*  if(status == false){
        if(getChargePointStatusService_B()->inferenceStatus() == ChargePointStatus::Available){
          getChargePointStatusService_B()->authorize(readIdTag, 2);
          //getChargePointStatusService_A()->setUnavailabilityStatus(true);
          status = true;
          Serial.println("Starting Transaction at Connector 2");
          return true;
        }
      }*/

      //     readConnectorVal = requestConnectorStatus();

      //     if(readConnectorVal > 0){
      //       bool result = assignEvseToConnector(readIdTag, readConnectorVal);
      //       if(result == true){
      //         Serial.println(F("Attached/Detached EVSE to the requested connector"));
      //       }else{
      //         Serial.println(F("Unable To attach/detach EVSE to the requested connector"));
      //       }
      //     }else{
      //       Serial.println(F("Invalid Connector Id Received"));
      //       delay(2000);
      //     }

      //   }
      // delay(100);
  }
  return status;
}

#if 0
/***************************************EVSE_READINPUT BLOCK*********************************************************/
String readIdTag = "";
bool EVSE_ReadInput(MFRC522* mfrc522)
{ // this funtion should be called only if there is Internet
  readIdTag = "";
  bool result = false;
  int readConnectorVal = 0;
  unsigned long tout = millis();
  bool status = false;
  int startTimer = 0;
  readIdTag = readRfidTag(true, mfrc522);
  if (readIdTag.equals("") == false)
  {
#if DWIN_ENABLED
    change_page[9] = 3; // change to page 3 and wait for input
    uint8_t err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
    delay(10);
    err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
    delay(10);
    err = DWIN_SET(select_connector, sizeof(select_connector) / sizeof(select_connector[0]));
    delay(10);
    flush_dwin();
    requestLed(BLUE, START, 1);
    requestLed(BLUE, START, 2);
    requestLed(BLUE, START, 3);
    while (millis() - tout < 15000)
    {

      readConnectorVal = dwin_input();
      /*
        @brief: If we want to use both the physical switches/display
      */
      // readConnectorVal = requestConnectorStatus();
      if (readConnectorVal > 0)
      {
        // change_page[9] = 6; // change to page 3 and wait for input
        // err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
        /*switch (readConnectorVal)
        {
          case 1: err = DWIN_SET(cid1, sizeof(cid1) / sizeof(cid1[0]));
            break;
          case 2: err = DWIN_SET(cid2, sizeof(cid2) / sizeof(cid2[0]));
            break;
          case 3: err = DWIN_SET(cid3, sizeof(cid3) / sizeof(cid3[0]));
            break;
        }*/
        // bool result = assignEvseToConnector(readIdTag, readConnectorVal);

        if (offline_connect)
        {
          currentIdTag = readIdTag;
          Serial.println(F("******selecting via offline*****"));
          //result = assignEvseToConnector_Offl(readIdTag, readConnectorVal);
        }
        else
        {
          uint8_t result_checker = 0;
          Serial.println(F("******selecting via online*****"));
          // result = assignEvseToConnector(readIdTag, readConnectorVal);
          /*
           * @brief ReserveNow for A,B,C condition checker.
           */

          if (reservation_start_flag)
          {
            if (readIdTag.equals(reserve_currentIdTag))
            {
              //result = assignEvseToConnector(readIdTag, readConnectorVal);
              if (result)
              {
                result_checker++;
              }
            }
            else
            {
              result = false;
            }
          }


          //if (result_checker == 0)
            //result = assignEvseToConnector(readIdTag, readConnectorVal);

        }
        if (result == true)
        {
          Serial.println(F("Attached/Detached EVSE to the requested connector"));
          break;
        }
        else
        {
          Serial.println(F("Unable To attach/detach EVSE to the requested connector"));
          // err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
          // delay(10);

          // break;
        }
      }
      else
      {
        Serial.println(F("Invalid Connector Id Received"));
        /*err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
          delay(10);
          change_page[9] = 0; // change to page 3 and wait for input
          uint8_t err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
          delay(10);*/
          // break;
          // delay(2000);
      }
    }
    change_page[9] = 0; // Now it should come back to home page
    err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
    delay(10);
#endif
    /*
     * @brief : Modified this to accomodate stop txn via rfid in offline mode as well.
     * G. Raja Sumant 11/07/2022
     */
    bool val = false;
    if (offline_connect)
    {
      val = StopTxnViaRfid_offline(readIdTag);
    }
    else
    {
      val = StopTxnViaRfid(readIdTag);
    }
    if (val)
    {
      return val;
    }

    startTimer = millis();
    while (millis() - startTimer < 15000)
    {
      ControlP_loop();
      if (EVSE_state == STATE_B)
      {
        if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available)
        {
          getChargePointStatusService_A()->authorize(readIdTag, 1);
          if (offline_connect)
          {
            startCPTransaction();
            offline_charging_A = true;
          }
          // getChargePointStatusService_B()->setUnavailabilityStatus(true);
#if LCD_ENABLED
          lcd.clear();

          lcd.setCursor(0, 1);
          lcd.print("SCAN DONE.");
          lcd.setCursor(0, 2);
          lcd.print("PREPARING TO START");
#endif

          status = true;
          Serial.println("Starting Transaction at Connector 1");
          return true;
        }
        else if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Reserved)
        {
          Serial.println("[MAIN] It is now reserved!");
          if (readIdTag.equals(reserve_currentIdTag) == true)
          {
            getChargePointStatusService_A()->authorize(readIdTag, 1);
            return true;
          }
          else
          {
            return false;
          }
        }
      }
    }

    /*  if(status == false){
        if(getChargePointStatusService_B()->inferenceStatus() == ChargePointStatus::Available){
          getChargePointStatusService_B()->authorize(readIdTag, 2);
          //getChargePointStatusService_A()->setUnavailabilityStatus(true);
          status = true;
          Serial.println("Starting Transaction at Connector 2");
          return true;

        }
      }*/

      //     readConnectorVal = requestConnectorStatus();

      //     if(readConnectorVal > 0){
      //       bool result = assignEvseToConnector(readIdTag, readConnectorVal);
      //       if(result == true){
      //         Serial.println(F("Attached/Detached EVSE to the requested connector"));
      //       }else{
      //         Serial.println(F("Unable To attach/detach EVSE to the requested connector"));
      //       }
      //     }else{
      //       Serial.println(F("Invalid Connector Id Received"));
      //       delay(2000);
      //     }

      //   }
      // delay(100);
  }
}
#endif

bool StopTxnViaRfid(String& readIdTag)
{
  bool result = false;
  if (getChargePointStatusService_A()->getIdTag() == readIdTag && getChargePointStatusService_A()->getTransactionId() != -1)
  {
    Serial.println("[EVSE_A] Stopping Transaction with RFID TAP ");
    gu8_rfid_tapped_A = 0;

#if LCD_ENABLED
    lcd.clear();

    // lcd.setCursor(0, 1);
    // lcd.print("SCAN DONE.");
    // lcd.setCursor(0, 2);
    // // lcd.print("PREPARING TO STOP");
    // lcd.print("SESSION STOPPED");

    lcd.setCursor(0, 2);
    lcd.print("RFID TAPPED");
#endif

    // Added this to prevent false alarm in 7.4KW
    reasonForStop = 3;
    EVSE_A_StopSession();
    Serial.println("[EVSE_A] EVSE_A_StopSession 1");
    result = true;
  } /*else if(getChargePointStatusService_B()->getIdTag() == readIdTag && getChargePointStatusService_B()->getTransactionId()!= -1){
     Serial.println(F("[EVSE_B] Stopping Transaction with RFID TAP"));
     EVSE_B_StopSession();
     result = true;
   }*/

  return result;
}

bool StopTxnViaRfid_offline(String& readIdTag)
{
  bool result = false;
  if (getChargePointStatusService_A()->getIdTag() == readIdTag)
  {
    Serial.println("[EVSE_A] Stopping Transaction with RFID TAP offline");
    gu8_rfid_tapped_A = 0;
#if LCD_ENABLED
    lcd.clear();

    // lcd.setCursor(0, 1);
    // lcd.print("SCAN DONE.");
    lcd.setCursor(0, 2);
    // lcd.print("PREPARING TO STOP");
    lcd.print("SESSION STOPPED");
#endif

    // Added this to prevent false alarm in 7.4KW
    reasonForStop = 3;
    // EVSE_A_StopSession();
    EVSE_A_StopSession_offline();
    result = true;
  }

  return result;
}

bool assignEvseToConnector(String readIdTag, int readConnectorVal)
{
  bool status = false;
  if (readConnectorVal == 1)
  {
    if (getChargePointStatusService_A()->getIdTag() == readIdTag && getChargePointStatusService_A()->getTransactionId() != -1)
    {
      // stop session
      Serial.println("[EVSE_A] Stopping Transaction with RFID TAP at connector 1");
      EVSE_A_StopSession();
      Serial.println("[EVSE_A] EVSE_A_StopSession 2");
      status = true;
    }
    else if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available)
    {
      getChargePointStatusService_A()->authorize(readIdTag, readConnectorVal);
      status = true;
    }

  } /*else if(readConnectorVal == 2){
     if(getChargePointStatusService_B()->getIdTag() == readIdTag && getChargePointStatusService_B()->getTransactionId() != -1){
       Serial.println(F("[EVSE_B] Stopping Transaction with RFID TAP"));
       EVSE_B_StopSession();
       status = true;
     }else if(getChargePointStatusService_B()->inferenceStatus() == ChargePointStatus::Available){
          getChargePointStatusService_B()->authorize(readIdTag, readConnectorVal);    //authorizing twice needed to be improvise
          status = true;
     }
   }else if(readConnectorVal == 3){
     if(getChargePointStatusService_C()->getIdTag() == readIdTag && getChargePointStatusService_C()->getTransactionId() != -1){
       Serial.println(F("[EVSE_C] Stopping Transaction with RFID TAP"));
       EVSE_C_StopSession();
       status = true;
     }else if(getChargePointStatusService_C()->inferenceStatus() == ChargePointStatus::Available){
       getChargePointStatusService_C()->authorize(readIdTag, readConnectorVal);
       status = true;
     }
   }*/
  else
  {
    Serial.println("Connector Unavailable");
    status = false;
  }

  return status;
}

/*
   Called by Websocket library on incoming message on the internet link
*/
#if WIFI_ENABLED || ETHERNET_ENABLED
// extern OnSendHeartbeat onSendHeartbeat;
int wscDis_counter = 0;
int wscConn_counter = 0;
void webSocketEvent(WStype_t type, uint8_t* payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    webSocketConncted = false;
    Serial.println("Counter:" + String(wscDis_counter));
    currentDisplay = 0;
    if (DEBUG_OUT)
      Serial.print("[WSc] Disconnected!!!\n");
    if (wscDis_counter++ > 2)
    {
      delay(200);
      Serial.println("Trying to reconnect to WSC endpoint");
      wscDis_counter = 0;
      gu8_online_flag = 0;
      online_to_offline_flag = true;
      gu8_check_online_count = 10;
      gu8_check_online_count2 = 10;
      Serial.println("URL:" + String(path_m));
      webSocket.begin(host_m, port_m, path_m, protocol);
      while (!webSocketConncted)
      { // how to take care if while loop fails
        Serial.print("..**..");
        delay(100); // bit**
        if (isWifiConnected)
        {
          webSocket.loop(); // after certain time stop relays and set fault state
        }
        if (wscConn_counter++ > 30)
        {
          wscConn_counter = 0;
          if (ethernet_connect)
          {
            counter_ethconnect++;
          }
          Serial.println("[Wsc] Unable To Connect");
          break;
        }
      }
    }
    // have to add websocket.begin if websocket is unable to connect //Static variable
    break;
  case WStype_CONNECTED:
    if (gu8_online_flag == 0)
    {
      evse_ChargePointStatus = NOT_SET;
      Serial.println("NOT_SET..........");
    }

    webSocketConncted = true;
    gu8_online_flag = 1;
    online_to_offline_flag = false;

    if (ethernet_connect)
    {
      counter_ethconnect = 0;
    }
    if (DEBUG_OUT)
      Serial.printf("[WSc] Connected to url: %s\n", payload);
    break;
  case WStype_TEXT:
    if (DEBUG_OUT)
      if (DEBUG_OUT)
        Serial.printf("[WSc] get text: %s\n", payload);

    if (!processWebSocketEvent(payload, length))
    { // forward message to OcppEngine
      if (DEBUG_OUT)
        Serial.print("[WSc] Processing WebSocket input event failed!\n");
    }
    break;
  case WStype_BIN:
    if (DEBUG_OUT)
      Serial.print("[WSc] Incoming binary data stream not supported");
    break;
  case WStype_PING:
    // pong will be send automatically
    if (DEBUG_OUT)
      Serial.print("[WSc] get ping\n");
    break;
  case WStype_PONG:
    // answer to a ping we send
    if (DEBUG_OUT)
      Serial.print("[WSc] get pong\n");
    break;
  }
}
#endif

#if 0
int8_t dwin_input()
{

  button = DWIN_read();
  Serial.printf("Button pressed : %d", button);
  // delay(50);
  return button;
}
#endif

// #if WIFI_ENABLED
int wifi_counter = 0;
void wifi_Loop()
{
  Serial.println("[WiFi_Loop]");
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WIFI NOT CONNECTED[0-3]" + String(wifi_counter));
    if (wifi_counter++ > 2 && (WiFi.status() != WL_CONNECTED))
    {
      wifi_counter = 0;
      Serial.print(".");
      WiFi.disconnect();
      delay(50);
      Serial.println("[WIFI] Trying to reconnect again");
      WiFi.begin(ssid_m.c_str(), key_m.c_str());
      wifi_connection_available = 1;
      delay(1000);
    }
  }
  else
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      online_to_offline_flag = false;
      if (wifi_connection_available == 1)
      {
        delay(100);
        wifi_connection_available = 0;
        connectToWebsocket();
      }
      if (isWifiConnected)
      {
        webSocket.loop();
      }
    }
#if DISPLAY_ENABLED
    while (millis() - cloud_refresh > 5000)
    {
      // cloud offline
      cloud_refresh = millis();
      cloudConnect_Disp(3);
      checkForResponse_Disp();
    }
#endif
  }
}
// #endif

#if DWIN_ENABLED
extern bool disp_evse_A;
extern bool disp_evse_B;
extern bool disp_evse_C;
void display_avail()
{
  // Serial.println(F("[DWIN MAIN] Trying to update status"));
  // if (millis() - timer_dwin_avail > 5000)
  //{
  // timer_dwin_avail = millis();
  uint8_t faulty_count = 0;
  uint8_t err = 0;
  if (isInternetConnected)
  {
    if (notFaulty_A && !EMGCY_FaultOccured_A && !disp_evse_A && !evse_A_unavail && !flag_nopower)
    {
      // avail[4] = 0X55;
      // // avail[5] = 0X00;
      // err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
      // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
      // delay(10);
    }
    else
    {
      // Serial.println(F("****Display A faulty or charging****"));
      faulty_count++;
      // err = DWIN_SET(unavail, sizeof(unavail) / sizeof(unavail[0]));
      //  delay(10);
    }

#if 0
    if (notFaulty_C && !EMGCY_FaultOccured_C && !disp_evse_C && !evse_C_unavail && !flag_nopower)
    {

      avail[4] = 0X55;
      avail[5] = 0X00;
      err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
      err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
    }
    else
    {
      // Serial.println(F("****Display C faulty or charging****"));
      faulty_count++;
    }
#endif
  }
  else

  {
    Serial.println(F("Internet not available : Hence not updating status."));
    /*
     * @brief: Here all 3 must go to unavailable! By G. Raja Sumant 02/09/2022
     */
#if DWIN_ENABLED
    if (!flag_nopower)
    {
      CONN_UNAVAIL[4] = 0X66;
      err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
      CONN_UNAVAIL[4] = 0X71;
      err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
      CONN_UNAVAIL[4] = 0X7B;
      err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
    }
    // When all 3 are not available!
    faulty_count = 3;
#endif
  }
  /*
   * @brief : This is kept outside as it has some issues inside
   */
#if 0
  if (isInternetConnected)
  {
    if (notFaulty_B && !EMGCY_FaultOccured_B && !disp_evse_B && !evse_B_unavail && !flag_nopower)
    {
#if DWIN_ENABLED
      avail[4] = 0X55;
      //avail[4] = 0X71;
      // avail[5] = 0X00;
      err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
      err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
      delay(10);
#endif
    }
    else
    {
      // Serial.println(F("****Display A faulty or charging****"));
      faulty_count++;
      //err = DWIN_SET(unavail, sizeof(unavail) / sizeof(unavail[0]));
     // delay(10);
    }
  }
#endif
  if (faulty_count >= 3)
  {
    // When all 3 are not available!
#if DWIN_ENABLED
    err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
    delay(10);
    clear_tap_rfid[4] = 0x51;
    err = DWIN_SET(clear_tap_rfid, sizeof(clear_tap_rfid) / sizeof(clear_tap_rfid[0]));
    clun[4] = 0x51;
    err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
    // err = DWIN_SET(unavail, sizeof(unavail) / sizeof(unavail[0]));
    // delay(10);
#endif
  }
  else
  {
#if DWIN_ENABLED
    avail[4] = 0X55;
    // err = DWIN_SET(clear_tap_rfid, sizeof(clear_tap_rfid) / sizeof(clear_tap_rfid[0]));
    // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
#endif
    // Serial.println(F("[TAP RFID]"));
  }
  //}
}
#endif

short int counterPing = 0;
void cloudConnectivityLed_Loop()
{

#if GSM_PING
  if (gsm_connect == true)
  {
    if (counterPing++ >= 3 && !flag_ping_sent)
    {
      // gu8_online_flag = 1;
      // sending ping after every 30 sec [if internet is not there sending ping packet itself consumes 10sec]
      Serial.println("sending ping");
      // char c = 0x09;
      // client.write(0X09); // send a ping
      // client.write("[9,\"heartbeat\"]"); // send a ping
      /*String p = "9";
      sendPingGsmStr(p);*/
      String p = "rockybhai";
      sendPingGsmStr(p);
      // sendFrame(WSop_ping,"HB",size_t(2),true,true);
      // flag_ping_sent = true;
      counterPing = 0;
      // check for pong inside gsmOnEvent
    }
  }
#endif

#if DWIN_ENABLED
  uint8_t err = 0;
#endif
#if 0
  if (wifi_connect == true)
  {
    if (counterPing++ >= 3)
    { // sending ping after every 30 sec [if internet is not there sending ping packet itself consumes 10sec]
      isInternetConnected = webSocket.sendPing();
      Serial.println("*Sending Ping To Server: " + String(isInternetConnected));
      if (isInternetConnected == 0)
      {
        // offline_connect = 1; // commented by shiva
        if (webSocketConncted == true)
        {
          gu8_websocket_begin_once = 1;
          webSocketConncted = false;
          online_to_offline_flag = 1;
        }
        Serial.println("online_to_offline_flag 1 : " + String(online_to_offline_flag));
      }
      else
      {
        online_to_offline_flag = 0;
        Serial.println("online_to_offline_flag 0 : " + String(online_to_offline_flag));
      }
      counterPing = 0;
    }
    if ((WiFi.status() != WL_CONNECTED || webSocketConncted == false || isInternetConnected == false) && getChargePointStatusService_A()->getEmergencyRelayClose() == false)
    { // priority is on fault
      if (millis() - timercloudconnect > 10000)
      { // updates in 10sec
        gu8_check_online_count2 = 10;
        wifi_reconnected_flag = false;
        requestLed(BLINKYWHITE, START, 1);
#if 0
        lcd.clear();
        lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
        lcd.print("STATUS: UNAVAILABLE");
        // lcd.setCursor(0, 1);
        // lcd.print("TAP RFID/SCAN QR");
        lcd.setCursor(0, 2);
        lcd.print("CONNECTION");
        lcd.setCursor(0, 3);
        lcd.print("CLOUD: OFFLINE");
#endif

#if DWIN_ENABLED
        err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
        avail[4] = 0x51;
        err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
        err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
#endif

        timercloudconnect = millis();
      }
    }

  }
#endif
  else if (gsm_connect == true && client.connected() == false && getChargePointStatusService_A()->getEmergencyRelayClose() == false)
  {
    if (millis() - timercloudconnect > 10000)
    { // updates in 10sec

      requestLed(BLINKYWHITE, START, 1);
#if 0
      lcd.clear();
      lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
      lcd.print("STATUS: UNAVAILABLE");
      // lcd.setCursor(0, 1);
      // lcd.print("TAP RFID/SCAN QR");
      lcd.setCursor(0, 2);
      lcd.print("CONNECTION");
      lcd.setCursor(0, 3);
      lcd.print("CLOUD: OFFLINE");
#endif
#if DWIN_ENABLED
      err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
#endif
      timercloudconnect = millis();
    }
  }

#if ETHERNET_ENABLED
  if (ethernet_connect)
  {
    if (counterPing++ >= 3)
    { // sending ping after every 30 sec [if internet is not there sending ping packet itself consumes 10sec]
      isInternetConnected = webSocket.sendPing();
      Serial.println("*Sending Ping To Server: " + String(isInternetConnected));
      counterPing = 0;
    }
    if ((Ethernet.linkStatus() != LinkON || webSocketConncted == false) && getChargePointStatusService_A()->getEmergencyRelayClose() == false)
    {
      if (millis() - timercloudconnect > 10000)
      { // updates in 5sec
#if LED_ENABLED
        requestLed(BLINKYWHITE_ALL, START, 1);
#endif
#if DWIN_ENABLED
        err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
#endif
        timercloudconnect = millis();
      }
    }
    if (ethernet_connect == true && webSocketConncted == true)
    {
#if DWIN_ENABLED
      // err = DWIN_SET(et, sizeof(et) / sizeof(et[0]));
#endif
    }
  }
#endif
}

void connectToWebsocket()
{

  // url_m = String(ws_url_prefix_m);
  // String cpSerial = String("");
  // EVSE_A_getChargePointSerialNumber(cpSerial);
  // url_m += cpSerial; //most OCPP-Server require URLs like this. Since we're testing with an echo server here, this is obsolete

  // #if WIFI_ENABLED || ETHERNET_ENABLED
  Serial.println("Connecting to: ");
  Serial.println(host_m);
  Serial.println(port_m);
  Serial.println(path_m);
  Serial.println(protocol);
  // webSocket.begin(host_m, port_m, url_m, protocol_m);
  webSocket.begin(host_m.c_str(), port_m, path_m.c_str(), protocol);
  // event handler

  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(20000, 3000, 2);

  // #endif
}

// Common fault

extern bool flag_GFCI_set_here;

void checkEmgcy_Gfci()
{
  if (getChargePointStatusService_A()->getOverCurrent() == true)
  {
    if (getChargePointStatusService_A()->getTransactionId() != -1)
    {
      EVSE_A_StopSession();
      Serial.println("[EVSE_A] EVSE_A_StopSession 3");
    }
  }

  uint8_t err = 0;
  if (millis() - TimerEmcy > 1500)
  {
    TimerEmcy = millis();
    bool EMGCY_status = requestEmgyStatus();
    Serial.println("EMGCY Status: " + String(EMGCY_status));
    // delay(200);
#if V_charge_lite1_4
    bool GFCI_status = digitalRead(GFCI_PIN);
    flag_GFCI_set_here = true;
#else
    bool GFCI_status = requestGFCIStatus();
#endif
    Serial.println("GFCI Status: " + String(GFCI_status));
    if (EMGCY_status == true)
    {
      fault_code_A = 8;
      reasonForStop = 0; // Emergency
    }
    if (GFCI_status == true)
    {
      notFaulty_A = false;
      fault_code_A = 6;
      reasonForStop = 4; // Other
    }
    if ((EMGCY_status == true || GFCI_status == true))
    {
      getChargePointStatusService_A()->setEmergencyRelayClose(true);
      Serial.println("EMGCY: ");
      evse_ChargePointStatus = Faulted;

      gu8_fault_occured = 1;
      if (GFCI_status)
      {
        reasonForStop = 4;
        requestLed(RED, START, 1);
#if 0
        lcd.clear();
        lcd.setCursor(4, 0); // Or setting the cursor in the desired position.
        lcd.print("FAULTED: GFCI");
#endif
#if DWIN_ENABLED
        client_reconnect_flag = 1;
        avail[4] = 0x55;
        err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
        err = DWIN_SET(GFCI_55, sizeof(GFCI_55) / sizeof(GFCI_55[0]));
        avail[4] = 0x66;
        err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
        err = DWIN_SET(GFCI_66, sizeof(GFCI_66) / sizeof(GFCI_66[0]));
        cloud_no_rfid_dwin_print();
#endif
      }
      else
      {
        reasonForStop = 0;
        requestLed(BLINKYRED, START, 1);
#if 0
        lcd.clear();
        lcd.setCursor(4, 0); // Or setting the cursor in the desired position.
        lcd.print("FAULTED: EMGY");
#endif
#if DWIN_ENABLED
        client_reconnect_flag = 1;
        fault_emgy[4] = 0x66;
        err = DWIN_SET(fault_emgy, sizeof(fault_emgy) / sizeof(fault_emgy[0]));
        fault_emgy[4] = 0x55;
        err = DWIN_SET(fault_emgy, sizeof(fault_emgy) / sizeof(fault_emgy[0]));
        cloud_no_rfid_dwin_print();
#endif
      }
      getChargePointStatusService_A()->setEmergencyRelayClose(true);
      // getChargePointStatusService_B()->setEmergencyRelayClose(true);

      EMGCY_GFCI_Fault_Occ = true;
    }
    else
    {

      Serial.println("No EMGY/GFCI Fault");
      getChargePointStatusService_A()->setEmergencyRelayClose(false);
      //  getChargePointStatusService_B()->setEmergencyRelayClose(false);
      EMGCY_GFCI_Fault_Occ = false;
    }
  }
}

#if 1

void check_connectivity(void)
{
  uint8_t err = 0;

  while ((internet == false) && (gu8_check_online_count2))
  {
    Serial.println("In Super loop");

    Serial.println("gu8_check_online_count2 = " + String(gu8_check_online_count2));
    gu8_check_online_count2--;

    if (wifi_enable == true && wifi_connect == true)
    {
#if 0
      Serial.println("Waiting for WiFi Connction...");

      if (WiFi.status() == WL_CONNECTED)
      {
        wifi_reconnected_flag = true;
        internet = true;
        gsm_connect = false;
        Serial.println("Connected via WiFi");
        delay(100);
        connectToWebsocket();
      }
      else if (WiFi.status() != WL_CONNECTED)
      {
        Serial.print(".");
        delay(10);
        wifi_Loop();
        Serial.println("Wifi Not Connected: " + String(counter_wifiNotConnected));
#if DWIN_ENABLED
        err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
        delay(50);
        err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
        delay(50);
#endif
        // if (counter_wifiNotConnected++ > 50)
        // {
        //   counter_wifiNotConnected = 0;
        //   if (gsm_enable == true)
        //   {
        //     wifi_connect = false;
        //     gsm_connect = true;
        //   }
        // }
      }
#endif
    }

    else if (gsm_enable == true && gsm_connect == true)
    {
      Serial.println("gsm_enable == true && gsm_connect == true");
      SetupGsm(); // redundant @optimise
      ConnectToServer();
      if (!client.connected())
      {
        gsm_Loop();
        //   bluetooth_Loop();

        Serial.println("GSM not Connected: " + String(counter_gsmNotConnected));
#if DWIN_ENABLED
        err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
        delay(50);
        err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
        delay(50);
#endif
        if (counter_gsmNotConnected++ > 2)
        { // 2 == 5min
          counter_gsmNotConnected = 0;

          if (wifi_enable == true)
          {
            wifi_connect = true;
            gsm_connect = false;
          }
        }
      }
      else if (client.connected())
      {
        internet = true;
        wifi_connect = false;
        Serial.println("connected via 4G");
        // #if LCD_ENABLED
        //         lcd.clear();
        //         lcd.setCursor(0, 2);
        //         lcd.print("CONNECTED VIA 4G");
        // #endif
#if DWIN_ENABLED
        // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
        // delay(50);
        // err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
        // delay(50);
        err = DWIN_SET(g, sizeof(g) / sizeof(g[0]));
        delay(50);
        err = DWIN_SET(g, sizeof(g) / sizeof(g[0]));
        delay(50);
#endif
      }
    }
  }
}
#endif

#if 0


void WiFiEvent(WiFiEvent_t event)
{
  Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
  case SYSTEM_EVENT_WIFI_READY:
    Serial.println("WiFi interface ready");
    break;
  case SYSTEM_EVENT_SCAN_DONE:
    Serial.println("Completed scan for access points");
    break;
  case SYSTEM_EVENT_STA_START:
    Serial.println("WiFi client started");
    break;
  case SYSTEM_EVENT_STA_STOP:
    Serial.println("WiFi clients stopped");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("Connected to access point");
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    gu8_check_online_count2 = 10;
    Serial.println("Disconnected from WiFi access point");
    break;
  case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
    Serial.println("Authentication mode of access point has changed");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.print("Obtained IP address: ");
    Serial.println(WiFi.localIP());
    break;
  case SYSTEM_EVENT_STA_LOST_IP:
    Serial.println("Lost IP address and IP address is reset to 0");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
    break;
  case SYSTEM_EVENT_AP_START:
    Serial.println("WiFi access point started");
    break;
  case SYSTEM_EVENT_AP_STOP:
    Serial.println("WiFi access point  stopped");
    break;
  case SYSTEM_EVENT_AP_STACONNECTED:
    Serial.println("Client connected");
    break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    Serial.println("Client disconnected");
    break;
  case SYSTEM_EVENT_AP_STAIPASSIGNED:
    Serial.println("Assigned IP address to client");
    break;
  case SYSTEM_EVENT_AP_PROBEREQRECVED:
    Serial.println("Received probe request");
    break;
  case SYSTEM_EVENT_GOT_IP6:
    Serial.println("IPv6 is preferred");
    break;
  case SYSTEM_EVENT_ETH_START:
    Serial.println("Ethernet started");
    break;
  case SYSTEM_EVENT_ETH_STOP:
    Serial.println("Ethernet stopped");
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    Serial.println("Ethernet connected");
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    Serial.println("Ethernet disconnected");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    Serial.println("Obtained IP address");
    break;
  default: break;
  }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  gu8_check_online_count2 = 4;
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  // Serial.println(info.disconnected.reason);

}

#endif

#if 1

void setup_WIFI_OTA_get_1(void)
{
  HTTPClient http;

  uint8_t gu8_wifi_count = 50;
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_STA); /*wifi station mode*/
  // WiFi.begin(ssid_, password);
  WiFi.begin(ssid_m.c_str(), key_m.c_str());
  Serial.println("\nConnecting");

  while ((WiFi.status() != WL_CONNECTED) && (gu8_wifi_count))
  {
    Serial.print(".");
    delay(100);
    gu8_wifi_count--;
    if (gu8_wifi_count <= 0)
    {
      gu8_wifi_count = 0;
    }
    // vTaskDelay(10000 / portTICK_PERIOD_MS);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  delay(50);
  // vTaskDelay(5000 / portTICK_PERIOD_MS);

  Serial.setDebugOutput(true);

  // WiFiMulti.addAP("EVRE", "Amplify5");

  // Client address
  Serial.print("WebSockets Client started @ IP address: ");
  Serial.println(WiFi.localIP());

  //  String URI = String("http://34.100.138.28/fota2.php");

#if TEST_OTA
  // String URI = String("http://34.100.138.28/evse_bm_7_4kw_ota.php");
  String URI = String("http://34.100.138.28/evse_bm_7_4kw_ota.php");
  // String URI = String("http://34.100.138.28/evse_jubilee_test_ac001_ota.php");

  Serial.println("[OTA] Test OTA Begin...");
#else
  String URI = String("http://34.100.138.28/evse_ota.php");
  Serial.println("[OTA] OTA Begin...");
#endif

  Serial.println("[HTTP] begin...");

  Serial.print("The URL given is:");
  //   Serial.println(uri);
  Serial.println(URI);

#if 1
  int updateSize = 0;

  // configure server and url
  // String post_data = "{\"version\":\"CP001/hello.ino.esp32\", \"deviceId\":\"CP001\"}";
  // String post_data = "{\"version\":\"display_TestUART.ino.esp32\",\"deviceId\":\"CP001\"}";
  /*http.begin("https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "keep-alive");*/

  // http.begin("http://34.93.75.210/fota2.php");
  http.begin(URI);

  http.setUserAgent(F("ESP32-http-Update"));
  http.addHeader(F("x-ESP32-STA-MAC"), WiFi.macAddress());
  http.addHeader(F("x-ESP32-AP-MAC"), WiFi.softAPmacAddress());
  // http.addHeader(F("x-ESP32-sketch-md5"), String(ESP.getSketchMD5()));
  http.addHeader(F("x-ESP32-STA-MAC"), ESP.getSdkVersion());
  http.addHeader(F("x-ESP32-STA-MAC"), String(ESP.getFreeSketchSpace()));
  // http.addHeader(F("x-ESP32-sketch-size"), String(ESP.getSketchSize()));
  // http.addHeader(F("x-ESP32-device-id: "), DEVICE_ID);
  http.addHeader(F("x-ESP32-device-id"), CP_Id_m);
  http.addHeader(F("x-ESP32-firmware-version"), EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION /* + "\r\n" */);
  http.addHeader(F("x-ESP32-sketch-md5"), String(ESP.getSketchMD5()) /*  + String("\r\n")) */);
  // http.addHeader(F("x-ESP32-sketch-md5"), 425b2e2a27e2308338f7c8ede108ee9f);

  // int httpCode = http.POST(post_data);
  // int httpCode = http.GET(post_data);
  int httpCode = http.GET();
  // int httpCode = http.POST();

  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.println("Checking for new firmware updates...");

    // If file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      // get lenght of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      updateSize = len;
      Serial.printf("[OTA] Update found, File size(bytes) : %d\n", len);
      String get_response = http.getString();
      Serial.printf("[OTA] http response : %s\n", get_response);
      Serial.println("[HTTP] connection closed or file end.\n");

      if (get_response.equals("true") == true)
      {
        Serial.print("OTA update available");
        gu8_OTA_update_flag = 2;
      }
      else if (get_response.equals("false") == false)
      {
        gu8_OTA_update_flag = 3;
        Serial.print("no OTA update");
      }
    }
    // If there is no file at server
    if (httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR)
    {
      Serial.println("[HTTP] No Updates");
      Serial.println();
      // ESP.restart();
    }
    Serial.println("[HTTP] Other response code");
    Serial.println(httpCode);
    Serial.println();
  }
  http.end();

#endif
}

void setup_WIFI_OTA_1(void)
{
  HTTPClient http;

  uint8_t gu8_wifi_count = 50;
#if 1
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA); /*wifi station mode*/
  // WiFi.begin(ssid_, password);
  WiFi.begin(ssid_m.c_str(), key_m.c_str());
  Serial.println("\nConnecting");

  while ((WiFi.status() != WL_CONNECTED) && (gu8_wifi_count))
  {
    Serial.print(".");
    delay(100);
    gu8_wifi_count--;
    if (gu8_wifi_count <= 0)
    {
      gu8_wifi_count = 0;
    }
    // vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
#endif
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  delay(50);
  // vTaskDelay(5000 / portTICK_PERIOD_MS);

  Serial.setDebugOutput(true);

  // WiFiMulti.addAP("EVRE", "Amplify5");

  // Client address
  Serial.print("WebSockets Client started @ IP address: ");
  Serial.println(WiFi.localIP());

  //  String URI = String("http://34.100.138.28/fota2.php");

#if TEST_OTA
  String URI = String("http://34.100.138.28/evse_bm_7_4kw_ota.php");
  // String URI = String("http://34.100.138.28/evse_jubilee_test_ac001_ota.php");

  Serial.println("[OTA] Test OTA Begin...");
#else
  String URI = String("http://34.100.138.28/evse_ota.php");
  Serial.println("[OTA] OTA Begin...");
#endif

  Serial.println("[HTTP] begin...");

  Serial.print("The URL given is:");
  //   Serial.println(uri);
  Serial.println(URI);

#if 1
  int updateSize = 0;

  // configure server and url
  // String post_data = "{\"version\":\"CP001/hello.ino.esp32\", \"deviceId\":\"CP001\"}";
  // String post_data = "{\"version\":\"display_TestUART.ino.esp32\",\"deviceId\":\"CP001\"}";
  /*http.begin("https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "keep-alive");*/

  // http.begin("http://34.93.75.210/fota2.php");
  http.begin(URI);

  http.setUserAgent(F("ESP32-http-Update"));
  http.addHeader(F("x-ESP32-STA-MAC"), WiFi.macAddress());
  http.addHeader(F("x-ESP32-AP-MAC"), WiFi.softAPmacAddress());
  http.addHeader(F("x-ESP32-sketch-md5"), String(ESP.getSketchMD5()));
  http.addHeader(F("x-ESP32-STA-MAC"), ESP.getSdkVersion());
  http.addHeader(F("x-ESP32-STA-MAC"), String(ESP.getFreeSketchSpace()));
  // http.addHeader(F("x-ESP32-sketch-size"), String(ESP.getSketchSize()));
  http.addHeader(F("x-ESP32-device-id"), CP_Id_m);
  // http.addHeader(F("x-ESP32-device-test-id: "), DEVICE_ID);
  http.addHeader(F("x-ESP32-firmware-version"), EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION /* + "\r\n" */);

  // int httpCode = http.POST(post_data);
  // int httpCode = http.GET(post_data);
  // int httpCode = http.GET();
  int httpCode = http.POST(CP_Id_m);

  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.println("Checking for new firmware updates...");

    // If file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      // get lenght of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      updateSize = len;
      Serial.printf("[OTA] Update found, File size(bytes) : %d\n", len);

#if 1
      // get tcp stream
      WiFiClient* client = http.getStreamPtr();
      // Serial.println();
      performUpdate_WiFi_1(*client, (size_t)updateSize);
      Serial.println("[HTTP] connection closed or file end.\n");
#endif
      Serial.println("[HTTP] connection closed or file end.\n");
    }
    // If there is no file at server
    if (httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR)
    {
      Serial.println("[HTTP] No Updates");
      Serial.println();
      // ESP.restart();
    }
    Serial.println("[HTTP] Other response code");
    Serial.println(httpCode);
    Serial.println();
  }
  http.end();
#endif
}

#endif

#if 1

// perform the actual update from a given stream
void performUpdate_WiFi_1(WiFiClient& updateSource, size_t updateSize)
{
  if (Update.begin(updateSize))
  {
    Serial.println("...Downloading File...");
    Serial.println();

    // Writing Update
    size_t written = Update.writeStream(updateSource);

    printPercent_1(written, updateSize);

    if (written == updateSize)
    {
      Serial.println("Written : " + String(written) + "bytes successfully");
    }
    else
    {
      Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      // ptr_leds->red_led();
      // for (uint8_t idx = 0; idx < NUM_LEDS; idx++)
      //   {
      //       leds[idx] = CRGB::Red;
      //       FastLED.show(COLOR_BRIGHTNESS);
      //   }
      requestLed(RED, START, 1);
    }
    if (Update.end())
    {
      Serial.println("OTA done!");
      if (Update.isFinished())
      {
        Serial.println("Update successfully completed. Rebooting...");
        Serial.println();
        ESP.restart();
      }
      else
      {
        Serial.println("Update not finished? Something went wrong!");
      }
    }
    else
    {
      Serial.println("Error Occurred. Error #: " + String(Update.getError()));
    }
  }
  else
  {
    Serial.println("Not enough space to begin OTA");
  }
}

void printPercent_1(uint32_t readLength, uint32_t contentLength)
{
  // If we know the total length
  if (contentLength != (uint32_t)-1)
  {
    Serial.print("\r ");
    Serial.print((100.0 * readLength) / contentLength);
    Serial.print('%');
  }
  else
  {
    Serial.println(readLength);
  }
}

void setup_WIFI_OTA_getconfig_1(void)
{
  HTTPClient http;

  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA); /*wifi station mode*/
  // WiFi.begin(ssid_, password);
  WiFi.begin(ssid_m.c_str(), key_m.c_str());
  Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
    // vTaskDelay(10000 / portTICK_PERIOD_MS);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  delay(50);
  // vTaskDelay(5000 / portTICK_PERIOD_MS);

  Serial.setDebugOutput(true);

  // WiFiMulti.addAP("EVRE", "Amplify5");

  // Client address
  Serial.print("WebSockets Client started @ IP address: ");
  Serial.println(WiFi.localIP());

  //  String URI = String("http://34.100.138.28/fota2.php");

#if TEST_OTA
  // String URI = String("http://34.100.138.28/evse_bm_7_4kw_ota.php");
  String URI_GET_CONFIG = String("http://34.100.138.28/evse-config-update.php");

  Serial.println("[OTA]  Get config Begin...");
#else
  String URI = String("http://34.100.138.28/evse_ota.php");
  Serial.println("[OTA] OTA Begin...");
#endif

  Serial.println("[HTTP] begin...");

  Serial.print("The URL given is:");
  //   Serial.println(uri);
  Serial.println(URI_GET_CONFIG);

#if 1
  int updateSize = 0;

  // configure server and url
  // String post_data = "{\"version\":\"CP001/hello.ino.esp32\", \"deviceId\":\"CP001\"}";
  // String post_data = "{\"version\":\"display_TestUART.ino.esp32\",\"deviceId\":\"CP001\"}";
  /*http.begin("https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "keep-alive");*/

  // http.begin("http://34.93.75.210/fota2.php");
  http.begin(URI_GET_CONFIG);

  http.setUserAgent("ESP32-http-Update");
  http.addHeader(F("x-ESP32-STA-MAC"), WiFi.macAddress());
  http.addHeader(F("x-ESP32-AP-MAC"), WiFi.softAPmacAddress());
  http.addHeader(F("x-ESP32-sketch-md5"), String(ESP.getSketchMD5()));
  http.addHeader(F("x-ESP32-STA-MAC: "), ESP.getSdkVersion());
  http.addHeader(F("x-ESP32-STA-MAC: "), String(ESP.getFreeSketchSpace()));
  // http.addHeader(F("x-ESP32-sketch-size"), String(ESP.getSketchSize()));
  http.addHeader(F("x-ESP32-device-id: "), CP_Id_m);
  // http.addHeader(F("x-ESP32-device-test-id: "), DEVICE_ID);
  http.addHeader(F("x-ESP32-firmware-version: "), EVSE_CHARGE_POINT_FIRMWARE_OTA_VERSION /* + "\r\n" */);

  // int httpCode = http.POST(post_data);
  // int httpCode = http.GET(post_data);
  int httpCode = http.GET();
  // int httpCode = http.POST();

  if (httpCode > 0)
  {
    // HTTP header has been send and Server response header has been handled
    Serial.println("Checking for new configs...");

    // If file found at server
    if (httpCode == HTTP_CODE_OK)
    {
      // get lenght of document (is -1 when Server sends no Content-Length header)
      int len = http.getSize();
      // updateSize = len;
      Serial.printf("[OTA] get config ,  : %d\n", len);
      String get_response = http.getString();
      Serial.printf("[OTA] http response : %s\n", get_response);
      Serial.println("[HTTP] connection closed or file end.\n");

#if 0 
      DeserializationError error = deserializeJson(server_config, get_response);

      //{"wifi":"EVRE","port":"80","otaupdatetime":"86400"}

      if (error)
      {
        Serial.print(F("DeserializeJson() failed: "));
        Serial.println(error.f_str());
        // return connectedToWifi;
      }
      if (server_config.containsKey("wifi"))
      {
        wifi_server = server_config["wifi"];
      }
      if (server_config.containsKey("port"))
      {
        port_server = server_config["port"];
      }

      if (server_config.containsKey("otaupdatetime"))
      {
        ota_update_time = server_config["otaupdatetime"];
      }

      get_response = "";
      // put_server_config(); 
      Serial.println("\r\nclient disconnected....");
#endif
    }
    // If there is no file at server
    if (httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR)
    {
      Serial.println("[HTTP] No Updates");
      Serial.println();
      // ESP.restart();
    }
    Serial.println("[HTTP] Other response code");
    Serial.println(httpCode);
    Serial.println();
  }
  http.end();

#endif
}
#endif
