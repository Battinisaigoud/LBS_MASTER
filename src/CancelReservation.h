// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef CANCELRESERVE_H
#define CANCELRESERVE_H

#include "Variants.h"

#include "OcppMessage.h"
#include "ChargePointStatusService.h"



enum class CancelReservationstatus {
	Accepted,
	Rejected,
	NOT_SET 	  //not part of OCPP 1.6
};

class CancelReservation : public OcppMessage {
private:
	String idTag;
	int connectorId = 1; 
	CancelReservationstatus currentStatus = CancelReservationstatus::NOT_SET;

public:
	CancelReservation();

	const char* getOcppOperationType();

	DynamicJsonDocument* createReq();

	void processConf(JsonObject payload);

	void processReq(JsonObject payload);

	DynamicJsonDocument* createConf();

	CancelReservationstatus getCancelReservation();

	void setCancelReservation(CancelReservationstatus ReserveStatus);

};



#endif
