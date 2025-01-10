// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef REMOTESTOPTRANSACTION_H
#define REMOTESTOPTRANSACTION_H

#include "Variants.h"

#include "OcppMessage.h"
#include "ChargePointStatusService.h"

class RemoteStopTransaction : public OcppMessage {
private:
	int transactionId;

	bool accepted = false;
public:
	RemoteStopTransaction();

	const char* getOcppOperationType();

    #if 0
	DynamicJsonDocument* createReq();

	void processConf(JsonObject payload);
    #endif

	void processReq(JsonObject payload);

	DynamicJsonDocument* createConf();
};

#endif
