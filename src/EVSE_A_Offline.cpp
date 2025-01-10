// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

/**
Edited by Pulkit Agrawal.
*/

#include "EVSE_A.h"
#include "Master.h"
#include "EVSE_A_Offline.h"
#include "MeteringService.h"
#include "OcppEngine.h"

#include "ControlPilot.h"

#include "LCD_I2C.h"

#if DISPLAY_ENABLED
#include "display.h"
extern bool flag_tapped;
#endif

extern int8_t fault_code_A;

#if DWIN_ENABLED
#include "dwin.h"

extern unsigned char v1[8];
extern unsigned char v2[8];
extern unsigned char v3[8];
extern unsigned char i1[8];
extern unsigned char i2[8];
extern unsigned char i3[8];
extern unsigned char e1[8];
extern unsigned char e2[8];
extern unsigned char e3[8];
extern unsigned char change_page[10];
extern unsigned char avail[22];
extern unsigned char charging[28];
extern unsigned char cid1[7];
extern unsigned char fault_emgy[28];
#endif

#if 0
//new variable names defined by @Pulkit. might break the build.
OnBoot_A onBoot_A;
OnReadUserId_A onReadUserId_A;
OnSendHeartbeat_A onSendHeartbeat_A;
OnAuthentication_A onAuthentication_A;
OnStartTransaction_A onStartTransaction_A;
OnStopTransaction_A onStopTransaction_A;
OnUnauthorizeUser_A onUnauthorizeUser_A;
#endif

extern uint8_t currentCounterThreshold_A;

extern bool disp_evse_A;

extern ulong timerHb;
extern unsigned int heartbeatInterval;

extern bool notFaulty_A;

// timeout for heartbeat signal.
extern ulong T_SENDHEARTBEAT;
extern bool timeout_active_A;
extern bool timer_initialize_A;
extern ulong timeout_start_A;
// Reason for stop
extern uint8_t reasonForStop_A;
extern uint8_t reasonForStop;

// new flag names. replace them with old names.
extern bool evIsPlugged_A;
extern bool flag_evseIsBooted_A;
extern bool flag_evseReadIdTag_A;
extern bool flag_evseAuthenticate_A;
extern bool flag_evseStartTransaction_A;
extern bool flag_evRequestsCharge_A;
extern bool flag_evseStopTransaction_A;
extern bool flag_evseUnauthorise_A;
extern bool flag_rebootRequired_A;
extern bool flag_evseSoftReset_A; // added by @Wamique

extern ATM90E36 eic;
extern bool flag_rebootRequired_B;
extern bool flag_rebootRequired_C;

// not used. part of Smart Charging System.
extern float chargingLimit_A;
String Ext_currentIdTag_A_Offl = "";
extern WebSocketsClient webSocket;

extern ulong timerDisplay;
extern bool EMGCY_FaultOccured;

extern short counter_suspendedstate_A;
short int counter_stop = 0;

extern LCD_I2C lcd;

extern MFRC522 mfrc522;
// extern String currentIdTag;
extern long int blinckCounter_A;
extern int counter1_A;
extern ulong t;
ulong t_A;
extern int connectorDis_counter_A;

String currentIdTag_A_Offl = "";

extern EVSE_states_enum EVSE_state;
extern Preferences preferences;
extern short int fault_counter_A;
extern bool flag_faultOccured_A;
extern short int counter_drawingCurrent_A;
extern float drawing_current_A;

extern bool webSocketConncted;
extern bool isInternetConnected;
extern short counter_faultstate_A;

// metering Flag
extern bool flag_MeteringIsInitialised;
extern MeteringService *meteringService;
extern bool wifi_connect;
extern bool gsm_connect;
extern bool offline_connect;
extern bool flag_internet;
extern bool flag_offline;
extern uint8_t gu8_fault_occured;
extern float minCurr;

float globalmeterstart = 0;
float globalmeterstop = 0;

extern Preferences resumeTxn_A;
extern TinyGsmClient client;

extern bool ongoingTxn_A;
extern String idTagData_A;

extern ulong relay_timer_A;
extern ulong faultTimer_A;

extern short EMGCY_counter_A;
extern bool EMGCY_FaultOccured_A;

unsigned long timer_green_offline = 0;
extern time_t lastsampledTimeA_off;

extern int globalmeterstartA;
extern unsigned long st_timeA;
const ulong TIMEOUT_EMERGENCY_RELAY_CLOSE_A = 120000;
bool offline_charging_A = false;
ulong timer_Offl_A;
bool flag_txndone_off_A = false;
extern bool flag_txndone_off_B;
extern bool flag_txndone_off_C;

bool flag_stop = false;

float offline_charging_Enargy_A = 0;

extern unsigned long offline_t_A;

extern Preferences energymeter;

extern bool EMGCY_GFCI_Fault_Occ;
extern ulong TimerEmcy;

/****************New Offline Functions********************************/

void EVSE_A_stopOfflineTxn()
{
  disp_evse_A = false;
  requestForRelay(STOP, 1);
  getChargePointStatusService_A()->stopEvDrawsEnergy();
  getChargePointStatusService_A()->unauthorize();
  if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
  {
    // requestLed(GREEN, START, 1);
  }

  // Display transaction finished
  energymeter.begin("MeterData", false);
  float meterStop = energymeter.getFloat("currEnergy_A", 0);
  energymeter.end();
  unsigned long stop_time = millis();

  flag_txndone_off_A = true;
#if LCD_ENABLED & 0
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop - globalmeterstartA) / 1000)));
  lcd.setCursor(0, 2);
  lcd.print("CONNECTOR A");
  lcd.setCursor(0, 3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds / 3600;                                                 // Number of seconds in an hour
  int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
  int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
  delay(5000);
#endif
}

#if 0
void showTxn_Finish()
{
  if(flag_txndone_off_A)
  {
    flag_txndone_off_A = false;
#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop - globalmeterstartA) / 1000)));
  lcd.setCursor(0, 3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds / 3600;                                                 // Number of seconds in an hour
  int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
  int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
  delay(5000);
#endif
  }
  if(flag_txndone_off_B)
  {
    flag_txndone_off_B = false;
#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop-globalmeterstartB)/1000)));
  lcd.setCursor(0,3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeB) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
   delay(5000);
#endif
  }
   if(flag_txndone_off_C)
  {
    flag_txndone_off_C = false;
#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop - globalmeterstartC) / 1000)));
  lcd.setCursor(0, 3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeC) / 1000;
  int hr = seconds / 3600;                                                 // Number of seconds in an hour
  int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
  int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
  delay(5000);
#endif
  }
}
#endif

// initialize function. called when EVSE is booting.
// NOTE: It should be also called when there is a reset or reboot required. create flag to control that. @Pulkit
/**********************************************************/
void EVSE_A_StopSession_offline()
{

  requestForRelay(STOP, 1);
  offline_charging_A = false;
  disp_evse_A = false;
  //	flag_evseReadIdTag_A = true;
  /*flagstartCPTransaction = false;
  flag_controlPAuthorise_A = false;
  flag_stopCPTransaction = false; */
  requestforCP_OUT(STOP); // stop pwm
  delay(500);
 /* if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
  {
    requestLed(GREEN, START, 1); // temp fix
  }*/

  getChargePointStatusService_A()->stopEnergyOffer();
  getChargePointStatusService_A()->stopEvDrawsEnergy();
  getChargePointStatusService_A()->stopTransaction();
  getChargePointStatusService_A()->unauthorize(); // can be buggy
  getChargePointStatusService_A()->setUnavailabilityStatus(false);

  Serial.println("[EVSE] Stopping Session offline: " + String(EVSE_state));

  // if(!offline_charging_A)
  // {
  // resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
  // resumeTxn_A.putBool("ongoingTxnoff_A", true);
  // resumeTxn_A.putInt("reasonforstop_Off",reasonForStop);
  // resumeTxn_A.end();
  // }
  
  unsigned long stop_time = millis();
  if (flag_stop)
  {
    flag_stop = false;
#if LCD_ENABLED & 0
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TRANSACTION FINISHED");
    lcd.setCursor(0, 1);
    lcd.print("KWH");
    lcd.setCursor(4, 1);
    EEPROM.begin(sizeof(EEPROM_Data));
    EEPROM.get(4, globalmeterstop);
    lcd.print(String(float((globalmeterstop - globalmeterstart) / 1000)));
    lcd.setCursor(0, 2);
    lcd.print("WH");
    lcd.setCursor(4, 2);
    lcd.print(globalmeterstop - globalmeterstart);
    lcd.setCursor(0, 3);
    unsigned long seconds = (stop_time - st_timeA) / 1000;
    int hr = seconds / 3600;                                                 // Number of seconds in an hour
    int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
    int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
    String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
    lcd.print(String(hrMinSec));
    delay(5000);

#endif
  }
  // Serial.println("inferenceStatus: "+ String(getChargePointStatusService_A()->inferenceStatus()));
}
/**************************************************************************/

void EVSE_A_startOfflineTxn()
{
  offline_charging_A = true;
  disp_evse_A = true;
  // requestForRelay(START, 1);
  // requestLed(ORANGE, START, 1);
  // delay(1200);
  // requestLed(WHITE, START, 1);
  // delay(1200);
  // requestLed(GREEN, START, 1);
  // delay(1000);
  // requestLed(BLINKYGREEN, START, 1);
  Serial.println("[EVSE] EV is connected and Started charging");
  if (DEBUG_OUT)
    Serial.println("[EVSE] Started Drawing Energy");
  st_timeA = millis();
  offline_t_A = millis();
  lastsampledTimeA_off = now();
  energymeter.begin("MeterData", false);
  globalmeterstartA = energymeter.getFloat("currEnergy_A", 0);
  energymeter.end();
}

void startCPTransaction()
{

  if (EVSE_state == STATE_B && getChargePointStatusService_A()->getEmergencyRelayClose() == false &&
      getChargePointStatusService_A()->getUnavailabilityStatus() == false)
  {

    // flag_controlPAuthorise_A = true;
    /*
     * @bug:  why this flag is set to true?
     */

    // flag_stopCPTransaction = true;
    requestforCP_OUT(START);
    delay(100);
    // requestForRelay(START, 1);
    delay(100);
    disp_evse_A = true;
    flag_stop = true;
    /*
     * @brief : Fetch energy from eeprom
     */
    EEPROM.begin(sizeof(EEPROM_Data));
    globalmeterstart = 0;
    EEPROM.get(4, globalmeterstart);
    EEPROM.end();
    // requestLed(BLINKYGREEN,START,1);
    //	flag_evseReadIdTag_A = false;
    Serial.println(F("******Staring PWM Signal********"));
  }
}

void EVSE_A_offline_Loop()
{
  ControlP_loop();
  if (offline_charging_A)
  {
    if (getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getEvDrawsEnergy() == false)
    {

      /***********************Control Pilot @Wamique******************/
      if (EVSE_state == STATE_C || EVSE_state == STATE_D || EVSE_state == STATE_B)
      {
        if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
        {
          EVSE_A_StartCharging();
        }
        else if (getChargePointStatusService_A()->getEmergencyRelayClose() == true)
        {
          Serial.println("The voltage / current / Temp is out or range. FAULTY CONDITION DETECTED.");
          EVSE_A_StopSession_offline();
        }
      }
      // else if (EVSE_state == STATE_SUS)
      // {
      //   EVSE_A_Suspended();
      //   Serial.println(counter1_A);
      //   if (counter1_A++ > 25)
      //   { // Have to implement proper timeout
      //     counter1_A = 0;
      //     Serial.println(F("[EVSE_A_offline] stopped as it is in suspended state."));
      //     EVSE_A_StopSession_offline();
      //   }
      // }
      else if (EVSE_state == STATE_DIS || EVSE_state == STATE_E || EVSE_state == STATE_A)
      {

        //  EVSE_StopSession();     // for the very first time cable can be in disconnected state

        // if(txn == true){           // can implement counter > 10 just to remove noise
        if (counter_stop++ > 5)
        {
          counter_stop = 0;
          Serial.println("[EVSE_A_offline] stopped as it is in error state.");
          EVSE_A_StopSession_offline();
        }
        //  }
      }
      else
      {

        Serial.println("[EVSE] STATE Error" + String(EVSE_state));
        delay(2000);

        //  requestLed(RED,START,1);
      }
    }

    if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
    {

      //  txn = true;

      if (EVSE_state == STATE_C || EVSE_state == STATE_D)
      {

        if (DEBUG_OUT)
          Serial.println("**[EVSE_CP] Drawing Energy**");
        #if 0
        // Current check
        drawing_current_A = eic.GetLineCurrentA();
        Serial.println("Current A: " + String(drawing_current_A));
        // if (drawing_current_A <= 0.15)
        if (drawing_current_A <= minCurr)
        {
          counter_drawingCurrent_A++;
          if (counter_drawingCurrent_A > currentCounterThreshold_A)
          {
            counter_drawingCurrent_A = 0;
            Serial.println(F("Stopping session due to No current"));

            EVSE_A_StopSession_offline();
          }
        }
        else
        {
          counter_drawingCurrent_A = 0;
          Serial.println(F("counter_drawing Current Reset"));
        }
        #endif
        if (millis() - t > 15000)
        {
          if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
          {
            if (offline_charging_A)
            {
              // requestLed(BLINKYGREEN, START, 1);
              delay(100);
              requestForRelay(START, 1);
              t = millis();
            }
          }
        }
      }
      else if (EVSE_state == STATE_A || EVSE_state == STATE_E || EVSE_state == STATE_B)
      { // Although CP Inp will never go to A,B state
        if (counter_faultstate_A++ > 5)
        {
          Serial.println("[EVSE_A_offline] stopped as it is in illegal state.");
          EVSE_A_StopSession_offline();
          counter_faultstate_A = 0;
        }
      }
      // else if (EVSE_state == STATE_SUS)
      // {
      //   /*
      //    * @brief : To avoid immediate off in suspended state.
      //    * By G. Raja Sumant 07/06/2022 for EQC mercedes car - 600 cycles.
      //    * 150 cycles - was tried but it is being made 200 for safety.
      //    */
      //   if (counter_suspendedstate_A++ > 600)
      //   {
      //     Serial.println(F("[EVSE_A] Offline suspended for > 600 cycles"));
      //     EVSE_A_Suspended();
      //     counter_suspendedstate_A = 0;
      //   }
      //   // EVSE_A_Suspended();    //pause transaction :update suspended state is considered in charging state
      // }
      else if (EVSE_state == STATE_DIS)
      {

        Serial.println("[EVSE] Connect the Connector with EV and Try again");
        EVSE_A_StopSession_offline();
      }
    }
  }
}

void EVSE_A_LED_loop()
{

  // if not faulted and not charging then take the led status to green once every 8 seconds

  if (getChargePointStatusService_A()->getEmergencyRelayClose() == false && offline_charging_A == false)
  {
    if (millis() - timer_green_offline > 4000)
    {
      timer_green_offline = millis();
      if ((notFaulty_A == true) && EMGCY_GFCI_Fault_Occ == false)
      {
        // requestLed(GREEN, START, 1);
#if LCD_ENABLED & 0
        lcd.clear();
        lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
        lcd.print("STATUS: AVAILABLE");
        lcd.setCursor(0, 1);
        lcd.print("TAP RFID/SCAN QR");
        lcd.setCursor(0, 2);
        lcd.print("CONNECTION");
        lcd.setCursor(0, 3);
        lcd.print("CLOUD: OFFLINE");
#endif
      }
    }
  }
  /* if(getChargePointStatusService_A()->getEmergencyRelayClose() && !EMGCY_GFCI_Fault_Occ)
   {
     requestLed(RED, START, 1);
   }*/
}

#if 1

void emergencyRelayClose_Loop_A_Offl()
{

  /*
            * @brief : getOverCurrent - Stop toggling. Moving this to emergency loop.
            By G. Raja Sumant 07/06/2022
            To avoid toggling of over current and stopping of transaction

            */
  if (getChargePointStatusService_A()->getOverCurrent() == true)
  {

#if 0
    lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1);
    lcd.print("A: OVER CURRENT");
#endif
    if (offline_charging_A)
    {
      EVSE_A_StopSession_offline();
    }
  }

  if (millis() - TimerEmcy > 1000)
  {
    TimerEmcy = millis();
    bool EMGCY_status = requestEmgyStatus();
    Serial.println("EMGCY Status: " + String(EMGCY_status));
    delay(200);
    bool GFCI_status = digitalRead(GFCI_PIN);
    Serial.println("GFCI Status: " + String(GFCI_status));

    if ((EMGCY_status == true || GFCI_status == true))
    {
      Serial.println("EMGCY: ");
      getChargePointStatusService_A()->setEmergencyRelayClose(true);
      if(EMGCY_status)
      {
        
        #if 0
        lcd.clear();
        lcd.setCursor(4, 0); // Or setting the cursor in the desired position.
        lcd.print("FAULTED: EMGY");
#endif
      }
      if(GFCI_status)
      {
        #if LCD_ENABLED
        lcd.clear();
        lcd.setCursor(4, 0); // Or setting the cursor in the desired position.
        lcd.print("FAULTED: GFCI");
#endif
      }
      // requestLed(BLINKYRED, START, 1);
      if (offline_charging_A)
      {
        EVSE_A_StopSession_offline();
      }
      EMGCY_GFCI_Fault_Occ = true;
    }
    else
    {
      Serial.println("NoEMGY/GFCI Fault");
      EMGCY_GFCI_Fault_Occ = false;
    }

    if (!EMGCY_GFCI_Fault_Occ)
    {
      float volt = eic.GetLineVoltageA();
      float current = eic.GetLineCurrentA();
      float temp = eic.GetTemperature();
      Serial.println("Voltage_A: " + String(volt) + ", Current_A: " + String(current) + ", Temperature: " + String(temp));
      if (getChargePointStatusService_A() != NULL)
      {
        if (getChargePointStatusService_A()->getOverVoltage() == true ||
            getChargePointStatusService_A()->getUnderVoltage() == true ||
            getChargePointStatusService_A()->getUnderTemperature() == true ||
            getChargePointStatusService_A()->getOverTemperature() == true ||
            getChargePointStatusService_A()->getEarthDisconnect() == true ||
            getChargePointStatusService_A()->getOverCurrent() == true)
        {
          Serial.println("[EVSE_A] Fault Occurred.");
          requestLed(RED, START, 1);
          notFaulty_A = false;
          getChargePointStatusService_A()->setEmergencyRelayClose(true);
          evse_ChargePointStatus = Faulted;

          gu8_fault_occured = 1;
          if (offline_charging_A)
          {
            EVSE_A_StopSession_offline();
          }
          if (!timer_initialize_A)
          {
            timeout_start_A = millis();
            timer_initialize_A = true;
          }
        }
        else if (getChargePointStatusService_A()->getOverVoltage() == false &&
                 getChargePointStatusService_A()->getUnderVoltage() == false &&
                 getChargePointStatusService_A()->getUnderTemperature() == false &&
                 getChargePointStatusService_A()->getOverTemperature() == false &&
                 getChargePointStatusService_A()->getEarthDisconnect() == false &&
                 getChargePointStatusService_A()->getOverCurrent() == false)
        {
          Serial.println("[EVSE_A] Not Faulty.");
          notFaulty_A = true;
          getChargePointStatusService_A()->setEmergencyRelayClose(false);
          			if (gu8_fault_occured == 1)
                {
                    if (flag_evRequestsCharge_A == false) //offline_charging_A
                    {
                        gu8_fault_occured = 0;
                        evse_ChargePointStatus = Available;

                    }
                    else if (flag_evRequestsCharge_A == true)//offline_charging_A
                    {
                        gu8_fault_occured = 0;
                        evse_ChargePointStatus = Charging;

                    }
                }
          // if (!timer_initialize){
          timeout_start_A = 0;
          timer_initialize_A = false;
          //}
        }
      }
    }

    if (timeout_active_A && offline_charging_A)
    {
      if (millis() - timeout_start_A >= TIMEOUT_EMERGENCY_RELAY_CLOSE)
      {
        Serial.println("[EVSE_A] Emergency Stop.");
        flag_evRequestsCharge_A = false;
        flag_evseStopTransaction_A = true;
        timeout_active_A = false;
        timer_initialize_A = false;
      }
    }
  }
}
#endif