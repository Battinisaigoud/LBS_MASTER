// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "StatusNotification.h"

#include <string.h>



#if DWIN_ENABLED
#include "dwin.h"
extern unsigned char fault_suspEV[28];
extern unsigned char fault_suspEVSE[28];
extern unsigned char charging[28];
extern unsigned char preparing[28];
extern unsigned char reserved[28];
extern unsigned char finishing[28];
extern unsigned char avail[28];
extern unsigned char CONN_UNAVAIL[30];
#endif

extern int8_t fault_code_A;

extern bool evse_A_unavail;
extern bool evse_B_unavail;
extern bool evse_C_unavail;

StatusNotification::StatusNotification(ChargePointStatus currentStatus, int connectorId) 
	: currentStatus(currentStatus), connectorId(connectorId) {

	if (!getJsonDateStringFromSystemTime(timestamp, JSONDATE_LENGTH)){
		Serial.print("[StatusNotification] Error reading time string. Expect format like 2020-02-01T20:53:32.486Z\n");
	}

	if (DEBUG_OUT) {
		Serial.print("[StatusNotification] New StatusNotification with timestamp ");
		Serial.print(timestamp);
		Serial.print(". New Status: ");
	}
	
//	EEPROM.begin(sizeof(EEPROM_Data));
	switch (currentStatus) {
		case (ChargePointStatus::Available):
			if (DEBUG_OUT) Serial.print("Available\n");
		//	EEPROM.put(8, "Available");
			break;
		case (ChargePointStatus::Preparing):
			if (DEBUG_OUT) Serial.print("Preparing\n");
		//	EEPROM.put(8, "Preparing");
			break;
		case (ChargePointStatus::Charging):
			if (DEBUG_OUT) Serial.print("Charging\n");
		//	EEPROM.put(8, "Charging");
			break;
		case (ChargePointStatus::SuspendedEVSE):
			if (DEBUG_OUT) Serial.print("SuspendedEVSE\n");
		//	EEPROM.put(8, "SuspendedEVSE");
			break;
		case (ChargePointStatus::SuspendedEV):
			if (DEBUG_OUT) Serial.print("SuspendedEV\n");
		//	EEPROM.put(8, "SuspendedEV");
			break;
		case (ChargePointStatus::Finishing):
			if (DEBUG_OUT) Serial.print("Finishing\n");
		//	EEPROM.put(8, "Finishing");
			break;
		case (ChargePointStatus::Reserved):
			if (DEBUG_OUT) Serial.print("Reserved\n");
		//	EEPROM.put(8, "Reserved");
			break;
		case (ChargePointStatus::Unavailable):
			if (DEBUG_OUT) Serial.print("Unavailable\n");
		//	EEPROM.put(8, "Unavailable");
			break;
		case (ChargePointStatus::Faulted):
			if (DEBUG_OUT) Serial.print("Faulted\n");
		//	EEPROM.put(8, "Faulted");
			break;
		case (ChargePointStatus::NOT_SET):
			Serial.print("*NOT_SET*\n");
			currentStatus = getChargePointStatusService_A()->inferenceStatus();
			//currentStatus = status_notification_inf_status;
			//Serial.print(currentStatus);
			break;
		default:
			Serial.print("[Error, invalid status code]\n");
		break;
	}
//	EEPROM.commit();
//	EEPROM.end();*/
}

const char* StatusNotification::getOcppOperationType(){
	return "StatusNotification";
}

//TODO if the status has changed again when sendReq() is called, abort the operation completely (note: if req is already sent, stick with listening to conf). The ChargePointStatusService will enqueue a new operation itself
DynamicJsonDocument* StatusNotification::createReq() {
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(6) + (JSONDATE_LENGTH + 25 + 1));
	JsonObject payload = doc->to<JsonObject>();

	payload["connectorId"] = connectorId;        //Hardcoded to be one because only one connector is supported
	payload["errorCode"] = "NoError";  //No error diagnostics support

	#if 0
	uint8_t err = 0;
	switch (connectorId)
	{
	case 1:

		switch (currentStatus)
		{
		case (ChargePointStatus::Available):
			evse_A_unavail = false;
			avail[4] = 0X55;
			//avail[4] = 0X66;
			//err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
			break;
		case (ChargePointStatus::Preparing):
			preparing[4] = 0X55;
			//preparing[4] = 0X66;
			err = DWIN_SET(preparing, sizeof(preparing) / sizeof(preparing[0]));
			break;
		case (ChargePointStatus::Charging):
			charging[4] = 0X66;
			//charging[4] = 0X55;
			err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));
			break;
		case (ChargePointStatus::SuspendedEVSE):
			//fault_suspEVSE[4] = 0X55;
			fault_suspEVSE[4] = 0X66;
			err = DWIN_SET(fault_suspEVSE, sizeof(fault_suspEVSE) / sizeof(fault_suspEVSE[0]));
			break;
		case (ChargePointStatus::SuspendedEV):
			//fault_suspEV[4] = 0X55;
			 fault_suspEV[4] = 0X66;
			err = DWIN_SET(fault_suspEV, sizeof(fault_suspEV) / sizeof(fault_suspEV[0]));
			break;
		case (ChargePointStatus::Finishing):
			//finishing[4] = 0X55;
			 finishing[4] = 0X66;
			err = DWIN_SET(finishing, sizeof(finishing) / sizeof(finishing[0]));
			break;
		case (ChargePointStatus::Reserved):
			//reserved[4] = 0X55;
			 reserved[4] = 0X55;
			err = DWIN_SET(reserved, sizeof(reserved) / sizeof(reserved[0]));
			break;
		case (ChargePointStatus::Unavailable):
			evse_A_unavail = true;
			//CONN_UNAVAIL[4] = 0X55;
			 CONN_UNAVAIL[4] = 0X66;
			err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
			break;
		}
		break;
	case 2:
		switch (currentStatus)
		{
		case (ChargePointStatus::Available):
			//evse_B_unavail = false;
			//avail[4] = 0X55;
			 avail[4] = 0X71;
			//err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
			break;
		case (ChargePointStatus::Preparing):
			preparing[4] = 0X55;
			 //preparing[4] = 0X71;
			 //Serial.print(F("Preparinggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg2\n"));
			err = DWIN_SET(preparing, sizeof(preparing) / sizeof(preparing[0]));
			break;
		case (ChargePointStatus::Charging):
			//charging[4] = 0X55;
			 charging[4] = 0X71;
			err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));
			break;
		case (ChargePointStatus::SuspendedEVSE):
			//fault_suspEVSE[4] = 0X55;
			 fault_suspEVSE[4] = 0X71;
			err = DWIN_SET(fault_suspEVSE, sizeof(fault_suspEVSE) / sizeof(fault_suspEVSE[0]));
			break;
		case (ChargePointStatus::SuspendedEV):
			//fault_suspEV[4] = 0X55;
			 fault_suspEV[4] = 0X71;
			err = DWIN_SET(fault_suspEV, sizeof(fault_suspEV) / sizeof(fault_suspEV[0]));
			break;
		case (ChargePointStatus::Finishing):
			//finishing[4] = 0X55;
			 finishing[4] = 0X71;
			err = DWIN_SET(finishing, sizeof(finishing) / sizeof(finishing[0]));
			break;
		case (ChargePointStatus::Reserved):
			//reserved[4] = 0X55;
			 reserved[4] = 0X71;
			err = DWIN_SET(reserved, sizeof(reserved) / sizeof(reserved[0]));
			break;
		case (ChargePointStatus::Unavailable):
			//evse_B_unavail = true;
			//CONN_UNAVAIL[4] = 0X55;
			 CONN_UNAVAIL[4] = 0X71;
			err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
			break;
		}
		break;
	case 3:
		switch (currentStatus)
		{
		case (ChargePointStatus::Available):
			//evse_C_unavail = false;
			avail[4] = 0X7B;
			//err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
			break;
		case (ChargePointStatus::Preparing):
			//preparing[4] = 0X7B;
			preparing[4] = 0X55;
			//Serial.print(F("Preparingggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg3\n"));
			err = DWIN_SET(preparing, sizeof(preparing) / sizeof(preparing[0]));
			break;
		case (ChargePointStatus::Charging):
			charging[4] = 0X7B;
			err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));
			break;
		case (ChargePointStatus::SuspendedEVSE):
			fault_suspEVSE[4] = 0X7B;
			err = DWIN_SET(fault_suspEVSE, sizeof(fault_suspEVSE) / sizeof(fault_suspEVSE[0]));
			break;
		case (ChargePointStatus::SuspendedEV):
			fault_suspEV[4] = 0X7B;
			err = DWIN_SET(fault_suspEV, sizeof(fault_suspEV) / sizeof(fault_suspEV[0]));
			break;
		case (ChargePointStatus::Finishing):
			finishing[4] = 0X7B;
			err = DWIN_SET(finishing, sizeof(finishing) / sizeof(finishing[0]));
			break;
		case (ChargePointStatus::Reserved):
			reserved[4] = 0X7B;
			err = DWIN_SET(reserved, sizeof(reserved) / sizeof(reserved[0]));
			break;
		case (ChargePointStatus::Unavailable):
			//evse_C_unavail = true;
			CONN_UNAVAIL[4] = 0X7B;
			err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
			break;
		}
		break;
	}
#endif


	/*
		* @brief: Bug  15/04/2022
		  Using a new variable inferStatus_A instead of currentStatus as it is buggy.
		* We need the inferenced status that is continuosly being updated in ChargePointService 
		*/

	ChargePointStatus inferStatus;
	switch(connectorId)
	{
		case 1:
		inferStatus = getChargePointStatusService_A()->inferenceStatus();
		break;
		//case 2:
		//inferStatus = getChargePointStatusService_B()->inferenceStatus();
		//break;
		default:
		Serial.println(F("[StatusNotification]***Wrong connector id passed for status notification***"));
		break;

	}
	

	//Part of Offline Implementation

	//EEPROM.begin(sizeof(EEPROM_Data));
	
	payload["errorCode"] = "NoError";
	switch (inferStatus) {
		
		case (ChargePointStatus::Available):
		payload["status"] = "Available";
	//	EEPROM.put(8, "Available");
	break;
		case (ChargePointStatus::Preparing):
		//Serial.print(F("Preparingggggggggggggggggggggggggggg4\n"));
		payload["status"] = "Preparing";
			//	EEPROM.put(8, "Preparing");
	break;
		case (ChargePointStatus::Charging):
		payload["status"] = "Charging";
	//	EEPROM.put(8, "Charging");
	break;
		case (ChargePointStatus::SuspendedEVSE):
		payload["status"] = "SuspendedEVSE";
	//	EEPROM.put(8, "SuspendedEVSE");
	break;
		case (ChargePointStatus::SuspendedEV):
		payload["status"] = "SuspendedEV";
	//	EEPROM.put(8, "SuspendedEV");
	break;
		case (ChargePointStatus::Finishing):
		payload["status"] = "Finishing";
	//	EEPROM.put(8, "Finishing");
	break;
		case (ChargePointStatus::Reserved):
		payload["status"] = "Reserved";
	//	EEPROM.put(8, "Reserved");
	break;
		case (ChargePointStatus::Unavailable):
		payload["status"] = "Unavailable";
	//	EEPROM.put(8, "Unavailable");
	break;
		case (ChargePointStatus::Faulted):
		payload["status"] = "Faulted";
		switch(fault_code_A)
		{
			case 0: payload["errorCode"] = "OverVoltage";
			        break;
			case 1: payload["errorCode"] = "UnderVoltage";
			        break;
			case 2: payload["errorCode"] = "OverCurrentFailure";
			        break;
			case 3: payload["errorCode"] = "UnderCurrent";
			        break;
			case 4: payload["errorCode"] = "HighTemperature";
			        break;
			case 5: payload["errorCode"] = "UnderTemperature";
			        break;
			case 6: payload["errorCode"] = "GroundFailure";
			        break;
			case 7: 
					payload["errorCode"] = "OtherError";
					payload["vendorId"] = "EVRE";
					payload["vendorErrorCode"] = "EARTH DISCONNECT";
			        break;
			case 8 :payload["errorCode"] = "OtherError";
					payload["vendorId"] = "EVRE";
					payload["vendorErrorCode"] = "EmergencyStop";
			        break;
			case 9:
				payload["errorCode"] = "OtherError";
				payload["vendorId"] = "EVRE";
				payload["vendorErrorCode"] = "PowerLoss";
			default:
				Serial.printf("[StatusNotification] Error code of A is %d",fault_code_A);
				payload["errorCode"] = "OtherError"; // No error diagnostics support
				break;
		}

		fault_code_A = -1; // clearing the fault code so that the next status notification will not be effected.
	//	EEPROM.put(8, "Faulted");
	break;
	default:
		//currentStatus = status_notification_inf_status;
		currentStatus = getChargePointStatusService_A()->inferenceStatus();
		//payload["status"] = "NOT_SET";
		switch (currentStatus) {
		case (ChargePointStatus::Available):
		payload["status"] = "Available";
	//	EEPROM.put(8, "Available");
	break;
		case (ChargePointStatus::Preparing):
		//Serial.print(F("Preparinggggggggggggggggggggg5\n"));
		payload["status"] = "Preparing";
	//	EEPROM.put(8, "Preparing");
	break;
		case (ChargePointStatus::Charging):
		payload["status"] = "Charging";
	//	EEPROM.put(8, "Charging");
	break;
		case (ChargePointStatus::SuspendedEVSE):
		payload["status"] = "SuspendedEVSE";
	//	EEPROM.put(8, "SuspendedEVSE");
	break;
		case (ChargePointStatus::SuspendedEV):
		payload["status"] = "SuspendedEV";
	//	EEPROM.put(8, "SuspendedEV");
	break;
		case (ChargePointStatus::Finishing):
		payload["status"] = "Finishing";
	//	EEPROM.put(8, "Finishing");
	break;
		case (ChargePointStatus::Reserved):
		payload["status"] = "Reserved";
	//	EEPROM.put(8, "Reserved");
	break;
		case (ChargePointStatus::Unavailable):
		payload["status"] = "Unavailable";
	//	EEPROM.put(8, "Unavailable");
	break;
		case (ChargePointStatus::Faulted):
		payload["status"] = "Faulted";
		switch(fault_code_A)
		{
			case 0: payload["errorCode"] = "OverVoltage";
			        break;
			case 1: payload["errorCode"] = "UnderVoltage";
			        break;
			case 2: payload["errorCode"] = "OverCurrentFailure";
			        break;
			case 3: payload["errorCode"] = "UnderCurrent";
			        break;
			case 4: payload["errorCode"] = "HighTemperature";
			        break;
			case 5: payload["errorCode"] = "UnderTemperature";
			        break;
			case 6: payload["errorCode"] = "GroundFailure";
			        break;
			case 7: 
					payload["errorCode"] = "OtherError";
					payload["vendorId"] = "EVRE";
					payload["vendorErrorCode"] = "EARTH DISCONNECT";
			        break;
			case 8 :payload["errorCode"] = "OtherError";
					payload["vendorId"] = "EVRE";
					payload["vendorErrorCode"] = "EmergencyStop";
			        break;
			default : payload["errorCode"] = "NoError"; //No error diagnostics support
			        break;
		}

		fault_code_A = -1; // clearing the fault code so that the next status notification will not be effected.

	//	EEPROM.put(8, "Faulted");
	break;
		default: 
		payload["status"] = "Unavailable";
		Serial.print("[StatusNotification] Error: Sending status is still NOT_SET!\n");
		break;
	}
		break;
	}
//	EEPROM.commit();
//	EEPROM.end();
if (!getJsonDateStringFromSystemTime(timestamp, JSONDATE_LENGTH)){
		Serial.print("[StatusNotification] Error reading time string. Expect format like 2020-02-01T20:53:32.486Z\n");
	}
	payload["timestamp"] = timestamp;

	return doc;
}


void StatusNotification::processConf(JsonObject payload) {
	/*
	* Empty payload
	*/
}


/* 
* For debugging only
*/
StatusNotification::StatusNotification() {

}

/*
* For debugging only
*/
void StatusNotification::processReq(JsonObject payload) {

}

/*
* For debugging only
*/
DynamicJsonDocument* StatusNotification::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(0);
	JsonObject payload = doc->to<JsonObject>();
	return doc;
}
