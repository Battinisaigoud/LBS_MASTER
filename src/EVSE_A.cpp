// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

/**
Edited by Pulkit Agrawal.
*/

#include "EVSE_A.h"
#include "EVSE_A_Offline.h"
#include "Master.h"
#include "ControlPilot.h"

// new variable names defined by @Pulkit. might break the build.
OnBoot_A onBoot_A;
OnReadUserId_A onReadUserId_A;
OnSendHeartbeat_A onSendHeartbeat_A;
OnAuthentication_A onAuthentication_A;
OnStartTransaction_A onStartTransaction_A;
OnStopTransaction_A onStopTransaction_A;
OnUnauthorizeUser_A onUnauthorizeUser_A;

extern bool wifi_reconnected_flag;
bool SessionStop = false;
#if DWIN_ENABLED
#include "dwin.h"

extern unsigned char v1[8];
extern unsigned char v2[8];
extern unsigned char v3[8];
extern unsigned char i1[8];
extern unsigned char i2[8];
extern unsigned char i3[8];
extern unsigned char e1[8];
extern unsigned char e2[8];
extern unsigned char e3[8];
extern unsigned char change_page[10];
extern unsigned char avail[22];
extern unsigned char charging[28];
extern unsigned char cid1[8];
extern unsigned char fault_emgy[28];
extern unsigned char GFCI[20];
#endif

extern int client_reconnect_flag;

// timeout for heartbeat signal.
bool relayTriggered_A = false;
ulong T_SENDHEARTBEAT = 60000;
bool timeout_active_A = false;
bool timer_initialize_A = false;
ulong timeout_start_A = 0;
int prevTxnId_A = -1;
// new flag names. replace them with old names.
bool evIsPlugged_A;
bool flag_evseIsBooted_A;
bool flag_evseReadIdTag_A;
bool flag_evseAuthenticate_A;
bool flag_evseStartTransaction_A;
bool flag_evRequestsCharge_A;
bool flag_evseStopTransaction_A;
bool flag_evseUnauthorise_A;
bool flag_rebootRequired_A;
bool flag_evseSoftReset_A;
extern uint8_t gu8_fault_occured;
// added by @Wamique
// not used. part of Smart Charging System.
float chargingLimit_A = 32.0f;
String Ext_currentIdTag_A = "";
// extern String currentIdTag;
long int blinckCounter_A = 0;
int counter1_A = 0;
ulong t;
int connectorDis_counter_A = 0;
String currentIdTag_A = "";
short int fault_counter_A = 0;
bool flag_faultOccured_A = false;
short int counter_drawingCurrent_A = 0;
float drawing_current_A = 0;
short counter_faultstate_A = 0;
short counter_suspendedstate_A = 0;
uint8_t currentCounterThreshold_A = 90;
extern Preferences resumeTxn_A;
#if 0
extern Preferences resumeTxn_A_2;
#endif
extern bool ongoingTxn_A;
extern String idTagData_A;
ulong relay_timer_A;
ulong faultTimer_A = 0;
ulong timerHb = 0;
unsigned int heartbeatInterval = 50;

bool lu8_send_status_flag = false;

extern ATM90E36 eic;
extern bool flag_rebootRequired_B;
extern bool flag_rebootRequired_C;
extern bool flag_controlPAuthorise_A;
extern WebSocketsClient webSocket;
extern MFRC522 mfrc522;
extern EVSE_states_enum EVSE_state;
extern Preferences preferences;
extern bool webSocketConncted;
extern bool isInternetConnected;
// metering Flag
extern bool flag_MeteringIsInitialised;
extern MeteringService* meteringService;
extern bool wifi_connect;
extern bool gsm_connect;
extern TinyGsmClient client;
extern bool EMGCY_GFCI_Fault_Occ;
extern int transactionId_A;
extern int globalmeterstartA;
extern uint8_t gu8_finishing_state;

extern uint8_t reasonForStop;

extern bool ethernet_enable;
extern bool ethernet_connect;

float minCurr = 0.05;

bool disp_evse_A = false;
bool notFaulty_A = false;

extern bool offline_connect;

extern bool offline_charging_A;
extern float discurrEnergy_A;
float LastPresentEnergy_A = 0;
extern float current_energy_A;
bool flag_evseReserveNow = false;		 // added by @mkrishna
bool flag_evseCancelReservation = false; // added by @mkrishna
extern time_t reservation_start_time;
time_t reservation_expiry_time = 0;
bool prepare_reserve_slot = false;
bool reservation_start_flag = false;
bool EMGCY_FaultOccured_A = false;

time_t current_reserveDate;
extern time_t reserveDate;
extern time_t reservedDuration;
extern String reserve_currentIdTag;

extern int8_t fault_code_A;

extern bool flag_stop;

extern uint8_t gu8_online_flag;
uint8_t Boot_Accepted = 0;
/**********************************************************************************************/
String resume_currentIdTag_A;
int resume_connector_id_A = 0;
uint8_t resume_stop_start_txn_A = 0;

String remote_idTag = "";
uint8_t gu8_remote_start_flag_A = 0;
uint8_t gu8_remote_start_flag_B = 0;
bool stopoffline_A = false;

/**********************************************************************************************/

volatile uint32_t Electric_paramater_count = 0;

#if LCD_ENABLED
#include "LCD_I2C.h"
extern LCD_I2C lcd;
#endif

evse_boot_stat_t evse_boot_state;
uint8_t gu8_state_change = 0;
uint8_t gu8_bootsuccess = 0;
extern uint8_t gu8_rfid_tapped_A;
uint8_t reasonforstopOff = 3;

// initialize function. called when EVSE is booting.
// NOTE: It should be also called when there is a reset or reboot required. create flag to control that. @Pulkit
/**********************************************************/
void EVSE_A_StopSession()
{

	if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
	{
		getChargePointStatusService_A()->stopEvDrawsEnergy();
	}

	// digitalWrite(32, LOW);
	gu8_rfid_tapped_A = 0;
	disp_evse_A = false;
	requestForRelay(STOP, 1); // commented by shiva for testing
	delay(500);
	requestforCP_OUT(STOP);
	Serial.println("stop cp 1");
	flag_evseReadIdTag_A = false;
	flag_evseAuthenticate_A = false;
	flag_evseStartTransaction_A = false;
	flag_evRequestsCharge_A = false;
	flag_evseStopTransaction_A = true;
	flag_evseUnauthorise_A = false;
	Serial.println("[EVSE] Stopping Session : " + String(EVSE_state));

	if (!offline_charging_A)
	{
		resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
		resumeTxn_A.putBool("ongoingTxnoff_A", true);
		resumeTxn_A.putInt("reasonforstop_Off", reasonForStop);
		//   Serial.println(resumeTxn_A.getInt("reasonforstop_Off",3));
		resumeTxn_A.end();
	}
}
/**************************************************************************/

/**************************SetUp********************************************/
void EVSE_A_setup()
{

#if 0
	resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
	idTagData_A = resumeTxn_A.getString("idTagData_A", "");
	ongoingTxn_A = resumeTxn_A.getBool("ongoingTxn_A", false);

	Serial.println("Stored ID:" + String(idTagData_A));
	Serial.println("Ongoing Txn: " + String(ongoingTxn_A));
#endif
#if 1

	/*********Preferences Block For Restarting previously running Txn [Power Cycle]***********/
	resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
	idTagData_A = resumeTxn_A.getString("idTagData_A", "");
	ongoingTxn_A = resumeTxn_A.getBool("ongoingTxn_A", false);
	transactionId_A = resumeTxn_A.getInt("TxnIdData_A", -1);
	globalmeterstartA = resumeTxn_A.getFloat("meterStart", false);
	stopoffline_A = resumeTxn_A.getBool("ongoingTxnoff_A", false);
	reasonforstopOff = resumeTxn_A.getInt("reasonforstop_Off", 3);
	resumeTxn_A.end();

#if 0
	resumeTxn_A_2.begin("resume", false); // opening preferences in R/W mode
	transactionId_A = resumeTxn_A_2.getInt("TxnIdData", -1);
	resumeTxn_A_2.end();
#endif
	Serial.println("Stored ID_A:" + String(idTagData_A));
	Serial.println("Ongoing Txn_A: " + String(ongoingTxn_A));
	Serial.println("Txn id_A is: " + String(transactionId_A));
	Serial.println("meterstart is: " + String(globalmeterstartA));
	Serial.println(" stopoffline_A is: " + String(stopoffline_A));
	Serial.println(" reasonforstop_Off is: " + String(reasonforstopOff));
	resume_currentIdTag_A = idTagData_A;

	/****************************************************************************************/

	flag_evseIsBooted_A = false;
	flag_evseSoftReset_A = false;
	flag_rebootRequired_A = false;

	/*
	 * @brief : Feature added by G. Raja Sumant to fix the session resume operation.
	 */
	 // This depends if the power fail scenario is evaluated as true.
	if (ongoingTxn_A)
	{
		SessionStop = true;
		// flag_evRequestsCharge_A = true;    temporary comment for testing
		if (DEBUG_OUT)
			Serial.println("****on going transaction is true*****");
	}
	else
	{
		if (DEBUG_OUT)
			Serial.println("****on going transaction is false****");
	}

#endif
	EVSE_A_setOnBoot([]()
		{
			// this is not in loop, that is why we need not update the flag immediately to avoid multiple copies of bootNotification.
			OcppOperation* bootNotification = makeOcppOperation(&webSocket, new BootNotification());
			initiateOcppOperation(bootNotification);

			if (gu8_online_flag == 1)
			{
				bootNotification->setOnReceiveConfListener([](JsonObject payload)
					{

						if (flag_MeteringIsInitialised == false) {
							Serial.println("[SetOnBooT] Initializing metering services");
							meteringService->init(meteringService);
#if LCD_ENABLED
							lcd.clear();
							lcd.setCursor(0, 2);
							lcd.print("TAP RFID/SCAN QR");
#endif
						}

						if (DEBUG_OUT) Serial.print("EVSE_setOnBoot Callback: Metering Services Initialization finished.\n");

						flag_evseIsBooted_A = true; //Exit condition for booting. 	
						flag_evseReadIdTag_A = true; //Entry condition for reading ID Tag.
						flag_evseAuthenticate_A = false;
						flag_evseStartTransaction_A = false;
						// flag_evRequestsCharge_A = false;
						evse_ChargePointStatus = Available;

						if (ongoingTxn_A)
						{
							// requestforCP_OUT(START);
							// delay(100);
							// requestForRelay(START, 1);
							// delay(100);
							// disp_evse_A = true;
							//flag_evRequestsCharge_A = true;            temporary comment for testing
							getChargePointStatusService_A()->startTransaction(transactionId_A);
							reasonForStop = 5;
							//fault_code_A = Power_Loss;
							EVSE_A_StopSession();
							Serial.println("[EVSE_A] EVSE_A_StopSession 4");
							if (DEBUG_OUT)
								Serial.println("*[EVSE_setOnBoot] on going transaction is true*");

							resume_stop_start_txn_A = 1;
						}
						else
						{
							flag_evRequestsCharge_A = false;
							//resume_stop_start_txn_A = 0 ;
							if (DEBUG_OUT)
								Serial.println("****[EVSE_setOnBoot] on going transaction is false*****");
						}
						flag_evseStopTransaction_A = false;
						flag_evseUnauthorise_A = false;
						if (DEBUG_OUT) Serial.print("EVSE_setOnBoot Callback: Closing Relays.\n");
#if LCD_ENABLED
						lcd.clear();
						lcd.setCursor(0, 3);
						lcd.print("TAP RFID/SCAN QR");
#endif


						if (DEBUG_OUT) Serial.print("EVSE_setOnBoot Callback: Boot successful. Calling Read User ID Block.\n"); });
			}
			else
			{

				if (flag_MeteringIsInitialised == false)
				{
					Serial.println("[SetOnBooT] Initializing metering services");
					meteringService->init(meteringService);
#if LCD_ENABLED
					lcd.clear();
					lcd.setCursor(0, 2);
					lcd.print("TAP RFID/SCAN QR");
#endif
				}

				if (DEBUG_OUT)
					Serial.print("EVSE: Metering Services Initialization finished.\n");

				flag_evseIsBooted_A = true;  // Exit condition for booting.
				flag_evseReadIdTag_A = true; // Entry condition for reading ID Tag.
				flag_evseAuthenticate_A = false;
				flag_evseStartTransaction_A = false;
				// flag_evRequestsCharge_A = false;
				evse_ChargePointStatus = Available;

				if (ongoingTxn_A)
				{
					// requestforCP_OUT(START);
					// delay(100);
					// requestForRelay(START, 1);
					// delay(100);
					// disp_evse_A = true;
					// flag_evRequestsCharge_A = true;            temporary comment for testing
					getChargePointStatusService_A()->startTransaction(transactionId_A);
					reasonForStop = 5;
					// fault_code_A = Power_Loss;
					EVSE_A_StopSession();
					Serial.println("[EVSE_A] EVSE_A_StopSession 5");
					if (DEBUG_OUT)
						Serial.println("****[EVSE_setOnBoot] on going transaction is true*****");

					resume_stop_start_txn_A = 1;
				}
				else
				{
					flag_evRequestsCharge_A = false;
					// resume_stop_start_txn_A = 0 ;
					if (DEBUG_OUT)
						Serial.println("****[EVSE_setOnBoot] on going transaction is false*****");
				}
				flag_evseStopTransaction_A = false;
				flag_evseUnauthorise_A = false;
			} });

			EVSE_A_setOnReadUserId([]()
				{
					if (DEBUG_OUT) Serial.print("EVSE_A waiting for User ID read...\n");
					static ulong timerForRfid = millis();
					currentIdTag_A = "";
					resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
					idTagData_A = resumeTxn_A.getString("idTagData_A", "");
					ongoingTxn_A = resumeTxn_A.getBool("ongoingTxn_A", false);
					// stopoffline_A = resumeTxn_A.getBool("ongoingTxnoff_A", false);
					reasonforstopOff = resumeTxn_A.getInt("reasonforstop_Off", 3);

					int connector = getChargePointStatusService_A()->getConnectorId();
					if (stopoffline_A == 1)
					{
						stopoffline_A = 0;
						reasonForStop = reasonforstopOff;
						OcppOperation* stopTransaction = makeOcppOperation(&webSocket, new StopTransaction(currentIdTag_A, transactionId_A, connector));
						initiateOcppOperation(stopTransaction);
						// prevTxnId_A = transactionId_A;

						if (webSocketConncted)
						{
							resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
							// resumeTxn_B.putBool("ongoingTxn_B", false);
							resumeTxn_A.putString("idTagData_A", "");
							resumeTxn_A.putInt("TxnIdData_A", -1);
							resumeTxn_A.putBool("ongoingTxnoff_A", false);
							resumeTxn_A.end();
						}
					}

#if 1
					if (gu8_online_flag == 0)
					{
						if (DEBUG_OUT) Serial.print("OFFLINE_EVSE waiting for User ID read...\n");
						if ((ongoingTxn_A == 1) && (idTagData_A != "") &&
							(getChargePointStatusService_A()->getEmergencyRelayClose() == false)) {  //giving priority to stored data
							currentIdTag_A = resumeTxn_A.getString("idTagData", "");

							Serial.println("OFFLINE Resuming Session");
							// requestLed(BLUE, START, 1);
							delay(500);
							// requestLed(BLINKYGREEN, START, 1);
						}
						else if ((getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
							getChargePointStatusService_A()->getUnavailable() == false) {
#if LED_ENABLED
							if (millis() - timerForRfid > 5000) { //timer for sending led request
								//   requestLed(GREEN, START, 1);
								timerForRfid = millis();
							}
#endif

							currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
							Serial.println("[OFFLINE]RFID");

						}
					}
#endif

					else if (ethernet_connect && 														//ethernet Block
						(getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
						(webSocketConncted == true) && getChargePointStatusService_A()->getUnavailable() == false)
					{
						if ((idTagData_A != "") && (ongoingTxn_A == 1)) {
							currentIdTag_A = resumeTxn_A.getString("idTagData_A", "");
							Serial.println("[EVSE_A] Resuming Session");
#if DISPLAY_ENABLED
							connAvail(1, "SESSION A RESUME");
							checkForResponse_Disp();
#endif
#if LCD_ENABLED
							lcd.clear();
							lcd.setCursor(3, 2);
							lcd.print("SESSION A RESUME");
#endif
							getChargePointStatusService_A()->authorize(currentIdTag_A); // so that Starttxn can easily fetch this data
							// requestLed(BLUE, START, 1);
						}
						else {
							if (millis() - timerForRfid > 8000) { //timer for sending led request

								/*
								* @brief : Check unavailable condition
								*/

								// requestLed(GREEN,START,1);
								timerForRfid = millis();
								// add lcd print to take it back to available and online via wifi
#if 0
#if LCD_ENABLED
					//lcd.clear();
								if (disp_evse_B)
								{
									lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
									lcd.print("                    "); // Clear the line
									lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
									lcd.print("B: CHARGING"); // Clear the line
								}
								else
								{
									lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
									lcd.print("                    "); // Clear the line
									lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
									lcd.print("B: AVAILABLE"); // Clear the line
								}
								if (disp_evse_C)
								{
									lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
									lcd.print("                    "); // Clear the line
									lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
									lcd.print("C: CHARGING"); // Clear the line
								}
								else
								{
									lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
									lcd.print("                    "); // Clear the line
									lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
									lcd.print("C: AVAILABLE"); // Clear the line
								}
								lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
								lcd.print("                    "); // Clear the line
								lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
								lcd.print("A: AVAILABLE");
								//lcd.setCursor(0, 1);
								//lcd.print("TAP RFID/SCAN QR");
								//lcd.setCursor(0, 2);
								//lcd.print("CONNECTION");
								lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
								lcd.print("                    "); // Clear the line
								lcd.setCursor(0, 3);
								lcd.print("CLOUD: ETH. TAP RFID");
#endif
#endif
							}

							currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
							Serial.println("[Wifi]RFID A");
						}

					}

					else if ((wifi_connect == true) && 														//Wifi Block
						(getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
						(webSocketConncted == true) &&
						(WiFi.status() == WL_CONNECTED) &&
						(getChargePointStatusService_A()->getUnavailabilityStatus() == false) &&
						(isInternetConnected == true)) {

						if ((idTagData_A != "") && (ongoingTxn_A == 1)) {
							currentIdTag_A = resumeTxn_A.getString("idTagData_A", "");
							Serial.println("[EVSE_A] Resuming Session");
#if LCD_ENABLED
							lcd.clear();
							lcd.setCursor(3, 2);
							lcd.print("RESUMING SESSION");
#endif

							getChargePointStatusService_A()->authorize(currentIdTag_A); // so that Starttxn can easily fetch this data
							if (getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Available)
							{
								// requestLed(BLUE, START, 1);
							}

						}
						else {
							if (millis() - timerForRfid > 8000) { //timer for sending led request
								// requestLed(GREEN,START,1);
								wifi_reconnected_flag = true;
								timerForRfid = millis();
							}

							currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
							Serial.println("[Wifi]RFID A");
						}

					}
					else if ((gsm_connect == true) &&													//GSM Block
						(getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
						(getChargePointStatusService_A()->getUnavailabilityStatus() == false) &&
						(client.connected() == true)) {

						if ((idTagData_A != "") && (ongoingTxn_A == 1)) {
							currentIdTag_A = resumeTxn_A.getString("idTagData_A", "");
							getChargePointStatusService_A()->authorize(currentIdTag_A); // so that Starttxn can easily fetch this data
							Serial.println("[EVSE_A] Resuming Session");
#if LCD_ENABLED
							lcd.clear();
							lcd.setCursor(3, 2);
							lcd.print("RESUMING SESSION");
#endif

							// requestLed(BLUE, START, 1);
						}
						else {
							if (millis() - timerForRfid > 8000) { //timer for sending led request
								// requestLed(GREEN,START,1);
								timerForRfid = millis();
							}

							currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
							Serial.println("[GSM]RFID A");
						}


					}
					resumeTxn_A.end();
					/*
					if((ongoingTxn_m == 1) && (idTagData_m != "") &&
						  (getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
						  (WiFi.status() == WL_CONNECTED)&&
						  (webSocketConncted == true)&&
						  (isInternetConnected == true)){   //giving priority to stored data
						currentIdTag_A = resumeTxn.getString("idTagData","");
						Serial.println("[EVSE_setOnReadUserId] Resuming Session");
						requestLed(BLUE,START,1);

					}else
					if((getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
							  (WiFi.status() == WL_CONNECTED) &&
							  (webSocketConncted == true) &&
							  (isInternetConnected == true)){
						  #if LED_ENABLED
						  if(millis() - timerForRfid > 10000){ //timer for sending led request
						  requestLed(GREEN,START,1);
						  timerForRfid = millis();
						  }
						  #endif
						currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
						Serial.println("********RFID A**********");
					}*/

					if (currentIdTag_A.equals("") == true) {
						flag_evseReadIdTag_A = true; //Looping back read block as no ID tag present.
						flag_evseAuthenticate_A = false;
						flag_evseStartTransaction_A = false;
						flag_evRequestsCharge_A = false;
						flag_evseStopTransaction_A = false;
						flag_evseUnauthorise_A = false;
					}
					else {
						flag_evseReadIdTag_A = false;
						if (ongoingTxn_A)
						{
							flag_evseAuthenticate_A = false;
							if (DEBUG_OUT)
								Serial.println("[EVSE] on going transaction is true");
						}
						else
						{
							//flag_evseAuthenticate_A = true; //Entry condition for authentication block.

				/*
						* @brief : Reservation implementation
						* by Krishna.M & Sumant 14/07/2022
						*/
						//if (flag_evseReserveNow)
							if (reservation_start_flag)
							{
								if (currentIdTag_A.equals(reserve_currentIdTag) == true)
								{
									flag_evseAuthenticate_A = true;
									if (DEBUG_OUT)
										Serial.println("[EVSE]Authentication ID TAG is truE");
								}
								else
								{
									flag_evseReadIdTag_A = true; // Looping back read block as no ID tag present.
									flag_evseAuthenticate_A = false;
									if (DEBUG_OUT) Serial.print("EVSE UnAuthentication ID TAG\n");
								}
							}
							else
							{
								flag_evseAuthenticate_A = true;
							}
						}
						flag_evseStartTransaction_A = false;
						flag_evRequestsCharge_A = false;
						flag_evseStopTransaction_A = false;
						flag_evseUnauthorise_A = false;
						//if (DEBUG_OUT) Serial.print(F("EVSE_setOnReadUserId Callback: Successful User ID Read. Calling Authentication Block.\n"));
					} });

					EVSE_A_setOnsendHeartbeat([]()
						{
							if (DEBUG_OUT) Serial.print("EVSESending heartbeat signal...\n");
							if (gu8_online_flag == 1)
							{
								OcppOperation* heartbeat = makeOcppOperation(&webSocket, new Heartbeat());
								initiateOcppOperation(heartbeat);
								heartbeat->setOnReceiveConfListener([](JsonObject payload) {
									const char* currentTime = payload["currentTime"] | "Invalid";
									if (strcmp(currentTime, "Invalid")) {
										if (setTimeFromJsonDateString(currentTime)) {
											if (DEBUG_OUT) Serial.print("EVSE Request has been accepted!\n");
										}
										else {
											Serial.print("EVSE_Request accepted. But Error reading time string. Expect format like 2020-02-01T20:53:32.486Z\n");
										}
									}
									else {
										Serial.print("EVSE_Request denied. Missing field currentTime. Expect format like 2020-02-01T20:53:32.486Z\n");
									}
									});
							} });

							EVSE_A_setOnAuthentication([]()
								{
									if (DEBUG_OUT)
										Serial.print("EVSE Authenticating...\n");
									flag_evseAuthenticate_A = false;
									OcppOperation* authorize = makeOcppOperation(&webSocket, new Authorize(currentIdTag_A));
									initiateOcppOperation(authorize);
									/*chargePointStatusService->authorize(currentIdTag_A, connectorId_A);  */ // have to edit
									if (gu8_online_flag == 1)
									{
										authorize->setOnReceiveConfListener([](JsonObject payload)
											{
												const char* status = payload["idTagInfo"]["status"] | "Invalid";
												if (!strcmp(status, "Accepted")) {
													flag_evseReadIdTag_A = false;
													flag_evseAuthenticate_A = false;
													flag_evseStartTransaction_A = true; //Entry condition for starting transaction.
													flag_evRequestsCharge_A = false;
													flag_evseStopTransaction_A = false;
													flag_evseUnauthorise_A = false;

													if (DEBUG_OUT) Serial.print("EVSE_Authorize request has been accepted!\n");
													// requestLed(BLUE,START,1);
#if ALPR_ENABLED
													alprAuthorized();
#endif
#if CP_A_ACTIVE 
													flag_controlPAuthorise_A = true;
#endif

												}
												else {
													flag_evseReadIdTag_A = false;
													flag_evseAuthenticate_A = false;
													flag_evseStartTransaction_A = false;
													flag_evRequestsCharge_A = false;
													flag_evseStopTransaction_A = false;
													flag_evseUnauthorise_A = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
													// getChargePointStatusService_B()->setUnavailabilityStatus(false);
#if CP_A_ACTIVE
													requestforCP_OUT(STOP);  //stop pwm
													Serial.println("stop cp 2");
#endif
#if ALPR_ENABLED
													//alprTxnStopped();
													alprunAuthorized();
#endif
													if (DEBUG_OUT) Serial.print("EVSE_Authorize request has been denied!\n");
												} });
									}
									else
									{
										flag_evseReadIdTag_A = false;
										flag_evseAuthenticate_A = false;
										flag_evseStartTransaction_A = false;
										flag_evRequestsCharge_A = false;
										flag_evseStopTransaction_A = false;
										flag_evseUnauthorise_A = true; // wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
										// getChargePointStatusService_B()->setUnavailabilityStatus(false);
#if CP_A_ACTIVE
										requestforCP_OUT(STOP); // stop pwm
										Serial.println("stop cp 3");
#endif
#if ALPR_ENABLED
										// alprTxnStopped();
										alprunAuthorized();
#endif
										if (DEBUG_OUT)
											Serial.print("EVSE Authorize request has been denied!\n");
									} });

									EVSE_A_setOnStartTransaction([]()
										{
											flag_evseStartTransaction_A = false;
											String idTag = "";
											int connectorId = 0;
											resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode

											if (getChargePointStatusService_A() != NULL)
											{
												if (!getChargePointStatusService_A()->getIdTag().isEmpty())
												{
													idTag = String(getChargePointStatusService_A()->getIdTag());
													connectorId = getChargePointStatusService_A()->getConnectorId();
												}
											}
#if POWER_RECYCLE
											if (resume_stop_start_txn_A == 2)
											{
												resume_stop_start_txn_A = 0;

												idTag = resume_currentIdTag_A;
												connectorId = 1;
												Serial.print("resume_currentIdTag_A : ");
												Serial.println(resume_currentIdTag_A);
												Serial.print("connectorId : ");
												Serial.println(connectorId);
												currentIdTag_A = resume_currentIdTag_A;
												Serial.print("currentIdTag_A : ");
												Serial.println(currentIdTag_A);
												getChargePointStatusService_A()->authorize(resume_currentIdTag_A); // so that Starttxn can easily fetch this data
												Serial.println("[EVSE_A_setOnReadUserId] Resuming Session");
											}
#endif

											if (gu8_remote_start_flag_A == 1)
											{
												gu8_remote_start_flag_A = 0;
												idTag = remote_idTag;
												connectorId = 1;
												Serial.print("remote_idTag : ");
												Serial.println(remote_idTag);
												Serial.print("connectorId : ");
												Serial.println(connectorId);
												currentIdTag_A = remote_idTag;
												Serial.print("currentIdTag_A : ");
												Serial.println(currentIdTag_A);
												getChargePointStatusService_A()->authorize(remote_idTag); // so that Starttxn can easily fetch this data
												Serial.println("[EVSE_A_setOnRemoteStart] Remote Start Trasaction");
											}

											OcppOperation* startTransaction = makeOcppOperation(&webSocket, new StartTransaction(idTag, connectorId));
											initiateOcppOperation(startTransaction);
											// getChargePointStatusService_B()->setUnavailabilityStatus(true);

											if (gu8_online_flag == 1)
											{
												startTransaction->setOnReceiveConfListener([](JsonObject payload)
													{
														const char* status = payload["idTagInfo"]["status"] | "Invalid";
														if (!strcmp(status, "Accepted") || !strcmp(status, "ConcurrentTx")) {

															flag_evseReadIdTag_A = false;
															flag_evseAuthenticate_A = false;
															flag_evseStartTransaction_A = false;
															flag_evRequestsCharge_A = true;
															flag_evseStopTransaction_A = false;
															flag_evseUnauthorise_A = false;
															if (DEBUG_OUT) Serial.print("EVSE_StartTransaction was successful\n");

#if ALPR_ENABLED
															alprTxnStarted();
#endif
#if LCD_ENABLED
															lcd.clear();
															lcd.setCursor(3, 2);
															lcd.print("SESSION STARTED");
#endif

															/************************************/
															int transactionId = payload["transactionId"] | -1;
															getChargePointStatusService_A()->startTransaction(transactionId);        //new block for Three connector
															getChargePointStatusService_A()->startEnergyOffer();
															// getChargePointStatusService_A()->startEvDrawsEnergy();
															resumeTxn_A.putString("idTagData_A", getChargePointStatusService_A()->getIdTag());
															resumeTxn_A.putBool("ongoingTxn_A", true);
															SessionStop = false;
															//*****Storing tag data in EEPROM****//
															if (currentIdTag_A.equals("") == true)
															{

																resumeTxn_A.putString("idTagData_A", currentIdTag_A);
																resumeTxn_A.putBool("ongoingTxn_A", false);
																resumeTxn_A.end();
																Serial.print("ongoingTxn_A: false \n");
															}
															else
															{
																resumeTxn_A.putString("idTagData", currentIdTag_A);
																resumeTxn_A.putBool("ongoingTxn", true);
																resumeTxn_A.putFloat("meterStart", globalmeterstartA);
																resumeTxn_A.putInt("TxnIdData_A", transactionId);

																resumeTxn_A.end();
																Serial.print("ongoingTxn_A: true \n");
															}
															/*
														  //   resumeTxn.putString("idTagData",currentIdTag);
														  //   resumeTxn.putBool("ongoingTxn",true);*/
														  //***********************************//

														}
														else
														{
															flag_evseReadIdTag_A = false;
															flag_evseAuthenticate_A = false;
															flag_evseStartTransaction_A = false;
															flag_evRequestsCharge_A = false;
															flag_evseStopTransaction_A = false;
															flag_evseUnauthorise_A = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
															if (DEBUG_OUT) Serial.print("EVSE_StartTransaction was unsuccessful\n");
#if ALPR_ENABLED
															alprTxnStopped();
#endif
#if LCD_ENABLED
															lcd.clear();
															lcd.setCursor(0, 2);
															lcd.print("SESSION NOT STARTED");
															lcd.setCursor(0, 3);
															lcd.print("CONCURRENT");
#endif

#if CP_A_ACTIVE
															requestforCP_OUT(STOP);  //stop pwm
															Serial.println("stop cp 4");
#endif
															//	getChargePointStatusService_B()->setUnavailabilityStatus(false);

																/*resume namespace
																resumeTxn.putBool("ongoingTxn",false);
																resumeTxn.putString("idTagData","");
																*/
															resumeTxn_A.putBool("ongoingTxn_A", false);
															resumeTxn_A.putString("idTagData_A", "");
															resumeTxn_A.end();
														} });
											}
											else
											{
												flag_evseReadIdTag_A = false;
												flag_evseAuthenticate_A = false;
												flag_evseStartTransaction_A = false;
												flag_evRequestsCharge_A = false;
												flag_evseStopTransaction_A = false;
												flag_evseUnauthorise_A = true; // wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
												if (DEBUG_OUT)
													Serial.print("EVSE_StartTransaction was unsuccessful\n");
#if ALPR_ENABLED
												alprTxnStopped();
#endif
#if LCD_ENABLED
												lcd.clear();
												lcd.setCursor(0, 2);
												lcd.print("SESSION NOT STARTED");
												lcd.setCursor(0, 3);
												lcd.print("CONCURRENT");
#endif

#if CP_A_ACTIVE
												requestforCP_OUT(STOP); // stop pwm
												Serial.println("stop cp 5");
#endif
												//	getChargePointStatusService_B()->setUnavailabilityStatus(false);

												/*resume namespace
												resumeTxn.putBool("ongoingTxn",false);
												resumeTxn.putString("idTagData","");
												*/
												resumeTxn_A.putBool("ongoingTxn_A", false);
												resumeTxn_A.putString("idTagData_A", "");
												resumeTxn_A.end();
											} });

											EVSE_A_setOnStopTransaction([]()
												{
													flag_evseStopTransaction_A = false;

													int txnId = getChargePointStatusService_A()->getTransactionId();
													int connector = getChargePointStatusService_A()->getConnectorId();

													if (txnId == -1)
													{

														flag_evseReadIdTag_A = false;
														flag_evseAuthenticate_A = false;
														flag_evseStartTransaction_A = false;
														flag_evRequestsCharge_A = false;
														flag_evseStopTransaction_A = false;
														flag_evseUnauthorise_A = true;
														getChargePointStatusService_A()->stopEvDrawsEnergy();
														getChargePointStatusService_A()->stopEnergyOffer();
														getChargePointStatusService_A()->stopTransaction();
														//	getChargePointStatusService_B()->setUnavailabilityStatus(false);
														getChargePointStatusService_A()->unauthorize();

														if (DEBUG_OUT)
															Serial.print("EVSE_A Transaction id is Invalid (-1) \n");

														/**********************Until Offline functionality is implemented***********/
														// Resume namespace(Preferences)
														resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
														if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
														{
															// requestLed(GREEN, START, 1); // temp fix
														}
														resumeTxn_A.putBool("ongoingTxn_A", false);
														// resumeTxn_A.putString("idTagData_A", "");
														// resumeTxn_A.putInt("TxnIdData_A", -1);
														// resumeTxn_A.putInt("reasonforstop_Off",reasonForStop);
														if (webSocketConncted)
														{
															resumeTxn_A.putInt("reasonforstop_Off", 3);
														}
														Serial.println("------Is the session running? ----- ");
														Serial.println(resumeTxn_A.getBool("ongoingTxn_A", false));
														Serial.println(resumeTxn_A.getInt("TxnIdData", -2));
														resumeTxn_A.end();
													}

													else if (txnId != prevTxnId_A)
													{
														OcppOperation* stopTransaction = makeOcppOperation(&webSocket, new StopTransaction(currentIdTag_A, txnId, connector));
														initiateOcppOperation(stopTransaction);
														prevTxnId_A = txnId;
														if (DEBUG_OUT)
															Serial.print("EVSE_A Closing Relays.\n");

#if ALPR_ENABLED
														alprTxnStopped();
#endif
#if LCD_ENABLED
														lcd.clear();
														lcd.setCursor(0, 2);
														// lcd.print("STOP SESSION");
														lcd.print("SESSION STOPPED");
#endif
														LastPresentEnergy_A = (discurrEnergy_A) * 1000 + LastPresentEnergy_A;


														/**********************Until Offline functionality is implemented***********/
														//Resume namespace(Preferences)
														resumeTxn_A.begin("resume_A", false); // opening preferences in R/W mode
														// if(getChargePointStatusService_A()->getEmergencyRelayClose() == false){
														// 	// requestLed(GREEN,START,1);   //temp fix
														// }
														resumeTxn_A.putBool("ongoingTxn_A", false);
														if (webSocketConncted)
														{
															resumeTxn_A.putInt("reasonforstop_Off", 3);
														}
														// resumeTxn_A.putString("idTagData_A","");
														// resumeTxn_A.putInt("TxnIdData_A", -1);
														// resumeTxn_A.putInt("reasonforstop_Off",reasonForStop);
														Serial.println("------Is the session running? ----- ");
														Serial.println(resumeTxn_A.getBool("ongoingTxn_A", false));
														Serial.println(resumeTxn_A.getInt("TxnIdData", -2));
														resumeTxn_A.end();

														// Clear the flag for on going transaction
														ongoingTxn_A = false;

														if (getChargePointStatusService_A()->getEmergencyRelayClose() == false) {
															// requestLed(GREEN,START,1);   //temp fix
														}

														if (ethernet_connect)
														{
															if (!webSocketConncted)
															{
																flag_evseReadIdTag_A = true;
																flag_evseAuthenticate_A = false;
																flag_evseStartTransaction_A = false;
																flag_evRequestsCharge_A = false;
																flag_evseStopTransaction_A = false;
																flag_evseUnauthorise_A = false;
																Serial.println("Clearing Stored ID tag in StopTransaction()");
															}
														}

														else if (wifi_connect == true) {
															if (!webSocketConncted || WiFi.status() != WL_CONNECTED || isInternetConnected == false) {
																getChargePointStatusService_A()->stopEnergyOffer();
																getChargePointStatusService_A()->stopEvDrawsEnergy();
																getChargePointStatusService_A()->stopTransaction();
																getChargePointStatusService_A()->unauthorize();  //can be buggy 
																//	getChargePointStatusService_B()->setUnavailabilityStatus(false);
																flag_evseReadIdTag_A = true;
																flag_evseAuthenticate_A = false;
																flag_evseStartTransaction_A = false;
																flag_evRequestsCharge_A = false;
																flag_evseStopTransaction_A = false;
																flag_evseUnauthorise_A = false;
																gu8_finishing_state = 1;
																Serial.println("Clearing Stored ID tag in StopTransaction()");
															}
														}
														else if (gsm_connect == true) {
															if (client.connected() == false) {
																getChargePointStatusService_A()->stopEnergyOffer();
																getChargePointStatusService_A()->stopEvDrawsEnergy();
																getChargePointStatusService_A()->stopTransaction();
																getChargePointStatusService_A()->unauthorize();  //can be buggy 
																//	getChargePointStatusService_B()->setUnavailabilityStatus(false);
																flag_evseReadIdTag_A = true;
																flag_evseAuthenticate_A = false;
																flag_evseStartTransaction_A = false;
																flag_evRequestsCharge_A = false;
																flag_evseStopTransaction_A = false;
																flag_evseUnauthorise_A = false;
																gu8_finishing_state = 1;
																Serial.println("Clearing Stored ID tag in StopTransaction()");
															}
														}

														requestForRelay(STOP, 1);

														delay(500);
														requestforCP_OUT(STOP);
														Serial.println("stop cp 6");
														if (gu8_online_flag == 1)
														{
															stopTransaction->setOnReceiveConfListener([](JsonObject payload) {
#if LCD_ENABLED
																lcd.clear();
																lcd.setCursor(0, 2);
																// lcd.print("STOP SUCCESS");
																lcd.print("SESSION STOPPED");
#endif
																flag_evseReadIdTag_A = false;
																flag_evseAuthenticate_A = false;
																flag_evseStartTransaction_A = false;
																flag_evRequestsCharge_A = false;
																flag_evseStopTransaction_A = false;
																flag_evseUnauthorise_A = true;
																getChargePointStatusService_A()->stopEvDrawsEnergy();
																getChargePointStatusService_A()->stopEnergyOffer();
																getChargePointStatusService_A()->stopTransaction();
																//	getChargePointStatusService_B()->setUnavailabilityStatus(false);
																getChargePointStatusService_A()->unauthorize();
																gu8_finishing_state = 1;
																if (DEBUG_OUT) Serial.print("EVSE Closing Relays. StopTransaction was successful\n");
#if POWER_RECYCLE
																if (resume_stop_start_txn_A == 1)
																{
																	resume_stop_start_txn_A = 2;

																	flag_evseReadIdTag_A = false;
																	flag_evseAuthenticate_A = false;
																	flag_evseStartTransaction_A = true; // Entry condition for starting transaction.
																	flag_evRequestsCharge_A = false;
																	flag_evseStopTransaction_A = false;
																	flag_evseUnauthorise_A = false;


																}
#endif

																});
														}
														else {

															flag_evseReadIdTag_A = false;
															flag_evseAuthenticate_A = false;
															flag_evseStartTransaction_A = false;
															flag_evRequestsCharge_A = false;
															flag_evseStopTransaction_A = false;
															flag_evseUnauthorise_A = true;
															getChargePointStatusService_A()->stopEvDrawsEnergy();
															getChargePointStatusService_A()->stopEnergyOffer();
															getChargePointStatusService_A()->stopTransaction();
															//	getChargePointStatusService_B()->setUnavailabilityStatus(false);
															getChargePointStatusService_A()->unauthorize();
															gu8_finishing_state = 1;
															if (DEBUG_OUT) Serial.print("EVSE Closing Relays. StopTransaction was successful\n");
#if POWER_RECYCLE
															if (resume_stop_start_txn_A == 1)
															{
																resume_stop_start_txn_A = 2;

																flag_evseReadIdTag_A = false;
																flag_evseAuthenticate_A = false;
																flag_evseStartTransaction_A = true; // Entry condition for starting transaction.
																flag_evRequestsCharge_A = false;
																flag_evseStopTransaction_A = false;
																flag_evseUnauthorise_A = false;


															}
#endif
														}
													}
													else
													{
														Serial.println("[EVSE_A] StopTransaction already called. Skipping make OCPP operation to avoid duplication");
													} });

													EVSE_A_setOnUnauthorizeUser([]()
														{
															if (flag_evseSoftReset_A == true) {
																//This 'if' block is developed by @Wamique.
																flag_evseReadIdTag_A = false;
																flag_evseAuthenticate_A = false;
																flag_evseStartTransaction_A = false;
																flag_evRequestsCharge_A = false;
																flag_evseStopTransaction_A = false;
																flag_evseUnauthorise_A = false;
																flag_rebootRequired_A = true;
																getChargePointStatusService_A()->unauthorize();
																if (DEBUG_OUT) Serial.println("EVSE Initiating Soft reset");
															}
															else if (flag_evseSoftReset_A == false) {
																flag_evseReadIdTag_A = true;
																flag_evseAuthenticate_A = false;
																flag_evseStartTransaction_A = false;
																flag_evRequestsCharge_A = false;
																flag_evseStopTransaction_A = false;
																flag_evseUnauthorise_A = false;
																if (DEBUG_OUT) Serial.print("EVSE Unauthorizing user \n");
																getChargePointStatusService_A()->unauthorize();

															} });
}

/*********************************************************************/
void EVSE_A_initialize()
{
	// if (DEBUG_OUT)
	// 	Serial.print(F("[EVSE] EVSE is powered on or reset. Starting Boot.\n"));
	// onBoot_A();

	if (gu8_online_flag == 1)
	{
		if (DEBUG_OUT)
		{
			Serial.print("[EVSE] EVSE is powered on or reset. Starting Boot.\n");
		}
		// onBoot_A();
		evse_boot_state = EVSE_BOOT_INITIATED;
	}
	else
	{
		if (DEBUG_OUT)
		{
			Serial.print("[EVSE] EVSE is powered on or reset.\n");
		}
	}

	faultTimer_A = millis();
}

// This is the main loop function which is controlling the whole charfing process. All the flags created are used to control the flow of the program.

void EVSE_A_loop()
{

	ControlP_loop();

	if (flag_evseIsBooted_A == false)
	{
#if 0
		if (DEBUG_OUT)
			Serial.println(F("[EVSE] Booting..."));
		delay(1000);
		// onBoot();
		t = millis();

		if (onBoot_A != NULL)
		{
			if (millis() - timerHb > (heartbeatInterval * 1000))
			{
				timerHb = millis();
				onBoot_A();
			}
		}

		return;
#endif
		switch (evse_boot_state)
		{
		case EVSE_BOOT_INITIATED:
		{
			Serial.println("CP LOOP: boot notification ");
			OcppOperation* bootNotification = makeOcppOperation(&webSocket, new BootNotification());
			initiateOcppOperation(bootNotification);
			meteringService->init(meteringService);
#if 0
			bootNotification->setOnReceiveConfListener([](JsonObject payload)
				{
					if (DEBUG_OUT)
						Serial.print(F("EVSE_setOnBoot Callback: boot massge recived.\n"));
					// gu8_evse_change_state = EVSE_READ_RFID;
					meteringService->init(meteringService); });
#endif
			evse_boot_state = EVSE_BOOT_SENT;
			Serial.print("\r\nevse_boot_state  :EVSE_BOOT_INITIATED\r\n");
			break;
		}
		case EVSE_BOOT_SENT:
		{
			if (gu8_state_change == 0)
			{
				gu8_state_change = 1;
				Serial.print("\r\nevse_boot_state :EVSE_BOOT_SENT\r\n");
			}
			break;
		}
		case EVSE_BOOT_ACCEPTED:
		{
			evse_boot_state = EVSE_BOOT_DEFAULT;

			flag_evseIsBooted_A = true;	 // Exit condition for booting.
			flag_evseReadIdTag_A = true; // Entry condition for reading ID Tag.
			flag_evseAuthenticate_A = false;
			flag_evseStartTransaction_A = false;
			flag_evseStopTransaction_A = false;
			flag_evseUnauthorise_A = false;
			Boot_Accepted = 1;
			// evse_ChargePointStatus = Available;
			if ((ongoingTxn_A == 0) && (stopoffline_A == 0))
			{
				if ((evse_ChargePointStatus != Preparing) && evse_ChargePointStatus != Faulted)
				{
					evse_ChargePointStatus = Available;
				}
			}
			if (ongoingTxn_A)
			{
				// flag_evRequestsCharge_A = true;            temporary comment for testing
				getChargePointStatusService_A()->startTransaction(transactionId_A);
				reasonForStop = 5;
				EVSE_A_StopSession();
				Serial.println("[EVSE_A] EVSE_A_StopSession 6");
				if (DEBUG_OUT)
					Serial.println("Ongoing transaction is true");
				resume_stop_start_txn_A = 1;
			}
			else
			{
				flag_evRequestsCharge_A = false;
				// resume_stop_start_txn_A = 0 ;
				if (DEBUG_OUT)
					Serial.println("Ongoing transaction is false");
			}

			///......................
			gu8_state_change = 0;
			gu8_bootsuccess = 1;
			Serial.print("\r\nevse_boot_state  :EVSE_BOOT_ACCEPTED\r\n");
			break;
		}
		case EVSE_BOOT_REJECTED:
		{
			evse_boot_state = EVSE_BOOT_INITIATED;
			Serial.print("\r\nevse_boot_state  :EVSE_BOOT_REJECTED\r\n");
			break;
		}
		default:
			break;
		}
	}
	else if (flag_evseIsBooted_A == true)
	{
		if (flag_evseReadIdTag_A == true)
		{
			if (onReadUserId_A != NULL)
			{
				onReadUserId_A();
				/*if(millis() - t > 5000){
					t= millis();
				//	onSendHeartbeat_A();
				}*/

				if (millis() - timerHb > (heartbeatInterval * 1000))
				{
					timerHb = millis();
					onSendHeartbeat_A();
				}
			}
			return;
		}
		else if (flag_evseAuthenticate_A == true)
		{
			if (onAuthentication_A != NULL)
			{
				// Add condition by checking if available or unavailable
				bool un = false;
				un = getChargePointStatusService_A()->getUnavailable();
				if (!un)
					onAuthentication_A();
				else
				{
					flag_evseReadIdTag_A = true; // Looping back read block as no ID tag present.
					flag_evseAuthenticate_A = false;
					flag_evseStartTransaction_A = false;
					flag_evRequestsCharge_A = false;
					flag_evseStopTransaction_A = false;
					flag_evseUnauthorise_A = false;
				}
			}
			return;
		}
		else if (flag_evseStartTransaction_A == true)
		{
			if (onStartTransaction_A != NULL)
			{
#if CP_A_ACTIVE
				// if((EVSE_state == STATE_B || EVSE_state == STATE_C || EVSE_state == STATE_D) && getChargePointStatusService_A()->getEmergencyRelayClose() == false){
				if ((EVSE_state == STATE_B) && getChargePointStatusService_A()->getEmergencyRelayClose() == false)
				{
					onStartTransaction_A();
				}
				else
				{
					Serial.println("Connect the Connector to EV / Or fault exist"); // here have to add timeout of 30 sec
					connectorDis_counter_A++;
#if LCD_ENABLED
					lcd.clear();
					lcd.setCursor(3, 0);
					lcd.print("CONNECT EV /");
					lcd.setCursor(3, 1);
					lcd.print("FAULT EXISTS");
#endif
					// EVSE_stopTransactionByRfid();
					if (connectorDis_counter_A > 25)
					{
						connectorDis_counter_A = 0;
						if (reasonForStop != 3 || reasonForStop != 4)
							reasonForStop = 1;
						EVSE_A_StopSession();
						Serial.println("[EVSE_A] EVSE_A_StopSession 7");
					}
				}
#endif

#if !CP_A_ACTIVE
				onStartTransaction_A();
#endif
			}
		}
		else if (flag_evRequestsCharge_A == true)
		{

#if CP_A_ACTIVE
			// flag_evRequestsCharge = false;
			if (getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getEvDrawsEnergy() == false)
			{

				/***********************Control Pilot @Wamique******************/
				if (EVSE_state == STATE_C || EVSE_state == STATE_D)
				{
					if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
					{
						if (notFaulty_A == true)
							EVSE_A_StartCharging();
					}
					else if (getChargePointStatusService_A()->getEmergencyRelayClose() == true)
					{
						Serial.println("FAULTY CONDITION DETECTED.");
					}
				}
				// else if (EVSE_state == STATE_SUS)
				// {
				// 	EVSE_A_Suspended();
				// 	Serial.print("[EVSE_A] stopped-suspended count:");
				// 	Serial.println(counter1_A);
				// 	// if (counter1_A++ > 25)
				// 	if (counter1_A++ > 600)
				// 	{ // Have to implement proper timeout
				// 		Serial.println("[EVSE_A] stopped-suspended state.");
				// 		counter1_A = 0;
				// 		reasonForStop = 1;
				// 		EVSE_A_StopSession();
				// 	}
				// }
				// else if (EVSE_state == STATE_DIS || EVSE_state == STATE_E || EVSE_state == STATE_B || EVSE_state == STATE_A)   //commented by shiva
				// else if (EVSE_state == STATE_DIS || EVSE_state == STATE_B || EVSE_state == STATE_A)
				// {
				else if (EVSE_state == STATE_DIS)
				{

					//	EVSE_StopSession();     // for the very first time cable can be in disconnected state

					// if(txn == true){           // can implement counter > 10 just to remove noise
					if (reasonForStop != 3 || reasonForStop != 4)
						reasonForStop = 1;
					EVSE_A_StopSession();//commented by sai for lbs
					//   Serial.println("[EVSE_A] EVSE_A_StopSession 8");
					//	}
				}
				else
				{

					Serial.println("[EVSE] STATE Error" + String(EVSE_state));
					delay(2000);

					//	requestLed(RED,START,1);
				}
			}
			if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
			{

				//	txn = true;

				if (EVSE_state == STATE_C || EVSE_state == STATE_D)
				{
					disp_evse_A = true;
					if (DEBUG_OUT)
						Serial.println("[EVSE_CP] Drawing Energy");
#if 0
					/*
					* @brief : Added a check for low current. Ignoring this feature as it is no longer required.
					G. Raja Sumant 17/06/2022
					*/
					// Current check
					drawing_current_A = eic.GetLineCurrentA();
					Serial.println("Current A: " + String(drawing_current_A));
					if (drawing_current_A <= minCurr)
					{
						counter_drawingCurrent_A++;
						if (counter_drawingCurrent_A > currentCounterThreshold_A)
						{
							counter_drawingCurrent_A = 0;
							if (reasonForStop != 3 || reasonForStop != 4)
								reasonForStop = 1;
							Serial.println(F("Stopping session due to No current"));
							EVSE_A_StopSession();
						}
					}
					else
					{
						counter_drawingCurrent_A = 0;
						Serial.println(F("counter_drawing Current Reset"));
					}
#endif
					if (millis() - t > 15000)
					{
						if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
						{
							// requestLed(BLINKYGREEN, START, 1);
							// delay(100);
							// requestForRelay(START, 1); // comment by shiva
							t = millis();
						}
					}
					/*
					if(blinckCounter++ % 2 == 0){
						requestLed(GREEN,START,1);
					}else{
						requestLed(GREEN,STOP,1);
					}*/
				}
				// else if (EVSE_state == STATE_A || EVSE_state == STATE_E || EVSE_state == STATE_B) // commented by shiva
				else if (EVSE_state == STATE_A || EVSE_state == STATE_B)
				{ // Although CP Inp will never go to A,B state
					if (counter_faultstate_A++ > 1)
					{
						// EVSE_A_StopSession();  COMMENTED BY FOR LBS
							// Serial.println("[EVSE_A] EVSE_A_StopSession 9");
						counter_faultstate_A = 0;
					}
				}
				// else if (EVSE_state == STATE_SUS)
				// {
				// 	/* added relay stop for whenever EVSE state into SUSpended.  */
				// 	// requestForRelay(STOP, 1);  //commented by shiva for testing

				// 	/*
				// 	 * @brief : To avoid immediate off in suspended state.
				// 	 * Making the count to 600.
				// 	 * By G. Raja Sumant 07/06/2022 according to the observation in Keya Homes for Mercedes benz EQC car.
				// 	 */

				// 	if (counter_suspendedstate_A++ > 600)
				// 	{
				// 		Serial.println("[EVSE_A] suspended for > 600 cycles");
				// 		EVSE_A_Suspended();
				// 		counter_suspendedstate_A = 0;
				// 	}
				// 	// EVSE_A_Suspended();    //pause transaction :update suspended state is considered in charging state
				// }
				else if (EVSE_state == STATE_DIS)
				{

					Serial.println("[EVSE] Connect the Connector with EV and Try again");
					reasonForStop = 1;
					EVSE_A_StopSession();
					Serial.println("[EVSE_A] EVSE_A_StopSession 10");
				}
			}

			/***Implemented Exit Feature with RFID @Wamique****/
//	  EVSE_stopTransactionByRfid_A();
#endif

#if !CP_A_ACTIVE
			if (getChargePointStatusService_A() != NULL && relayTriggered_A == false /*&& getChargePointStatusService_A()->getEvDrawsEnergy() == false*/)
			{
				if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
				{
					// getChargePointStatusService_A()->startEvDrawsEnergy();
					relayTriggered_A = true;
					if (DEBUG_OUT)
						Serial.print(F("[EVSE_A] Opening Relays.\n"));
					requestForRelay(START, 1);
					requestLed(ORANGE, START, 1);
					delay(1200);
					requestLed(WHITE, START, 1);
					delay(1200);
					requestLed(GREEN, START, 1);
					delay(1000);
					if (DEBUG_OUT)
						Serial.println(F("[EVSE_A] Started Drawing Energy"));
				}
				else if (getChargePointStatusService_A()->getEmergencyRelayClose() == true)
				{
					Serial.println(F("The voltage or current is out or range. FAULTY CONDITION DETECTED."));
				}
			}
			if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
			{
				// delay(250);

				if (DEBUG_OUT)
					Serial.println(F("[EVSE_A] Drawing Energy"));

				// blinking green Led
				if (millis() - t > 5000)
				{
					// if((WiFi.status() == WL_CONNECTED) && (webSocketConncted == true) && (isInternetConnected == true)&& getChargePointStatusService()->getEmergencyRelayClose() == false){
					// 	requestLed(BLINKYGREEN_EINS,START,1);
					// 	t = millis();
					// }
					//	onSendHeartbeat_A();
					if (getChargePointStatusService_A()->getEmergencyRelayClose() == false)
					{
						requestLed(BLINKYGREEN, START, 1);
						t = millis();

						if (millis() - relay_timer_A > 15000)
						{

							requestForRelay(START, 1);
							relay_timer_A = millis();
						}
					}
				}
				// Current check
				drawing_current_A = eic.GetLineCurrentA();
				Serial.println("Current A: " + String(drawing_current_A));
				if (drawing_current_A <= 0.15)
				{
					counter_drawingCurrent_A++;
					if (counter_drawingCurrent_A > 120)
					{
						counter_drawingCurrent_A = 0;
						Serial.println(F("Stopping session due to No current"));

						EVSE_A_StopSession();
					}
				}
				else
				{
					counter_drawingCurrent_A = 0;
					Serial.println(F("counter_drawing Current Reset"));
				}
			}
			// Implemented Exit Feature with RFID @Wamique//
// EVSE_A_stopTransactionByRfid();
#endif
			// this is the only 'else if' block which is calling next else if block. the control is from this file itself. the control is not changed from any other file. but the variables are required to be present as extern in other file to decide calling of other functions.
			return;
		}
		else if (flag_evseStopTransaction_A == true)
		{
			if (getChargePointStatusService_A() != NULL)
			{
				// getChargePointStatusService_A()->stopEvDrawsEnergy();
				// getChargePointStatusService_A()->stopEnergyOffer();
			}
			if (onStopTransaction_A != NULL)
			{
				onStopTransaction_A();
				relayTriggered_A = false;
#if CP_A_ACTIVE
				requestforCP_OUT(STOP); // stop pwm
				Serial.println("stop cp 7");
#endif
			}
			return;
		}
		else if (flag_evseUnauthorise_A == true)
		{
			if (onUnauthorizeUser_A != NULL)
			{
				onUnauthorizeUser_A();
#if CP_A_ACTIVE
				requestforCP_OUT(STOP); // stop pwm
				Serial.println("stop cp 8");
#endif
			}
			return;
		}
		else if (flag_rebootRequired_A == true /*&& flag_rebootRequired_B == true && flag_rebootRequired_C == true*/)
		{
			// soft reset execution.
			//  flag_evseIsBooted_A = false;
			//  flag_rebootRequired_A = false;
			//  flag_evseSoftReset_A = false;
			if (getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Charging
				// getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Charging
				//	getChargePointStatusService_C()->inferenceStatus() == ChargePointStatus::Charging
				)
			{
				if (DEBUG_OUT)
					Serial.print("[EVSE_A] rebooting in 5 seconds...\n");
				delay(5000);
				ESP.restart();
			}
		}
		else
		{
			if (DEBUG_OUT)
				Serial.print("[EVSE_A] waiting for response...\n");
			delay(100);
		}
	}
}

void EVSE_Reservation_loop()
{
#if 0
	int i = 0;
	while (i < ReserveNowSlotList.size()) {
		reserveNow_slot* el = ReserveNowSlotList.get(i);
		boolean success = el->isAccepted;
		if (success) {
			flag_evseReserveNow = true;
			currentIdTag = el->idTag;
			connectorId = el->connectorId;
			reservationId = el->reservationId;
			reservation_start_time = el->start_time;
			reservation_expiry_time = el->expiry_time;

			ReserveNowSlotList.remove(i);
			//TODO Review: are all recources freed here?
			delete el;
			//go on with the next element in the queue, which is now at ReserveNowSlotList[i]
		}
		else {
			//There will be another attempt to send this conf message in a future loop call.
			//Go on with the next element in the queue, which is now at ReserveNowSlotList[i+1]
			i++;
		}
	}
#endif

	if (flag_evseReserveNow)
	{
		// Reserve Now execution.
		if (DEBUG_OUT)
			Serial.print("[EVSE] Reserve Now ...\n");

		// time_t start_reserve_time_delta = reservation_start_time - now();
		time_t reserve_time_delta = reserveDate - now();
#if 0
		if ((start_reserve_time_delta <= 30) && start_reserve_time_delta > 0)
		{

			if (DEBUG_OUT)
				Serial.print(F("[EVSE] Reserve Now  1 ...\n"));
			Serial.print(F("[EVSE] start Reserve time delta "));
			Serial.println(start_reserve_time_delta);

		}
		else
#endif
			/*
			 * @brief : Fixed the bug by Shiva Poola 26/11/2022
			 * Reserve now changed from fixed time to dynamic time
			 */
			if ((reserve_time_delta <= reservedDuration) && (reserve_time_delta > 0))
				/*
				 * @brief : If you are using steve, uncomment the below and comment above
				 */
				 // if ((reserve_time_delta <= reserveDate) && (reserve_time_delta > 0))
			{
				/*
				 * @brief : Trigger a status notification of Reserved only once.
				 */
#if 0
				 //fire StatusNotification
			 //TODO check for online condition: Only inform CS about status change if CP is online
			 //TODO check for too short duration condition: Only inform CS about status change if it lasted for longer than MinimumStatusDuration
				OcppOperation* statusNotification = makeOcppOperation(webSocket,
					new StatusNotification(currentStatus));
				initiateOcppOperation(statusNotification);
#endif

				if (lu8_send_status_flag && reservation_start_flag)
				{
					lu8_send_status_flag = false;
					getChargePointStatusService_A()->setReserved(true);
					evse_ChargePointStatus = Reserved;
				}

				if (!reservation_start_flag)
				{

					if (getChargePointStatusService_A()->getTransactionId() != -1)
					{
						EVSE_A_StopSession();
						Serial.println("[EVSE_A] EVSE_A_StopSession 11");
					}
					// requestLed(BLUE, START, 1);
					reservation_start_flag = true;
					flag_evseReadIdTag_A = true; // Entry condition for reading ID Tag.
					flag_evseAuthenticate_A = false;
					lu8_send_status_flag = true;
					if (DEBUG_OUT)
						Serial.print("[EVSE] Reserve Now  2 1 ...\n");
				}
				if (DEBUG_OUT)
					Serial.print("[EVSE] Reserve Now  2 ...\n");
				Serial.print("[EVSE] reserve time delta ");
				Serial.println(reserve_time_delta);
				if (getChargePointStatusService_A()->getEvDrawsEnergy() == false && notFaulty_A && getChargePointStatusService_A()->getEmergencyRelayClose() == false)
				{
					// requestLed(BLUE, START, 1);
				}
#if 0
				/*
				 * @brief : Feature added by G. Raja Sumant 29/07/2022
				 * This will take the charge point to reserved state when ever it is available during the reservation loop
				 */
				if (reservation_start_flag)
				{
					if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available)
					{
						getChargePointStatusService_A()->setReserved(true);
					}
				}
#endif
			}
			else
			{
				if ((reserve_time_delta <= 0))
				{
					flag_evseReserveNow = false;
					getChargePointStatusService_A()->setReserved(false);
					/*
					* @bug : Session should not stop just because the reservation expired.
					Resolved by G. Raja Sumant 20/07/2022
					if (getChargePointStatusService_A()->getTransactionId() != -1)
					{
						EVSE_StopSession();
					}*/
					if (DEBUG_OUT)
						Serial.print("[EVSE]stopped-Reservation timeout.....!\n");
					reservation_start_flag = false;
				}
			}
		if (flag_evseCancelReservation)
		{
			flag_evseCancelReservation = false;
			flag_evseReserveNow = false;
			getChargePointStatusService_A()->setReserved(false);
			// if(getChargePointStatusService_A()->getTransactionId() != -1)
			// {
			// 	EVSE_StopSession();
			// }
			if (DEBUG_OUT)
				Serial.print("[EVSE]stopped due to Cancel Reservation.....!\n");
			reservation_start_flag = false;
		}
	}
}

// short EMGCY_counter_A =0;
// bool EMGCY_FaultOccured_A = false;

// void checkEmgcy_Gfci(){
// 	if(millis() - TimerEmcy > 2000){
// 		bool EMGCY_status = requestEmgyStatus();
// 		Serial.println("EMGCY Status: " +String(EMGCY_status));
// 		delay(200);
// 		bool GFCI_status = requestGFCIStatus();
// 		Serial.println("GFCI Status: "+ String(GFCI_status));

// 		if((EMGCY_status == true || GFCI_status == true)){
// 			Serial.println("EMGCY: ")
// 			requestLed(BLINKYRED,START,1);
// 			getChargePointStatusService_A()->setEmergencyRelayClose(true);
// 			EMGCY_GFCI_Fault_Occ = true;
// 		}else{

// 			Serial.println("No Fault");
// 			getChargePointStatusService_A()->setEmergencyRelayClose(false);
// 			EMGCY_GFCI_Fault_Occ = false;
// 		}

// 	}
// }

void emergencyRelayClose_Loop_A()
{

	/*
						* @brief : getOverCurrent - Stop toggling. Moving this to emergency loop.
						By G. Raja Sumant 07/06/2022
						To avoid toggling of over current and stopping of transaction

						*/
	if (getChargePointStatusService_A()->getOverCurrent() == true)
	{
#if 0
		lcd.clear();
		lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
		lcd.print("STATUS: FAULTED");
		lcd.setCursor(0, 1);
		lcd.print("A: OVER CURRENT");
#endif
		EVSE_A_StopSession();
		Serial.println("[EVSE_A] EVSE_A_StopSession 12");
		/*
		if(!offline_connect)
		{
		if(getChargePointStatusService_A()->getTransactionId() != -1)
		{
		EVSE_A_StopSession();
		}
		}
		else
		{
			if(offline_charging_A)
		EVSE_A_StopSession_offline();
		}
		*/
	}

	if (EMGCY_GFCI_Fault_Occ == true && getChargePointStatusService_A()->getTransactionId() != -1)
	{

		flag_evseReadIdTag_A = false;
		flag_evseAuthenticate_A = false;
		flag_evseStartTransaction_A = false;
		flag_evRequestsCharge_A = false;
		flag_evseStopTransaction_A = true;

		if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
		{
			reasonForStop = 0;
			EVSE_A_StopSession();
			Serial.println("[EVSE_A] EVSE_A_StopSession 13");
		}
	}
	else if (EMGCY_GFCI_Fault_Occ == false)
	{

		if (Electric_paramater_count >= 1000)
		{
		}

		float volt = eic.GetLineVoltageA();
		float current = eic.GetLineCurrentA();
		float temp = eic.GetTemperature();

		Serial.println("Voltage_A: " + String(volt) + ", Current_A: " + String(current) + ", Temperature: " + String(temp));

		if (getChargePointStatusService_A() != NULL)
		{
			if (getChargePointStatusService_A()->getOverVoltage() == true ||
				getChargePointStatusService_A()->getUnderVoltage() == true ||
				getChargePointStatusService_A()->getUnderTemperature() == true ||
				getChargePointStatusService_A()->getOverTemperature() == true ||
				getChargePointStatusService_A()->getEarthDisconnect() == true ||
				getChargePointStatusService_A()->getOverCurrent() == true)
			{
				Serial.println("[EVSE_A] Fault Occurred.");
				getChargePointStatusService_A()->setEmergencyRelayClose(true);
				notFaulty_A = false;
				evse_ChargePointStatus = Faulted;

				gu8_fault_occured = 1;
				if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
				{
					if (getChargePointStatusService_A()->getTransactionId() != -1)
					{
						EVSE_A_StopSession();
						Serial.println("[EVSE_A] EVSE_A_StopSession 14");
					}
				}
				//
#if 0
				if (getChargePointStatusService_A()->getTransactionId() != -1)
				{
					EVSE_A_StopSession();
				}
#endif
				if (!timer_initialize_A)
				{
					timeout_start_A = millis();
					timer_initialize_A = true;
				}
			}
			else if (getChargePointStatusService_A()->getOverVoltage() == false &&
				getChargePointStatusService_A()->getUnderVoltage() == false &&
				getChargePointStatusService_A()->getUnderTemperature() == false &&
				getChargePointStatusService_A()->getOverTemperature() == false &&
				getChargePointStatusService_A()->getEarthDisconnect() == false &&
				getChargePointStatusService_A()->getOverCurrent() == false)
			{
				Serial.println("[EVSE_A] Not Faulty.");
				notFaulty_A = true;
				client_reconnect_flag = 2;
				getChargePointStatusService_A()->setEmergencyRelayClose(false);
				if (gu8_fault_occured == 1)
				{
					if (flag_evRequestsCharge_A == false)
					{
						gu8_fault_occured = 0;
						evse_ChargePointStatus = Available;
					}
					else if (flag_evRequestsCharge_A == true)
					{
						gu8_fault_occured = 0;
						evse_ChargePointStatus = Charging;
					}
				}
				// if (!timer_initialize){
				timeout_start_A = 0;
				timer_initialize_A = false;
				//}
			}

			if (getChargePointStatusService_A()->getEmergencyRelayClose() == true)
			{
				timeout_active_A = true;
				disp_evse_A = false;
				// requestForRelay(STOP, 1);  //commented by shiva for testing
				// reasonForStop = 0;
				delay(50);
#if LED_ENABLED
				requestLed(RED, START, 1);
#endif

				flag_faultOccured_A = true;
			}
			else if (getChargePointStatusService_A()->getEmergencyRelayClose() == false && flag_faultOccured_A == true)
			{
				timeout_active_A = false;
				if ((getChargePointStatusService_A()->getTransactionId() != -1))
				{ // can be buggy
					if (fault_counter_A++ > 1)
					{
						fault_counter_A = 0;
						// requestForRelay(START, 1);
						delay(50);
						// Serial.println(F("[EmergencyRelay_A] Starting Txn"));
						flag_evRequestsCharge_A = true;
						evse_ChargePointStatus = Charging;
						getChargePointStatusService_A()->startEvDrawsEnergy();
						getChargePointStatusService_A()->setEmergencyRelayClose(false);
						flag_faultOccured_A = false;
					}
				}
			}

			if (timeout_active_A && getChargePointStatusService_A()->getTransactionId() != -1)
			{
				if (millis() - timeout_start_A >= TIMEOUT_EMERGENCY_RELAY_CLOSE)
				{
					Serial.println("[EVSE_A] Emergency Stop.");
					flag_evRequestsCharge_A = false;
					flag_evseStopTransaction_A = true;
					timeout_active_A = false;
					timer_initialize_A = false;
				}
			}
		}
	}
}

/*
 * @param limit: expects current in amps from 0.0 to 32.0
 */
void EVSE_A_setChargingLimit(float limit)
{
	if (DEBUG_OUT)
		Serial.print("[EVSE] New charging limit set. Got ");
	if (DEBUG_OUT)
		Serial.print(limit);
	if (DEBUG_OUT)
		Serial.print("\n");
	chargingLimit_A = limit;
}

bool EVSE_A_EvRequestsCharge()
{
	return flag_evRequestsCharge_A;
}

bool EVSE_A_EvIsPlugged()
{
	return evIsPlugged_A;
}

void EVSE_A_setOnBoot(OnBoot_A onBt_A)
{
	onBoot_A = onBt_A;
}

void EVSE_A_setOnReadUserId(OnReadUserId_A onReadUsrId_A)
{
	onReadUserId_A = onReadUsrId_A;
}

void EVSE_A_setOnsendHeartbeat(OnSendHeartbeat_A onSendHeartbt_A)
{
	onSendHeartbeat_A = onSendHeartbt_A;
}

void EVSE_A_setOnAuthentication(OnAuthentication_A onAuthenticatn_A)
{
	onAuthentication_A = onAuthenticatn_A;
}

void EVSE_A_setOnStartTransaction(OnStartTransaction_A onStartTransactn_A)
{
	onStartTransaction_A = onStartTransactn_A;
}

void EVSE_A_setOnStopTransaction(OnStopTransaction_A onStopTransactn_A)
{
	onStopTransaction_A = onStopTransactn_A;
}

void EVSE_A_setOnUnauthorizeUser(OnUnauthorizeUser_A onUnauthorizeUsr_A)
{
	onUnauthorizeUser_A = onUnauthorizeUsr_A;
}

// void EVSE_A_getSsid(String &out) {
// 	out += "Pied Piper";
// }
// void EVSE_getPass(String &out) {
// 	out += "plmzaq123";
// }

void EVSE_A_getChargePointSerialNumber(String& out)
{

	out += preferences.getString("chargepoint", "");

	/*
	#if STEVE
	out += "dummyCP002";
	#endif

	#if EVSECEREBRO
	out += "testpodpulkit";
	#endif
	*/
}

#if 0
char* EVSE_A_getChargePointVendor()
{
	return CHARGE_POINT_VENDOR;
}

char* EVSE_A_getChargePointModel()
{
	return CHARGE_POINT_MODEL;
}
#endif

String EVSE_A_getCurrnetIdTag(MFRC522* mfrc522)
{
	String currentIdTag = "";
	//	currentIdTag = EVSE_A_readRFID(mfrc522);    // masking rfid direct read from EVSE_A

	if (getChargePointStatusService_A()->getIdTag().isEmpty() == false)
	{
		if (DEBUG_OUT)
			Serial.println("[EVSE] Reading from Charge Point Station Service ID Tag stored.");
		currentIdTag = getChargePointStatusService_A()->getIdTag();
		if (DEBUG_OUT)
			Serial.print("[EVSE] ID Tag: ");
		if (DEBUG_OUT)
			Serial.println(currentIdTag);
		Serial.flush();
	}

	return currentIdTag;
}

String EVSE_A_readRFID(MFRC522* mfrc522)
{
	String currentIdTag_A;
	currentIdTag_A = readRfidTag(true, mfrc522);
	return currentIdTag_A;
}

/********Added new funtion @Wamique***********************/

// void EVSE_A_stopTransactionByRfid(){

// 	Ext_currentIdTag_A = EVSE_A_readRFID(&mfrc522);
// 	if(currentIdTag_A.equals(Ext_currentIdTag_A) == true){
// 		flag_evRequestsCharge_A = false;
// 		flag_evseStopTransaction_A = true;
// 	}else{
// 			if(Ext_currentIdTag_A.equals("")==false)
// 			if(DEBUG_OUT) Serial.println("\n[EVSE_A] Incorrect ID tag\n");
// 		}
// }

#if CP_A_ACTIVE
/**************CP Implementation @mwh*************/
void EVSE_A_StartCharging()
{

	if (getChargePointStatusService_A()->getEvDrawsEnergy() == false)
	{
		getChargePointStatusService_A()->startEvDrawsEnergy();
	}
	if (DEBUG_OUT)
		Serial.print("[EVSE_A] Opening Relays.\n");
	//   pinMode(32,OUTPUT);
	//  digitalWrite(32, HIGH); //RELAY_1
	// digitalWrite(RELAY_2, RELAY_HIGH);
	// requestForRelay(START, 1);
	// requestLed(ORANGE, START, 1);
	// delay(1200);
	// requestLed(WHITE, START, 1);
	// delay(1200);
	// requestLed(GREEN, START, 1);
	// delay(1000);
	Serial.println("[EVSE_A] EV is connected and Started charging");
	if (DEBUG_OUT)
		Serial.println("[EVSE_A] Started Drawing Energy");
	delay(500);
}

void EVSE_A_Suspended()
{

	if (getChargePointStatusService_A()->getEvDrawsEnergy() == true)
	{
		getChargePointStatusService_A()->stopEvDrawsEnergy();
	}
	// requestLed(BLUE, START, 1);
	// requestForRelay(STOP, 1);  //commented by shiva for testing
	//	delay(1000);
	Serial.println("[EVSE_A] EV Suspended");
}

/**************************************************/

#endif