// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "MeterValues.h"
#include "OcppEngine.h"
#include "MeteringService.h"
#include <EEPROM.h>

#include "Variants.h"

extern bool flag_trigger_metervalues;
extern float online_charging_Enargy_A;



// can only be used for echo server debugging
MeterValues::MeterValues()
{
  sampleTime = LinkedList<time_t>(); // not used in server mode but needed to keep the destructor simple
  power = LinkedList<float>();
  energy = LinkedList<float>();
  voltage = LinkedList<float>();
  current = LinkedList<float>();
  temperature = LinkedList<float>();
}

MeterValues::MeterValues(LinkedList<time_t>* sampleTimeReg, LinkedList<float>* powerReg, LinkedList<float>* energyReg, LinkedList<float>* voltageReg, LinkedList<float>* currentReg, LinkedList<float>* temperatureReg)
{
  sampleTime = LinkedList<time_t>();
  power = LinkedList<float>();
  energy = LinkedList<float>();
  voltage = LinkedList<float>();
  current = LinkedList<float>();
  temperature = LinkedList<float>();
  for (int i = 0; i < sampleTimeReg->size(); i++)
    sampleTime.add(sampleTimeReg->get(i));
  for (int i = 0; i < powerReg->size(); i++)
    power.add(powerReg->get(i));
  for (int i = 0; i < energyReg->size(); i++)
    energy.add(energyReg->get(i));
  for (int i = 0; i < voltageReg->size(); i++)
    voltage.add(voltageReg->get(i));
  for (int i = 0; i < currentReg->size(); i++)
    current.add(currentReg->get(i));
  for (int i = 0; i < temperatureReg->size(); i++)
    temperature.add(temperatureReg->get(i));
}

MeterValues::~MeterValues()
{
  sampleTime.clear();
  power.clear();
  energy.clear();
  voltage.clear();
  current.clear();
  temperature.clear();
}

const char* MeterValues::getOcppOperationType()
{
  return "MeterValues";
}

DynamicJsonDocument* MeterValues::createReq()
{

  DynamicJsonDocument* doc = new DynamicJsonDocument(1024);

  JsonObject payload = doc->to<JsonObject>();
  // Required field conn id
  payload["connectorId"] = getChargePointStatusService_A()->getConnectorId();
  // Required field metervalue
  JsonArray meterValues = payload.createNestedArray("meterValue");

  /*
   * @brief : Serve the trigger message if any.
   */

  if (flag_trigger_metervalues)
  {
    flag_trigger_metervalues = false;
    JsonObject meterValue = meterValues.createNestedObject();
    char timestamp[JSONDATE_LENGTH + 1] = { '\0' };
    // getJsonDateStringFromGivenTime(timestamp, JSONDATE_LENGTH + 1, sampleTime.get(i));
    // get the current time and fill it below
    if (getJsonDateStringFromSystemTime(timestamp, JSONDATE_LENGTH))
    {

      meterValue["timestamp"] = timestamp;
    }
    else
    {
      // Error caught!
      meterValue["timestamp"] = "2020-10-01T20:53:32.486Z";
      Serial.println("[MeterValues] Warning! Invalid timestamp. Yet, sending it!");
    }

    JsonArray sampledValue = meterValue.createNestedArray("sampledValue");

    /*
     * @brief : Fetch energy from eeprom
     */
    EEPROM.begin(sizeof(EEPROM_Data));
    float lastEnergy;
    EEPROM.get(4, lastEnergy);
    EEPROM.end();
   
    JsonObject sampledValue_2 = sampledValue.createNestedObject();
    sampledValue_2["value"] = String(lastEnergy);
    sampledValue_2["context"] = "Sample.Periodic";
    sampledValue_2["format"] = "Raw";
    sampledValue_2["measurand"] = "Energy.Active.Import.Register";
    sampledValue_2["phase"] = "L1";
    sampledValue_2["location"] = "Cable";
    sampledValue_2["unit"] = "Wh";

    JsonObject sampledValue_1 = sampledValue.createNestedObject();
    sampledValue_1["value"] = String(eic.GetLineCurrentA() * eic.GetLineVoltageA());
    sampledValue_1["context"] = "Sample.Periodic";
    sampledValue_1["format"] = "Raw";
    sampledValue_1["measurand"] = "Power.Active.Import";
    sampledValue_1["phase"] = "L1";
    sampledValue_1["location"] = "Cable";
    sampledValue_1["unit"] = "kW";

    JsonObject sampledValue_3 = sampledValue.createNestedObject();
    sampledValue_3["value"] = String(eic.GetLineVoltageA());
    sampledValue_3["context"] = "Sample.Periodic";
    sampledValue_3["format"] = "Raw";
    sampledValue_3["measurand"] = "Voltage";
    sampledValue_3["phase"] = "L1";
    sampledValue_3["location"] = "Cable";
    sampledValue_3["unit"] = "V";

    JsonObject sampledValue_4 = sampledValue.createNestedObject();
    sampledValue_4["value"] = String(eic.GetLineCurrentA());
    sampledValue_4["context"] = "Sample.Periodic";
    sampledValue_4["format"] = "Raw";
    sampledValue_4["measurand"] = "Current.Import";
    sampledValue_4["phase"] = "L1";
    sampledValue_4["location"] = "Cable";
    sampledValue_4["unit"] = "A";

    JsonObject sampledValue_5 = sampledValue.createNestedObject();
    sampledValue_5["value"] = String(eic.GetTemperature());
    sampledValue_5["context"] = "Sample.Periodic";
    sampledValue_5["format"] = "Raw";
    sampledValue_5["measurand"] = "Temperature";
    sampledValue_5["unit"] = "Celsius";
  }
  else
  {
    for (int i = 0; i < sampleTime.size(); i++)
    {
      JsonObject meterValue = meterValues.createNestedObject();
      char timestamp[JSONDATE_LENGTH + 1] = { '\0' };
      getJsonDateStringFromGivenTime(timestamp, JSONDATE_LENGTH + 1, sampleTime.get(i));
      meterValue["timestamp"] = timestamp;
      JsonArray sampledValue = meterValue.createNestedArray("sampledValue");
      if (energy.size() - 1 >= i)
      {
        JsonObject sampledValue_2 = sampledValue.createNestedObject();
        sampledValue_2["value"] = String(energy.get(i));
        sampledValue_2["context"] = "Sample.Periodic";
        sampledValue_2["format"] = "Raw";
        sampledValue_2["measurand"] = "Energy.Active.Import.Register";
        sampledValue_2["phase"] = "L1";
        sampledValue_2["location"] = "Cable";
        sampledValue_2["unit"] = "Wh";
        online_charging_Enargy_A = sampledValue_2["value"];
        /*sampledValue_2["setValue"] = true;
        sampledValue_2["setContext"] = true;
        sampledValue_2["setFormat"] = true;
        sampledValue_2["setMeasurand"] = true;
        sampledValue_2["setPhase"] = true;
        sampledValue_2["setLocation"] = true;
        sampledValue_2["setUnit"] = true;*/
      }

      if (power.size() - 1 >= i)
      {
        JsonObject sampledValue_1 = sampledValue.createNestedObject();
        sampledValue_1["value"] = String(power.get(i));
        sampledValue_1["context"] = "Sample.Periodic";
        sampledValue_1["format"] = "Raw";
        sampledValue_1["measurand"] = "Power.Active.Import";
        sampledValue_1["phase"] = "L1";
        sampledValue_1["location"] = "Cable";
        sampledValue_1["unit"] = "kW";
        /*sampledValue_1["setValue"] = true;
        sampledValue_1["setContext"] = true;
        sampledValue_1["setFormat"] = true;
        sampledValue_1["setMeasurand"] = true;
        sampledValue_1["setPhase"] = true;
        sampledValue_1["setLocation"] = true;
        sampledValue_1["setUnit"] = true;*/
      }

      if (voltage.size() - 1 >= i)
      {
        JsonObject sampledValue_3 = sampledValue.createNestedObject();
        sampledValue_3["value"] = String(voltage.get(i));
        sampledValue_3["context"] = "Sample.Periodic";
        sampledValue_3["format"] = "Raw";
        sampledValue_3["measurand"] = "Voltage";
        sampledValue_3["phase"] = "L1";
        sampledValue_3["location"] = "Cable";
        sampledValue_3["unit"] = "V";
        /*sampledValue_3["setValues"] = true;
        sampledValue_3["setContext"] = true;
        sampledValue_3["setFormat"] = true;
        sampledValue_3["setMeasurand"] = true;
        sampledValue_3["setPhase"] = true;
        sampledValue_3["setLocation"] = true;
        sampledValue_3["setUnit"] = true;*/
      }
      if (current.size() - 1 >= i)
      {
        JsonObject sampledValue_4 = sampledValue.createNestedObject();
        sampledValue_4["value"] = String(current.get(i));
        sampledValue_4["context"] = "Sample.Periodic";
        sampledValue_4["format"] = "Raw";
        sampledValue_4["measurand"] = "Current.Import";
        sampledValue_4["phase"] = "L1";
        sampledValue_4["location"] = "Cable";
        sampledValue_4["unit"] = "A";
        /*sampledValue_4["setValue"] = true;
        sampledValue_4["setContext"] = true;
        sampledValue_4["setFormat"] = true;
        sampledValue_4["setMeasurand"] = true;
        sampledValue_4["setPhase"] = true;
        sampledValue_4["setLocation"] = true;
        sampledValue_4["setUnit"] = true;*/
      }
      if (temperature.size() - 1 >= i)
      {
        JsonObject sampledValue_5 = sampledValue.createNestedObject();
        sampledValue_5["value"] = String(temperature.get(i));
        sampledValue_5["context"] = "Sample.Periodic";
        sampledValue_5["format"] = "Raw";
        sampledValue_5["measurand"] = "Temperature";
        // sampledValue_5["phase"] = NULL;
        // sampledValue_5["location"] = "Body";
        sampledValue_5["unit"] = "Celsius";
        /*sampledValue_5["setValue"] = true;
        sampledValue_5["setContext"] = true;
        sampledValue_5["setFormat"] = true;
        sampledValue_5["setMeasurand"] = true;
        sampledValue_5["setPhase"] = false;
        sampledValue_5["setLocation"] = true;
        sampledValue_5["setUnit"] = true;*/
      }
      /*meterValue["setSampledValue"] = true;
      meterValue["setTimestamp"] = true;*/
    }
  }
  // optional field trxn id. Since it is not mandatory, avoiding the  -1
  if (getChargePointStatusService_A() != NULL)
  {
    // Here we have to check if txn id is -1 or not.
    int32_t t = getChargePointStatusService_A()->getTransactionId();
    // payload["transactionId"] = getChargePointStatusService()->getTransactionId();
    if (t != -1)
    {
      payload["transactionId"] = t;
    }
    else
    {
      return doc; // since it is optional
    }
  }

  /*payload["setConnectorId"] = true;
  payload["setTransactionId"] = true;
  payload["setMeterValue"] = true;*/

  return doc;
}

void MeterValues::processConf(JsonObject payload)
{
  if (DEBUG_OUT)
    Serial.print("[MeterValues] Request has been confirmed!\n");
}

void MeterValues::processReq(JsonObject payload)
{

  /**
   * Ignore Contents of this Req-message, because this is for debug purposes only
   */
}

DynamicJsonDocument* MeterValues::createConf()
{
  DynamicJsonDocument* doc = new DynamicJsonDocument(0);
  JsonObject payload = doc->to<JsonObject>();
  /*
   * empty payload
   */
  return doc;
}
