// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "StartTransaction.h"
#include "TimeHelper.h"
#include "OcppEngine.h"
#include "MeteringService.h"
#include "Master.h"
extern uint8_t reasonForStop;
unsigned long st_timeA = 0;
float globalmeterstartA = 0;
bool flag_start_accepted = false;
int globalmeterstartB = 0;
unsigned long st_timeB = 0;
int globalmeterstartC = 0;
bool disp_once_A = false;

StartTransaction::StartTransaction() {
	/*if (getChargePointStatusService() != NULL) {
		if (!getChargePointStatusService()->getIdTag().isEmpty()) {
			idTag = String(getChargePointStatusService()->getIdTag());
			connectorId = getChargePointStatusService()->getConnectorId();
		}
	}*/
	//if (idTag.isEmpty()) idTag = String("wrongIDTag"); //Use a default payload. In the typical use case of this library, you probably you don't even need Authorization at all
}

StartTransaction::StartTransaction(String &idTag) {
	this->idTag = String(idTag);
}

StartTransaction::StartTransaction(String &idTag, int &connectorId) {
	this->idTag = String(idTag);
	this->connectorId = connectorId;
}


const char* StartTransaction::getOcppOperationType(){
	return "StartTransaction";
}

DynamicJsonDocument* StartTransaction::createReq() {
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(5) + (JSONDATE_LENGTH + 1) + (idTag.length() + 1));
	JsonObject payload = doc->to<JsonObject>();

	payload["connectorId"] = connectorId;
	MeteringService* meteringService = getMeteringService();
	if (meteringService != NULL) {
		if(connectorId == 1){
			payload["meterStart"] = int(meteringService->currentEnergy_A()); // Considering it as int instead of float as per OCPP
		globalmeterstartA = payload["meterStart"];
		st_timeA = millis();
		disp_once_A = true;
		}else if(connectorId ==2){
		//	payload["meterStart"] = meteringService->currentEnergy_B();
			payload["meterStart"] = int(meteringService->currentEnergy_A()); // Considering it as int instead of float as per OCPP
		}else if(connectorId == 3){
		//	payload["meterStart"] = meteringService->currentEnergy_C();
		}
	}
	char timestamp[JSONDATE_LENGTH + 1] = {'\0'};
	getJsonDateStringFromGivenTime(timestamp, JSONDATE_LENGTH + 1, now());
	payload["timestamp"] = timestamp;
	payload["idTag"] = idTag;

	return doc;
}

void StartTransaction::processConf(JsonObject payload) {

	const char* idTagInfoStatus = payload["idTagInfo"]["status"] | "Invalid";
	int transactionId = payload["transactionId"] | -1;
	//EEPROM.begin(sizeof(EEPROM_Data));
	//EEPROM.put(68, transactionId);
	//EEPROM.commit();
	//EEPROM.end();

	if (!strcmp(idTagInfoStatus, "Accepted") || !strcmp(idTagInfoStatus, "ConcurrentTx") ) {
		if (DEBUG_OUT) Serial.print("[StartTransaction] Request has been accepted!\n");
		reasonForStop = 3;
		flag_start_accepted = true;
		requestforCP_OUT(START);
		//delay(10);
		
		//requestForRelay(START,1);
		//delay(100);

		// ChargePointStatusService *cpStatusService = getChargePointStatusService();
		// if (cpStatusService != NULL){
		// 	cpStatusService->startTransaction(transactionId);
		// 	cpStatusService->startEnergyOffer();
		// }

		SmartChargingService *scService = getSmartChargingService();
		if (scService != NULL){
			scService->beginChargingNow();
		}

	} else {
		Serial.print("[StartTransaction] Request has been denied!\n");
	}
}


void StartTransaction::processReq(JsonObject payload) {

	/**
	* Ignore Contents of this Req-message, because this is for debug purposes only
	*/

}

DynamicJsonDocument* StartTransaction::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2));
	JsonObject payload = doc->to<JsonObject>();

	JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
	idTagInfo["status"] = "Accepted";
	payload["transactionId"] = 123456; //sample data for debug purpose

	return doc;
}
