/*
 * FOTA_OCPP.cpp
 * 
 * Copyright 2022 raja <raja@raja-IdeaPad-Gaming-3-15IMH05>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include "Variants.h"

#include "FOTA_OCPP.h"
#include "OcppEngine.h"

/*
 * @breif: Instantiate an object FotaNotification
 */ 
Preferences evse_preferences_fota;
bool fota_available = false;
String uri;
FotaNotification::FotaNotification() {

}

/*
 * @breif: Method - getOcppOperationType => This method gives the type of Ocpp operation
 */ 

const char* FotaNotification::getOcppOperationType(){
	return "FotaNotification";
}

/*
 * @breif: Method - createReq => This method creates Fota request to be sent to the OCPP server
 * Field Name   | Field Type         | Card. | Description
 * location     | anyURI(char *)     | 1..1  | Required. This contains a string containing a URI pointing to a location from which to retrieve the firmware.
 * retries      | integer(uint_8t)   | 0..1  | Optional. This specifies how many times Charge Point must try to download the firmware before giving up. If this field is not present, it i left to Charge Point to decide how many time it wants to retry.
 * retrieveDate | dateTime(struct dt)| 1..1  | Required. This contains the date and time after which the Charge Point must retrieve the (new) firmware.
   getJsonDateStringFromSystemTime(currentTime, JSONDATE_LENGTH);
 * 
 * retryInterval| integer(uint_8t)   | 0..1  | Optional. The interval in seconds after which a retry may be attempted. If this field is not present, it is left to Charge Point to decide how long to wait between attempts.
 * customData OCPP2.0 | DataTransfer message OCPP1.6 https://www.youtube.com/watch?v=WKt5DI6qTbk
 * [User-Agent] => ESP32-OCPP-Update
[x-ESP32-STA-MAC] => 18:FE:AA:AA:AA:AA
[x-ESP32-AP-MAC] => 1A:FE:AA:AA:AA:AA
[x-ESP32-free-space] => 671744
[x-ESP32-sketch-size] => 373940
[x-ESP32-sketch-md5] => a56f8ef78a0bebd812f62067daf1408a
[x-ESP32-chip-size] => 4194304
[x-ESP32-sdk-version] => 1.3.0
[x-ESP32-version] => DOOR-7-g14f53a19
[x-ESP32-mode] => sketch
* Ex: "createReq": {
"properties": {
"customData": {
"$ref": "#/definitions/CustomDataType"
},
"location": {
"type": "char *"
},
"retries": {
"type": "number"
},
"retrieveDate": {
"type": "dateTime"
},
"phaseToUse": {
"type": "integer"
}
},
"required": [
"location",
"retrieveDate"
]
}
Modified bootnotification : 
[2,"531531531","BootNotification",{"chargePointVendor":"Agrawal","chargePointSerialNumber":"dummyLocal2","chargePointModel":"Pulkit"},"User-Agent":"ESP32-OCPP-Update","x-ESP32-STA-MAC":"18:FE:AA:AA:AA:AA","x-ESP32-AP-MAC":"1A:FE:AA:AA:AA:AA","x-ESP32-free-space":"671744","x-ESP32-sketch-size":"373940","x-ESP32-sketch-md5":"a56f8ef78a0bebd812f62067daf1408a","x-ESP32-chip-size":"4194304","x-ESP32-sdk-version":"1.3.0","x-ESP32-version":"POD_V2_4","x-ESP32-mode","sketch"]
 */ 

void FotaNotification::processReq(JsonObject payload) {

	// For now the object size is 2, but with custom data, it will increase
	//DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(3) + strlen(EVSE_getChargePointVendor()) + 1 + cpSerial.length() + 1 + strlen(EVSE_getChargePointModel()) + 1);
	/*int url_length = 0;
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(2) + (JSONDATE_LENGTH + 1) + url_length);
	JsonObject payload = doc->to<JsonObject>();
	payload["location"] = "https://url";
	char currentTime[JSONDATE_LENGTH + 1] = {'\0'};
	getJsonDateStringFromSystemTime(currentTime, JSONDATE_LENGTH);
	payload["retrieveDate"] = currentTime;*/
	//Need nested json for customData here -

	const char *uri = payload["location"] | "Invalid";
	uint8_t retries = payload["retries"];
	const char *date = payload["retrieveDate"];

	String fota_str = uri; 

	 evse_preferences_fota.begin("fota_url", false);
     evse_preferences_fota.putString("fota_uri", uri);
     evse_preferences_fota.putUInt("fota_retries", retries);
     evse_preferences_fota.putString("fota_date", date);
     evse_preferences_fota.putBool("fota_avial", true);
     evse_preferences_fota.end();

	Serial.println("[FOTA_OCPP] got the uri to update FOTA");
	Serial.println(uri);

	/*if (!strcmp(type, "Hard")){
		Serial.print(F("[Reset] Warning: received request to perform hard reset, but this implementation is only capable of soft reset!\n"));
		//Hard_Reset(); To be implemented
		//delay(5000);
		//ESP.restart();
		reasonForStop = 2;
		softReset();
	} 
	else if (!strcmp(type, "Soft")){
		if(DEBUG_OUT) Serial.println(F("Soft Reset is Requested"));
		reasonForStop = 8;
		softReset();
	}*/
	
}

void FotaNotification::processConf(JsonObject payload){
	/*
	 * OTA update should be processed.
	 */ 
}

DynamicJsonDocument*  FotaNotification::createReq(JsonObject payload){
	/*
	* Ignore Contents of this Req-message, because this is for debug purposes only
	*/
}

#if 1
/*
* @breif: Added by G. Raja Sumant on the lines of heartbeat which has
* No fields are defined.
*/
DynamicJsonDocument* FotaNotification::createConf(){
	//As per OCPP 1.6 no conf is being given similar to Heartbeat
	DynamicJsonDocument *doc = new DynamicJsonDocument(0);
  JsonObject payload = doc->to<JsonObject>();
  	fota_available = true;

  /*
   * Empty payload
   */
  return doc;
}
#endif

#if 0
void FotaNotification::createConf(JsonObject payload){
	/*
	 * OTA update should be processed.
	 */ 
	
}
#endif 