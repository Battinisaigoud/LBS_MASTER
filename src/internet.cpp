#include "internet.h"

#if DWIN_ENABLED
#include "dwin.h"
extern unsigned char unavail[22]; // not available
extern unsigned char CONN_UNAVAIL[30];
extern unsigned char clear_avail[28];
#endif

void wifi_Loop();
void cloudConnectivityLed_Loop();

extern bool flag_evseIsBooted_A;
extern bool isInternetConnected;
extern TinyGsmClient client;
extern WebSocketsClient webSocket;

extern bool wifi_connect;
extern bool wifi_enable;

extern bool gsm_connect;
extern bool gsm_enable;

extern bool ethernet_enable;
extern bool ethernet_connect;

int counter_gsmconnect = 0;
int counter_wifiConnect = 0;
int16_t counter_ethconnect = 0;

void internetLoop(){

	if(wifi_enable == true && gsm_enable == true){
		//
		wifi_gsm_connect();

	}
	#if ETHERNET_ENABLED
	else if(ethernet_enable && gsm_enable)
	{
		eth_4g_connect();
	}
	else if(ethernet_enable)
	{
        eth_connect();
	}
	#endif
	else if(wifi_enable == true){
		//
		wifi_connect = true;
		gsm_connect  = false;
		// wifi_Loop();
		// webSocket.loop();
		// cloudConnectivityLed_Loop();

	}else if(gsm_enable == true){

		gsm_connect = true;
		wifi_connect = false;  //redundant
		gsm_Loop();
	}

}

#if ETHERNET_ENABLED
void eth_connect()
{

if(ethernet_connect)
{
	webSocket.loop();
    if(flag_evseIsBooted_A == true){
  		Ethernet.maintain();	
    }
    ethernetLoop();
}
}
#endif

void eth_4g_connect()
{
#if ETHERNET_ENABLED
if(ethernet_connect)
{
	webSocket.loop();
	if(Ethernet.linkStatus() != LinkON || isInternetConnected == false)
	{
			Serial.println("[eth] counter_ethConnect"+ String(counter_ethconnect));
			counter_ethconnect++;
	}
    if(flag_evseIsBooted_A == true){
  		Ethernet.maintain();	
    }
    ethernetLoop();
	if(counter_ethconnect > 20)
	{
		counter_ethconnect = 0;
		isInternetConnected = false;
		Serial.println(F("Switching to 4G"));
		#if DWIN_ENABLED
		uint8_t err = 0;
    CONN_UNAVAIL[4] = 0X66;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
    CONN_UNAVAIL[4] = 0X71;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
    CONN_UNAVAIL[4] = 0X7B;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
	err = DWIN_SET(unavail, sizeof(unavail) / sizeof(unavail[0]));
    delay(10);
	#endif
			ethernet_connect = false;
			gsm_connect = true;
	}
}
#endif
	 if(gsm_connect == true)
	{
		wifi_connect  = false;
		Serial.println(F("[eth_gsm_connect] GSM "));
		gsm_Loop();

		if(!client.connected()){
			Serial.println("[gsm] counter_gsmconnect:"+ String(counter_gsmconnect));
			if(counter_gsmconnect++ >= 1){   //almost 5 min
				counter_gsmconnect = 0;
				isInternetConnected = false;
				Serial.println(F("Switching to ethernet"));
				#if DWIN_ENABLED
				uint8_t err = 0;
    CONN_UNAVAIL[4] = 0X66;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
    CONN_UNAVAIL[4] = 0X71;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
    CONN_UNAVAIL[4] = 0X7B;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
	err = DWIN_SET(unavail, sizeof(unavail) / sizeof(unavail[0]));
    delay(10);
	#endif
				ethernet_connect = true;
				gsm_connect = false;
			}
		}
}   
}

void wifi_gsm_connect(){

	if(wifi_connect == true){
		gsm_connect = false;
		Serial.println(F("[wifi_gsm_connect] WiFi"));
		wifi_Loop();
		webSocket.loop();
		// cloudConnectivityLed_Loop();
		if(WiFi.status() != WL_CONNECTED || isInternetConnected == false){
			Serial.println("[wifi] counter_wifiConnect->"+ String(counter_wifiConnect));
			counter_wifiConnect++;
			if(counter_wifiConnect > 50){
				Serial.println(F("Switching To gsm"));
				WiFi.disconnect();
				counter_wifiConnect = 0;
				wifi_connect = false;
				gsm_connect = true;
			}

		}else{
			counter_wifiConnect = 0;
			Serial.println(F("default Counter_wifiConnect"));
		}
	}else if(gsm_connect == true){
		wifi_connect  = false;
		Serial.println(F("[wifi_gsm_connect] GSM "));
		gsm_Loop();

		if(!client.connected()){
			Serial.println("[gsm] counter_gsmconnect:"+ String(counter_gsmconnect));
			if(counter_gsmconnect++ > 1){   //almost 5 min
				counter_gsmconnect = 0;
				Serial.println(F("Switching to WIFI"));
				#if DWIN_ENABLED
				uint8_t err = 0;
    CONN_UNAVAIL[4] = 0X66;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
    CONN_UNAVAIL[4] = 0X71;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
    CONN_UNAVAIL[4] = 0X7B;
    err = DWIN_SET(CONN_UNAVAIL, sizeof(CONN_UNAVAIL) / sizeof(CONN_UNAVAIL[0]));
	//err = DWIN_SET(unavail, sizeof(unavail) / sizeof(unavail[0]));
    delay(10);
	#endif
				wifi_connect = true;
				gsm_connect = false;
			}

		}



	}



}