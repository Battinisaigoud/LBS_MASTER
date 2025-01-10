// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License



#include "UnlockConnector.h"
#include "OcppEngine.h"


UnlockConnector::UnlockConnector() {

}

const char* UnlockConnector::getOcppOperationType(){
	return "UnlockConnector";
}

void UnlockConnector::processReq(JsonObject payload) {
	
	connectorId = payload["connectorId"].as<int>();
	if(connectorId!=1)
    {
        accepted = false;
    }
    else
    {
        accepted = true;
    }
}

DynamicJsonDocument* UnlockConnector::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();
    if(accepted)
	payload["status"] = "NotSupported";
    else
    payload["status"] = "NotSupported";
	
	return doc;
}

DynamicJsonDocument* UnlockConnector::createReq() {
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();

	payload["idTag"] = "fefed1d19876";
	payload["connectorId"] = 1;

	return doc;
}

void UnlockConnector::processConf(JsonObject payload){
	String status = payload["status"] | "Invalid";

	if (status.equals("Accepted")) {
		if (DEBUG_OUT) Serial.print(F("[UnlockConnector] Request has been accepted!\n"));
		} else {
			Serial.print(F("[UnlockConnector] Request has been denied!"));
	}
}
