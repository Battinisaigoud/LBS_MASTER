// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "StopTransaction.h"
#include "OcppEngine.h"
#include "MeteringService.h"
#if LCD_ENABLED
#include "LCD_I2C.h"
extern LCD_I2C lcd;
#endif

uint8_t gu8_finishing_state = 0xff;

String lbs_energy = "";
String lbs_time = "";

extern uint8_t repeat_flag;

extern bool SessionStop;
#if DWIN_ENABLED
#include "dwin.h"

extern unsigned char kwh[8];
extern unsigned char change_page[10];
extern unsigned char HR[8];
extern unsigned char MINS[8];
extern unsigned char SEC[8];
extern unsigned char cid1[8];
extern unsigned char cid2[8];
extern unsigned char cid3[8];
#endif

extern uint8_t reasonForStop;
static const char* resonofstop_str[] = { "EmergencyStop", "EVDisconnected", "HardReset", "Local", "Other", "PowerLoss", "Reboot", "Remote", "SoftReset", "UnlockCommand", "DeAuthorized" };
extern bool reservation_start_flag;

extern float globalmeterstartA;
extern unsigned long st_timeA;
extern float globalmeterstartB;
extern int globalmeterstartC;
extern unsigned long st_timeB;
extern bool disp_evse_A;
extern bool disp_once_A;

extern uint8_t resume_stop_start_txn_A;

extern bool flag_evseIsBooted_A;
extern bool flag_evseReadIdTag_A;
extern bool flag_evseAuthenticate_A;
extern bool flag_evseStartTransaction_A;
extern bool flag_evRequestsCharge_A;
extern bool flag_evseStopTransaction_A;
extern bool flag_evseUnauthorise_A;
extern bool flag_rebootRequired_A;
extern bool flag_evseSoftReset_A;
extern Preferences energymeter;
extern uint8_t gu8_online_flag;

extern bool wifi_connect;
extern bool gsm_connect;
extern Preferences resumeTxn_A;

StopTransaction::StopTransaction(String idTag, int transactionId, int connectorId)
{
  this->idTag = idTag;
  this->transactionId = transactionId;
  this->connectorId = connectorId;
}

StopTransaction::StopTransaction()
{
}

const char* StopTransaction::getOcppOperationType()
{
  return "StopTransaction";
}

DynamicJsonDocument* StopTransaction::createReq()
{

  // String idTag = String('\0');

  // if (getChargePointStatusService() != NULL) {
  //   idTag += getChargePointStatusService()->getIdTag();
  // }

  DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(5) + (idTag.length() + 1) + (JSONDATE_LENGTH + 1));
  JsonObject payload = doc->to<JsonObject>();

  if (!idTag.isEmpty())
  { // if there is no idTag present, we shouldn't add a default one
    payload["idTag"] = idTag;
  }

  float meterStop = 0.0f;
  unsigned long stop_time = 0;
  stop_time = millis();
  if (getMeteringService() != NULL)
  {
    if (connectorId == 1)
    {

      meterStop = getMeteringService()->currentEnergy_A();
      if (meterStop > (20000000 - 100000))
      {
        energymeter.begin("MeterData", false);
        energymeter.putFloat("currEnergy_A", 0);
        energymeter.end();
        Serial.println("[Metering init] Reinitialized currEnergy_A");
      }
#if LCD_ENABLED
      if (disp_once_A)
      {
        disp_once_A = false;
        lcd.clear();
        if (gu8_online_flag == 1)
        {
          if (wifi_connect)
          {
            lcd.clear();
            lcd.setCursor(15, 0);
            lcd.print("WI-FI");
          }
          else if (gsm_connect)
          {
            lcd.clear();
            lcd.setCursor(15, 0);
            lcd.print("4G");
          }
        }
        else
        {
          lcd.clear();
          lcd.setCursor(13, 0);
          lcd.print("OFFLINE");
        }
        repeat_flag = 1;

        lcd.setCursor(0, 1);
        lcd.print("CHARGING COMPLETE");
        lcd.setCursor(0, 2);
        lcd.print("E:");
        lcd.setCursor(2, 2);
        lcd.print(String(float((meterStop - globalmeterstartA) / 1000)));
        lbs_energy = String(float((meterStop - globalmeterstartA) / 1000));
        lcd.setCursor(6, 2);
        lcd.print("kWh ;");
        // lcd.setCursor(0, 2);
        // lcd.print("Wh          :");
        // lcd.setCursor(13, 2);
        // lcd.print(meterStop - globalmeterstartA);
        unsigned long seconds = (stop_time - st_timeA) / 1000;
        int hr = seconds / 3600;                                                 // Number of seconds in an hour
        int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
        int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
        String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
        lbs_time = hrMinSec;
        lcd.setCursor(10, 2);
        lcd.print("T:");
        lcd.setCursor(12, 2);
        lcd.print(String(hrMinSec));
        delay(3000);
      }
#endif
#if DWIN_ENABLED
      uint8_t err = 0;
      stop_time = millis();
      unsigned long seconds = (stop_time - st_timeA) / 1000;
      // unsigned long seconds = (st_timeA - stop_time) / 1000;
      Serial.print("##########################################################################Stop time:");
      Serial.println(stop_time);
      Serial.print("##########################################################################Strt time:");
      Serial.println(st_timeA);
      int hr = seconds / 3600;                   // Number of seconds in an hour
      int mins = (seconds - hr * 3600) / 60;     // Remove the number of hours and calculate the minutes.
      int sec = seconds - hr * 3600 - mins * 60; // Remove the number of hours and minutes, leaving only seconds.

      Serial.print("Hours:");
      Serial.println(hr);
      Serial.print("minutes:");
      Serial.println(mins);
      Serial.print("Seconds:");
      Serial.println(sec);

      err = DWIN_SET(cid1, sizeof(cid1) / sizeof(cid1[0]));
      // Take to page 2.
      change_page[9] = 2;
      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
      // kwh[7] = float((meterStop-globalmeterstartA)/1000);
      // kwh[7] = int((meterStop-globalmeterstartA)/1000);
      int jaf = meterStop - globalmeterstartA;
      kwh[6] = jaf >> 8;
      kwh[7] = jaf & 0xff;
      // Since Kwh is with 3 decimal points multiply by 1000
      // kwh[7]  = jaf/1000;
      err = DWIN_SET(kwh, sizeof(kwh) / sizeof(kwh[0]));
      HR[7] = hr;
      MINS[7] = mins;
      SEC[7] = sec;

      Serial.print("Hours2...:");
      Serial.println(HR[7]);
      Serial.print("minutes2...:");
      Serial.println(MINS[7]);
      Serial.print("Seconds2...:");
      Serial.println(SEC[7]);

      err = DWIN_SET(HR, sizeof(HR) / sizeof(HR[0]));
      err = DWIN_SET(MINS, sizeof(MINS) / sizeof(MINS[0]));
      err = DWIN_SET(SEC, sizeof(SEC) / sizeof(SEC[0]));
      delay(3000);
      // Take to page 2.
      change_page[9] = 0;
      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
      delay(50);
#endif
    }
    else if (connectorId == 2)
    {
      //   meterStop = getMeteringService()->currentEnergy_B();
#if DWIN_ENABLED
      uint8_t err = 0;
      unsigned long seconds = (stop_time - st_timeA) / 1000;
      int hr = seconds / 3600;                   // Number of seconds in an hour
      int mins = (seconds - hr * 3600) / 60;     // Remove the number of hours and calculate the minutes.
      int sec = seconds - hr * 3600 - mins * 60; // Remove the number of hours and minutes, leaving only seconds.

      err = DWIN_SET(cid2, sizeof(cid2) / sizeof(cid2[0]));

      // Take to page 2.
      change_page[9] = 2;
      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
      // kwh[7] = float((meterStop-globalmeterstartA)/1000);
      // int jaf = int((meterStop - globalmeterstartB) / 1000);
      int jaf = int(meterStop - globalmeterstartB);
      kwh[6] = jaf >> 8;
      kwh[7] = jaf & 0xff;
      err = DWIN_SET(kwh, sizeof(kwh) / sizeof(kwh[0]));
      HR[7] = hr;
      MINS[7] = mins;
      SEC[7] = sec;
      err = DWIN_SET(HR, sizeof(HR) / sizeof(HR[0]));
      err = DWIN_SET(MINS, sizeof(MINS) / sizeof(MINS[0]));
      err = DWIN_SET(SEC, sizeof(SEC) / sizeof(SEC[0]));
      delay(3000);
      // Take to page 2.
      change_page[9] = 0;
      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
      delay(50);

#endif
    }
    else if (connectorId == 3)
    {
#if DWIN_ENABLED
      uint8_t err = 0;
      unsigned long seconds = (stop_time - st_timeA) / 1000;
      int hr = seconds / 3600;                   // Number of seconds in an hour
      int mins = (seconds - hr * 3600) / 60;     // Remove the number of hours and calculate the minutes.
      int sec = seconds - hr * 3600 - mins * 60; // Remove the number of hours and minutes, leaving only seconds.

      // Take to page 2.
      err = DWIN_SET(cid3, sizeof(cid3) / sizeof(cid3[0]));
      change_page[9] = 2;
      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
      // kwh[7] = float((meterStop-globalmeterstartA)/1000);
      // int jaf = int((meterStop - globalmeterstartC) / 1000);
      int jaf = int(meterStop - globalmeterstartC);
      kwh[6] = jaf >> 8;
      kwh[7] = jaf & 0xff;
      err = DWIN_SET(kwh, sizeof(kwh) / sizeof(kwh[0]));
      HR[7] = hr;
      MINS[7] = mins;
      SEC[7] = sec;
      err = DWIN_SET(HR, sizeof(HR) / sizeof(HR[0]));
      err = DWIN_SET(MINS, sizeof(MINS) / sizeof(MINS[0]));
      err = DWIN_SET(SEC, sizeof(SEC) / sizeof(SEC[0]));
      delay(3000);
      // Take to page 2.
      change_page[9] = 0;
      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
      delay(50);

#endif
      //  meterStop = getMeteringService()->currentEnergy_C();
    }
  }

  // 15/04/2022 updated this meterStop to int as per OCPP standard. - G. Raja Sumant

  payload["meterStop"] = int(meterStop); // TODO meterStart is required to be in Wh, but measuring unit is probably inconsistent in implementation
  char timestamp[JSONDATE_LENGTH + 1] = { '\0' };
  getJsonDateStringFromSystemTime(timestamp, JSONDATE_LENGTH);
  payload["timestamp"] = timestamp;

  // int transactionId = -1;
  // if (getChargePointStatusService() != NULL) {
  //   transactionId = getChargePointStatusService()->getTransactionId();
  // }
  payload["transactionId"] = transactionId;
#if LCD_ENABLED & 0
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH");
  lcd.setCursor(4, 1);
  lcd.print(String(float((meterStop - globalmeterstartA) / 1000)));
  lcd.setCursor(0, 2);
  lcd.print("WH");
  lcd.setCursor(4, 2);
  lcd.print(meterStop - globalmeterstartA);

  unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds / 3600;                                                 // Number of seconds in an hour
  int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
  int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.

  lcd.setCursor(0, 3);
  lcd.print("Time :");
  lcd.setCursor(4, 3);
  lcd.print(String(hrMinSec));

  delay(5000);
#endif

  payload["reason"] = resonofstop_str[reasonForStop];

  // if (getChargePointStatusService() != NULL) {
  //   getChargePointStatusService()->stopEnergyOffer();
  // }

  return doc;
}

void StopTransaction::processConf(JsonObject payload)
{

  // no need to process anything here

  // ChargePointStatusService *cpStatusService = getChargePointStatusService();
  // if (cpStatusService != NULL){
  //   //cpStatusService->stopEnergyOffer(); //No. This should remain in createReq
  //   cpStatusService->stopTransaction();
  //   cpStatusService->unauthorize();
  // }

  SmartChargingService* scService = getSmartChargingService();
  if (scService != NULL)
  {
    scService->endChargingNow();
  }

  if (DEBUG_OUT)
    Serial.print("[StopTransaction] Request has been accepted!\n");

  // Clearing the reason.
  reasonForStop = 3;
  disp_evse_A = false;
  if (SessionStop == true)
  {
    evse_ChargePointStatus = Preparing;
  }
  else
  {
    evse_ChargePointStatus = Finishing;
  }
  gu8_finishing_state = 1;
  resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
  // resumeTxn_A.putBool("ongoingTxn_A", false);
  resumeTxn_A.putString("idTagData_A", "");
  resumeTxn_A.putInt("TxnIdData_A", -1);
  resumeTxn_A.putBool("ongoingTxnoff_A", false);
  // // resumeTxn_A.putInt("reasonforstop_Off",3);
  resumeTxn_A.end();
  /*
   * @brief : Feature added by G. Raja Sumant 29/07/2022
   * This will take the charge point to reserved state when ever it is available during the reservation loop
   */
#if 1
  if (reservation_start_flag)
  {
    getChargePointStatusService_A()->setReserved(true);
    evse_ChargePointStatus = Reserved;
  }
#endif

  // if(resume_stop_start_txn_A == 1 )
  // {
  //   resume_stop_start_txn_A = 2 ;

  //   flag_evseReadIdTag_A = false;
  // 	flag_evseAuthenticate_A = false;
  // 	flag_evseStartTransaction_A = true; //Entry condition for starting transaction.
  // 	flag_evRequestsCharge_A = false;
  // 	flag_evseStopTransaction_A = false;
  // 	flag_evseUnauthorise_A = false;
  // }
}

void StopTransaction::processReq(JsonObject payload)
{
  /**
   * Ignore Contents of this Req-message, because this is for debug purposes only
   */
}

DynamicJsonDocument* StopTransaction::createConf()
{
  DynamicJsonDocument* doc = new DynamicJsonDocument(2 * JSON_OBJECT_SIZE(1));
  JsonObject payload = doc->to<JsonObject>();

  JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
  idTagInfo["status"] = "Accepted";

  return doc;
}
