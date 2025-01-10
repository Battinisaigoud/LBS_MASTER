#include "display_meterValues.h"
#include "Variants.h"
#include "ATM90E36.h"
#include "display.h"

#include "CustomGsm.h"
extern TinyGsmClient client;
uint8_t currentDisplay = 0; // Initialize to 0 (unknown state)
extern String lbs_energy;
extern float lbs_metervalues;
extern String lbs_time;

extern float powerToSend;

uint8_t repeat_flag = 0;
extern uint8_t dis_phase_type;
extern volatile float device_load;
extern ATM90E36 eic;
extern float globalmeterstartA;
extern int globalmeterstartB;
extern int globalmeterstartC;
float online_charging_Enargy_A = 0;
float online_charging_Enargy_B = 0;
float online_charging_Enargy_C = 0;
extern bool EMGCY_FaultOccured_A;
extern bool EMGCY_FaultOccured_B;
extern bool EMGCY_FaultOccured_C;
extern bool isInternetConnected;
// extern bool flag_freeze;
// extern bool flag_tapped;
// extern bool flag_unfreeze;

extern float offline_charging_Enargy_A;
extern float offline_charging_Enargy_B;
extern float offline_charging_Enargy_C;

unsigned long offline_t_A = 0;
unsigned long offline_t_B = 0;
unsigned long offline_t_C = 0;

int client_reconnect_flag = 0;

time_t lastsampledTimeA_off = 0;
extern uint8_t gu8_online_flag;
extern uint8_t gu8_rfid_tapped_A;

#if LCD_ENABLED
#include "LCD_I2C.h"
extern LCD_I2C lcd;
#endif
extern bool webSocketConncted;

extern bool wifi_connect;
extern bool gsm_connect;
extern bool ethernet_connect;
extern bool offline_charging_A;
extern float discurrEnergy_A;
extern unsigned long st_timeA;

#if DWIN_ENABLED
#include "dwin.h"
unsigned long onTime = 0;
uint8_t state_timer = 0;
uint8_t disp_evse = 0;
extern bool disp_evse_A;
extern bool disp_evse_B;
extern bool disp_evse_C;
extern bool notFaulty_A;
extern bool notFaulty_B;
extern bool notFaulty_C;
extern int8_t fault_code_A;
extern int8_t fault_code_B;
extern int8_t fault_code_C;

extern unsigned char preparing[28];
extern unsigned char fault_suspEVSE[28];
extern unsigned char fault_suspEV[28];
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
extern bool flag_faultOccured_A;
extern bool flag_faultOccured_B;
extern bool flag_faultOccured_C;
extern unsigned char v1[8];
extern unsigned char v2[8];
extern unsigned char v3[8];
extern unsigned char i1[8];
extern unsigned char i2[8];
extern unsigned char i3[8];
extern unsigned char e1[8];
extern unsigned char e2[8];
extern unsigned char e3[8];
extern unsigned char charging[28];
extern unsigned char cid1[7];
extern unsigned char cid2[7];
extern unsigned char cid3[7];
extern unsigned char unavail[30];
extern unsigned char fault_emgy[28];
extern unsigned char fault_noearth[28];
extern unsigned char fault_overVolt[28];
extern unsigned char fault_underVolt[28];
extern unsigned char fault_overTemp[28];
extern unsigned char fault_overCurr[28];
extern unsigned char fault_underCurr[28];
extern unsigned char reserved[28];
extern unsigned char clear_avail[28];
extern unsigned char wi_rfid_not_avail[22];
extern unsigned char g_rfid_not_avail[22];
extern unsigned char fault_nopower[28];
extern unsigned char eth_rfid_not_avail[25];

extern bool EMGCY_GFCI_Fault_Occ;

extern bool wifi_reconnected_flag;

extern uint8_t gu8_online_flag;

void stateTimer()
{
#if 0
  switch (state_timer)
  {
  case 0:
    onTime = millis();
    state_timer = 1;
    disp_evse = 1;
    break;
  case 1:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 2;
    }
    break;
  case 2:
    onTime = millis();
    state_timer = 3;
    disp_evse = 2;
    break;
  case 3:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 4;
    }
    break;
  case 4:
    onTime = millis();
    state_timer = 5;
    disp_evse = 3;
    break;
  case 5:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 6;
    }
    break;
  case 6:
    state_timer = 0;
  }
#endif
  state_timer = 1;
  disp_evse = 1;
}

uint8_t avail_counter = 0;

#if 1
void disp_dwin_meter()
{
  uint8_t err = 0;
  float instantCurrrent_A = eic.GetLineCurrentA();
  int instantVoltage_A = eic.GetLineVoltageA();
  float instantPower_A = 0.0f;

  if ((online_charging_Enargy_A - globalmeterstartA) > 0)
    instantPower_A = (online_charging_Enargy_A - globalmeterstartA) / 1000;
  else
    instantPower_A = 0; // Since it will initially be -ve due to float - int conversion

  // if (instantCurrrent_A < 0.15) {
  //   instantPower_A = 0;
  // } else {
  //   instantPower_A = (instantCurrrent_A * instantVoltage_A) / 1000.0;
  // }
#if 0
  float instantCurrrent_B = eic.GetLineCurrentB();
  int instantVoltage_B = eic.GetLineVoltageB();
  float instantPower_B = 0.0f;

  if (instantCurrrent_B < 0.15) {
    instantPower_B = 0;
  }
  else {
    instantPower_B = (instantCurrrent_B * instantVoltage_B) / 1000.0;
  }

  float instantCurrrent_C = eic.GetLineCurrentC();
  int instantVoltage_C = eic.GetLineVoltageC();
  float instantPower_C = 0.0f;

  if (instantCurrrent_C < 0.15) {
    instantPower_C = 0;
  }
  else {
    instantPower_C = (instantCurrrent_C * instantVoltage_C) / 1000.0;
  }
#endif
  switch (disp_evse)
  {
  case 1:
    if (disp_evse_A)
    {
      if (notFaulty_A && !EMGCY_GFCI_Fault_Occ)
      {
        change_page[9] = 4;

        v1[4] = 0X6A;
        instantVoltage_A = instantVoltage_A * 10;
        v1[6] = instantVoltage_A >> 8;
        v1[7] = instantVoltage_A & 0xff;
        i1[4] = 0X77;
        // i1[7] = instantCurrrent_A * 10;
        int S_instantCurrent_A = instantCurrrent_A * 10;
        i1[6] = S_instantCurrent_A >> 8;
        i1[7] = S_instantCurrent_A & 0xff;
        e1[4] = 0X6E;
        e1[7] = instantPower_A * 100;
        err = DWIN_SET(cid1, sizeof(cid1) / sizeof(cid1[0]));
        err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0
        charging[4] = 0X66;
        charging[5] = 0X00;
        err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));
        err = DWIN_SET(v1, sizeof(v1) / sizeof(v1[0]));
        err = DWIN_SET(i1, sizeof(i1) / sizeof(i1[0]));
        err = DWIN_SET(e1, sizeof(e1) / sizeof(e1[0]));
      }
    }
    else
    {

      ChargePointStatus inferencedStatus;
      inferencedStatus = getChargePointStatusService_A()->inferenceStatus();

      switch (inferencedStatus)
      {
      case ChargePointStatus::Preparing:
        err = DWIN_SET(preparing, sizeof(preparing) / sizeof(preparing[0]));
        break;
      case ChargePointStatus::SuspendedEVSE:
        err = DWIN_SET(fault_suspEVSE, sizeof(fault_suspEVSE) / sizeof(fault_suspEVSE[0]));
        break;
      case ChargePointStatus::SuspendedEV:
        err = DWIN_SET(fault_suspEV, sizeof(fault_suspEV) / sizeof(fault_suspEV[0]));
        break;
      case ChargePointStatus::Charging:
        charging[4] = 0X66;
        charging[5] = 0X00;
        err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));
        break;
      case ChargePointStatus::Available:

        if (wifi_connect && wifi_reconnected_flag)
        {
          avail[4] = 0x55;
          err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          delay(10);
          avail[4] = 0x51;
          err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
          err = DWIN_SET(wi, sizeof(wi) / sizeof(wi[0]));
        }
        if (gsm_connect)
        {
          avail[4] = 0x55;
          err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
          delay(10);
          avail[4] = 0x51;
          err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
          err = DWIN_SET(g, sizeof(g) / sizeof(g[0]));
        }
        if (ethernet_connect)
        {
          avail[4] = 0x51;
          err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
          err = DWIN_SET(et, sizeof(et) / sizeof(et[0]));
        }

        break;
      case ChargePointStatus::Unavailable:
        client_reconnect_flag = 1;
        avail[4] = 0x51;
        err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
        err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
        delay(10);
        cloud_no_rfid_dwin_print();

        break;
      case ChargePointStatus::Reserved:
        client_reconnect_flag = 1;
        avail[4] = 0x55;
        err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
        delay(10);
        reserved[4] = 0x55;
        err = DWIN_SET(reserved, sizeof(reserved) / sizeof(reserved[0]));
        delay(10);
        cloud_no_rfid_dwin_print();

        break;
      case ChargePointStatus::Faulted:
        client_reconnect_flag = 1;
        switch (fault_code_A)
        {
        case -1:
          break; // It means default.
        case 0:
          if (instantVoltage_A > 275)
          {
            fault_overVolt[4] = 0x55;
            err = DWIN_SET(fault_overVolt, sizeof(fault_overVolt) / sizeof(fault_overVolt[0]));
            delay(10);
            cloud_no_rfid_dwin_print();
          }
          break;
        case 1:
          // if(instantVoltage_A>20)
          // {
          fault_underVolt[4] = 0x55;
          err = DWIN_SET(fault_underVolt, sizeof(fault_underVolt) / sizeof(fault_underVolt[0]));
          cloud_no_rfid_dwin_print();
          //   }
          // else
          // {
          // err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
          //  cloud_no_rfid_dwin_print();
          // }
          break;
        case 2:
          fault_overCurr[4] = 0x55;
          err = DWIN_SET(fault_overCurr, sizeof(fault_overCurr) / sizeof(fault_overCurr[0]));
          cloud_no_rfid_dwin_print();
          break;
        case 3:
          fault_underCurr[4] = 0x55;
          err = DWIN_SET(fault_underCurr, sizeof(fault_underCurr) / sizeof(fault_underCurr[0]));
          cloud_no_rfid_dwin_print();

          break;
        case 4:
          fault_overTemp[4] = 0x55;
          err = DWIN_SET(fault_overTemp, sizeof(fault_overTemp) / sizeof(fault_overTemp[0]));
          cloud_no_rfid_dwin_print();
          break;
        case 5:
          // lcd.print("A: UNDER TEMPERATURE");
          break;
        case 6:
          // lcd.print("A: GFCI"); // Not implemented in AC001
          break;
        case 7:
          delay(10);
          fault_noearth[4] = 0x55;
          err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
          cloud_no_rfid_dwin_print();
          break;
        case 8:
          //  notFaulty_A = false;
          //  delay(10);
          fault_emgy[4] = 0x55;
          err = DWIN_SET(fault_emgy, sizeof(fault_emgy) / sizeof(fault_emgy[0]));
          // avail[4] = 0x51;
          // err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
          cloud_no_rfid_dwin_print();
          break;
        case 9:
          // lcd.print("A-POWER FAIL");
          break;
        default:
          // lcd.print("*****FAULTED A*****"); // You can make spaces using well... spacesbreak;
          break;
        }
      }
    }
#if 0
  case 2: if (disp_evse_B)
  {
    if (notFaulty_B && !EMGCY_FaultOccured_B)
    {

      change_page[9] = 5;

      v2[4] = 0X75;
      instantVoltage_B = instantVoltage_B * 10;
      v2[6] = instantVoltage_B >> 8;
      v2[7] = instantVoltage_B & 0xff;

      i2[4] = 0X77;
      i2[7] = instantCurrrent_B * 10;

      e2[4] = 0X79;
      e2[7] = instantPower_B * 10;

      err = DWIN_SET(cid2, sizeof(cid2) / sizeof(cid2[0]));

      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0

      charging[4] = 0X71;
      charging[5] = 0X00;
      err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));


      err = DWIN_SET(v2, sizeof(v2) / sizeof(v2[0]));

      err = DWIN_SET(i2, sizeof(i2) / sizeof(i2[0]));

      err = DWIN_SET(e2, sizeof(e2) / sizeof(e2[0]));

    }
  }

        break;
  case 3: if (disp_evse_C)
  {
    if (notFaulty_C && !EMGCY_FaultOccured_C)
    {
      change_page[9] = 6;

      v3[4] = 0X7F;
      instantVoltage_C = instantVoltage_C * 10;
      v3[6] = instantVoltage_C >> 8;
      v3[7] = instantVoltage_C & 0xff;

      i3[4] = 0X82;
      i3[7] = instantCurrrent_C * 10;

      e3[4] = 0X84;
      e3[7] = instantPower_C * 10;
      err = DWIN_SET(cid3, sizeof(cid3) / sizeof(cid3[0]));

      err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0

      charging[4] = 0X7B;
      charging[5] = 0X00;
      err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));

      err = DWIN_SET(v3, sizeof(v3) / sizeof(v3[0]));

      err = DWIN_SET(i3, sizeof(i3) / sizeof(i3[0]));

      err = DWIN_SET(e3, sizeof(e3) / sizeof(e3[0]));
    }
  }


        break;
#endif
  default:
    Serial.println(F("**Display default**"));
    break;
  }
}
#endif
#endif

#if DISPLAY_ENABLED
unsigned long onTime = 0;
uint8_t state_timer = 0;
uint8_t disp_evse = 0;
extern bool disp_evse_A;
extern bool disp_evse_B;
extern bool disp_evse_C;
extern bool notFaulty_A;
extern bool notFaulty_B;
extern bool notFaulty_C;
extern int8_t fault_code_A;
extern int8_t fault_code_B;
extern int8_t fault_code_C;
void stateTimer()
{
  switch (state_timer)
  {
  case 0:
    onTime = millis();
    state_timer = 1;
    disp_evse = 1;
    break;
  case 1:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 2;
    }
    break;
  case 2:
    onTime = millis();
    state_timer = 3;
    disp_evse = 2;
    break;
  case 3:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 4;
    }
    break;
  case 4:
    onTime = millis();
    state_timer = 5;
    disp_evse = 3;
    break;
  case 5:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 6;
    }
    break;
  case 6:
    state_timer = 0;
  }
}

uint8_t avail_counter = 0;

void disp_lcd_meter()
{
  float instantCurrrent_A = eic.GetLineCurrentA();
  float instantVoltage_A = eic.GetLineVoltageA();
  float instantPower_A = 0.0f;

  if (instantCurrrent_A < 0.15)
  {
    instantPower_A = 0;
  }
  else
  {
    instantPower_A = (instantCurrrent_A * instantVoltage_A) / 1000.0;
  }

  float instantCurrrent_B = eic.GetLineCurrentB();
  int instantVoltage_B = eic.GetLineVoltageB();
  float instantPower_B = 0.0f;

  if (instantCurrrent_B < 0.15)
  {
    instantPower_B = 0;
  }
  else
  {
    instantPower_B = (instantCurrrent_B * instantVoltage_B) / 1000.0;
  }

  float instantCurrrent_C = eic.GetLineCurrentC();
  int instantVoltage_C = eic.GetLineVoltageC();
  float instantPower_C = 0.0f;

  if (instantCurrrent_C < 0.15)
  {
    instantPower_C = 0;
  }
  else
  {
    instantPower_C = (instantCurrrent_C * instantVoltage_C) / 1000.0;
  }
  switch (disp_evse)
  {
  case 1:
    if (disp_evse_A)
    {
      if (notFaulty_A && !EMGCY_FaultOccured_A)
      {
        // connector, voltage, current, power
        displayEnergyValues_Disp_AC("1", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
      }
      else
      {
        avail_counter++;
        switch (fault_code_A)
        {
        case -1:
          break; // It means default.
        case 0:
          displayEnergyValues_Disp_AC("1-Over Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 1:
          displayEnergyValues_Disp_AC("1-Under Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 2:
          displayEnergyValues_Disp_AC("1-Over Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 3:
          displayEnergyValues_Disp_AC("1-Under Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 4:
          displayEnergyValues_Disp_AC("1-Over Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));

          break;
        case 5:
          displayEnergyValues_Disp_AC("1-Under Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 6:
          displayEnergyValues_Disp_AC("1-GFCI", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 7:
          displayEnergyValues_Disp_AC("1-Earth Disc", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        default:
          Serial.println(F("Default in display"));
        }
      }
      checkForResponse_Disp();
    }
    else // we shall use this case to refresh home page.
    {
      if (!notFaulty_A || EMGCY_FaultOccured_A)
      {
        avail_counter++;
      }
    }

    break;
  case 2:
    if (disp_evse_B)
    {
      if (notFaulty_B && !EMGCY_FaultOccured_B)
      {
        displayEnergyValues_Disp_AC("2", String(instantVoltage_B), String(instantCurrrent_B), String(instantPower_B));
      }
      else
      {
        avail_counter++;
        switch (fault_code_B)
        {
        case -1:
          break; // It means default.
        case 0:
          displayEnergyValues_Disp_AC("2-Over Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 1:
          displayEnergyValues_Disp_AC("2-Under Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 2:
          displayEnergyValues_Disp_AC("2-Over Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 3:
          displayEnergyValues_Disp_AC("2-Under Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 4:
          displayEnergyValues_Disp_AC("2-Over Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));

          break;
        case 5:
          displayEnergyValues_Disp_AC("2-Under Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 6:
          displayEnergyValues_Disp_AC("2-GFCI", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 7:
          displayEnergyValues_Disp_AC("2-Earth Disc", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        default:
          Serial.println(F("Default in display"));
        }
      }
      checkForResponse_Disp();
    }
    else
    {
      if (!notFaulty_B || EMGCY_FaultOccured_B)
      {
        avail_counter++;
      }
    }

    break;
  case 3:
    if (disp_evse_C)
    {
      if (notFaulty_C && !EMGCY_FaultOccured_C)
      {
        displayEnergyValues_Disp_AC("3", String(instantVoltage_C), String(instantCurrrent_C), String(instantPower_C));
      }
      else
      {
        avail_counter++;
        switch (fault_code_C)
        {
        case -1:
          break; // It means default.
        case 0:
          displayEnergyValues_Disp_AC("3-Over Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 1:
          displayEnergyValues_Disp_AC("3-Under Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 2:
          displayEnergyValues_Disp_AC("3-Over Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 3:
          displayEnergyValues_Disp_AC("3-Under Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 4:
          displayEnergyValues_Disp_AC("3-Over Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));

          break;
        case 5:
          displayEnergyValues_Disp_AC("3-Under Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 6:
          displayEnergyValues_Disp_AC("3-GFCI", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        case 7:
          displayEnergyValues_Disp_AC("3-Earth Disc", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
          break;
        default:
          Serial.println(F("Default in display"));
        }
      }

      checkForResponse_Disp();
    }
    else
    {
      if (!notFaulty_C || EMGCY_FaultOccured_C)
      {
        avail_counter++;
      }
    }

    break;
  default:
    Serial.println(F("**Display default**"));
    break;
  }
  /*
    @brief: If all 3 are faulted, then set the response to rfid unavailable.
  */
  if (avail_counter == 3)
  {
    setHeader("RFID UNAVAILABLE");
    checkForResponse_Disp();
    avail_counter = 0;
  }

  if (flag_tapped)
  {
    setHeader("TAP RFID TO START/STOP");
    checkForResponse_Disp();
    flag_tapped = false;
  }

  if (flag_unfreeze)
  {
    if (disp_evse_A == false)
    {
      if (disp_evse_B == false)
      {
        if (disp_evse_C == false)
        {
          flag_freeze = false;
          flag_unfreeze = false;
        }
      }
    }
  }

  // if(disp_evse_A || disp_evse_B || disp_evse_C)
  if (flag_freeze)
  {
    Serial.println(F("**skip**"));
  }
  else
  {
    if (isInternetConnected)
    {
      if (notFaulty_A && !EMGCY_FaultOccured_A && !disp_evse_A)
      {
        connAvail(1, "AVAILABLE");
        checkForResponse_Disp();
        setHeader("TAP RFID TO START/STOP");
        checkForResponse_Disp();
      }
      if (notFaulty_B && !EMGCY_FaultOccured_B && !disp_evse_B)
      {
        connAvail(2, "AVAILABLE");
        checkForResponse_Disp();
        setHeader("TAP RFID TO START/STOP");
        checkForResponse_Disp();
      }
      if (notFaulty_C && !EMGCY_FaultOccured_C && !disp_evse_C)
      {
        connAvail(3, "AVAILABLE");
        checkForResponse_Disp();
        setHeader("TAP RFID TO START/STOP");
        checkForResponse_Disp();
      }
    }
  }
}

#endif

#if LCD_ENABLED
unsigned long onTime = 0;
uint8_t state_timer = 0;
uint8_t disp_evse = 0;
extern bool disp_evse_A;
// extern bool disp_evse_B;
// extern bool disp_evse_C;
extern bool notFaulty_A;
// extern bool notFaulty_B;
// extern bool notFaulty_C;
extern int8_t fault_code_A;
// extern int8_t fault_code_B;
// extern int8_t fault_code_C;
void stateTimer()
{
#if 0
  switch (state_timer)
  {

  case 0:
    onTime = millis();
    state_timer = 1;
    disp_evse = 1;
    break;
    //Since there is only 1 connector
  case 1:
    state_timer = 0;

  case 1:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 2;
    }
    break;
  case 2:
    onTime = millis();
    state_timer = 3;
    disp_evse = 2;
    break;
  case 3:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 4;
    }
    break;
  case 4:
    onTime = millis();
    state_timer = 5;
    disp_evse = 3;
    break;
  case 5:
    if ((millis() - onTime) > 3000)
    {
      state_timer = 6;
    }
    break;
  case 6:
    state_timer = 0;

  default: Serial.printf("***Default vaiko! %d****", state_timer);
    state_timer = 0;
    break;
  }
#endif
#if 1
  state_timer = 1;
  disp_evse = 1;
#endif
}

void disp_lcd_meter()
{
  float instantCurrrent_A = eic.GetLineCurrentA();
  float instantVoltage_A = eic.GetLineVoltageA();
  float instantPower_A = 0.0f;
  // Serial.printf("The value of state_timer is %d and disp_evse is %d",state_timer,disp_evse);
  Serial.print("The value of state_timer is:" + String(state_timer));
  Serial.println("and disp_evse is:" + String(disp_evse));

  if (instantCurrrent_A < 0.15)
  {
    instantPower_A = 0;
  }
  else
  {
    instantPower_A = (instantCurrrent_A * instantVoltage_A) / 1000.0;
    // powerToSend = instantPower_A;
  }

#if 1
  if (webSocketConncted == 1)
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
      lcd.setCursor(18, 0);
      lcd.print("4G");
    }
  }
  else
  {
    lcd.clear();
    lcd.setCursor(17, 0);
    lcd.print("OFL");
  }

#endif

#if 0

  if (webSocketConncted == 1)
  {
    if (wifi_connect && currentDisplay != 1)
    {
      lcd.clear();
      lcd.setCursor(15, 0);
      lcd.print("WI-FI");
      currentDisplay = 1; // Update to Wi-Fi state
    }
    else if (gsm_connect && currentDisplay != 2)
    {
      lcd.clear();
      lcd.setCursor(18, 0);
      lcd.print("4G");
      currentDisplay = 2; // Update to 4G state
    }
  }
  else if (currentDisplay != 3) // Check if already in the "OFFLINE" state
  {
    lcd.clear();
    lcd.setCursor(12, 0);
    lcd.print("OFFLINE");
    currentDisplay = 3; // Update to "OFFLINE" state
  }

#endif

  switch (evse_ChargePointStatus)
  {
  case Available:
    // requestLed(GREEN, START, 1);
    Serial.println("Available");
    repeat_flag = 0;
    if (webSocketConncted == 1)
    {
      lcd.setCursor(0, 2);
      // lcd.print("OUTPUT AVAILABLE");
      lcd.print("STATUS: AVAILABLE");
      // lcd.setCursor(0, 2);
      // lcd.print("TAP RFID/SCAN QR");
      // lcd.print("CONNECT VEHICLE");
      // lcd.setCursor(0, 3);
      // lcd.print("TO START");
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("CHARGER UNAVAILABLE");
    }

    break;
  case Preparing:
    Serial.print("Preparing");

    if (webSocketConncted == 1)
    {
      if (gu8_rfid_tapped_A == 1 && repeat_flag == 0)
      {
        //   // requestLed(BLINKYBLUE, START, 1);
        //   // lcd.clear();
        lcd.setCursor(0, 2);
        //   // lcd.print("OUTPUT PREPARING");
        // lcd.print("EV PLUGGED-IN");
        //   // lcd.setCursor(0, 2);
        //   // lcd.print("SCAN DONE");
        //   // lcd.setCursor(0, 3);
        //   // lcd.print("CONNECTOR");
        // lcd.setCursor(0, 2);
        lcd.print("RFID TAPPED");
      }
      else if (repeat_flag == 0)
      {
        // requestLed(BLUE, START, 1);
        // lcd.clear();

        lcd.setCursor(0, 2);
        // lcd.print("OUTPUT PREPARING");
        lcd.print("EV PLUGGED-IN");
        // lcd.setCursor(0, 2);
        // lcd.print("TAP RFID/SCAN QR");
      }
      else if (repeat_flag == 1)
      {
#if 0
        // repeat_flag = 0;
        lcd.setCursor(0, 1);
        lcd.print("STATUS :FINISHING");
        lcd.setCursor(0, 2);
        lcd.print("TRANSACTION DONE");
#endif

        lcd.setCursor(0, 1);
        lcd.print("CHARGING COMPLETE");
        lcd.setCursor(0, 2);
        lcd.print("E:");
        lcd.setCursor(2, 2);//lbs_energy
        lcd.print(String(lbs_energy));
        lcd.setCursor(6, 2);
        lcd.print("kWh ;");
        lcd.setCursor(10, 2);
        lcd.print("T:");
        lcd.setCursor(12, 2);
        lcd.print(String(lbs_time));
        repeat_flag = 1;
      }
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("CHARGER UNAVAILABLE");
    }

    // lcd.setCursor(0, 2);
    // lcd.print("AUTHENTICATING");
    // gu8_rfidflag = 1;
    break;
  case Charging:
    // requestLed(BLINKYGREEN, START, 1);
    gu8_rfid_tapped_A = 0;
    Serial.print("Charging");
    switch (disp_evse)
    {
    case 1:
      if (disp_evse_A)
      {
        lcd.clear();
        lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
        if (notFaulty_A)
        {
          lcd.clear();
          if (webSocketConncted == 1)
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
          else if (webSocketConncted == 0)
          {
            lcd.setCursor(17, 0);
            lcd.print("OFL");
          }
          lcd.setCursor(0, 0);
          lcd.print("CHARGING");
          switch (dis_phase_type)
          {
          case 0:
            lcd.setCursor(0, 1);
            lcd.print("PHASE : R");
            // lcd.setCursor(15, 1);
            // lcd.print(device_load);

            break;

          case 1:
            lcd.setCursor(0, 1);
            lcd.print("PHASE : Y");
            // lcd.setCursor(15, 1);
            // lcd.print(device_load);
            break;

          case 2:
            lcd.setCursor(0, 1);
            lcd.print("PHASE : B");
            // lcd.setCursor(15, 1);
            // lcd.print(device_load);

            break;

          default:
            break;
          }
          lcd.setCursor(12, 1);
          lcd.print("P:");
          lcd.setCursor(14, 1); // Or setting the cursor in the desired position.
          lcd.print(String(instantPower_A));
          lcd.setCursor(18, 1);
          lcd.print("KW");
          lcd.setCursor(0, 2);
          lcd.print("E:");
          lcd.setCursor(2, 2);
          if ((online_charging_Enargy_A - globalmeterstartA) > 0)
          {
            lcd.print(String(discurrEnergy_A));
          }
          // lcd.print(String(globalmeterstartA - lbs_metervalues));
          // lcd.print(String(lbs_metervalues - globalmeterstartA));
          lcd.setCursor(7, 2);
          lcd.print("kWh");
          lcd.setCursor(12, 2);
          lcd.print("V:");
          lcd.setCursor(14, 2); // Or setting the cursor in the desired position.
          lcd.print(String(instantVoltage_A));
          lcd.setCursor(19, 2);
          lcd.print("V");
          // lcd.setCursor(0, 1);
          // lcd.print("CHARGING @");
          // lcd.setCursor(0, 0);
          // lcd.print("STATUS: CHARGING");
          // lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
          // lcd.print(String(instantPower_A));
          // lcd.setCursor(0, 2);
          // lcd.print("ENERGY(kWh):       ");
          // lcd.setCursor(12, 2);

          // lcd.print("*****CHARGING 1*****"); // You can make spaces using well... spaces
          // lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
          // lcd.print("VOLTAGE(v):");
          // lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
          // lcd.print(String(instantVoltage_A));
          // lcd.setCursor(0, 2);
          // lcd.print("CURRENT(A):");
          // lcd.setCursor(12, 2); // Or setting the cursor in the desired position.
          // lcd.print(String(instantCurrrent_A));
          // lcd.setCursor(0, 3);
          /*
          lcd.print("POWER(KW) :");
          lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
          lcd.print(String(instantPower_A));
          */

          // lcd.setCursor(0, 3);
          // lcd.print("KWH       :");
          // lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
          if (!offline_charging_A)
          {
            // float online_charging_Enargy_A = meteringService->currentEnergy_A();
            // lcd.print(String(float((online_charging_Enargy_A-globalmeterstartA)/1000)));
            // energymeter.begin("MeterData",false);
            //  float online_charging_Enargy_A = energymeter.getFloat("currEnergy_A",0);
            // energymeter.end();
            if ((online_charging_Enargy_A - globalmeterstartA) > 0)
            {
              // lcd.print(String(discurrEnergy_A));
            }
            // lcd.print(String(float((online_charging_Enargy_A - globalmeterstartA) / 1000)));
            else
              // lcd.print("0");
              Serial.println("[DISP]* online charging**");
          }
          else
          {
            // lcd.print(String(float((offline_charging_Enargy_A - globalmeterstartA) / 1000)));
            Serial.println("[DISP] offline charging**");
          }
          unsigned long stop_time = millis();
#if 0
          lcd.setCursor(0, 3);
          lcd.print("TIME       :");
          lcd.setCursor(13, 3); // Or setting the cursor in the desired position.
          unsigned long seconds = (stop_time - st_timeA) / 1000;
          int hr = seconds / 3600;                                                 // Number of seconds in an hour
          int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
          int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
          String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
          lcd.print(String(hrMinSec));
#endif
          lcd.setCursor(0, 3);
          lcd.print("T:");
          lcd.setCursor(2, 3);

          unsigned long seconds = (stop_time - st_timeA) / 1000;
          int hr = seconds / 3600;          // Number of hours
          int mins = (seconds % 3600) / 60; // Remaining minutes
          int sec = seconds % 60;           // Remaining seconds

          String hrs = (hr < 10) ? "0" + String(hr) : String(hr);
          String minss = (mins < 10) ? "0" + String(mins) : String(mins);
          String secs = (sec < 10) ? "0" + String(sec) : String(sec);
          // Helper function to add leading zero
          // auto addLeadingZero = [](int value)
          // {
          //   return (value < 10) ? "0" + String(value) : String(value);
          // };

          // // Format time as HH:MM:SS
          // String hrMinSec = addLeadingZero(hr) + ":" + addLeadingZero(mins) + ":" + addLeadingZero(sec);

          String hrMinSec = (String(hrs) + ":" + String(minss) + ":" + String(secs));
          lcd.print(hrMinSec);
          lcd.setCursor(12, 3);
          lcd.print("C:");
          lcd.setCursor(14, 3); // Or setting the cursor in the desired position.
          lcd.print(String(instantCurrrent_A));
          lcd.setCursor(19, 3);
          lcd.print("A");

        }
        else
        {
          Serial.println("Something wrong [Display A]");
#if 0
          switch (fault_code_A)
          {
          case -1: break; //It means default.
          case 0: lcd.print("Connector1-Over Voltage");
            break;
          case 1: lcd.print("Connector1-Under Voltage");
            break;
          case 2: lcd.print("Connector1-Over Current");
            break;
          case 3: lcd.print("Connector1-Under Current");
            break;
          case 4: lcd.print("Connector1-Over Temp");
            break;
          case 5: lcd.print("Connector1-Under Temp");
            break;
          case 6: lcd.print("Connector1-GFCI"); // Not implemented in AC001
            break;
          case 7: lcd.print("Connector1-Earth Disc");
            break;
          default: lcd.print("*****FAULTED 1*****"); // You can make spaces using well... spacesbreak;
          }
#endif
        }
      }
    }
    break;
  case SuspendedEVSE:
    requestLed(BLINKYBLUE, START, 1);
    Serial.print("SuspendedEVSE");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print("READY TO CHARGE");
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    break;
  case SuspendedEV:
    Serial.print("SuspendedEV");
    // lcd.setCursor(0, 1);
    // lcd.print("                    ");
    // lcd.setCursor(0, 2);
    // lcd.print("SuspendedEV");
    // lcd.setCursor(0, 3);
    // lcd.print("                    ");
    break;
  case Finishing:
    // requestLed(BLUE, START, 1);
    // lcd.clear();
    if (webSocketConncted == 1)
    {
#if 0
      lcd.setCursor(0, 1);
      lcd.print("STATUS :FINISHING");
      lcd.setCursor(0, 2);
      lcd.print("TRANSACTION DONE");

#endif
      lcd.setCursor(0, 1);
      lcd.print("CHARGING COMPLETE");
      lcd.setCursor(0, 2);
      lcd.print("E:");
      lcd.setCursor(2, 2);//lbs_energy
      lcd.print(String(lbs_energy));
      lcd.setCursor(6, 2);
      lcd.print("kWh ;");
      lcd.setCursor(10, 2);
      lcd.print("T:");
      lcd.setCursor(12, 2);
      lcd.print(String(lbs_time));




      repeat_flag = 1;
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("CHARGER UNAVAILABLE");
    }

    Serial.print("Finishing");
    break;
  case Reserved:
    // requestLed(BLUE, START, 1);
    Serial.print("Reserved");
    break;
  case Unavailable:
    // requestLed(BLINKYWHITE, START, 1);
    Serial.print("Unavailable");
    // lcd.clear();
    // lcd.setCursor(12, 0);
    // lcd.print("OFFLINE");
    lcd.setCursor(0, 1);
    lcd.print("OUTPUT :UNAVAILABLE");
    break;
  case Faulted:
    Serial.print("Faulted");
    switch (fault_code_A)
    {
    case 0:
      // ADDED BY SAI
      // requestLed(RED, START, 1);
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("CHARGER FAULTED ");
      lcd.setCursor(0, 2);
      lcd.print("OVER VOLTAGE");

      // gu8_fault_flag = 1;

      break;
    case 1:
      // requestLed(RED, START, 1);
      if (instantVoltage_A > 20)
      {
        // lcd.clear();
        lcd.setCursor(0, 1);
        lcd.print("CHARGER FAULTED");
        lcd.setCursor(0, 2);
        lcd.print("UNDER VOLTAGE");
      }
      else
      {
        lcd.setCursor(0, 1);
        lcd.print(" CHARGER FAULTED    ");
        lcd.setCursor(0, 2);
        lcd.print("   POWER FAILURE    ");
        lcd.setCursor(0, 3);
        lcd.print("                    ");
      }
      // gu8_fault_flag = 2;
      // lcd.print("A: UNDER VOLTAGE");
      // else
      // lcd.print("A: NO POWER");

      break;
    case 2:
      // requestLed(RED, START, 1);
      // lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("CHARGER FAULTED");
      lcd.setCursor(0, 2);
      lcd.print("OVER CURRENT");

      // lcd.print("A: OVER CURRENT");
      break;
    case 3:
      // ADDED BY SAI
      // requestLed(RED, START, 1);
      // lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("CHARGER FAULTED");
      lcd.setCursor(0, 2);
      lcd.print("UNDER CURRENT");

      // lcd.print("A: UNDER CURRENT");
      break;
    case 4:
      // requestLed(RED, START, 1);
      lcd.print("A: OVER TEMPERATURE");
      break;
    case 5:
      // requestLed(RED, START, 1);
      lcd.print("A: UNDER TEMPERATURE");
      break;
    case 6:
      // ADDED BY SAI
      // requestLed(RED, START, 1);
      // lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("CHARGER FAULTED");
      lcd.setCursor(0, 2);
      lcd.print("GFCI                ");

      // gu8_fault_flag = 3;
      // lcd.print("A: GFCI"); // Not implemented in AC001
      break;
    case 7:
      // added by sai
      // requestLed(RED, START, 1);
      // lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("CHARGER FAULTED");
      lcd.setCursor(0, 2);
      lcd.print("EARTH DISCONNECT    ");

      // gu8_fault_flag = 4;
      // lcd.print("A:EARTH DISCONNECTED");
      break;
    case 8:
      // added by sai
      // requestLed(BLINKYRED, START, 1);
      // lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("CHARGER FAULTED");
      lcd.setCursor(0, 2);
      lcd.print("EMERGENCY           ");

      // gu8_fault_flag = 5;
      break;
    case 9:
      lcd.setCursor(0, 1);
      lcd.print(" CHARGER FAULTED    ");
      lcd.setCursor(0, 2);
      lcd.print("   POWER FAILURE    ");
      lcd.setCursor(0, 3);
      lcd.print("                    ");
      break;
    default:

      break;
    }

    break;
  default:

    break;
  }
}
#if 0
void disp_lcd_meter()
{
  float instantCurrrent_A = eic.GetLineCurrentA();
  float instantVoltage_A = eic.GetLineVoltageA();
  float instantPower_A = 0.0f;
  // Serial.printf("The value of state_timer is %d and disp_evse is %d",state_timer,disp_evse);
  Serial.print("The value of state_timer is:" + String(state_timer));
  Serial.println("and disp_evse is:" + String(disp_evse));

  if (instantCurrrent_A < 0.15) {
    instantPower_A = 0;
  }
  else {
    instantPower_A = (instantCurrrent_A * instantVoltage_A) / 1000.0;
  }

#if 0
  float instantCurrrent_B = eic.GetLineCurrentB();
  int instantVoltage_B = eic.GetLineVoltageB();
  float instantPower_B = 0.0f;

  if (instantCurrrent_B < 0.15) {
    instantPower_B = 0;
  }
  else {
    instantPower_B = (instantCurrrent_B * instantVoltage_B) / 1000.0;
  }

  float instantCurrrent_C = eic.GetLineCurrentC();
  int instantVoltage_C = eic.GetLineVoltageC();
  float instantPower_C = 0.0f;

  if (instantCurrrent_C < 0.15) {
    instantPower_C = 0;
  }
  else {
    instantPower_C = (instantCurrrent_C * instantVoltage_C) / 1000.0;
  }
#endif



  ChargePointStatus inferencedStatus;
  inferencedStatus = getChargePointStatusService_A()->inferenceStatus();
  lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
  lcd.print("                    ");
  lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
  switch (inferencedStatus)
  {
  case ChargePointStatus::Preparing:
    lcd.print("PREPARING");
    break;
  case ChargePointStatus::SuspendedEVSE:
    lcd.print("SuspendedEVSE");
    break;
  case ChargePointStatus::SuspendedEV:
    lcd.print("SuspendedEV");
    break;
  case ChargePointStatus::Charging:
    lcd.print("CHARGING");
    break;
  case ChargePointStatus::Available:
    lcd.print("STATUS: AVAILABLE");
    if (isInternetConnected)
    {
      if (wifi_connect)
      {
        lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
        lcd.print("                    "); // Clear the line
        lcd.setCursor(0, 3);
        lcd.print("CLOUD: WIFI. TAP RFID");
      }
      if (gsm_connect && client.connected())
      {
        lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
        lcd.print("                    "); // Clear the line
        lcd.setCursor(0, 3);
        lcd.print("CLOUD: 4G. TAP RFID");
      }
      if (ethernet_connect)
      {
        lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
        lcd.print("                    "); // Clear the line
        lcd.setCursor(0, 3);
        lcd.print("CLOUD: ETH. TAP RFID");
      }
      lcd.setCursor(0, 2); // Or setting the cursor in the desired position.
      lcd.print("                    ");
    }
    else
    {
      lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
      lcd.print("                    ");
      lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
      lcd.print("STATUS: UNAVAILABLE");
      lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
      lcd.print("                    "); // Clear the line
      lcd.setCursor(0, 3);
      lcd.print("OFFLINE. NO RFID!");
    }

    break;
  case ChargePointStatus::Unavailable:
    lcd.print("STATUS: UNAVAILABLE");
    cloud_no_rfid_lcd_print();
    break;
  case ChargePointStatus::Reserved:
    lcd.print("STATUS: RESERVED");
    cloud_no_rfid_lcd_print();
    break;
  case ChargePointStatus::Faulted:
    switch (fault_code_A)
    {
    case -1:
      break; // It means default.
    case 0:
      if (instantVoltage_A > 275)
      {
        lcd.print("OVER VOLTAGE");
        cloud_no_rfid_lcd_print();
      }
      break;
    case 1:
      if (instantVoltage_A > 20)
      {
        lcd.print("UNDER VOLTAGE");
        cloud_no_rfid_lcd_print();
      }
      else
      {
        lcd.print("NO POWER");
        cloud_no_rfid_lcd_print();
      }
      break;
    case 2:
      lcd.print("OVER CURRENT");
      cloud_no_rfid_lcd_print();
      break;
    case 3:
      lcd.print("UNDER CURRENT");
      cloud_no_rfid_lcd_print();
      break;
    case 4:
      lcd.print("OVER TEMPERATURE");
      cloud_no_rfid_lcd_print();
      break;
    case 5:
      lcd.print("UNDER TEMPERATURE");
      cloud_no_rfid_lcd_print();
      break;
    case 6:
      lcd.print("GFCI"); // Not implemented in AC001
      cloud_no_rfid_lcd_print();
      break;
    case 7:
      lcd.print("EARTH DISCONNECTED");
      cloud_no_rfid_lcd_print();
      break;
    case 8:
      lcd.print("EMERGENCY");
      cloud_no_rfid_lcd_print();
      break;
    case 9:
      lcd.print("POWER FAIL");
      cloud_no_rfid_lcd_print();
      break;
    default:
      lcd.print("*****FAULTED*****"); // You can make spaces using well... spacesbreak;
      cloud_no_rfid_lcd_print();
      break;
    }
    break;
  }



  switch (disp_evse)
  {
  case 1: if (disp_evse_A)
  {
    Serial.println(F("**going here too**"));
    lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    if (notFaulty_A)
    {
      Serial.println(F("**going here 3**"));
      lcd.print("*****CHARGING 1*****"); // You can make spaces using well... spaces
      lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
      lcd.print("VOLTAGE(v):");
      lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
      lcd.print(String(instantVoltage_A));
      lcd.setCursor(0, 2);
      lcd.print("CURRENT(A):");
      lcd.setCursor(12, 2); // Or setting the cursor in the desired position.
      lcd.print(String(instantCurrrent_A));
      lcd.setCursor(0, 3);
      lcd.print("POWER(KW) :");
      lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
      lcd.print(String(instantPower_A));
    }
    else
    {
      Serial.println(F("**going here 4**"));
      switch (fault_code_A)
      {
      case -1:  Serial.println(F("**going here 4**"));break; //It means default.
      case 0: lcd.print("Connector1-Over Voltage");
        break;
      case 1: lcd.print("Connector1-Under Voltage");
        break;
      case 2: lcd.print("Connector1-Over Current");
        break;
      case 3: lcd.print("Connector1-Under Current");
        break;
      case 4: lcd.print("Connector1-Over Temp");
        break;
      case 5: lcd.print("Connector1-Under Temp");
        break;
      case 6: lcd.print("Connector1-GFCI"); // Not implemented in AC001
        break;
      case 7: lcd.print("Connector1-Earth Disc");
        break;
      default: lcd.print("*****FAULTED 1*****"); // You can make spaces using well... spacesbreak;
      }
    }
  }
        break;
#if 0
  case 2: if (disp_evse_B)
  {
    lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    if (notFaulty_B)
    {
      lcd.print("*****CHARGING 2*****"); // You can make spaces using well... spaces
      lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
      lcd.print("VOLTAGE(v):");
      lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
      lcd.print(String(instantVoltage_B));
      lcd.setCursor(0, 2);
      lcd.print("CURRENT(A):");
      lcd.setCursor(12, 2); // Or setting the cursor in the desired position.
      lcd.print(String(instantCurrrent_B));
      lcd.setCursor(0, 3);
      lcd.print("POWER(KW) :");
      lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
      lcd.print(String(instantPower_B));
    }
    else
    {
      switch (fault_code_B)
      {
      case -1: break; //It means default.
      case 0: lcd.print("Connector2-Over Voltage");
        break;
      case 1: lcd.print("Connector2-Under Voltage");
        break;
      case 2: lcd.print("Connector2-Over Current");
        break;
      case 3: lcd.print("Connector2-Under Current");
        break;
      case 4: lcd.print("Connector2-Over Temp");
        break;
      case 5: lcd.print("Connector2-Over Temp");
        break;
      case 6: lcd.print("Connector2-GFCI"); // Not implemented in AC001
        break;
      case 7: lcd.print("Connector2-Earth Disc");
        break;
      default: lcd.print("*****FAULTED 2*****"); // You can make spaces using well... spacesbreak;
      }
    }

  }
        break;
  case 3: if (disp_evse_C)
  {
    lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    if (notFaulty_C)
    {
      lcd.print("*****CHARGING 3*****"); // You can make spaces using well... spaces
      lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
      lcd.print("VOLTAGE(v):");
      lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
      lcd.print(String(instantVoltage_C));
      lcd.setCursor(0, 2);
      lcd.print("CURRENT(A):");
      lcd.setCursor(12, 2); // Or setting the cursor in the desired position.
      lcd.print(String(instantCurrrent_C));
      lcd.setCursor(0, 3);
      lcd.print("POWER(KW) :");
      lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
      lcd.print(String(instantPower_C));
    }
    else
    {
      switch (fault_code_C)
      {
      case -1: break; //It means default.
      case 0: lcd.print("Connector3-Over Voltage");
        break;
      case 1: lcd.print("Connector3-Under Voltage");
        break;
      case 2: lcd.print("Connector3-Over Current");
        break;
      case 3: lcd.print("Connector3-Under Current");
        break;
      case 4: lcd.print("Connector3-Over Temp");
        break;
      case 5: lcd.print("Connector3-Over Temp");
        break;
      case 6: lcd.print("Connector3-GFCI"); // Not implemented in AC001
        break;
      case 7: lcd.print("Connector3-Earth Disc");
        break;
      default: lcd.print("*****FAULTED 3*****"); // You can make spaces using well... spacesbreak;
      }
    }



  }
        break;
#endif
  default: Serial.println(F("**Display default**"));
    break;
  }
}
#endif
#endif

#if LCD_ENABLED
void cloud_no_rfid_lcd_print(void)
{
  if (wifi_connect)
  {
    lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
    lcd.print("                    "); // Clear the line
    lcd.setCursor(0, 3);
    lcd.print("CLOUD: WIFI. NO RFID");
  }
  if (gsm_connect && client.connected())
  {
    lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
    lcd.print("                    "); // Clear the line
    lcd.setCursor(0, 3);
    lcd.print("CLOUD: 4G. NO RFID");
  }
  if (ethernet_connect)
  {
    lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
    lcd.print("                    "); // Clear the line
    lcd.setCursor(0, 3);
    lcd.print("CLOUD: ETH. NO RFID");
  }
}
#endif

#if DWIN_ENABLED
void cloud_no_rfid_dwin_print(void)
{
  uint8_t err = 0;
  if (wifi_connect)
  {
    avail[4] = 0x51;
    err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
    err = DWIN_SET(wi_rfid_not_avail, sizeof(wi_rfid_not_avail) / sizeof(wi_rfid_not_avail[0]));
  }
  else if (gsm_connect)
  {
    avail[4] = 0x51;
    err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
    err = DWIN_SET(g_rfid_not_avail, sizeof(g_rfid_not_avail) / sizeof(g_rfid_not_avail[0]));
  }
  else if (ethernet_connect)
  {
    // avail[4] = 0x51;
    // err = DWIN_SET(avail, sizeof(clear_avail) / sizeof(clear_avail[0]));
    err = DWIN_SET(eth_rfid_not_avail, sizeof(eth_rfid_not_avail) / sizeof(eth_rfid_not_avail[0]));
  }
  else
  {
    // err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
  }
}
#endif
uint8_t led_A = 0;
void EVSE_Led_loop(void)
{
  if (webSocketConncted == 1)
  {
    switch (evse_ChargePointStatus)
    {
    case Available:
      requestLed(GREEN, START, 1);
      gu8_rfid_tapped_A = 0;
      break;
    case Preparing:
      if (gu8_rfid_tapped_A == 1)
      {
        requestLed(BLINKYBLUE, START, 1);
      }
      else
      {
        requestLed(BLUE, START, 1);
      }
      break;
    case Charging:
      requestLed(BLINKYGREEN, START, 1);
      break;
    case Finishing:
      // requestLed(BLINKYBLUE, START, 1);
      requestLed(BLUE, START, 1);
      break;
    case Reserved:
      requestLed(WHITE, START, 1);
      break;
    case Unavailable:
      requestLed(BLINKYWHITE, START, 1);
      break;
    case Faulted:
      if (EMGCY_FaultOccured_A)
      {
        requestLed(BLINKYRED, START, 1);
      }
      else
      {
        requestLed(RED, START, 1);
      }
      break;
    default:
      requestLed(BLINKYWHITE, START, 1);
      break;
    }
  }
  else if (webSocketConncted == 0)
  {
    switch (evse_ChargePointStatus)
    {
    case Available:
      delay(20);
      requestLed(BLINKYWHITE, START, 1);
      delay(20);
      break;
    case Preparing:
      if (gu8_rfid_tapped_A == 1)
      {
        requestLed(BLINKYBLUE, START, 1);
      }
      else
      {
        requestLed(BLUE, START, 1);
      }
      break;
    case Charging:
    {
      led_A++;
      if (led_A >= 3)
      {
        led_A = 0;
        requestLed(BLINKYWHITE, START, 1);
      }
      else
      {
        requestLed(BLINKYGREEN, START, 1);
      }
    }
    break;
    case Finishing:
      // requestLed(BLINKYBLUE, START, 1);
      requestLed(BLUE, START, 1);
      break;
    case Reserved:
      requestLed(WHITE, START, 1);
      break;
    case Unavailable:
      requestLed(BLINKYWHITE, START, 1);
      break;
    case Faulted:
      if (EMGCY_FaultOccured_A)
      {
        requestLed(BLINKYRED, START, 1);
      }
      else
      {
        requestLed(RED, START, 1);
      }
      break;
    default:
      break;
    }
  }
}
