#include "bluetoothConfig.h"



BluetoothSerial SerialBT;
//********Websocket Data********//
const char* ws_url_prefix = "";
const char* chargepoint = "";

// const char* host = "";
// int port;
// const char* protocol = "";

bool wifi = false;
bool gsm = false;
bool offline  = false;
bool ethernet = false;
const char* ssid = "";
const char* key  = "";

// bool wifiEnabled = false;
// bool gsmEnabled = false;

String message = "";

char* adminBT = "ESP_BT";
char* psswdBT = "54321";

char* fetchCmd = "Settings";

const char* ip_addr = "";
const char* dns = "";

const char* gateway = "";
const char* subnet = "";
const char* mac_addr = "";


char incomingChar;

StaticJsonDocument<1000> docBT;

const char* admin = "";
const char* psswd = "";


const char* fetchMsg = "";
// const char* ssid = "";
// const char* key = "";

 bool verified = false;
 bool otaenable = false;
extern uint32_t ble_timerDelay;

/*
void wifiConnectWithStoredCredential(){
  
  String esid="";
  String epass="";
  int counterBT = 0;
  Serial.println("Trying to connect with wifi with stored credential");
  SerialBT.println("Trying to connect with wifi with stored credential");
  
  delay(1000);
  EEPROM.begin(512); //Initialasing EEPROM

  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);

  SerialBT.print("SSID: ");
  SerialBT.println(esid);
  
  Serial.println("Reading EEPROM pass");
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass.c_str());

  SerialBT.println("PASS:*******");
  
  WiFi.begin(esid.c_str(), epass.c_str());
  delay(1000);
  SerialBT.println("Waiting for connection");
  while(WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
  
    Serial.print('.');
    if(counterBT++ > 60){
      Serial.println("Invalid WiFi Credential");
      SerialBT.println("\nInvalid WiFi Credential, Bluetooth endpoint is opened"); 
      WiFi.disconnect();
    //  WiFi.end();
      break;    
    }
  }
    if(WiFi.status() == WL_CONNECTED){
        Serial.print('\n');
        Serial.println("Connection established!");  
        Serial.print("IP address:\t");
        Serial.println(WiFi.localIP()); 

        SerialBT.print('\n');
        SerialBT.println("Connection established!");  
        SerialBT.print("IP address:\t");
        SerialBT.println(WiFi.localIP());       
    }  
}
*/

/*
preferences structure in EEPROM

credentials{
  "ws_url_prefix" : ""
  "chargepoint"   : ""
  "host"          : 
  "port"          : ""
  "protocol"      : ""
  "wifiEnabled"   : ""
  "gsmEnabled"    : ""
}
*/


Preferences preferences;

void eepromStoreCred(){

  preferences.begin("credentials", false);    //creating namespace "credentials" in EEPROM, false --> R/W mode
  preferences.clear();

  // preferences.remove("ssid");
  // preferences.remove("key");
  // preferences.remove("ws_url_prefix");
  // preferences.remove("chargepoint");
  // preferences.remove("host");
  // preferences.remove("port");
  // preferences.remove("protocol");

  // delay(1000);

  preferences.putString("ssid", ssid);
  preferences.putString("key", key);
  preferences.putString("ws_url_prefix",ws_url_prefix);
  preferences.putString("chargepoint",chargepoint);
  // preferences.putString("host",host);
  // preferences.putInt("port",port);
  // preferences.putString("protocol",protocol);
  preferences.putBool("wifi",wifi);
  preferences.putBool("gsm",gsm);
  preferences.putBool("ethernet",ethernet);
  preferences.putBool("offline",offline);	

  preferences.putString("ip_addr",ip_addr);
  preferences.putString("gateway",gateway);
  preferences.putString("dns",dns);
  preferences.putString("subnet",subnet);
  preferences.putString("mac_addr",mac_addr);					


  Serial.println("***************EEPROM******************");
  Serial.println(String(preferences.getString("ssid", "")));
  Serial.println(String(preferences.getString("key", "")));            
  Serial.println(String(preferences.getString("ws_url_prefix","")));
  Serial.println(String(preferences.getString("chargepoint","")));
  Serial.println(String(preferences.getString("host","")));
  Serial.println(String(preferences.getInt("port",0)));
  Serial.println(String(preferences.getString("protocol","")));
  Serial.println(String(preferences.getString("ip_addr","192.168.0.71")));
  Serial.println(String(preferences.getString("gateway","")));
  Serial.println(String(preferences.getString("dns","")));
  Serial.println(String(preferences.getString("subnet","")));
  Serial.println(String(preferences.getString("mac_addr","")));

  Serial.println("WiFi: "+    String(preferences.getBool("wifi", 0)));
  Serial.println("GSM: "+     String(preferences.getBool("gsm", 0)));
  Serial.println("OFFLINE: "+ String(preferences.getBool("offline", 0)));
  delay(1000);

 // preferences.putString("wifiEnabled",wifiEnabled);
 // preferences.putString("gsmEnabled",gsmEnabled);

  Serial.println(F("Received Credential:"));
  Serial.println(ws_url_prefix);
  Serial.println(chargepoint);
  // Serial.println(host);
  // Serial.println(port);
  // Serial.println(protocol);
  Serial.println("wifi: "+String(wifi));
  Serial.println("gsm: "+String(gsm));
  Serial.println("ethernet: "+String(ethernet));
  Serial.println("Stored SSID: "+String(ssid));
  Serial.println("Stored Key: "+ String(key));

  delay(2000);

  SerialBT.println(F("Credential:"));
  SerialBT.println(ws_url_prefix);
  SerialBT.println(chargepoint);
  // SerialBT.println(host);
  // SerialBT.println(port);
  // SerialBT.println(protocol);
  SerialBT.println(ssid);
  SerialBT.println(key);
  SerialBT.println("wifi: "+String(wifi));
  SerialBT.println("gsm: "+String(gsm));
  SerialBT.println("Offline: "+ String(offline));	

  SerialBT.print("IP address:");
  SerialBT.println(String(preferences.getString("ip_addr","192.168.0.50")));
  SerialBT.print("Gateway:");
  SerialBT.println(String(preferences.getString("gateway","192.168.0.1")));
  SerialBT.print("DNS:");
  SerialBT.println(String(preferences.getString("dns","192.168.0.1")));
  SerialBT.print("Subnet:");
  SerialBT.println(String(preferences.getString("subnet","255.255.255.0")));
  SerialBT.print("MAC address:");
  SerialBT.println(String(preferences.getString("mac_addr","DE:AD:BE:EF:FE:ED")));	
  												
  SerialBT.println("Restarting Device");

  Serial.println("Credentials Saved using Preferences");
  SerialBT.println("Credentials Saved using Preferences");

  preferences.putBool("otaenable", otaenable);


  preferences.end();
  delay(2000);
  Serial.println(F("Restarting ESP"));
  ESP.restart();
  //String ssid_l = String(ssid).c_str();
  //String key_l = String(key).c_str();
}


bool VerifyCred(String message){

   bool status = false;  
   SerialBT.println("Verifying Credential");
   Serial.println(F("Verifying Credential"));
   DeserializationError error = deserializeJson(docBT, message);
   if (error) {
    Serial.println(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    SerialBT.println(error.f_str());
    delay(1000);
    return status;
  }

  admin = docBT["admin"];
  psswd = docBT["psswd"];

  ws_url_prefix = docBT["ws_url"];
  chargepoint = docBT["cp"];
  // host = docBT["host"];
  // port = docBT["port"];
  // protocol = docBT["protocol"];

  wifi = docBT["wifi"];
  gsm  = docBT["gsm"];
  ethernet  = docBT["ethernet"];
  
  ssid = docBT["ssid"];
  key = docBT["key"];
  offline           = docBT["offline"];

  ip_addr = docBT["ip_addr"];
  dns = docBT["dns"];
  gateway = docBT["gateway"];
  subnet = docBT["subnet"];
  mac_addr = docBT["mac_addr"];

  fetchMsg = docBT["fetch"];

otaenable = docBT["otaenable"];


  //wifiEnabled = docBT["wifi"];
  //gsmEnabled = docBT["gsm"];
  Serial.println(String("SSID + PASSWORD: ")+String(ssid) + String(key));
  delay(1000);
  /*
  if((strcmp(adminBT,admin)==0) && (strcmp(psswdBT,psswd)==0) && strcmp(fetchCmd,fetchMsg)==0)
  {
    preferences.begin("credentials", false);
    SerialBT.print("IP address:");
     SerialBT.println(String(preferences.getString("ip_addr","192.168.0.50")));
     SerialBT.print("Gateway:");
  SerialBT.println(String(preferences.getString("gateway","192.168.0.1")));
  SerialBT.print("DNS:");
  SerialBT.println(String(preferences.getString("dns","192.168.0.1")));
  SerialBT.print("Subnet:");
  SerialBT.println(String(preferences.getString("subnet","255.255.255.0")));
  SerialBT.print("MAC address:");
  SerialBT.println(String(preferences.getString("mac_addr","DE:AD:BE:EF:FE:ED")));
     preferences.end();

     return false;
  } */
  
  if((strcmp(adminBT,admin)==0) && (strcmp(psswdBT,psswd)==0)){
      Serial.println("\nLogin success");
      SerialBT.println("Login successfully....storing data in EEPROM");
      status = true;
      SerialBT.println("Wait...device will reboot");
    
      return status;
  } 

return false;

}


void startingBTConfig(){ //

  //if(!SerialBT.begin("ESP_AC001")){
  if(!SerialBT.begin("ESP_AC001_Offl"+String(WiFi.macAddress()))){
    Serial.println(F("An error occurred initializing Bluetooth"));
  }else{
    SerialBT.println("Bluetooth initialized");
    Serial.println(F("Bluetooth initialized"));
  }
  delay(2000);
  SerialBT.println("Configuration mode is ON");
  Serial.println(ESP.getFreeHeap());
  // bluetooth_Loop();


}

void bluetooth_Loop(void)
{
  while (SerialBT.available())
  {
    char incomingChar = SerialBT.read();
    if (incomingChar != '\r' && incomingChar != '\n')
    {
      message += String(incomingChar);
      //    Serial.print(incomingChar,HEX);
    }
    else
    {
      Serial.print(message);
      if (!message.equals(""))
      {
        verified = VerifyCred(message);
        if (verified)
          eepromStoreCred();
      }
      break;
    }
    if(incomingChar == '{')
    {
       ble_timerDelay = 120000;
    }
  }
  message = "";
}

void setupBLT(void)
{ //
  // if(!SerialBT.begin("ESP_AC001")){
  if (!SerialBT.begin("ESP_AC001_Offl" + String(WiFi.macAddress())))
  {
    Serial.println(F("An error occurred initializing Bluetooth"));
  }
  else
  {
    SerialBT.println("Bluetooth initialized");
    Serial.println(F("Bluetooth initialized"));
  }
  delay(2000);
  SerialBT.println("Configuration mode is ON");
  Serial.println(ESP.getFreeHeap());
  // bluetooth_Loop();
}

