
/*
* WiFi Events

0  ARDUINO_EVENT_WIFI_READY               < ESP32 WiFi ready
1  ARDUINO_EVENT_WIFI_SCAN_DONE                < ESP32 finish scanning AP
2  ARDUINO_EVENT_WIFI_STA_START                < ESP32 station start
3  ARDUINO_EVENT_WIFI_STA_STOP                 < ESP32 station stop
4  ARDUINO_EVENT_WIFI_STA_CONNECTED            < ESP32 station connected to AP
5  ARDUINO_EVENT_WIFI_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  ARDUINO_EVENT_WIFI_STA_GOT_IP               < ESP32 station got IP from connected AP
8  ARDUINO_EVENT_WIFI_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  ARDUINO_EVENT_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 ARDUINO_EVENT_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 ARDUINO_EVENT_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 ARDUINO_EVENT_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 ARDUINO_EVENT_WIFI_AP_START                 < ESP32 soft-AP start
14 ARDUINO_EVENT_WIFI_AP_STOP                  < ESP32 soft-AP stop
15 ARDUINO_EVENT_WIFI_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 ARDUINO_EVENT_WIFI_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 ARDUINO_EVENT_WIFI_AP_GOT_IP6               < ESP32 ap interface v6IP addr is preferred
19 ARDUINO_EVENT_WIFI_STA_GOT_IP6              < ESP32 station interface v6IP addr is preferred
20 ARDUINO_EVENT_ETH_START                < ESP32 ethernet start
21 ARDUINO_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 ARDUINO_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 ARDUINO_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 ARDUINO_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
19 ARDUINO_EVENT_ETH_GOT_IP6              < ESP32 ethernet interface v6IP addr is preferred
25 ARDUINO_EVENT_MAX
*/

#include "evse_wifi.h"

extern uint8_t currentDisplay;

// Set Your WiFi Credentials Here
// extern const char *evse_ssid;     // Replace with Your Router SSID
// extern const char *evse_password; // Replace with Your Router Password

extern String key_m;
extern String ssid_m;
uint8_t Wifi_event = 0;
// // Set your access point network credentials
// const char* ssid = "7S_9101982";
// const char* password = "123456789";
// IPAddress local_ip(192,168,0,1);
// IPAddress gateway(192,168,0,1);
// IPAddress subnet(255,255,255,0);
// const int   channel        = 14;                        // WiFi Channel number between 1 and 13
// const bool  hide_SSID      = true;                     // To disable SSID broadcast -> SSID will not appear in a basic WiFi scan
// const int   max_connection = 2;

uint8_t disconnection_err_count = 0;
uint8_t isWifiConnected = 0;
uint8_t reasonLost = 0;
int32_t rssi = 0;
uint8_t evse_WifiConnected = 0;

uint8_t wifi_Connect = 0;
int chan;
WiFiEventId_t eventID = 0;
bool wifi_deinit_flag = 0;

void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.print(F("EVSE Connected to Acess Point: \""));
        Serial.print(ssid_m);
        Serial.print(F("\" Mac Id: "));
        Serial.println(WiFi.BSSIDstr());
        disconnection_err_count = 0;
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        isWifiConnected = 0;
        Wifi_event = 1;
        Serial.print(F("Disconnected from WiFi access point\r\n"));
        currentDisplay = 0;
        chan = WiFi.channel();
        // Serial.print("Wi-Fi CHANNEL ");
        // Serial.println(WiFi.channel());
        if (reasonLost == WIFI_REASON_NO_AP_FOUND ||
            reasonLost == WIFI_REASON_ASSOC_LEAVE ||
            reasonLost == WIFI_REASON_AUTH_EXPIRE)
        {
            if (disconnection_err_count++ < 5)
            {
                delay(5000);
                esp_wifi_connect();
                break;
            }
            else
            {
                if (wifi_deinit_flag == 0)
                {
                    disconnection_err_count = 0;
                    Serial.print(F("WIFI retries exceeded..!\r\n"));
                    WiFi.disconnect(true);
                    delay(1000);
                    WiFi.mode(WIFI_STA);
                    // WiFi.begin(evse_ssid, evse_password);
                    WiFi.begin(ssid_m.c_str(), key_m.c_str());
                    delay(6500);
                }
            }
        }
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());
        disconnection_err_count = 0;
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        delay(500);
        esp_wifi_connect();
        delay(500);
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        Serial.println("WiFi access point started");
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        Serial.println("WiFi access point  stopped");
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        Serial.println("Client connected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        // isWifiConnected = 0;
        Serial.println("Client disconnected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        Serial.println("Assigned IP address to client");
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        Serial.println("Received probe request");
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        Serial.println("AP IPv6 is preferred");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.println("STA IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        Serial.println("Ethernet IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_START:
        Serial.println("Ethernet started");
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("Ethernet stopped");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("Ethernet connected");
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("Ethernet disconnected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.println("Obtained IP address");
        break;
    default:
        break;
    }
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("*******************WiFi connected***********************");
    // Serial.println("WiFi connected");
    // Serial.println("IP address: ");
    // Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
    isWifiConnected = 1;
    Wifi_event = 0;
    evse_WifiConnected = 1;
    chan = WiFi.channel();
    // Serial.print("Wi-Fi CHANNEL ");
    // Serial.println(WiFi.channel());
    // reConnectAttempt = 0;
}

void wifi_init(void)
{
    // delete old config
    WiFi.disconnect(true);
    wifi_deinit_flag = 0;

    delay(1000);

    // Examples of different ways to register wifi events
    WiFi.onEvent(WiFiEvent);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    eventID = WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info)
                           {
                               Serial.print(F("WiFi lost connection. Reason: "));
                               reasonLost = info.wifi_sta_disconnected.reason;
                               Serial.println(reasonLost);
                               // Serial.println(info.wifi_sta_disconnected.reason);
                           },
                           WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    // Remove WiFi event
    Serial.print(F("WiFi Event ID: "));
    Serial.println(eventID);
    // WiFi.removeEvent(eventID);

    WiFi.mode(WIFI_STA);
    // // Setting the ESP as an access point
    // Serial.print("Setting AP (Access Point)…");
    // // Remove the password parameter, if you want the AP (Access Point) to be open
    // // WiFi.softAP(ssid, password);
    // WiFi.softAPConfig(local_ip, gateway, subnet);
    // WiFi.softAP(ssid, password, channel, hide_SSID, max_connection);
    // // WiFi.softAP(ssid, password);

    // IPAddress IP = WiFi.softAPIP();
    // Serial.print("AP IP address: ");
    // Serial.println(IP);

    // WiFi.begin(evse_ssid, evse_password);
    WiFi.begin(ssid_m.c_str(), key_m.c_str());

    // Serial.println();
    // Serial.println();
    Serial.println("Wait for WiFi... ");
    while ((WiFi.status() != WL_CONNECTED) && (wifi_Connect++ < 10))
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            wifi_Connect = 0;
            break;
        }
        Serial.print(F("* "));
        delay(1500);
    }

    Serial.print(F("EVSE MAC Address:  "));
    Serial.println(WiFi.macAddress());
    // chan = WiFi.channel();
    // Serial.print(F("Wi-Fi CHANNEL "));
    // Serial.println(WiFi.channel());
}

void wifi_deinit(void)
{
    WiFi.removeEvent(eventID);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.print(F("WiFi De-Initialized\r\n"));
    delay(1000);
}
