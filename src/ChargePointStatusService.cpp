// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "ChargePointStatusService.h"
#include "StatusNotification.h"
#include "OcppEngine.h"
#include "SimpleOcppOperationFactory.h"
#include "Master.h"

#include <string.h>

#if DWIN_ENABLED
#include "dwin.h"
extern unsigned char charging[28];
extern unsigned char change_page[10];
extern unsigned char avail[28];
extern unsigned char not_avail[28];
extern unsigned char fault_emgy[28];
extern unsigned char fault_noearth[28];
extern unsigned char fault_overVolt[28];
extern unsigned char fault_underVolt[28];
extern unsigned char fault_overTemp[28];
extern unsigned char fault_overCurr[28];
extern unsigned char fault_underCurr[28];
extern unsigned char fault_suspEV[28];
extern unsigned char fault_suspEVSE[28];
#endif

bool flag_stop_finishing = false;

int transactionId_A = -1;
extern uint8_t gu8_fault_occured;
#if LCD_ENABLED
#include "LCD_I2C.h"
extern LCD_I2C lcd;
#endif
extern uint8_t gu8_online_flag; 

ChargePointStatus evse_ChargePointStatus;
extern bool webSocketConncted;

extern bool wifi_connect;
extern bool gsm_connect;
extern bool ethernet_connect;

extern bool flag_GFCI_set_here;

extern bool flag_start_accepted;

ChargePointStatusService::ChargePointStatusService() {} // dummy constructor

ChargePointStatusService::ChargePointStatusService(WebSocketsClient *webSocket)
	: webSocket(webSocket)
{
	setChargePointStatusService(this);
}

ChargePointStatus ChargePointStatusService::inferenceStatus()
{
         return evse_ChargePointStatus;
}


void ChargePointStatusService::loop()
{
    ChargePointStatus inferencedStatus = inferenceStatus();
    if (inferencedStatus != currentStatus)
    {
        currentStatus = inferencedStatus;
        Serial.print("CP LOOP:");
        switch(currentStatus)
        {
            case Available:
                Serial.println("Available");
                break;
            case Preparing:
                Serial.print("Preparing");
                break;
            case Charging:
                Serial.print("Charging");
                break;
            case SuspendedEVSE:
                Serial.print("SuspendedEVSE");
                break;
            case SuspendedEV:
                Serial.print("SuspendedEV");
                break;
            case Finishing:    //not supported by this client
                Serial.print("Finishing");
                break;
            case Reserved:     //Implemented reserve now
                Serial.print("Reserved");
                break;
            case Unavailable:  //Implemented Change Availability
                Serial.print("Unavailable");
                break;
            case Faulted:      //Implemented Faulted.
                Serial.print("Faulted");
                break;
            default:
                    // evse_ChargePointStatus = Available;
                    //Serial.print("Available");
               break;
        }
        if (DEBUG_OUT)
            Serial.print("[ChargePointStatusService] Status changed\n");
        if((gu8_online_flag == 1) && (currentStatus != NOT_SET))
		{
            // fire StatusNotification
            // TODO check for online condition: Only inform CS about status change if CP is online
            // TODO check for too short duration condition: Only inform CS about status change if it lasted for longer than MinimumStatusDuration
			// OcppOperation *statusNotification = makeOcppOperation(webSocket, new StatusNotification(currentStatus));
			// initiateOcppOperation(statusNotification);
			OcppOperation *statusNotification = makeOcppOperation(webSocket,
															  new StatusNotification(currentStatus, connectorId));
		initiateOcppOperation(statusNotification);
        
		}
		if((gu8_online_flag == 1) && (currentStatus == NOT_SET))
		{
			if(evse_ChargePointStatus == NOT_SET)
			{
				gu8_fault_occured = 1;
			}
		}
    }
}

#if 0
ChargePointStatus ChargePointStatusService::inferenceStatus()
{
	/*
	 * @brief : For finsihing
	 */
	if (flag_stop_finishing)
	{
		return ChargePointStatus::Finishing;
	}

	if (reserved)
	{
		// reserved = false;
		return ChargePointStatus::Reserved;
	}
	/*
	 * @brief : Feature added by G. Raja Sumant
	 * 09/07/2022 as part of change availability
	 */
	if (unavailable)
	{
		return ChargePointStatus::Unavailable;
	}

	if (emergencyRelayClose)
	{
		return ChargePointStatus::Faulted;
	}
	else if (unavailable)
	{
#if 0
		lcd.clear();
		lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: UNAVAILABLE");
		lcd.setCursor(0, 3);
		if (wifi_connect)
			lcd.print("CLOUD: WIFI");
		else if (gsm_connect)
			lcd.print("CLOUD: 4G");
		else if(ethernet_connect)
			lcd.print("CLOUD: ETHERNET");
		else
			lcd.print("CLOUD: OFFLINE");
#endif
		return ChargePointStatus::Unavailable;
	}
	else if (!authorized)
	{
#if 0
		lcd.clear();
		lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: AVAILABLE");
		lcd.setCursor(0, 1);
		lcd.print("TAP RFID/SCAN QR");
		lcd.setCursor(0, 2);
		lcd.print("CONNECTION");
		lcd.setCursor(0, 3);
		if (wifi_connect)
			lcd.print("CLOUD: WIFI");
		else if (gsm_connect)
			lcd.print("CLOUD: 4G");
		else if(ethernet_connect)
			lcd.print("CLOUD: ETHERNET");
		else
			lcd.print("CLOUD: OFFLINE");
#endif
		return ChargePointStatus::Available;
	}
	else if (!transactionRunning)
	{
		// Preparing should be sent only when it has been accepted.
		if (flag_start_accepted)
		{
			return ChargePointStatus::Preparing;
		}
		else
		{
			return ChargePointStatus::Available;
		}
	}
	else if (!evDrawsEnergy)
	{
#if LCD_ENABLED
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("STATUS: SUSPENDED EV");
#endif
		return ChargePointStatus::SuspendedEV;
	}
	else if (!evseOffersEnergy)
	{
#if LCD_ENABLED
		lcd.clear();
		lcd.setCursor(0, 0);
		lcd.print("STATUS: SUSPENDED EVSE");
#endif
		return ChargePointStatus::SuspendedEVSE;
	}
	else
	{
		return ChargePointStatus::Charging;
	}
	// if (!authorized) {
	// 	if (emergencyRelayClose){
	// 		return ChargePointStatus::Faulted;
	// 	} else {
	// 		return ChargePointStatus::Available;
	// 	}

	// } else if (!transactionRunning) {
	// 	return ChargePointStatus::Preparing;
	// } else {
	// 	//Transaction is currently running
	// 	if (emergencyRelayClose){
	// 		return ChargePointStatus::Faulted;
	// 	} else {

	// 			if (!evDrawsEnergy) {
	// 				return ChargePointStatus::SuspendedEV;
	// 			}
	// 			if (!evseOffersEnergy) {
	// 				return ChargePointStatus::SuspendedEVSE;
	// 			}
	// 			return ChargePointStatus::Charging;
	// 	}
	// }
}

void ChargePointStatusService::loop()
{
	if (DEBUG_OUT)
		Serial.println(("[ChargePointStatusService] for Connector ID:" + String(connectorId)));

	ChargePointStatus inferencedStatus = inferenceStatus();
	// status_notification_inf_status = inferencedStatus;
	// if (DEBUG_OUT) Serial.println(("[ChargePointStatusService] inferenced status:"+String(status_notification_inf_status)));

	if (inferencedStatus != currentStatus)
	{
		currentStatus = inferencedStatus;
		#if DWIN_ENABLED

		uint8_t err = 0;
		#endif
#if 0
if (!authorized)
		{

			if (emergencyRelayClose)
			{

				// return ChargePointStatus::Faulted;
				// Capture the reason for stop.
				if (getChargePointStatusService_A()->getOverVoltage() == true)
				{
					// reasonForStop = Other;
				}
				else if (getChargePointStatusService_A()->getUnderVoltage() == true)
				{
					if (eic.GetLineVoltageA() < 170 && eic.GetLineVoltageA() > 50)
					{
						// EVSE_StopSession();//No earth
						getChargePointStatusService_A()->stopTransaction();
						// reasonForStop = Other;
					}
					else
					{

						// reasonForStop = Other;
					}
				}
				else if (getChargePointStatusService_A()->getUnderCurrent() == true)
				{
					// reasonForStop = EVDisconnected;
				}
				else if (getChargePointStatusService_A()->getOverCurrent() == true)
				{
					// reasonForStop = Other;
				}
				else if (getChargePointStatusService_A()->getUnderTemperature() == true)
				{
					// reasonForStop = Other;
				}
				else if (getChargePointStatusService_A()->getOverTemperature() == true)
				{
					// reasonForStop = Other;
				}
				else if (emergencyRelayClose)
				{

					bool EMGCY_status = requestEmgyStatus();
					bool GFCI_status = requestGFCIStatus();

					if (EMGCY_status)
					{
						// reasonForStop = EmergencyStop;
					}
					else if (GFCI_status)
					{
						// reasonForStop = Other;
					}
				}
			}
			else
			{

				// return ChargePointStatus::Available;
			}
		}
		else if (!transactionRunning)
		{
			// reasonForStop = Local;
			/*bool GFCI_status  = requestGfciStatus();
			if(GFCI_status)
			{
				if(DEBUG_OUT) Serial.println(F("**** reason for stop = GFCI 1******"));
				//reasonForStop = Other;
			}*/

			if (flag_GFCI_set_here)
			{
				if (DEBUG_OUT)
					Serial.println(F("**** reason for stop = GFCI 1******"));
				// reasonForStop = Other;
			}

			// return ChargePointStatus::Preparing;
		}
		else
		{

			// lcd.setCursor(0, 0);
			// Transaction is currently running
			if (emergencyRelayClose)
			{

				// reasonForStop = Local;

				bool EMGCY_status = requestEmgyStatus();
				// bool GFCI_status  = requestGfciStatus();

				if (EMGCY_status)
				{
					// reasonForStop = EmergencyStop;
				}
				/*if(GFCI_status)
				{
					if(DEBUG_OUT) Serial.println(F("**** reason for stop = GFCI 2******"));
					//reasonForStop = Other;
				}*/
				if (flag_GFCI_set_here)
				{
					if (DEBUG_OUT)
						Serial.println(F("**** reason for stop = GFCI 2******"));
					// reasonForStop = Other;
				}

				if (getChargePointStatusService_A()->getOverVoltage() == true)
				{
					// Added a new condition to check the toggling of relays in no earth state.
					// G. Raja Sumant - 06/05/2022
					getChargePointStatusService_A()->stopEvDrawsEnergy();
					// reasonForStop = Other;
				}
				else if (getChargePointStatusService_A()->getUnderVoltage() == true)
				{
					if (eic.GetLineVoltageA() < 170 && eic.GetLineVoltageA() > 50)
					{
						// getChargePointStatusService_A()->stopTransaction();//No earth
						//  Added a new condition to check the toggling of relays in no earth state.
						// G. Raja Sumant - 06/05/2022
						getChargePointStatusService_A()->stopEvDrawsEnergy();
						// reasonForStop = Other;
					}
					else
					{
						// Added a new condition to check the toggling of relays in no earth state.
						// G. Raja Sumant - 06/05/2022
						getChargePointStatusService_A()->stopEvDrawsEnergy();
						// reasonForStop = Other;
					}
				}
				else if (getChargePointStatusService_A()->getUnderCurrent() == true)
				{
					// if(reasonForStop!= 3 || reasonForStop!= 4)
					// reasonForStop = EVDisconnected;
				}
				else if (getChargePointStatusService_A()->getOverCurrent() == true)
				{
					// reasonForStop = Other;
				}
				else if (getChargePointStatusService_A()->getUnderTemperature() == true)
				{
					// Added a new condition to check the toggling of relays in no earth state.
					// G. Raja Sumant - 06/05/2022
					getChargePointStatusService_A()->stopEvDrawsEnergy();
					// reasonForStop = Other;
				}
				else if (getChargePointStatusService_A()->getOverTemperature() == true)
				{
					// Added a new condition to check the toggling of relays in no earth state.
					// G. Raja Sumant - 06/05/2022
					getChargePointStatusService_A()->stopEvDrawsEnergy();
					// reasonForStop = Other;
				}
			}
			else
			{

				if (!evDrawsEnergy)
				{
					// return ChargePointStatus::SuspendedEV;
					////reasonForStop = Local;
				}

				if (!evseOffersEnergy)
				{
					// return ChargePointStatus::SuspendedEVSE;
					////reasonForStop = Local;
				}
			}
		}
#endif

#if DISPLAY_ENABLED
		displayStatus((int)currentStatus);
#endif


		if (DEBUG_OUT)
			Serial.print(("[ChargePointStatusService] Status changed for Connector ID:" + String(connectorId)));

		if (emergencyRelayClose)
		{
		}
		else if (unavailable)
		{
			#if 0
		lcd.clear();
		lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: UNAVAILABLE");
		lcd.setCursor(0, 3);
		if (wifi_connect)
			lcd.print("CLOUD: WIFI");
		else if (gsm_connect)
			lcd.print("CLOUD: 4G");
		else if(ethernet_connect)
			lcd.print("CLOUD: ETHERNET");
		else
			lcd.print("CLOUD: OFFLINE");
#endif

		}
		else if (!authorized)
		{
#if 0
			lcd.clear();
			lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
			lcd.print("STATUS: AVAILABLE");
			lcd.setCursor(0, 1);
			lcd.print("TAP RFID/SCAN QR");
			lcd.setCursor(0, 2);
			lcd.print("CONNECTION");
			lcd.setCursor(0, 3);
			if (wifi_connect)
				lcd.print("CLOUD: WIFI");
			else if (gsm_connect)
				lcd.print("CLOUD: 4G");
			else if(ethernet_connect)
				lcd.print("CLOUD: ETHERNET");
			else
				lcd.print("CLOUD: OFFLINE");
#endif
#if DWIN_ENABLED
				change_page[9] = 0;
				// avail[4] = 0X51;
				// uint8_t err = DWIN_SET(avail,sizeof(avail)/sizeof(avail[0]));
				// delay(50);
				err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
				delay(50);
#endif
		}
		else if (!transactionRunning)
		{
		}
		else if (!evDrawsEnergy)
		{
#if 0
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("STATUS: SUSPENDEDEV");
#endif
		}
		else if (!evseOffersEnergy)
		{
#if 0
			lcd.clear();
			lcd.setCursor(0, 0);
			lcd.print("STATUS: SUSPENDEDEVSE");
#endif
		}
		else
		{
		}

		// fire StatusNotification
		// TODO check for online condition: Only inform CS about status change if CP is online
		// TODO check for too short duration condition: Only inform CS about status change if it lasted for longer than MinimumStatusDuration
		OcppOperation *statusNotification = makeOcppOperation(webSocket,
															  new StatusNotification(currentStatus, connectorId));
		initiateOcppOperation(statusNotification);
	}
}
#endif

void ChargePointStatusService::authorize(String &idTag, int connectorId)
{
	this->idTag = String(idTag);
	this->connectorId = connectorId;
	authorize();
}

void ChargePointStatusService::authorize(String &idTag)
{
	this->idTag = String(idTag);
	authorize();
}

void ChargePointStatusService::authorize()
{
	if (authorized == true)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService] Authorize\n");
		return;
	}
	authorized = true;
}

int &ChargePointStatusService::getConnectorId()
{
	return connectorId;
}

void ChargePointStatusService::setConnectorId(int connectorId)
{
	this->connectorId = connectorId;
}
String &ChargePointStatusService::getIdTag()
{
	return idTag;
}

bool ChargePointStatusService::getOverVoltage()
{
	return overVoltage;
}

void ChargePointStatusService::setOverVoltage(bool ov)
{
	this->overVoltage = ov;
}

bool ChargePointStatusService::getUnderVoltage()
{
	return underVoltage;
}

void ChargePointStatusService::setUnderVoltage(bool uv)
{
	this->underVoltage = uv;
}

bool ChargePointStatusService::getOverTemperature()
{
	return overTemperature;
}

void ChargePointStatusService::setOverTemperature(bool ot)
{
	this->overTemperature = ot;
}

bool ChargePointStatusService::getUnderTemperature()
{
	return underTemperature;
}

void ChargePointStatusService::setUnderTemperature(bool ut)
{
	this->underTemperature = ut;
}

bool ChargePointStatusService::getOverCurrent()
{
	return overCurrent;
}

void ChargePointStatusService::setOverCurrent(bool oc)
{
	this->overCurrent = oc;
}

bool ChargePointStatusService::getUnderCurrent()
{
	return underCurrent;
}

void ChargePointStatusService::setUnderCurrent(bool uc)
{
	this->underCurrent = uc;
}

bool ChargePointStatusService::getEmergencyRelayClose()
{
	return emergencyRelayClose;
}
void ChargePointStatusService::setEmergencyRelayClose(bool erc)
{
	this->emergencyRelayClose = erc;
}

bool ChargePointStatusService::getUnavailabilityStatus()
{
	return unavailable;
}

void ChargePointStatusService::setUnavailabilityStatus(bool val)
{
	this->unavailable = val;
}

void ChargePointStatusService::unauthorize()
{
	if (authorized == false)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]unauthorize\n");
		return;
	}
	if (DEBUG_OUT)
		Serial.print("[ChargePointStatusService]\n");
	this->idTag.clear();
	authorized = false;
}

void ChargePointStatusService::startTransaction(int transId)
{
	if (transactionRunning == true)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]started transaction \n");
	}
	transactionId = transId;
	transactionRunning = true;
}

void ChargePointStatusService::boot()
{
	// TODO Review: Is it necessary to check in inferenceStatus(), if the CP is booted at all? Problably not ...
}

void ChargePointStatusService::stopTransaction()
{
	if (transactionRunning == false)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]stop transaction\n");
	}
	transactionRunning = false;
	transactionId = -1;
	// EEPROM.begin(sizeof(EEPROM_Data));
	// EEPROM.put(68,transactionId);
	// EEPROM.commit();
	// EEPROM.end();
}

int ChargePointStatusService::getTransactionId()
{
	if (transactionId < 0)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]getTransactionId()\n");
	}
	if (transactionRunning == false)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]getTransactionId()\n");
	}
	return transactionId;
}

void ChargePointStatusService::startEvDrawsEnergy()
{
	if (evDrawsEnergy == true)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]startEvDrawsEnergy \n");
	}
	evDrawsEnergy = true;
}

void ChargePointStatusService::stopEvDrawsEnergy()
{
	if (evDrawsEnergy == false)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]stopEvDrawsEnergy\n");
	}
	evDrawsEnergy = false;
}
void ChargePointStatusService::startEnergyOffer()
{
	if (evseOffersEnergy == true)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService]startEnergyOffer\n");
	}
	evseOffersEnergy = true;
}

void ChargePointStatusService::stopEnergyOffer()
{
	if (evseOffersEnergy == false)
	{
		if (DEBUG_OUT)
			Serial.print("[ChargePointStatusService] stopEnergyOffer \n");
	}
	evseOffersEnergy = false;
}

bool ChargePointStatusService::getEarthDisconnect()
{
	return EarthDisconnect;
}

void ChargePointStatusService::setEarthDisconnect(bool ed)
{
	this->EarthDisconnect = ed;
}

/*****Added new Definition @wamique***********/
bool ChargePointStatusService::getEvDrawsEnergy()
{
	return evDrawsEnergy;
}

/*********************************************/

/*
 * @brief : Feature added by G. Raja Sumant
 * 09/07/2022 For Change availability
 */
void ChargePointStatusService::setUnavailable(bool su)
{
	this->unavailable = su;
}

bool ChargePointStatusService::getUnavailable()
{
	return unavailable;
}

/*
 * @brief : Feature added by G. Raja Sumant
 * 19/07/2022 For ReserveNow
 */
void ChargePointStatusService::setReserved(bool re)
{
	this->reserved = re;
}

bool ChargePointStatusService::getReserved()
{
	return reserved;
}
