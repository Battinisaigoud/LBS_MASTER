#include "Master.h"
#include "Variants.h"

SoftwareSerial masterSerial(25, 33); // 25 Rx, 33 Tx

uint8_t dis_phase_type = 0;

volatile float device_load = 0;
#define alprSerial Serial1

#if 0
StaticJsonDocument<250> txM_doc;
StaticJsonDocument<250> rxM_doc;
#else
DynamicJsonDocument txM_doc(250);
DynamicJsonDocument rxM_doc(250);
#endif

DynamicJsonDocument txA_doc(200);
DynamicJsonDocument rxA_doc(200);

bool flag_GFCI_set_here = false;

void customflush()
{
  while (masterSerial.available() > 0)
    masterSerial.read();
}

int requestConnectorStatus()
{
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  int connectorId = 0;
  int startTime = 0;
  bool success = false;

  Serial.println("Master: ");
  txM_doc["type"] = "request";
  txM_doc["object"] = "connector";

  serializeJson(txM_doc, masterSerial); // data send to slave
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while (millis() - startTime < 20000 /*WAIT_TIMEOUT*/)
  { // waiting for response from slave
    if (masterSerial.available())
    {

      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(" Invalid input! Not a JSON\n");
        break;
      case DeserializationError::NoMemory:
        Serial.print("Error: Not enough memory\n");
        break;
      default:
        Serial.print("Deserialization failed\n");
        break;
      }

      if (!success)
      {
        // propably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        Serial.println("***Sending request again***");
        delay(100);
        serializeJson(txM_doc, masterSerial); // data send to slave again
        serializeJson(txM_doc, Serial);
        // return false;
      }
      else
      {
        type = rxM_doc["type"] | "Invalid";
        connectorId = rxM_doc["connectorId"];
        // dis_phase_type = rxM_doc["phase"];
        if (strcmp(type, "response") == 0)
        {
          Serial.println("Received connectorId from slave--->" + String(connectorId));

          return connectorId;
        }
      }
    }
  }
  Serial.println("No response from Slave");
  return connectorId; // returns 0 if connectorId (push button is not pressed)
}

bool requestForRelay(int action, int connectorId)
{

  if (connectorId == 0 || connectorId > 3)
  {
    return false;
  }

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);

  txM_doc.clear();
  txM_doc.clear();

  const char* type = "";
  const char* object = "";

  // const char* statusRelay = "";
  int statusRelay = 0;
  bool success = false;
  int startTime = 0;

  Serial.println("Relay:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "relay";

  txM_doc["action"] = action;
  txM_doc["connectorId"] = connectorId;

  serializeJson(txM_doc, masterSerial); // data send to slave
  serializeJson(txM_doc, Serial);
  delay(2000);

  startTime = millis();
  while (millis() - startTime < 6000)
  { // waiting for response from slave
    if (masterSerial.available())
    {
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(" Invalid input! Not a JSON\n");
        break;
      case DeserializationError::NoMemory:
        Serial.print("Error: Not enough memory\n");
        break;
      default:
        Serial.print("Deserialization failed\n");
        break;
      }

      if (!success)
      {
        // propably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        Serial.println("***Sending request again***");
        delay(100);
        serializeJson(txM_doc, masterSerial); // data send to slave again
        // return false;
      }
      else
      {
        type = rxM_doc["type"] | "Invalid";
        statusRelay = rxM_doc["status"];
        connectorId = rxM_doc["connectorId"];
        if ((strcmp(type, "response") == 0) && (action == statusRelay))
        {
          Serial.println("Received Status--->" + String(statusRelay) + " for ConnectorId: " + String(connectorId));
          return true;
        }
      }
    } // end of .available
  } // end of while loop
  Serial.println("No response from Slave");
  return false;
}

bool requestLed(int colour, int action, int connectorId)
{

  if (connectorId == 0 || connectorId > 4)
  {
    return false;
  }

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";

  // const char* statusRelay = "";
  int statusLed = 0;
  bool success = false;
  int startTime = 0;

  Serial.println("Led:");
  txM_doc["type"] = "request";
  if (connectorId == 4)
  {
    txM_doc["object"] = "rfid";
  }
  else
  {
    txM_doc["object"] = "led";
  }
  txM_doc["colour"] = colour;
  txM_doc["action"] = action;
  txM_doc["connectorId"] = connectorId;

  serializeJson(txM_doc, masterSerial); // data send to slave
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while (millis() - startTime < WAIT_TIMEOUT)
  { // waiting for response from slave
    if (masterSerial.available())
    {
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F(" Invalid input! Not a JSON\n"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Error: Not enough memory\n"));
        break;
      default:
        Serial.print(F("Deserialization failed\n"));
        break;
      }

      if (!success)
      {
        // propably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        Serial.println("***Sending request again***");
        delay(100);
        serializeJson(txM_doc, masterSerial); // data send to slave again
        // return false;
      }
      else
      {
        type = rxM_doc["type"] | "Invalid";
        connectorId = rxM_doc["connectorId"];
        statusLed = rxM_doc["status"];
        if ((strcmp(type, "response") == 0) && (action == statusLed))
        {
          Serial.println("Received Status--->" + String(statusLed) + " for ConnectorId: " + String(connectorId));
          return true;
        }
      }
    } // end of .available
  } // end of while loop
  Serial.println("TimeOut");
  return false;
}

bool requestEmgyStatus()
{

  // DynamicJsonDocument txM_doc(100);
  // DynamicJsonDocument rxM_doc(100);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";
  bool success = false;
  int startTime = 0;
  bool statusE = 0;
  Serial.println("EMGY:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "emgy";

  serializeJson(txM_doc, masterSerial); // data send to slave
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while (millis() - startTime < WAIT_TIMEOUT)
  { // waiting for response from slave
    if (masterSerial.available())
    {
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F(" Invalid input! Not a JSON\n"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Error: Not enough memory\n"));
        break;
      default:
        Serial.print(F("Deserialization failed\n"));
        break;
      }

      if (!success)
      {
        // propably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        Serial.println("***Sending request again***");
        delay(100);
        serializeJson(txM_doc, masterSerial); // data send to slave again
        // return false;
      }
      else
      {
        type = rxM_doc["type"] | "Invalid";
        object = rxM_doc["object"] | "Invalid";
        statusE = rxM_doc["status"];
        if ((strcmp(type, "response") == 0) && (strcmp(object, "emgy") == 0))
        {
          Serial.println("Received Status--->" + String(statusE));
          if (statusE == true)
          {
            return true;
          }
          else if (statusE == false)
          {
            return false;
          }
        }
      }
    } // end of .available
  } // end of while loop
  Serial.println("TimeOut");
  return false;
}

bool requestGFCIStatus()
{

  // DynamicJsonDocument txM_doc(100);
  // DynamicJsonDocument rxM_doc(100);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";
  bool success = false;
  int startTime = 0;
  bool statusE = 0;
  Serial.println("GFCI:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "gfci";

  serializeJson(txM_doc, masterSerial); // data send to slave
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while (millis() - startTime < WAIT_TIMEOUT)
  { // waiting for response from slave
    if (masterSerial.available())
    {
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F(" Invalid input! Not a JSON\n"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Error: Not enough memory\n"));
        break;
      default:
        Serial.print(F("Deserialization failed\n"));
        break;
      }

      if (!success)
      {
        // propably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        Serial.println("***Sending request again***");
        delay(100);
        serializeJson(txM_doc, masterSerial); // data send to slave again
        // return false;
      }
      else
      {
        type = rxM_doc["type"] | "Invalid";
        object = rxM_doc["object"] | "Invalid";
        statusE = rxM_doc["status"];
        if ((strcmp(type, "response") == 0) && (strcmp(object, "gfci") == 0))
        {
          Serial.println("Received Status--->" + String(statusE));
          if (statusE == true)
          {
            flag_GFCI_set_here = true;
            return true;
          }
          else if (statusE == false)
          {
            return false;
          }
        }
      }
    } // end of .available
  } // end of while loop
  Serial.println("TimeOut");
  return false;
}

// CP
bool requestforCP_OUT(int action)
{

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";

  bool success = false;
  int startTime = 0;
  bool status = 0;

  Serial.println("Control Pilot:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "cpout";

  txM_doc["action"] = action;
  //  txM_doc["connectorId"] = connectorId;   future Implemnetation

  serializeJson(txM_doc, masterSerial); // data send to slave
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while (millis() - startTime < WAIT_TIMEOUT)
  { // waiting for response from slave
    if (masterSerial.available())
    {
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F(" Invalid input! Not a JSON\n"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Error: Not enough memory\n"));
        break;
      default:
        Serial.print(F("Deserialization failed\n"));
        break;
      }

      if (!success)
      {
        // propably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        Serial.println("***Sending request again***");
        delay(100);
        serializeJson(txM_doc, masterSerial); // data send to slave again
        // return false;
      }
      else
      {
        type = rxM_doc["type"] | "Invalid";
        object = rxM_doc["object"] | "Invalid";
        status = rxM_doc["status"];
        if ((strcmp(type, "response") == 0) && (strcmp(object, "cpout") == 0))
        {
          Serial.println("Received Status--->" + String(status));
          return true;
        }
      }
    } // end of .available
  } // end of while loop
  Serial.println("No response from Slave");
  return false;
}

int requestforCP_IN()
{

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";

  bool success = false;
  int startTime = 0;
  int value = 0;

  Serial.println("Control Pilot:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "cpin";

  //  txM_doc["connectorId"] = connectorId;   future Implemnetation

  serializeJson(txM_doc, masterSerial); // data send to slave
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while (millis() - startTime < WAIT_TIMEOUT)
  { // waiting for response from slave
    if (masterSerial.available())
    {
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F(" Invalid input! Not a JSON\n"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Error: Not enough memory\n"));
        break;
      default:
        Serial.print(F("Deserialization failed\n"));
        break;
      }

      if (!success)
      {
        // propably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        serializeJson(txM_doc, masterSerial); // data send to slave again
        // return false;
      }
      else
      {
        type = rxM_doc["type"] | "Invalid";
        object = rxM_doc["object"] | "Invalid";
        value = rxM_doc["value"];
        dis_phase_type = rxM_doc["value1"];
        device_load = rxM_doc["value2"];
        if ((strcmp(type, "response") == 0) && (strcmp(object, "cpin") == 0))
        {
          Serial.println("Received value--->" + String(value));
          Serial.println("Received phase from slave--->" + String(dis_phase_type));
          Serial.println("Received phase from slave--->" + String(device_load));
          return value;
        }
      }
    } // end of .available
  } // end of while loop
  Serial.println("No response from Slave");
  return false;
}

#if ALPR_ENABLED
void alprRead_loop()
{
  rxA_doc.clear();
  txA_doc.clear();

  bool success = false;

  const char* licenseData = "";
  char s[500] = { 0 };
  int i = 0;
  String payload;

  if (alprSerial.available())
  {

    // Serial.println(alprSerial.readLine().decode("utf-8").rstrip());
    // while(alprSerial.available()>0){
    payload = alprSerial.readString();
    //  Serial.println(s);
    //}

    Serial.println(payload);
    delay(1000);
    // ReadLoggingStream loggingStream(alprSerial, Serial);
    DeserializationError err = deserializeJson(rxA_doc, payload);
    //  Serial.print(rxM_doc);
    switch (err.code())
    {
    case DeserializationError::Ok:
      success = true;
      break;
    case DeserializationError::InvalidInput:
      Serial.print(F(" Invalid input! Not a JSON\n"));
      break;
    case DeserializationError::NoMemory:
      Serial.print(F("Error: Not enough memory\n"));
      break;
    default:
      Serial.print(F("Deserialization failed\n"));
      break;
    }

    if (!success)
    {
      // propably the payload just wasn't a JSON message
      rxA_doc.clear();
      delay(1000);
    }
    else
    {
      // licenseData = rxA_doc["LicensePlate"];
      licenseData = rxA_doc["license"];

      String data = String(licenseData);

      if ((strcmp(licenseData, "") != 0))
      {
        Serial.println("Received value--->" + String(licenseData));

        getChargePointStatusService_A()->authorize(data);

        txA_doc["ESP_Status"] = "OK";
        serializeJson(txA_doc, alprSerial); // data send to slave
        serializeJson(txA_doc, Serial);
        delay(100);
      }
    }
  }
}

void alprAuthorized()
{
  txA_doc.clear();
  txA_doc["Authorization"] = 1;
  serializeJson(txA_doc, alprSerial);
  serializeJson(txA_doc, Serial);
  Serial.println();
  delay(1000);
}

void alprunAuthorized()
{
  txA_doc.clear();
  txA_doc["Authorization"] = 0;
  serializeJson(txA_doc, alprSerial);
  serializeJson(txA_doc, Serial);
  Serial.println();
  delay(1000);
}

void alprTxnStarted()
{
  txA_doc.clear();
  txA_doc["Txn_Started"] = 1;
  serializeJson(txA_doc, alprSerial);
  serializeJson(txA_doc, Serial);
  Serial.println();
  delay(1000);
}

void alprTxnStopped()
{
  txA_doc.clear();
  txA_doc["Txn_Stopped"] = 0;
  serializeJson(txA_doc, alprSerial);
  serializeJson(txA_doc, Serial);
  Serial.println();
  delay(1000);
}
#endif
void Master_setup()
{
  // put your setup code here, to run once:

  // Serial.begin(115200);
  // It is important to do this in order to avoid deserialization error
  masterSerial.begin(9600, SWSERIAL_8N1, 25, 33, false, 256);
  alprSerial.begin(115200, SERIAL_8N1, 26, 27);
  Serial.println("***VERSION HISTORY***");
  // Serial.println(F("***Firmware_ver-----EVSE_7.4KW_V1.2.0***"));
  Serial.print(F("***Firmware_ver-----:"));
  Serial.println(String(VERSION) + "***");

#if V_charge_lite1_4
  Serial.println("***Hardware_ver-----VL_1.4***");
#else
  Serial.println("***Hardware_ver-----VL_1.3***");
#endif

#if DWIN_ENABLED
  Serial.println("Integrated DWIN 3.5 Inch Display");
#endif

#if LCD_ENABLED
  Serial.println("Integrated LCD 20x4 Display");
#endif

  Serial.println("Implemented power recycle-after power recycle older transaction will stop, new transaction with existing tag ID will be initiated.");
  Serial.println("Relay toggle removed when the PWM starts.(Master wont initiate the relay driving, slave itself take caring the relay driving based on CP status)");
  Serial.println("Implmented single start and stop transaction request.");
  Serial.println("Increased the suspended state time to 10 min");
  Serial.println("Invalid type added in json deserialization for master and slave communication");
  Serial.println("while EVSE device is charging:");
  Serial.println("1. Connectivity is changed from Online to offline then, meter values not pushed in the OCCP out-going Message Queues.");
  Serial.println("2. if Device restart, it will check onging transaction is exist or not, if yes then resume it, with Relay ON, green and white led blink will indicated.");
  Serial.println("3. if Internet off and on, then it will reconnect the internet and update ocpp message to CMS");
  Serial.println("Added Wifi on-Event for checking wifi-level Connectivity");
  Serial.println("Added GSM based functionality.");
  Serial.println("removed the Wifi on-Event for checking wifi-level Connectivity.");
  Serial.println("Starts device charging even-though the start transaction response is ConcurrentTx.");
  Serial.println("Last update date:19-04-2023");
  Serial.println("Updated by: Krishna Marri & Shiva Poola");
  Serial.println("Complete core profile, reserve now and ALPR integrated!");
  Serial.println("Updated by Shiva Poola & Abhigna Valmiki. Date:27-12-2023");
  Serial.println("Fixed Websocket reconnect Issue.");
  Serial.println("Fixed Connector are showing Preparing State after sessions terminated Issue.");
  Serial.println("Fixed Invalid Meter values starting from zero,after session resume on Power recycle Issue.");
  Serial.println("Fixed Session is stop through Emergency Switch,  Meter values are not updating in offline, sending stop with online last updated Watt-Hour Issue.");
  Serial.println("Dynamic URL_Parser added.");
  Serial.println("Fixed Multiple status notification.");
  Serial.println("Fixed Relays are starting in offline once emergency pressed and released Issue.");
  Serial.println("last Updated by Abhigna Valmiki. Date:13-03-2024");
  Serial.println("LCD Driver implementation");
  Serial.println("LED Driver Implemetation");
  Serial.println("Updating metervalue to 0 in stop transaction if the meter stop value exceeds by 19900000");
  Serial.println("Added configuration based OTA - WIFI & 4G,Device ID Based OTA For WIFI & 4G.");
  Serial.println("Chargepoint id in boot notification");
  Serial.println("Retry for Bootnotification till accpted if boot notification Timed out");
  Serial.println("Changed complete wifi functionality(WiFi Driver).");
  Serial.println("Changed arduino websocket version from 2.3.6 - 2.3.7");
  Serial.println("Sends status notification as preparing instead of finishing if session is stopped due to powerloss ");
  Serial.println("Offline Consumption in 4G implemented");
  Serial.print("[BOOT] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
}

/*
void loop() {
  // put your main code here, to run repeatedly:
   //requestLed(RED,START,1);

  bool emgcy = requestEmgyStatus();
  Serial.println("Emergency Button Status--> " +String(emgcy));
  if(emgcy == 0){
    for(;;){

      Serial.println("EMGY is pressed");
       bool emgcy = requestEmgyStatus();
       if(emgcy ==1) break;
      }
  }
  int connector = requestConnectorStatus();
  Serial.println("Connector: " + String(connector)+ "\n");
  delay(1000);
  if(connector > 0){
   bool statusRelay = requestForRelay(START,connector);
   Serial.println("statusRelay--->" + String(statusRelay));

  delay(1000);

  bool ch1 = requestForRelay(STOP,connector);
  Serial.println("ch1 --->" +String(ch1));

  delay(1000);

  bool ch2 = requestLed(RED,START,connector);
  Serial.println("ch2--->" +String(ch2));

  delay(1000);

  requestLed(GREEN,START,connector);

  delay(1000);

    requestLed(BLUE,START,connector);

  delay(1000);
    requestLed(WHITE,START,connector);

  delay(1000);
    requestLed(RED,STOP,connector);

  //  delay(1000);
   // requestLed(RED,START,connector);
  }
}
*/

/*Load_balancing*/
#if LOAD_CONTROL && 0
bool sendPower(float power)
{
  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);

  int startTime = 0;
  bool success = false;
  const char* type = "";
  const char* object = "";
  bool status = 0;

  Serial.println("Control Pilot:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "powersend";
  power
    txM_doc["power"] = ; // Send power as an argument

  serializeJson(txM_doc, masterSerial); // Data sent to slave
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while (millis() - startTime < WAIT_TIMEOUT_SHORT)
  { // Waiting for response from slave
    if (masterSerial.available())
    {
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      // Serial.print(rxM_doc);
      switch (err.code())
      {
      case DeserializationError::Ok:
        success = true;
        break;
      case DeserializationError::InvalidInput:
        Serial.print(F(" Invalid input! Not a JSON\n"));
        break;
      case DeserializationError::NoMemory:
        Serial.print(F("Error: Not enough memory\n"));
        break;
      default:
        Serial.print(F("Deserialization failed\n"));
        break;
      }

      if (!success)
      {
        // Probably the payload just wasn't a JSON message
        rxM_doc.clear();
        delay(200);
        customflush();
        Serial.println("***Sending request again***");
        delay(100);
        serializeJson(txM_doc, masterSerial); // Data sent to slave again
      }
#if 0
      else
      {
        type = rxM_doc["type"] | "Invalid";
        object = rxM_doc["object"] | "Invalid";
        status = rxM_doc["status"];
        if ((strcmp(type, "response") == 0) && (strcmp(object, "cpout") == 0))
        {
          Serial.println("Received Status--->" + String(status));
          return true;
        }
      }
#endif

    } // end of .available
  } // end of while loop

  // Serial.println("No response from Slave");
  return false;
}

#endif

/* Load Balancing */
#if LOAD_CONTROL
bool sendPower(float power)
{
  // DynamicJsonDocument txM_doc(200);

  Serial.println("Control Pilot:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "powersend";
  txM_doc["power"] = power; // Send power as an argument

  serializeJson(txM_doc, masterSerial); // Data sent to slave
  serializeJson(txM_doc, Serial);       // Optional: Print data to Serial for debugging
  delay(100);                           // Small delay to ensure data transmission

  return true; // Return true as a confirmation of sending the data
}

#endif
