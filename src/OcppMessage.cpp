// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "OcppMessage.h"

OcppMessage::OcppMessage(){}

OcppMessage::~OcppMessage(){}
  
const char* OcppMessage::getOcppOperationType(){
    Serial.print("[OcppMessage]  Unsupported operation: getOcppOperationType() is not implemented!\n");
    return "CustomOperation";
}

DynamicJsonDocument* OcppMessage::createReq() {
    Serial.print("[OcppMessage]  Unsupported operation: createReq() is not implemented!\n");
    return new DynamicJsonDocument(0);
}

void OcppMessage::processConf(JsonObject payload) {
    Serial.print("[OcppMessage]  Unsupported operation: processConf() is not implemented!\n");
}

void OcppMessage::processReq(JsonObject payload) {
    Serial.print("[OcppMessage]  Unsupported operation: processReq() is not implemented!\n");
}

DynamicJsonDocument* OcppMessage::createConf() {
    Serial.print("[OcppMessage]  Unsupported operation: createConf() is not implemented!\n");
    return NULL;
}
