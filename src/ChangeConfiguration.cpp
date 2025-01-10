#include "ChangeConfiguration.h"
#include "OcppEngine.h"
#include<Preferences.h>

extern unsigned int meterSampleInterval;
extern unsigned int heartbeatInterval;
bool flag_AuthorizeRemoteTxRequests;

extern Preferences change_config;

ChangeConfiguration::ChangeConfiguration()
{
}

const char *ChangeConfiguration::getOcppOperationType()
{
	return "ChangeConfiguration";
}

/*
 * @brief : ChangeConfiguration implemented by
 * G. Raja Sumant 08/07/2022
 * It can either accept , reject or say not supported.
 */

void ChangeConfiguration::processReq(JsonObject payload)
{
	String key = String(payload["key"].as<String>());
	unsigned int value = 0;

	if (key.equals("MeterValueSampleInterval"))
	{
		value = String(payload["value"].as<String>()).toInt();
		if (value == 0)
		{
			// Serial.println("Setting MeterValueSampleInterval to Max");
			// meterSampleInterval = 43200;
			// acceptance = true;
			Serial.println("[ChangeConfig] Rejecting the change as invalid value has been received!");
			acceptance = false;
			rejected = true;
		}
		else
		{
			Serial.println("[ChangeConfig] Setting MeterValueSampleInterval: " + String(meterSampleInterval));
			meterSampleInterval = value;
			acceptance = true;
			rejected = false;
			change_config.begin("configurations", false);
			change_config.putInt("meterSampleInterval", value);
			change_config.end();
		}
	}
	else if (key.equals("HeartbeatInterval"))
	{
		value = String(payload["value"].as<String>()).toInt();
		if (value == 0)
		{
			// Serial.println("Setting MeterValueSampleInterval to Max");
			// meterSampleInterval = 43200;
			// acceptance = true;
			Serial.println("[ChangeConfig] Rejecting the change as invalid value has been received!");
			acceptance = false;
			rejected = true;
		}
		else
		{
			Serial.println("[ChangeConfig] Setting heartbeatInterval: " + String(value));
			heartbeatInterval = value;
			acceptance = true;
			rejected = false;
			change_config.begin("configurations", false);
			change_config.putInt("heartbeatInterval", value);
			change_config.end();
		}
	}
	else if (key.equals("AuthorizeRemoteTxRequests"))
	{
		//value = String(payload["value"].as<String>()).toInt();
		String val = payload["value"];
		/*if (val == "true")
		{
			// Serial.println("Setting MeterValueSampleInterval to Max");
			// meterSampleInterval = 43200;
			// acceptance = true;
			Serial.println(F("[ChangeConfig] Rejecting the change as invalid value has been received!"));
			acceptance = false;
			rejected = true;
		}
		else
		{
			Serial.println("[ChangeConfig] Setting auth remote start: " + String(val));
			acceptance = true;
			rejected = false;
			change_config.begin("configurations", false);
			change_config.putBool("authRemoteStart", val);
			change_config.end();
		}*/
		if(val=="true" || val=="True" || val=="TRUE")
		{
			Serial.println("[ChangeConfiguration] AuthorizeRemoteTxRequests set to true");
			acceptance = true;
			rejected = false;
			change_config.begin("configurations", false);
			change_config.putBool("authRemoteStart", true);
			change_config.end();
			flag_AuthorizeRemoteTxRequests = true;
		}
		else if(val=="false" || val=="False" || val=="FALSE")
		{
			Serial.println("[ChangeConfiguration] AuthorizeRemoteTxRequests set to false");
			acceptance = true;
			rejected = false;
			change_config.begin("configurations", false);
			change_config.putBool("authRemoteStart", false);
			change_config.end();
			flag_AuthorizeRemoteTxRequests = false;
		}
		else
		{
			Serial.println("[ChangeConfiguration] AuthorizeRemoteTxRequests Rejected the change config!");
			Serial.println(String(payload["value"].as<String>()));
			acceptance = false;
			rejected = true;
		}
	}
	else
	{
		// to give scope for not supported.
		rejected = false;
		acceptance = false;
	}
}

DynamicJsonDocument *ChangeConfiguration::createConf()
{

	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();
	if (acceptance)
	{
		payload["status"] = "Accepted";
	}
	else
	{
		if (rejected)
			payload["status"] = "Rejected";
		else
			payload["status"] = "NotSupported";
	}
	acceptance = false;
	return doc;
}

DynamicJsonDocument *ChangeConfiguration::createReq()
{

	/****Can be implemented for testing purpose****/
}

void ChangeConfiguration::processConf(JsonObject payload)
{

	/****Can be implemented for testing purpose****/
}