//Developed by Wamique, EVRE

#ifndef CHANGECONFIGURATION_H
#define CHANGECONFIGURATION_H

#include "OcppMessage.h"

class ChangeConfiguration : public OcppMessage {
private:
    bool acceptance;
    bool rejected; //added by G. Raja Sumant 08/07/2022 to reject invalid values.
public:
  ChangeConfiguration();

  const char* getOcppOperationType();

  DynamicJsonDocument* createReq();

  void processConf(JsonObject payload);
  
  DynamicJsonDocument* createConf();
  
  void processReq(JsonObject payload);

};

#endif