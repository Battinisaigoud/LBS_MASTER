// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "DataTransfer.h"
#include "OcppEngine.h"
#include "Variants.h"

DataTransfer::DataTransfer(String &msg) {
    this->msg = String(msg);
}
DataTransfer::DataTransfer() {
   
}
const char* DataTransfer::getOcppOperationType(){
    return "DataTransfer";
}

DynamicJsonDocument* DataTransfer::createReq() {
  DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(2) + (msg.length() + 1));
  JsonObject payload = doc->to<JsonObject>();
  payload["vendorId"] = "CustomVendor";
  payload["data"] = msg;
  return doc;
}

void DataTransfer::processConf(JsonObject payload){
    String status = payload["status"] | "Invalid";

    if (status.equals("Accepted")) {
        if (DEBUG_OUT) Serial.print("[DataTransfer] Request has been accepted!\n");
    } else {
        Serial.print("[DataTransfer] Request has been denied!");
    }
}

void DataTransfer::processReq(JsonObject payload){
    vendorId = String(payload["vendorId"].as<String>());
    messageId = String(payload["messageId"].as<String>());
    data = String(payload["data"].as<String>());
    Serial.println(vendorId);
    Serial.println(messageId);
    Serial.println(data);
}

DynamicJsonDocument* DataTransfer::createConf(){
    DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
    JsonObject payload = doc->to<JsonObject>();
    if (this->vendorId.equals("")) {
        payload["status"] = "UnknownVendorId"; 
    }else{
        payload["status"] = "Accepted";
    }
    return doc;
}