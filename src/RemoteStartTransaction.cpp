// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "RemoteStartTransaction.h"
#include "OcppEngine.h"
#include "ChargePointStatusService.h"
#include "ControlPilot.h"

extern bool flag_AuthorizeRemoteTxRequests;

/*
 * @brief: For EVSE_A
 */
extern bool flag_evseReadIdTag_A;
extern bool flag_evseAuthenticate_A;
extern bool flag_evseStartTransaction_A; // Entry condition for starting transaction.
extern bool flag_evRequestsCharge_A;
extern bool flag_evseStopTransaction_A;
extern bool flag_evseUnauthorise_A;
extern bool flag_controlPAuthorise_A;

/*
 * @brief: For EVSE_B
 */
extern bool flag_evseReadIdTag_B;
extern bool flag_evseAuthenticate_B;
extern bool flag_evseStartTransaction_B; // Entry condition for starting transaction.
extern bool flag_evRequestsCharge_B;
extern bool flag_evseStopTransaction_B;
extern bool flag_evseUnauthorise_B;
extern bool reservation_start_flag;
extern String reserve_currentIdTag;

bool A;
bool B;

extern uint8_t gu8_remote_start_flag_A;
extern uint8_t gu8_remote_start_flag_B;

extern EVSE_states_enum EVSE_state;

extern String remote_idTag;

RemoteStartTransaction::RemoteStartTransaction()
{
}

const char *RemoteStartTransaction::getOcppOperationType()
{
	return "RemoteStartTransaction";
}

void RemoteStartTransaction::processReq(JsonObject payload)
{
	idTag = String(payload["idTag"].as<String>());
	connectorId = payload["connectorId"].as<int>();
	Serial.println("Connector ID: " + String(connectorId));
	// Serial.println(String(getChargePointStatusService_A()->inferenceStatus());
    
	// if (getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Available)
	// {
	// 	if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Reserved)
	// 	{
	// 		Serial.println("[RemoteStartTransaction] Trying remote start in reserved state");
	// 	}
	// 	else
	// 	{
	// 		accepted = false;
	// 		return;
	// 	}
	// }

	if (reservation_start_flag)
	{
		Serial.println("REMOTE START 1_RESERVE");
		if (idTag.equals(reserve_currentIdTag) == true)
		{
			accepted = true;
			A = true;
			getChargePointStatusService_A()->authorize(idTag, connectorId);
			remote_idTag = idTag;
			gu8_remote_start_flag_A = 1;
			/*
			 * @brief : Change made by G. Raja Sumant for removing reservation.
			 */
			getChargePointStatusService_A()->setReserved(false);
		}
		else
		{
			accepted = false;
		}
	}
	else
	{

		if (((EVSE_state == STATE_B) && getChargePointStatusService_A()->getEmergencyRelayClose() == false))
		{
			Serial.println("REMOTE START 1");
			// if (connectorId == 1 && getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Preparing)
			if (connectorId == 1 && (evse_ChargePointStatus == Preparing || evse_ChargePointStatus == Finishing))
			{
				getChargePointStatusService_A()->authorize(idTag, connectorId);
				
				remote_idTag = idTag;
				gu8_remote_start_flag_A = 1;
				// Removed the setUnavailabilityStatus as it did not make logical sense.
				// Raja Sumant 13/04/2022
				// getChargePointStatusService_B()->setUnavailabilityStatus(true);
				A = true;
				accepted = true;
				Serial.println("*** going to start 1****");
			}
		}
		#if 0
		else if (((EVSE_state == STATE_B) && getChargePointStatusService_B()->getEmergencyRelayClose() == false))
		{
			if (connectorId == 2 && getChargePointStatusService_B()->inferenceStatus() == ChargePointStatus::Available)
			{
				getChargePointStatusService_B()->authorize(idTag, connectorId);
				remote_idTag = idTag;
				gu8_remote_start_flag_B = 1;
				// Removed the setUnavailabilityStatus as it did not make logical sense.
				// Raja Sumant 13/04/2022
				// getChargePointStatusService_A()->setUnavailabilityStatus(true);
				B = true;
				Serial.println(F("*** going to start 2****"));
			} // else if(connectorId ==3 && getChargePointStatusService_C()->inferenceStatus() == ChargePointStatus::Available){
			// 	getChargePointStatusService_C()->authorize(idTag,connectorId);
			// }
			else
			{
				Serial.println(F("Unable to start txn Connector is busy"));
			}
		}
		#endif
		else
		{
			Serial.println("REMOTE START 1 REJECTED");

			remote_idTag =  ""; 
			gu8_remote_start_flag_A = 0;
			gu8_remote_start_flag_B = 0;
			accepted = 0;
		}
	}
		/*
		if(getChargePointStatusService_A()->transactionId == -1){
			if(getChargePointStatusService_B()->connectorId != connectorId && getChargePointStatusService_C()->connectorId != connectorId){
				getChargePointStatusService_A()->authorize(idTag,connectorId);    //authorizing twice needed to be improvise
			}else{
				Serial.println("[A] Unable to start txn as Connector is busy");
			}
		}else if(getChargePointStatusService_B->transactionId == -1){
			if(getChargePointStatusService_A()->connectorId != connectorId && getChargePointStatusService_C()->connectorId != connectorId){
				getChargePointStatusService_B()->authorize(idTag,connectorId);    //authorizing twice needed to be improvise
			}else{
				Serial.println("[B] Unable to start txn as Connector is busy");
			}
		}else if(getChargePointStatusService_C()->transactionId == -1){
			if(getChargePointStatusService_A()->connectorId != connectorId && getChargePointStatusService_B()->connectorId != connectorId){
				getChargePointStatusService_C()->authorize(idTag,connectorId);    //authorizing twice needed to be improvise
			}else{
				Serial.println("[B] Unable to start txn as Connector is busy");
			}
		}else{
			Serial.println("ALL connectors are busy");
		}*/
		//	getChargePointStatusService()->authorize(idTag, connectorId);
}

	DynamicJsonDocument *RemoteStartTransaction::createConf()
	{
		DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
		JsonObject payload = doc->to<JsonObject>();
		if (accepted)
			payload["status"] = "Accepted";
		else
			payload["status"] = "Rejected";
		if (A)
		{
			A = false;
			Serial.println("**accepted to start A****");
			if (!flag_AuthorizeRemoteTxRequests)
			{
				flag_evseReadIdTag_A = false;
				flag_evseAuthenticate_A = false;
				flag_evseStartTransaction_A = true; // Entry condition for starting transaction.
				flag_controlPAuthorise_A = true;	// remote start requires pwm of cpout to start.
				flag_evRequestsCharge_A = false;
				flag_evseStopTransaction_A = false;
				flag_evseUnauthorise_A = false;
			}
		}
		else if (B)
		{
			B = false;
			Serial.println("**accepted to start B****");
			if (!flag_AuthorizeRemoteTxRequests)
			{
				flag_evseReadIdTag_B = false;
				flag_evseAuthenticate_B = false;
				flag_evseStartTransaction_B = true; // Entry condition for starting transaction.
				flag_evRequestsCharge_B = false;
				flag_evseStopTransaction_B = false;
				flag_evseUnauthorise_B = false;
			}
		}
		else
		{
			payload["status"] = "Rejected";
			Serial.println("[RemoteStartTransaction] Something wrong condition");
		}
		return doc;
	}

	DynamicJsonDocument *RemoteStartTransaction::createReq()
	{
		DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
		JsonObject payload = doc->to<JsonObject>();

		payload["idTag"] = "fefed1d19876";
		payload["connectorId"] = 1;

		return doc;
	}

	void RemoteStartTransaction::processConf(JsonObject payload)
	{
		String status = payload["status"] | "Invalid";

		if (status.equals("Accepted"))
		{
			if (DEBUG_OUT)
				Serial.print("[RemoteStartTransaction] Request has been accepted!\n");
		}
		else
		{
			Serial.print("[RemoteStartTransaction] Request has been denied!");
		}
	}
