#include"CustomGsm.h"
#include "libraries/arduinoWebSockets-master/src/WebSocketsClient.h"
#include <WiFi.h>

#if ETHERNET_ENABLED
#include "CustomEthernet.h"
void eth_connect();
#endif

void internetLoop();

void eth_4g_connect();
void wifi_gsm_connect();
void cloudConnectivityLed_Loop();
