#include "ATM90E36.h"
#include "OcppEngine.h"
#include "EVSE_A.h"
#include "ChargePointStatusService.h"
#include "display_meterValues.h"
#if LCD_ENABLED
#include "LCD_I2C.h"
extern LCD_I2C lcd;
#endif
/*
* @brief: Feature added by Raja
* This feature will avoid hardcoding of messages. 
*/

uint8_t reasonForStop = 3; //Local is the default value
typedef enum resonofstop { EmergencyStop, EVDisconnected , HardReset, Local , Other , PowerLoss, Reboot,Remote, Softreset,UnlockCommand,DeAuthorized};

#if DWIN_ENABLED
#include "dwin.h"
extern unsigned char avail[28];
extern unsigned char fault_emgy[28];
extern unsigned char fault_noearth[28];
extern unsigned char fault_overVolt[28];
extern unsigned char fault_underVolt[28];
extern unsigned char fault_overTemp[28];
extern unsigned char fault_overCurr[28];
extern unsigned char fault_underCurr[28];
extern unsigned char fault_suspEV[28];
extern unsigned char fault_suspEVSE[28];
extern unsigned char charging[28];
extern unsigned char fault_nopower[28];
extern unsigned char i1[8];
#endif
extern int client_reconnect_flag;
extern uint8_t reasonForStop_A; 
extern uint8_t flag_ed_A;
uint8_t under_voltage_reading_skip_for_4times = 0;
static const char *resonofstop_str[] = { "EmergencyStop", "EVDisconnected" , "HardReset", "Local" , "Other" , "PowerLoss", "Reboot","Remote", "SoftReset","UnlockCommand","DeAuthorized"};
extern bool flag_single_phase;
bool phaseB = false;
bool phaseC = false;
/*
* @brief: Feature added by Raja for vendor specific error codes in status notification.
* 08/07/2022
* This feature will avoid hardcoding of messages. 
*/

typedef enum faultCode { OverVoltage, UnderVoltage , OverCurrentFailure, UnderCurrent , HighTemperature , UnderTemperature, GFCI,EarthDisconnect,Emergency,Power_Loss,};

int8_t fault_code_A = -1;
int8_t fault_code_B = -1;
int8_t fault_code_C = -1;

uint8_t flag_nopower = 0;

ATM90E36::ATM90E36(int pin) 	// Object
{
  //energy_IRQ = 2; 	// (In development...)
  _energy_CS = pin; 	// SS PIN
  //energy_WO = 8; 		// (In development...)
}

/* CommEnergyIC - Communication Establishment */
/*
- Defines Register Mask
- Treats the Register and SPI Comms
- Outputs the required value in the register
*/
unsigned short ATM90E36::CommEnergyIC(unsigned char RW, unsigned short address, unsigned short val) 
{
  unsigned char* data = (unsigned char*)&val;
  unsigned char* adata = (unsigned char*)&address;
  unsigned short output;
  unsigned short address1;

  // Slows the SPI interface to communicate
#if !defined(ENERGIA) && !defined(ESP8266) && !defined(ARDUINO_ARCH_SAMD)
  SPISettings settings(200000, MSBFIRST, SPI_MODE0);
#endif

#if defined(ESP8266)
  SPISettings settings(200000, MSBFIRST, SPI_MODE2);
#endif

#if defined(ARDUINO_ARCH_SAMD)
  SPISettings settings(200000, MSBFIRST, SPI_MODE3);
#endif

  // Switch MSB and LSB of value
  output = (val >> 8) | (val << 8);
  val = output;

  // Set R/W flag
  address |= RW << 15;

  // Swap byte address
  address1 = (address >> 8) | (address << 8);
  address = address1;

  // Transmit & Receive Data
#if !defined(ENERGIA)
  //SPI.beginTransaction(settings);
  hspi->beginTransaction(settings);
#endif

  // Chip enable and wait for SPI activation
  digitalWrite (_energy_CS, LOW);
  delayMicroseconds(10);

  // Write address byte by byte
  for (byte i=0; i<2; i++)
  {
    hspi->transfer (*adata);
    adata++;
  }

  // SPI.transfer16(address);
  /* Must wait 4 us for data to become valid */
  delayMicroseconds(4);

  // READ Data
  // Do for each byte in transfer
  if (RW)
  {
	for (byte i=0; i<2; i++)
    {
      *data = hspi->transfer (0x00);
      data++;
    }
    //val = SPI.transfer16(0x00);
  }
  else
  {
	for (byte i=0; i<2; i++)
    {
      hspi->transfer(*data);
      data++;
    }
    // SPI.transfer16(val);
  }

  // Chip enable and wait for transaction to end
  digitalWrite(_energy_CS, HIGH);
  delayMicroseconds(10);
#if !defined(ENERGIA)
  hspi->endTransaction();
#endif

  output = (val >> 8) | (val << 8); // reverse MSB and LSB
  return output;

  // Use with transfer16
  // return val;
}

/* Parameters Functions*/
/*
- Gets main electrical parameters,
such as: Voltage, Current, Power, Energy,
and Frequency
- Also gets the temperature
*/
// VOLTAGE
double  ATM90E36::GetLineVoltageA() {
  uint8_t err = 0;
  unsigned short voltage = CommEnergyIC(READ, UrmsA, 0xFFFF);
  double volt = (double)voltage / 100;
  volt = ((volt * 0.989649057) + 0.315538775);
// if(volt < 20)
// {
//   flag_nopower = 1;
// }
// else
// {
//    flag_nopower = 0;
// }
#if EARTH_DISCONNECT & 0
if(flag_nopower)
  {
    Serial.println("***Power fail****");
    reasonForStop_A = PowerLoss;
    fault_code_A = Power_Loss;
    evse_ChargePointStatus = Faulted;
    if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
    {
      EVSE_A_StopSession();
    }
    #if LCD_ENABLED & 0

//lcd.clear();
      #if 0
      lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      lcd.print("                    "); // Clear the line
      lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      lcd.print("STATUS: FAULTED");
      #endif
      lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      lcd.print("                    "); // Clear the line
      lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
      lcd.print("                    "); // Clear the line
      lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
      lcd.print("                    "); // Clear the line
      lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
      lcd.print("***POWER FAILURE***");

#endif
#if DWIN_ENABLED
    fault_nopower[4] = 0X66; // In the fourth page.
    fault_nopower[5] = 0X00; // In the fourth page.
    err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
    fault_nopower[4] = 0X71; // In the fifth page.
    fault_nopower[5] = 0X00; // In the fourth page.
    err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
    fault_nopower[4] = 0X71; // In the fifth page.
    fault_nopower[5] = 0X50; // In the fourth page.
    err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
    fault_nopower[4] = 0X7B; // In the sixth page.
    fault_nopower[5] = 0X00; // In the fourth page.
    err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
    fault_nopower[4] = 0X7B; // In the sixth page.
    fault_nopower[5] = 0X50; // In the fourth page.
    err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
#endif
  }
  else if (flag_nopower == 0)
  {
    // if (digitalRead(earth_disconnect_pin))
    if((volt < 130) && (volt > 80))
    {
      Serial.println("*****earth disconnect*****");
#if DISPLAY_ENABLED
      connAvail(1, "EARTH DISCONNECTED");
      checkForResponse_Disp();
      connAvail(2, "EARTH DISCONNECTED");
      checkForResponse_Disp();
      connAvail(3, "EARTH DISCONNECTED");
      checkForResponse_Disp();
      setHeader("RFID UNAVAILABLE");
      checkForResponse_Disp();
#endif
      getChargePointStatusService_A()->setEarthDisconnect(true);
      // evse_ChargePointStatus = Faulted;
      fault_code_A = EarthDisconnect;
      flag_ed_A = 1;
      reasonForStop_A = Other;
#if DWIN_ENABLED
      err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
      cloud_no_rfid_dwin_print();
      fault_noearth[4] = 0X66; // In the fourth page.
      fault_noearth[5] = 0X00; // In the fourth page.
      err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
      // fault_noearth[4] = 0X51; // In the fourth page.
      // fault_noearth[5] = 0X00; // In the fourth page.
      // err = DWIN_SET(fault_noearth,sizeof(fault_noearth)/sizeof(fault_noearth[0]));
      fault_noearth[4] = 0X71; // In the fifth page.
      fault_noearth[5] = 0X00; // In the fourth page.
      err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
      fault_noearth[4] = 0X71; // In the fifth page.
      fault_noearth[5] = 0X50; // In the fourth page.
      err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
      fault_noearth[4] = 0X7B; // In the sixth page.
      fault_noearth[5] = 0X00; // In the fourth page.
      err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
      fault_noearth[4] = 0X7B; // In the sixth page.
      fault_noearth[5] = 0X50; // In the fourth page.
      err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
#endif
#if LCD_ENABLED & 0

//lcd.clear();
      #if 0
      lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      lcd.print("                    "); // Clear the line
      lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      lcd.print("STATUS: FAULTED");
      #endif
      lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      lcd.print("                    "); // Clear the line
      lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      if (volt > 20)
      {
        lcd.print("EARTH DISCONNECTED");
        lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
        lcd.print("                    "); // Clear the line
        lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
        lcd.print("                    "); // Clear the line
        lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
        lcd.print("                    "); // Clear the line
        lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
        lcd.print("                    "); // Clear the line
      }
      else
        lcd.print("NO POWER");

#endif
    }
    else
    {
      getChargePointStatusService_A()->setEarthDisconnect(false);
      // lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
      // lcd.print("                    ");//Clear the line
      flag_ed_A = 0;
    }
  }
  
#endif


if (volt<200)
{
  // if ((flag_ed_A == 0) && (flag_nopower == 0))
  // {
    client_reconnect_flag = 1;
    getChargePointStatusService_A()->setUnderVoltage(true);
    getChargePointStatusService_A()->setOverVoltage(false);
    

    #if 0
      if (volt > 20)
      {
        fault_underVolt[4] = 0X66; // In the fourth page.
        fault_underVolt[5] = 0X00; // In the fourth page.
        err = DWIN_SET(fault_underVolt, sizeof(fault_underVolt) / sizeof(fault_underVolt[0]));
      }
      else
      {
        fault_nopower[4] = 0X66; // In the fourth page.
        fault_nopower[5] = 0X00; // In the fourth page.
        err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
      }
// fault_underVolt[4] = 0X51; // In the fourth page.
// fault_underVolt[5] = 0X00; // In the fourth page.
// err = DWIN_SET(fault_underVolt,sizeof(fault_underVolt)/sizeof(fault_underVolt[0]));
#endif
    //if(DEBUG_OUT) Serial.println("Under Voltage");
    #if 1
    if(volt<20)
    {
    reasonForStop = PowerLoss;
    fault_code_A = Power_Loss;
    Serial.println("Power Loss");

    evse_ChargePointStatus = Faulted;

    if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
    {
      if(getChargePointStatusService_A()->getTransactionId() != -1)
      {
            EVSE_A_StopSession();
      }
    }
     #if 0
   //  //lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("A: POWER LOSS");
    #endif
    #if DWIN_ENABLED
    fault_nopower[4] = 0X66; // In the fourth page.
    fault_nopower[5] = 0X00; // In the fourth page.
    err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
    fault_nopower[4] = 0X55;
    err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
    cloud_no_rfid_dwin_print();
    #endif
    }
    else if (volt<130)
    {
    reasonForStop = Other;
    fault_code_A = EarthDisconnect;
    Serial.println("Earth Disconnect");
    getChargePointStatusService_A()->setEarthDisconnect(true);
    #if 0
   //  //lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("A: EARTH DISCONNECT");
    #endif
    #if DWIN_ENABLED
    //fault_noearth[4] = 0X55;
    err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
    fault_noearth[4] = 0X66; // In the fourth page.
    fault_noearth[5] = 0X00; // In the fourth page.
    err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
    fault_noearth[4] = 0X55;
    err = DWIN_SET(fault_noearth, sizeof(fault_noearth) / sizeof(fault_noearth[0]));
    cloud_no_rfid_dwin_print();
    i1[4] = 0X77;
    err = DWIN_SET(i1, sizeof(i1) / sizeof(i1[0]));
    #endif
    }
    else
    {
            reasonForStop = Other;
            fault_code_A = UnderVoltage;
            Serial.println("Under Voltage");
            under_voltage_reading_skip_for_4times++;
            Serial.print("under_voltage_reading_skip_for_4times count");
            Serial.println(under_voltage_reading_skip_for_4times);
            if (under_voltage_reading_skip_for_4times >= 4)
            {
              Serial.println("under_voltage_reading_skip_for_4times count finished");
              under_voltage_reading_skip_for_4times = 0;
              EVSE_A_StopSession();
            }
    }
    #endif

     #if 0
   //  //lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("A: UNDER VOLTAGE");
    #endif
    #if DWIN_ENABLED
    fault_underVolt[4] = 0X66; // In the fourth page.
    fault_underVolt[5] = 0X00; // In the fourth page.
    err = DWIN_SET(fault_underVolt, sizeof(fault_underVolt) / sizeof(fault_underVolt[0]));
    fault_underVolt[4] = 0x55;
    err = DWIN_SET(fault_underVolt, sizeof(fault_underVolt) / sizeof(fault_underVolt[0]));
    cloud_no_rfid_dwin_print();
    i1[4] = 0X77;
    err = DWIN_SET(i1, sizeof(i1) / sizeof(i1[0]));
    #endif
    return volt;
  // } 
}
  else if (volt >  275 ){ 
    // if ((flag_ed_A == 0) && (flag_nopower == 0))
    // {
    client_reconnect_flag = 1;
	  getChargePointStatusService_A()->setUnderVoltage(false);
	  getChargePointStatusService_A()->setOverVoltage(true);
    #if DWIN_ENABLED
      fault_overVolt[4] = 0X66;
      fault_overVolt[5] = 0X00;
      err = DWIN_SET(fault_overVolt, sizeof(fault_overVolt) / sizeof(fault_overVolt[0]));
      fault_overVolt[4] = 0X55;
      fault_overVolt[5] = 0X00;
      err = DWIN_SET(fault_overVolt,sizeof(fault_overVolt)/sizeof(fault_overVolt[0]));
      cloud_no_rfid_dwin_print();
      i1[4] = 0X77;
      err = DWIN_SET(i1, sizeof(i1) / sizeof(i1[0]));
    #endif
	  if(DEBUG_OUT) Serial.println("Over Voltage");
    fault_code_A = OverVoltage;
    #if 0
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("A: OVER VOLTAGE");
    #endif
    reasonForStop = Other;
	  return volt;
  // }
  } else {
	  getChargePointStatusService_A()->setUnderVoltage(false);
	  getChargePointStatusService_A()->setOverVoltage(false);
    getChargePointStatusService_A()->setEarthDisconnect(false);
	  if(DEBUG_OUT) Serial.println("Normal Voltage Range.");
	  return volt;
  }  
}

double  ATM90E36::GetLineVoltageB() {
  uint8_t err = 0;
  unsigned short voltage = CommEnergyIC(READ, UrmsB, 0xFFFF);
  double volt = (double)voltage / 100;
 if (volt<200){
    // getChargePointStatusService_A()->setUnderVoltage(true);
    getChargePointStatusService_A()->setOverVoltage(false);
    #if 0
      if (volt > 20)
      {
        fault_underVolt[4] = 0X71;
        fault_underVolt[5] = 0X00;
        err = DWIN_SET(fault_underVolt, sizeof(fault_underVolt) / sizeof(fault_underVolt[0]));
      }
      else
      {
        fault_nopower[4] = 0X71; // In the fifth page.
        fault_nopower[5] = 0X00; // In the fourth page.
        err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
      }

// fault_underVolt[4] = 0X51;
// fault_underVolt[5] = 0X00;
// err = DWIN_SET(fault_underVolt,sizeof(fault_underVolt)/sizeof(fault_underVolt[0]));
#endif
    if(DEBUG_OUT) Serial.println("Under Voltage");
    // if(volt<20)
    // {
    // reasonForStop = PowerLoss;
    // fault_code_A = Power_Loss;
    // }
    // else if (volt<180)
    // {
    // reasonForStop = Other;
    // fault_code_A = EarthDisconnect;
    // }
    // else
    // {
    // reasonForStop = Other;
    // fault_code_A = UnderVoltage;
    // }
    return volt;
  }
   else if (volt > 275){
	  getChargePointStatusService_A()->setUnderVoltage(false);
	  // getChargePointStatusService_A()->setOverVoltage(true);
    #if 0
      fault_overVolt[4] = 0X71;
      fault_overVolt[5] = 0X00;
      err = DWIN_SET(fault_overVolt, sizeof(fault_overVolt) / sizeof(fault_overVolt[0]));
// fault_overVolt[4] = 0X51;
// fault_overVolt[5] = 0X00;
// err = DWIN_SET(fault_overVolt,sizeof(fault_overVolt)/sizeof(fault_overVolt[0]));
#endif
    // fault_code_A = OverVoltage;
    // reasonForStop = Other;
     #if LCD_ENABLED & 0
     //lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 2); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 2); // Or setting the cursor in the desired position.
    lcd.print("B: OVER VOLTAGE");
    #endif
	  return volt;
  } else {
	  getChargePointStatusService_A()->setUnderVoltage(false);
	  getChargePointStatusService_A()->setOverVoltage(false);
    phaseB = false;
	  return volt;
  }
}

double  ATM90E36::GetLineVoltageC() {
  uint8_t err = 0;
  unsigned short voltage = CommEnergyIC(READ, UrmsC, 0xFFFF);
  double volt = (double)voltage / 100;
 if (volt<200){
    // getChargePointStatusService_A()->setUnderVoltage(true);
    getChargePointStatusService_A()->setOverVoltage(false);
    #if 0
      if (volt > 20)
      {
        fault_underVolt[4] = 0X7B;
        fault_underVolt[5] = 0X00;
        err = DWIN_SET(fault_underVolt, sizeof(fault_underVolt) / sizeof(fault_underVolt[0]));
      }
      else
      {
        fault_nopower[4] = 0X7B;
        fault_nopower[5] = 0X00;
        err = DWIN_SET(fault_nopower, sizeof(fault_nopower) / sizeof(fault_nopower[0]));
      }
#endif
    if(DEBUG_OUT) Serial.println("Under Voltage");
    // if(volt<20)
    // {
    // reasonForStop = PowerLoss;
    // fault_code_A = Power_Loss;
    // }
    // else if (volt<180)
    // {
    // reasonForStop = Other;
    // fault_code_A = EarthDisconnect;
    // }
    // else
    // {
    // reasonForStop = Other;
    // fault_code_A = UnderVoltage;
    // }
    return volt;
  }
  else if (volt > 275){
	  getChargePointStatusService_A()->setUnderVoltage(false);
	  // getChargePointStatusService_A()->setOverVoltage(true);
    #if 0
      fault_overVolt[4] = 0X7B;
      fault_overVolt[5] = 0X00;
      err = DWIN_SET(fault_overVolt, sizeof(fault_overVolt) / sizeof(fault_overVolt[0]));
// fault_underVolt[4] = 0X51;
// fault_underVolt[5] = 0X00;
// err = DWIN_SET(fault_underVolt,sizeof(fault_underVolt)/sizeof(fault_underVolt[0]));
#endif
    // fault_code_A = OverVoltage;
    // reasonForStop = Other;
     #if LCD_ENABLED & 0
     //lcd.clear();
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 3); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 3); // Or setting the cursor in the desired position.
    lcd.print("C: OVER VOLTAGE");
    #endif
	  return volt;
  } else {
	  getChargePointStatusService_A()->setUnderVoltage(false);
	  getChargePointStatusService_A()->setOverVoltage(false);
    phaseC = false;
	  return volt;
  }
}

// CURRENT
double ATM90E36::GetLineCurrentA() {
  uint8_t err = 0;
  unsigned short current = CommEnergyIC(READ, IrmsA, 0xFFFF);
	double currentA = (double)current / 1000;
	double curr;
	// if (currentA>8.5){
  //     curr = currentA * 2.21;
  //   }else if ((currentA>0)&&(currentA<=8.5)){
  //     curr = currentA * 2.202;
  //   }
    if (currentA > 8.5)
  {
    curr = currentA * 2.21;
    curr = (curr * 0.967836) - 0.02957;
    // curr = (currentA * 0.98042)-0.02957;
    if (curr < 0)
    {
      curr = 0;
    }
  }

  else if ((currentA > 0) && (currentA <= 8.5))
  {
    curr = currentA * 2.202;
    // curr = (currentA * 0.98042)-0.02957;
    curr = (curr * 0.967836) - 0.02957;
    if (curr < 0)
    {
      curr = 0;
    }
  }
  currentA = curr;
	if (curr > 35){
		getChargePointStatusService_A()->setOverCurrent(true);
    fault_code_A = OverCurrentFailure;

    #if 0
    fault_overCurr[4] = 0X66;
    fault_overCurr[5] = 0X00;
    err = DWIN_SET(fault_overCurr, sizeof(fault_overCurr) / sizeof(fault_overCurr[0]));
    fault_overCurr[4] = 0X51;
    fault_overCurr[5] = 0X00;
    err = DWIN_SET(fault_overCurr, sizeof(fault_overCurr) / sizeof(fault_overCurr[0]));
#endif 
    #if 0
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("A: OVER CURRENT");
    #endif
   
   reasonForStop = Other;
		return curr;
	} else {
		getChargePointStatusService_A()->setOverCurrent(false);
		return curr;
	}
}

double ATM90E36::GetLineCurrentB() {
  uint8_t err = 0;
  unsigned short current = CommEnergyIC(READ, IrmsB, 0xFFFF);
  double currentB  = (double)current / 1000;
  double curr;

  if(currentB > 8.5){
    curr = currentB * 2.21;
  }else if((currentB>0) && (currentB<=8.5)){
    curr = currentB * 2.202;
  }

  if(curr > 35){
    // getChargePointStatusService_A()->setOverCurrent(true);
    // reasonForStop = Other;
    // fault_code_A = OverCurrentFailure;
      #if LCD_ENABLED & 0
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 2); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 2); // Or setting the cursor in the desired position.
    lcd.print("B: OVER CURRENT");
    #endif
    #if 0
    fault_overCurr[4] = 0X71;
    fault_overCurr[5] = 0X00;
    err = DWIN_SET(fault_overCurr, sizeof(fault_overCurr) / sizeof(fault_overCurr[0]));
    fault_overCurr[4] = 0X51;
    fault_overCurr[5] = 0X00;
    err = DWIN_SET(fault_overCurr, sizeof(fault_overCurr) / sizeof(fault_overCurr[0]));
#endif
    return curr;
  }else{
    getChargePointStatusService_A()->setOverCurrent(false);
    return curr;
  }
}
double ATM90E36::GetLineCurrentC() {
  uint8_t err = 0;
  unsigned short current = CommEnergyIC(READ, IrmsC, 0xFFFF);
  double currentC = (double)current / 1000;
  double curr;

  if(currentC > 8.5){
    curr = currentC * 2.21;
  }else if((currentC > 0) && (currentC <= 8.5)){
    curr = currentC * 2.202;
  }

  if(curr > 35){
    // getChargePointStatusService_A()->setOverCurrent(true);
    // reasonForStop = Other;
    // fault_code_A = OverCurrentFailure;
      #if LCD_ENABLED & 0
       
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 3); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 3); // Or setting the cursor in the desired position.
    lcd.print("C: OVER CURRENT");
    #endif
    #if 0
    fault_overCurr[4] = 0X7B;
    fault_overCurr[5] = 0X00;
    err = DWIN_SET(fault_overCurr, sizeof(fault_overCurr) / sizeof(fault_overCurr[0]));
    // fault_overCurr[4] = 0X51;
    // fault_overCurr[5] = 0X00;
    err = DWIN_SET(fault_overCurr, sizeof(fault_overCurr) / sizeof(fault_overCurr[0]));
#endif
    return curr;
  }else{
    getChargePointStatusService_A()->setOverCurrent(false);
    return curr;
  }
}
double ATM90E36::GetLineCurrentN() {
  unsigned short current = CommEnergyIC(READ, IrmsN0, 0xFFFF);
  return (double)current / 1000;
}

// ACTIVE POWER
double ATM90E36::GetActivePowerA() {
  signed short apower = (signed short) CommEnergyIC(READ, PmeanA, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetActivePowerB() {
  signed short apower = (signed short) CommEnergyIC(READ, PmeanB, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetActivePowerC() {
  signed short apower = (signed short) CommEnergyIC(READ, PmeanC, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetTotalActivePower() {
  signed short apower = (signed short) CommEnergyIC(READ, PmeanT, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 250;
}

// REACTIVE POWER
double ATM90E36::GetReactivePowerA() {
  signed short apower = (signed short) CommEnergyIC(READ, QmeanA, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetReactivePowerB() {
  signed short apower = (signed short) CommEnergyIC(READ, QmeanB, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetReactivePowerC() {
  signed short apower = (signed short) CommEnergyIC(READ, QmeanC, 0xFFFF);
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetTotalReactivePower() {
  signed short apower = (signed short) CommEnergyIC(READ, QmeanT, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 250;
}

// APPARENT POWER
double ATM90E36::GetApparentPowerA() {
  signed short apower = (signed short) CommEnergyIC(READ, SmeanA, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetApparentPowerB() {
 signed short apower = (signed short) CommEnergyIC(READ, SmeanB, 0xFFFF);
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetApparentPowerC() {
  signed short apower = (signed short) CommEnergyIC(READ, SmeanC, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 1000;
}
double ATM90E36::GetTotalApparentPower() {
  signed short apower = (signed short) CommEnergyIC(READ, SmeanT, 0xFFFF); 
  if (apower & 0x8000) {
    apower= (apower & 0x7FFF) * -1;
  }
  return (double)apower / 250;
}

// FREQUENCY
double ATM90E36::GetFrequency() {
  unsigned short freq = CommEnergyIC(READ, Freq, 0xFFFF);
  return (double)freq / 100;
}

// POWER FACTOR
double ATM90E36::GetPowerFactorA() {
  short pf = (short) CommEnergyIC(READ, PFmeanA, 0xFFFF); 
  //if negative
  if (pf & 0x8000) {
    pf = (pf & 0x7FFF) * -1;
  }
  return (double)pf / 1000;
}
double ATM90E36::GetPowerFactorB() {
  short pf = (short) CommEnergyIC(READ, PFmeanB, 0xFFFF); 
  if (pf & 0x8000) {
    pf = (pf & 0x7FFF) * -1;
  }
  return (double)pf / 1000;
}
double ATM90E36::GetPowerFactorC() {
  short pf = (short) CommEnergyIC(READ, PFmeanC, 0xFFFF); 
  //if negative
  if (pf & 0x8000) {
    pf = (pf & 0x7FFF) * -1;
  }
  return (double)pf / 1000;
}
double ATM90E36::GetTotalPowerFactor() {
  short pf = (short) CommEnergyIC(READ, PFmeanT, 0xFFFF); 
  //if negative
  if (pf & 0x8000) {
    pf = (pf & 0x7FFF) * -1;
  }
  return (double)pf / 1000;
}

// PHASE ANGLE
double ATM90E36::GetPhaseA() {
  signed short apower = (signed short) CommEnergyIC(READ, PAngleA, 0xFFFF);
  return (double)apower / 10;
}
double ATM90E36::GetPhaseB() {
  signed short apower = (signed short) CommEnergyIC(READ, PAngleB, 0xFFFF);
  return (double)apower / 10;
}
double ATM90E36::GetPhaseC() {
  signed short apower = (signed short) CommEnergyIC(READ, PAngleC, 0xFFFF);
  return (double)apower / 10;
}

// TEMPERATURE
double ATM90E36::GetTemperature() {
  uint8_t err = 0;
  short int temp = (short int) CommEnergyIC(READ, Temp, 0xFFFF); 
  //return (double)temp;     
    if (temp<-25){
	  getChargePointStatusService_A()->setUnderTemperature(true);
	  getChargePointStatusService_A()->setOverTemperature(false);
	  if(DEBUG_OUT) Serial.println("Under Temperature"+String(temp));
    fault_code_A = UnderTemperature;
    #if 0
    fault_overTemp[4] = 0X66;
    fault_overTemp[5] = 0X00;
    err = DWIN_SET(fault_overTemp, sizeof(fault_overTemp) / sizeof(fault_overTemp[0]));
    // fault_overTemp[4] = 0X51;
    // fault_overTemp[5] = 0X00;
    err = DWIN_SET(fault_overTemp, sizeof(fault_overTemp) / sizeof(fault_overTemp[0]));
    #endif
 #if LCD_ENABLED & 0
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("A: UNDER TEMP");
    #endif

    reasonForStop = Other;
	  return temp;
  } else if (temp>80){
	  getChargePointStatusService_A()->setUnderTemperature(false);
	  getChargePointStatusService_A()->setOverTemperature(true);
	  if(DEBUG_OUT) Serial.println("Over Temperature"+String(temp));
    fault_code_A = HighTemperature;
 #if LCD_ENABLED & 0
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("                    ");//Clear the line
    lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
    lcd.print("A: OVER TEMP");
    #endif
    

    reasonForStop = Other;
    #if 0
    fault_overTemp[4] = 0X66;
    fault_overTemp[4] = 0X50;
    err = DWIN_SET(fault_overTemp, sizeof(fault_overTemp) / sizeof(fault_overTemp[0]));
    // fault_overTemp[4] = 0X51;
    // fault_overTemp[4] = 0X00;
    err = DWIN_SET(fault_overTemp, sizeof(fault_overTemp) / sizeof(fault_overTemp[0]));
    #endif
	  return temp;
  } else {
	  getChargePointStatusService_A()->setUnderTemperature(false);
	  getChargePointStatusService_A()->setOverTemperature(false);
	  if(DEBUG_OUT) Serial.println("Normal Temperature: "+String(temp));
	  return temp;
  }  
}

/* Gets the Register Value if Desired */
// REGISTER
unsigned short ATM90E36::GetValueRegister(unsigned short registerRead) {
  return (CommEnergyIC(READ, registerRead, 0xFFFF)); //returns value register
}

// ENERGY MEASUREMENT
double ATM90E36::GetImportEnergy() {
  unsigned short ienergyT = CommEnergyIC(READ, APenergyT, 0xFFFF);
  // unsigned short ienergyA = CommEnergyIC(READ, APenergyA, 0xFFFF);
  // unsigned short ienergyB = CommEnergyIC(READ, APenergyB, 0xFFFF);
  // unsigned short ienergyC = CommEnergyIC(READ, APenergyC, 0xFFFF);

  // unsigned short renergyT = CommEnergyIC(READ, RPenergyT, 0xFFFF);
  // unsigned short renergyA = CommEnergyIC(READ, RPenergyA, 0xFFFF);
  // unsigned short renergyB = CommEnergyIC(READ, RPenergyB, 0xFFFF);
  // unsigned short renergyC = CommEnergyIC(READ, RPenergyC, 0xFFFF);

  // unsigned short senergyT = CommEnergyIC(READ, SAenergyT, 0xFFFF);
  // unsigned short senergyA = CommEnergyIC(READ, SenergyA, 0xFFFF);
  // unsigned short senergyB = CommEnergyIC(READ, SenergyB, 0xFFFF);
  // unsigned short senergyC = CommEnergyIC(READ, SenergyC, 0xFFFF);

  return (double)ienergyT / 100 / 3200; //returns kWh
}

double ATM90E36::GetExportEnergy() {

  unsigned short eenergyT = CommEnergyIC(READ, ANenergyT, 0xFFFF);
  // unsigned short eenergyA = CommEnergyIC(READ, ANenergyA, 0xFFFF);
  // unsigned short eenergyB = CommEnergyIC(READ, ANenergyB, 0xFFFF);
  // unsigned short eenergyC = CommEnergyIC(READ, ANenergyC, 0xFFFF);

  // unsigned short reenergyT = CommEnergyIC(READ, RNenergyT, 0xFFFF);
  // unsigned short reenergyA = CommEnergyIC(READ, RNenergyA, 0xFFFF);
  // unsigned short reenergyB = CommEnergyIC(READ, RNenergyB, 0xFFFF);
  // unsigned short reenergyC = CommEnergyIC(READ, RNenergyC, 0xFFFF);

  return (double)eenergyT / 100 / 3200; //returns kWh 
}

/* System Status Registers */
unsigned short ATM90E36::GetSysStatus0() {    
  return CommEnergyIC(READ, SysStatus0, 0xFFFF);
}
unsigned short ATM90E36::GetSysStatus1() {
  return CommEnergyIC(READ, SysStatus1, 0xFFFF);
}
unsigned short  ATM90E36::GetMeterStatus0() {
  return CommEnergyIC(READ, EnStatus0, 0xFFFF);
}
unsigned short  ATM90E36::GetMeterStatus1() {
  return CommEnergyIC(READ, EnStatus1, 0xFFFF);
}


/* Checksum Error Function */
bool ATM90E36::calibrationError()
{
  bool CS0, CS1, CS2, CS3;
  unsigned short systemstatus0 = GetSysStatus0();

  if (systemstatus0 & 0x4000)
  {
    CS0 = true;
  }
  else
  {
    CS0 = false;
  } 

  if (systemstatus0 & 0x0100)
  {
    CS1 = true; 
  } 
  else 
  {
    CS1 = false;
  }
  if (systemstatus0 & 0x0400)
  {
    CS2 = true;
  }
  else
  {
    CS2 = false;
  } 
  if (systemstatus0 & 0x0100)
  {
    CS3 = true;
  }
  else 
  {
    CS3 = false;
  }

#ifdef DEBUG_SERIAL
    Serial.print("Checksum 0: ");
    Serial.println(CS0);
    Serial.print("Checksum 1: ");
    Serial.println(CS1);
    Serial.print("Checksum 2: ");
    Serial.println(CS2);
    Serial.print("Checksum 3: ");
    Serial.println(CS3);
#endif

  if (CS0 || CS1 || CS2 || CS3) return (true); 
  else return (false);

}

/* BEGIN FUNCTION */
/* 
- Define the pin to be used as Chip Select
- Set serialFlag to true for serial debugging
- Use SPI MODE 0 for the ATM90E36
*/
void ATM90E36::begin()
{  
  // pinMode(energy_IRQ, INPUT); // (In development...)
  pinMode(_energy_CS, OUTPUT);
  // pinMode(energy_WO, INPUT);  // (In development...)

  /* Enable SPI */
  hspi->begin();
#if defined(ENERGIA)
  hspi->setBitOrder(MSBFIRST);
  hspi->setDataMode(SPI_MODE0);
  hspi->setClockDivider(SPI_CLOCK_DIV16);
#endif

  CommEnergyIC(WRITE, SoftReset, 0x789A);   // Perform soft reset
  CommEnergyIC(WRITE, FuncEn0, 0x0000);     // Voltage sag
  CommEnergyIC(WRITE, FuncEn1, 0x0000);     // Voltage sag
  CommEnergyIC(WRITE, SagTh, 0x0001);       // Voltage sag threshold

  /* SagTh = Vth * 100 * sqrt(2) / (2 * Ugain / 32768) */
  
  //Set metering config values (CONFIG)
  CommEnergyIC(WRITE, ConfigStart, 0x5678); // Metering calibration startup 
  CommEnergyIC(WRITE, PLconstH, 0x0861);    // PL Constant MSB (default)
  CommEnergyIC(WRITE, PLconstL, 0xC468);    // PL Constant LSB (default)
  CommEnergyIC(WRITE, MMode0, 0x1087);      // Mode Config (60 Hz, 3P4W)
  CommEnergyIC(WRITE, MMode1, 0x1500);      // 0x5555 (x2) // 0x0000 (1x)
  CommEnergyIC(WRITE, PStartTh, 0x0000);    // Active Startup Power Threshold
  CommEnergyIC(WRITE, QStartTh, 0x0000);    // Reactive Startup Power Threshold
  CommEnergyIC(WRITE, SStartTh, 0x0000);    // Apparent Startup Power Threshold
  CommEnergyIC(WRITE, PPhaseTh, 0x0000);    // Active Phase Threshold
  CommEnergyIC(WRITE, QPhaseTh, 0x0000);    // Reactive Phase Threshold
  CommEnergyIC(WRITE, SPhaseTh, 0x0000);    // Apparent  Phase Threshold
  CommEnergyIC(WRITE, CSZero, 0x4741);      // Checksum 0
  
  //Set metering calibration values (CALIBRATION)
  CommEnergyIC(WRITE, CalStart, 0x5678);    // Metering calibration startup 
  CommEnergyIC(WRITE, GainA, 0x0000);       // Line calibration gain
  CommEnergyIC(WRITE, PhiA, 0x0000);        // Line calibration angle
  CommEnergyIC(WRITE, GainB, 0x0000);       // Line calibration gain
  CommEnergyIC(WRITE, PhiB, 0x0000);        // Line calibration angle
  CommEnergyIC(WRITE, GainC, 0x0000);       // Line calibration gain
  CommEnergyIC(WRITE, PhiC, 0x0000);        // Line calibration angle
  CommEnergyIC(WRITE, PoffsetA, 0x0000);    // A line active power offset
  CommEnergyIC(WRITE, QoffsetA, 0x0000);    // A line reactive power offset
  CommEnergyIC(WRITE, PoffsetB, 0x0000);    // B line active power offset
  CommEnergyIC(WRITE, QoffsetB, 0x0000);    // B line reactive power offset
  CommEnergyIC(WRITE, PoffsetC, 0x0000);    // C line active power offset
  CommEnergyIC(WRITE, QoffsetC, 0x0000);    // C line reactive power offset
  CommEnergyIC(WRITE, CSOne, 0x0000);       // Checksum 1
  
  //Set metering calibration values (HARMONIC)
  CommEnergyIC(WRITE, HarmStart, 0x5678);   // Metering calibration startup 
  CommEnergyIC(WRITE, POffsetAF, 0x0000);   // A Fund. active power offset
  CommEnergyIC(WRITE, POffsetBF, 0x0000);   // B Fund. active power offset
  CommEnergyIC(WRITE, POffsetCF, 0x0000);   // C Fund. active power offset
  CommEnergyIC(WRITE, PGainAF, 0x0000);     // A Fund. active power gain
  CommEnergyIC(WRITE, PGainBF, 0x0000);     // B Fund. active power gain
  CommEnergyIC(WRITE, PGainCF, 0x0000);     // C Fund. active power gain
  CommEnergyIC(WRITE, CSTwo, 0x0000);       // Checksum 2 

  //Set measurement calibration values (ADJUST)
  CommEnergyIC(WRITE, AdjStart, 0x5678);    // Measurement calibration
  CommEnergyIC(WRITE, UgainA, 0x0002);      // A SVoltage rms gain
  CommEnergyIC(WRITE, IgainA, 0xFD7F);      // A line current gain
  CommEnergyIC(WRITE, UoffsetA, 0x0000);    // A Voltage offset
  CommEnergyIC(WRITE, IoffsetA, 0x0000);    // A line current offset
  CommEnergyIC(WRITE, UgainB, 0x0002);      // B Voltage rms gain
  CommEnergyIC(WRITE, IgainB, 0xFD7F);      // B line current gain
  CommEnergyIC(WRITE, UoffsetB, 0x0000);    // B Voltage offset
  CommEnergyIC(WRITE, IoffsetB, 0x0000);    // B line current offset
  CommEnergyIC(WRITE, UgainC, 0x0002);      // C Voltage rms gain
  CommEnergyIC(WRITE, IgainC, 0xFD7F);      // C line current gain
  CommEnergyIC(WRITE, UoffsetC, 0x0000);    // C Voltage offset
  CommEnergyIC(WRITE, IoffsetC, 0x0000);    // C line current offset
  CommEnergyIC(WRITE, IgainN, 0xFD7F);      // C line current gain
  CommEnergyIC(WRITE, CSThree, 0x02F6);     // Checksum 3

  // Done with the configuration
  CommEnergyIC(WRITE, ConfigStart, 0x5678);
  CommEnergyIC(WRITE, CalStart, 0x5678);    // 0x6886 //0x5678 //8765);
  CommEnergyIC(WRITE, HarmStart, 0x5678);   // 0x6886 //0x5678 //8765);    
  CommEnergyIC(WRITE, AdjStart, 0x5678);    // 0x6886 //0x5678 //8765);  

  CommEnergyIC(WRITE, SoftReset, 0x789A);   // Perform soft reset  
}