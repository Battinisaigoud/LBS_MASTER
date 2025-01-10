#if ETHERNET_ENABLED
#ifndef CUST_ETH
#define CUST_ETH

#include <Ethernet.h>
#include <Preferences.h>

void ethernetSetup();

void ethernetLoop();


#endif

#endif