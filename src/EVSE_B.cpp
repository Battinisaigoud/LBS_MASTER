#include "EVSE_B.h"
#include "Master.h"
#include "ControlPilot.h"

#if LCD_ENABLED
#include "LCD_I2C.h"
extern LCD_I2C lcd;
#endif

OnBoot_B onBoot_B;
OnReadUserId_B onReadUserId_B;
OnSendHeartbeat_B onSendHeartbeat_B;
OnAuthentication_B onAuthentication_B;
OnStartTransaction_B onStartTransaction_B;
OnStopTransaction_B onStopTransaction_B;
OnUnauthorizeUser_B onUnauthorizeUser_B;

extern bool wifi_reconnected_flag;

#if DWIN_ENABLED
#include "dwin.h"
extern SoftwareSerial dwin;
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
extern unsigned char cid2[8];
#endif

bool evIsPlugged_B; 
bool flag_evseIsBooted_B;
bool flag_evseReadIdTag_B;
bool flag_evseAuthenticate_B;
bool flag_evseStartTransaction_B;
bool flag_evRequestsCharge_B;
bool flag_evseStopTransaction_B;
bool flag_evseUnauthorise_B;
bool flag_rebootRequired_B;
bool flag_evseSoftReset_B;

bool relayTriggered_B = false; 

bool notFaulty_B = false;
bool disp_evse_B = false;

extern bool flag_rebootRequired_C;
extern bool flag_rebootRequired_A;

float chargingLimit_B = 32.0f;
String Ext_currentIdTag_B = "";

long int blinckCounter_B =0;
int counter1_B =0;

ulong t_B;
int connectorDis_counter_B=0;
short int counter_drawingCurrent_B = 0;
float drawing_current_B = 0;

int prevTxnId_B = -1;

String currentIdTag_B = "";
extern WebSocketsClient webSocket;
extern EVSE_states_enum EVSE_state;
extern Preferences preferences;
extern MFRC522 mfrc522;
extern String currentIdTag;
extern bool webSocketConncted;
extern bool isInternetConnected;
extern bool wifi_connect;
extern bool gsm_connect;

Preferences resumeTxn_B;
extern TinyGsmClient client;

extern bool ethernet_enable;
extern bool ethernet_connect;

bool ongoingTxn_B;
String idTagData_B = "";

bool timeout_active_B = false;
bool timer_initialize_B = false;
ulong timeout_start_B = 0;
short int fault_counter_B = 0;
bool flag_faultOccured_B = false;
ulong relay_timer_B = 0;
ulong faultTimer_B =0;

extern bool EMGCY_GFCI_Fault_Occ;
/**********************************************************/
void EVSE_B_StopSession(){

	// if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){
	// 	getChargePointStatusService_B()->stopEvDrawsEnergy();
	// }
	
	//digitalWrite(32, LOW);
    requestForRelay(STOP,2); 
    delay(500);           	
    flag_evseReadIdTag_B = false;
	flag_evseAuthenticate_B = false;
	flag_evseStartTransaction_B = false;
	flag_evRequestsCharge_B = false;
	flag_evseStopTransaction_B = true;
	flag_evseUnauthorise_B = false;
    Serial.println("[EVSE] Stopping Session : " +String(EVSE_state));
}
/**************************************************************************/

/**************************************************************************/

void EVSE_B_initialize() {
	if(DEBUG_OUT) Serial.print("[EVSE_B] Starting Boot.\n");
	//onBoot_B();
	flag_evseIsBooted_B = true; //Exit condition for booting. 	
	flag_evseReadIdTag_B = true; //Entry condition for reading ID Tag.
	faultTimer_B = millis();
}

/**************************SetUp********************************************/
void EVSE_B_setup(){

	resumeTxn_B.begin("resume_B",false); //opening preferences in R/W mode
  	idTagData_B = resumeTxn_B.getString("idTagData_B","");
  	ongoingTxn_B = resumeTxn_B.getBool("ongoingTxn_B",false);

  	Serial.println("Stored ID:"+String(idTagData_B));
  	Serial.println("Ongoing Txn: "+String(ongoingTxn_B));

	EVSE_B_setOnBoot([]() {
		//this is not in loop, that is why we need not update the flag immediately to avoid multiple copies of bootNotification.
		OcppOperation *bootNotification = makeOcppOperation(&webSocket,	new BootNotification());
		initiateOcppOperation(bootNotification);
		bootNotification->setOnReceiveConfListener([](JsonObject payload) {

      	// if( flag_MeteringIsInitialised == false){
      	// 	Serial.println("[SetOnBooT] Initializing metering services");
      	// //	meteringService->init(meteringService);
      	// }

      if (DEBUG_OUT) Serial.print("EVSE Metering Services Initialization finished.\n");

			flag_evseIsBooted_B = true; //Exit condition for booting. 	
			flag_evseReadIdTag_B = true; //Entry condition for reading ID Tag.
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
      if (DEBUG_OUT) Serial.print("EVSE_: Closing Relays.\n");

			if (DEBUG_OUT) Serial.print("EVSE_ Boot successful.\n");
		});
	});

	EVSE_B_setOnReadUserId([] () {
		if (DEBUG_OUT) Serial.print(" EVSE_B waiting for User ID read...\n");
		static ulong timerForRfid = millis();
		currentIdTag_B = "";  
		idTagData_B = resumeTxn_B.getString("idTagData_B","");
  		ongoingTxn_B = resumeTxn_B.getBool("ongoingTxn_B",false);

		if(ethernet_connect && 														//ethernet Block
  			(getChargePointStatusService_B()->getEmergencyRelayClose() == false)&&
  			(webSocketConncted == true) && getChargePointStatusService_B()->getUnavailable() == false)
		{
			if((idTagData_B != "") && (ongoingTxn_B == 1)){
  				currentIdTag_B = resumeTxn_B.getString("idTagData_B", "");
  				Serial.println("[EVSE_B] Resuming Session");
				  #if LCD_ENABLED
  				lcd.clear();
  				lcd.setCursor(3, 2);
  				lcd.print("SESSION B RESUME");
				#endif
				  #if DISPLAY_ENABLED
      			  connAvail(2,"SESSION B RESUME");
  				  checkForResponse_Disp();
				  #endif
  				getChargePointStatusService_B()->authorize(currentIdTag_B); // so that Starttxn can easily fetch this data
  				requestLed(BLUE, START, 2);
  			}else{
				if(millis() - timerForRfid > 10000){ //timer for sending led request
	    		  	requestLed(GREEN,START,2);
	    		  	timerForRfid = millis();
					#if 0
					#if LCD_ENABLED
					//lcd.clear();
					#if 0
					if(ongoingTxn_A)
					{
					lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					}
					if(ongoingTxn_C)
					{
					lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 2);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 3);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					}
					lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
					lcd.print("B: AVAILABLE");
					//lcd.setCursor(0, 1);
					//lcd.print("TAP RFID/SCAN QR");
					//lcd.setCursor(0, 2);
					//lcd.print("CONNECTION");
					lcd.setCursor(0, 3);
					lcd.print("CLOUD: ETH. TAP RFID");
					#endif
					if(disp_evse_A)
					{
					lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      				lcd.print("A: CHARGING"); // Clear the line
					}
					else
					{
					lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 0);               // Or setting the cursor in the desired position.
      				lcd.print("A: AVAILABLE"); // Clear the line
					}
					if(disp_evse_C)
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
					lcd.setCursor(0, 1);               // Or setting the cursor in the desired position.
      				lcd.print("                    "); // Clear the line
					lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
					lcd.print("B: AVAILABLE");
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
			
				currentIdTag_B = EVSE_B_getCurrnetIdTag(&mfrc522);
				Serial.println("[Wifi]**RFID B**");
  			}

		}

  		if((wifi_connect == true)&& 														//Wifi Block
  			(getChargePointStatusService_B()->getEmergencyRelayClose() == false)&&
  			(getChargePointStatusService_B()->getUnavailabilityStatus() == false)&&
  			(webSocketConncted == true)&&
  			(WiFi.status() == WL_CONNECTED)&&
  			(isInternetConnected == true)){

  			if((idTagData_B != "") && (ongoingTxn_B == 1)){
  				currentIdTag_B = resumeTxn_B.getString("idTagData_B", "");
  				Serial.println("[EVSE_B] Resuming Session");
  				getChargePointStatusService_B()->authorize(currentIdTag_B); // so that Starttxn can easily fetch this data
  				requestLed(BLUE, START, 1);
  			}else{
				if(millis() - timerForRfid > 10000){ //timer for sending led request
	    		  	requestLed(GREEN,START,1);
					wifi_reconnected_flag = true;
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_B = EVSE_B_getCurrnetIdTag(&mfrc522);
				Serial.println("[Wifi]**RFID B**");
  			}

  		}else if((gsm_connect == true)&&													//GSM Block
  				(getChargePointStatusService_B()->getEmergencyRelayClose() == false)&&
  				(getChargePointStatusService_B()->getUnavailabilityStatus() == false)&&
  				(client.connected() == true)){

  			if((idTagData_B != "") && (ongoingTxn_B == 1)){
  				currentIdTag_B = resumeTxn_B.getString("idTagData_B", "");
  				getChargePointStatusService_B()->authorize(currentIdTag_B); // so that Starttxn can easily fetch this data
  				Serial.println("[EVSE_B] Resuming Session");
  				requestLed(BLUE, START, 1);
  			}else{
				if(millis() - timerForRfid > 10000){ //timer for sending led request
	    		  	requestLed(GREEN,START,1);
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_B = EVSE_B_getCurrnetIdTag(&mfrc522);
				Serial.println("[GSM]**RFID B**");
  			}


  		}


		/*	
		idTagData_m = resumeTxn.getString("idTagData","");
  		ongoingTxn_m = resumeTxn.getBool("ongoingTxn",false);

		if((ongoingTxn_m == 1) && (idTagData_m != "") && 
		      (getChargePointStatusService_B()->getEmergencyRelayClose() == false) &&
		      (WiFi.status() == WL_CONNECTED)&&
		      (webSocketConncted == true)&&
		      (isInternetConnected == true)){   //giving priority to stored data
			currentIdTag_B = resumeTxn.getString("idTagData","");
			Serial.println("[EVSE_setOnReadUserId] Resuming Session");
      		requestLed(BLUE,START,1);
         
		}else*/ 
		// if((getChargePointStatusService_B()->getEmergencyRelayClose() == false) &&
		//           (WiFi.status() == WL_CONNECTED) &&
		//           (webSocketConncted == true) && 
		//           (isInternetConnected == true)){
		// 	  #if LED_ENABLED
		// 	  if(millis() - timerForRfid > 5000){ //timer for sending led request
  //   		  requestLed(GREEN,START,2);
  //   		  timerForRfid = millis();
  //   		  }
  //   		  #endif
		// 	currentIdTag_B = EVSE_B_getCurrnetIdTag(&mfrc522);
		// 	Serial.println("********RFID B**********");
		// }

		if (currentIdTag_B.equals("") == true) {
			flag_evseReadIdTag_B = true; //Looping back read block as no ID tag present.
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
		} else {
			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = true; //Entry condition for authentication block.
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
			if (DEBUG_OUT) Serial.print("EVSE_B_: Successful User ID Read.\n");
		}	
	});
	
	EVSE_B_setOnsendHeartbeat([] () {
    if (DEBUG_OUT) Serial.print("EVSE Sending heartbeat signal...\n");
    OcppOperation *heartbeat = makeOcppOperation(&webSocket, new Heartbeat());
    initiateOcppOperation(heartbeat); 
    heartbeat->setOnReceiveConfListener([](JsonObject payload) {
        const char* currentTime = payload["currentTime"] | "Invalid";
        if (strcmp(currentTime, "Invalid")) {
          if (setTimeFromJsonDateString(currentTime)) {
            if (DEBUG_OUT) Serial.print("EVSE Request has been accepted!\n");
          } else {
            Serial.print("EVSE Request accepted.\n");
          }
        } else {
          Serial.print("EVSE: Request denied.\n");
        }
    });   
  });
 
	EVSE_B_setOnAuthentication([] () {
		if (DEBUG_OUT) Serial.print("EVSE_B: Authenticating...\n");
		flag_evseAuthenticate_B = false;
		OcppOperation *authorize = makeOcppOperation(&webSocket, new Authorize(currentIdTag_B));
		initiateOcppOperation(authorize);
		/*chargePointStatusService->authorize(currentIdTag_B, connectorId_B);  */    //have to edit
		authorize->setOnReceiveConfListener([](JsonObject payload) {
			const char* status = payload["idTagInfo"]["status"] | "Invalid";
			if (!strcmp(status, "Accepted")) {
				flag_evseReadIdTag_B = false;
				flag_evseAuthenticate_B = false;
				flag_evseStartTransaction_B = true; //Entry condition for starting transaction.
				flag_evRequestsCharge_B = false;
				flag_evseStopTransaction_B = false;
				flag_evseUnauthorise_B = false;
				requestLed(BLUE,START,1);
				if (DEBUG_OUT) Serial.print("EVSE_B: Authorize request has been accepted! .\n");
				#if CP_ACTIVE 
				flag_controlPAuthorise = true;
				#endif

			} else {
				flag_evseReadIdTag_B = false;
				flag_evseAuthenticate_B = false;
				flag_evseStartTransaction_B = false;
				flag_evRequestsCharge_B = false;
				flag_evseStopTransaction_B = false;
				flag_evseUnauthorise_B = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
				getChargePointStatusService_A()->setUnavailabilityStatus(false);
				if (DEBUG_OUT) Serial.print("EVSE_B Authorize request has been denied! \n");
			}  
		});
	});
	
	EVSE_B_setOnStartTransaction([] () {
		flag_evseStartTransaction_B = false;
		String idTag = "";
		int connectorId = 0;

		if (getChargePointStatusService_B() != NULL) {
			if (!getChargePointStatusService_B()->getIdTag().isEmpty()) {
				idTag = String(getChargePointStatusService_B()->getIdTag());
				connectorId = getChargePointStatusService_B()->getConnectorId();
			}
		}
		OcppOperation *startTransaction = makeOcppOperation(&webSocket, new StartTransaction(idTag,connectorId));
		initiateOcppOperation(startTransaction);
		getChargePointStatusService_A()->setUnavailabilityStatus(true);
		startTransaction->setOnReceiveConfListener([](JsonObject payload) {
    		const char* status = payload["idTagInfo"]["status"] | "Invalid";
      if (!strcmp(status, "Accepted")) { 

      flag_evseReadIdTag_B = false;
      flag_evseAuthenticate_B = false;
      flag_evseStartTransaction_B = false;
      flag_evRequestsCharge_B = true;
      flag_evseStopTransaction_B = false;
      flag_evseUnauthorise_B = false;
      if (DEBUG_OUT) Serial.print("EVSE_B StartTransaction was successful\n");

      /************************************/
      int transactionId = payload["transactionId"] | -1;
      getChargePointStatusService_B()->startTransaction(transactionId);        //new block for Three connector
      getChargePointStatusService_B()->startEnergyOffer();
      getChargePointStatusService_B()->startEvDrawsEnergy();
      resumeTxn_B.putString("idTagData_B",getChargePointStatusService_B()->getIdTag());
      resumeTxn_B.putBool("ongoingTxn_B",true);

      //*****Storing tag data in EEPROM****//
      /*
	  resumeTxn.putString("idTagData",currentIdTag);
      resumeTxn.putBool("ongoingTxn",true);*/
      //***********************************//

      } else {
        flag_evseReadIdTag_B = false;
        flag_evseAuthenticate_B = false;
        flag_evseStartTransaction_B = false;
        flag_evRequestsCharge_B = false;
        flag_evseStopTransaction_B = false;
        flag_evseUnauthorise_B = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.


        if (DEBUG_OUT) Serial.print("EVSE_B StartTransaction was unsuccessful\n");


    	#if CP_ACTIVE
		requestforCP_OUT(STOP);  //stop pwm
		#endif
		getChargePointStatusService_A()->setUnavailabilityStatus(false);
		/*resume namespace
		resumeTxn.putBool("ongoingTxn",false);
		resumeTxn.putString("idTagData","");
		*/

      }
		});
	});
			
	EVSE_B_setOnStopTransaction([] () {
		flag_evseStopTransaction_B = false;
		// if (getChargePointStatusService_B() != NULL) {
  //         getChargePointStatusService_B()->stopEnergyOffer();
  //         getChargePointStatusService_B()->stopTransaction();
		//     	getChargePointStatusService_B()->unauthorize();
  //       }
        int txnId = getChargePointStatusService_B()->getTransactionId();
        int connector = getChargePointStatusService_B()->getConnectorId();
		if(txnId !=prevTxnId_B)
		{
		OcppOperation *stopTransaction = makeOcppOperation(&webSocket, new StopTransaction(currentIdTag_B, txnId, connector));
		initiateOcppOperation(stopTransaction);
		prevTxnId_B = txnId;
    	if (DEBUG_OUT) Serial.print("EVSE_B Closing Relays.\n");

    	/**********************Until Offline functionality is implemented***********/
    	//Resume namespace(Preferences)
    	if(getChargePointStatusService_B()->getEmergencyRelayClose() == false){
    		requestLed(GREEN,START,1);   //temp fix
    	}
    	resumeTxn_B.putBool("ongoingTxn_B",false);
    	resumeTxn_B.putString("idTagData_B","");

		if(ethernet_connect)
		{
			if(!webSocketConncted)
			{
				flag_evseReadIdTag_B = true;
		        flag_evseAuthenticate_B = false;
		        flag_evseStartTransaction_B = false;
		        flag_evRequestsCharge_B = false;
		        flag_evseStopTransaction_B = false;
		        flag_evseUnauthorise_B = false;
	    		Serial.println("Clearing Stored ID tag in StopTransaction()");
			}
		}

    	else if(wifi_connect == true){
	    	if(!webSocketConncted || WiFi.status() != WL_CONNECTED || isInternetConnected == false ){
	    		getChargePointStatusService_B()->stopEvDrawsEnergy();
	    		getChargePointStatusService_B()->stopEnergyOffer();
		      getChargePointStatusService_B()->stopTransaction();
		    	getChargePointStatusService_B()->unauthorize();  //can be buggy 
		    	getChargePointStatusService_A()->setUnavailabilityStatus(false);
		        flag_evseReadIdTag_B = true;
		        flag_evseAuthenticate_B = false;
		        flag_evseStartTransaction_B = false;
		        flag_evRequestsCharge_B = false;
		        flag_evseStopTransaction_B = false;
		        flag_evseUnauthorise_B = false;
    			Serial.println("Clearing Stored ID tag in StopTransaction()");
    		}
    	}else if(gsm_connect == true){
    		if(client.connected() == false){
    			getChargePointStatusService_B()->stopEvDrawsEnergy();
    			getChargePointStatusService_B()->stopEnergyOffer();
    			getChargePointStatusService_B()->stopTransaction();
		    	getChargePointStatusService_B()->unauthorize();  //can be buggy 
		    	getChargePointStatusService_A()->setUnavailabilityStatus(false);
		        flag_evseReadIdTag_B = true;
		        flag_evseAuthenticate_B = false;
		        flag_evseStartTransaction_B = false;
		        flag_evRequestsCharge_B = false;
		        flag_evseStopTransaction_B = false;
		        flag_evseUnauthorise_B = false;
    			Serial.println("Clearing Stored ID tag in StopTransaction()");
    		}
    	}

    	requestForRelay(STOP,2);
    
    	delay(500);
		stopTransaction->setOnReceiveConfListener([](JsonObject payload) {
			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = true;
			getChargePointStatusService_B()->stopEvDrawsEnergy();
			getChargePointStatusService_B()->stopTransaction();
			getChargePointStatusService_B()->stopEnergyOffer();
			getChargePointStatusService_A()->setUnavailabilityStatus(false);
		  getChargePointStatusService_B()->unauthorize();
      		if (DEBUG_OUT) Serial.print("EVSE_B Closing Relays.\n");
			if (DEBUG_OUT) Serial.print("EVSE_B StopTransaction was successful\n");
			if (DEBUG_OUT) Serial.print("EVSE_B Reinitializing for new transaction. \n");
		});
		}
	else
	{
		Serial.println("[EVSE_B]StopTransaction already called. Ignoring.");
	}
	});
	
	EVSE_B_setOnUnauthorizeUser([] () {
		if(flag_evseSoftReset_B == true){
			//This 'if' block is developed by @Wamique.
			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
			flag_rebootRequired_B = true;
			getChargePointStatusService_B()->unauthorize();
			if (DEBUG_OUT) Serial.println("EVSE_B Initiating Soft reset");
		} else if(flag_evseSoftReset_B == false){
			flag_evseReadIdTag_B = true;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
			if (DEBUG_OUT) Serial.print("EVSE_B Unauthorizing user and setting up for new user ID read.\n");
			getChargePointStatusService_B()->unauthorize();
			
		}
	});
}

/*********************************************************************/

void EVSE_B_loop() {
	
	if (flag_evseIsBooted_B == false){
		if(DEBUG_OUT) Serial.println("[EVSE] Booting...");
		delay(1000);
		//onBoot();
		t_B = millis();
		return;
	} else if (flag_evseIsBooted_B == true) {
		if (flag_evseReadIdTag_B == true) {
			if (onReadUserId_B != NULL) {
				onReadUserId_B();
			}
			return;
		} else if (flag_evseAuthenticate_B == true) {
			if (onAuthentication_B != NULL) {
				onAuthentication_B();
				
			}
			return;
		} else if (flag_evseStartTransaction_B == true) {
			if (onStartTransaction_B != NULL) {
				#if CP_ACTIVE
				if((EVSE_state == STATE_C || EVSE_state == STATE_D) && getChargePointStatusService()->getEmergencyRelayClose() == false){
					onStartTransaction_B();
				}else{
					Serial.println(F("Connect the Connector to EV / Or fault exist"));     //here have to add timeout of 30 sec
					connectorDis_counter_B++;
					//EVSE_stopTransactionByRfid();
					if(connectorDis_counter_B > 25){
						connectorDis_counter_B = 0;

						EVSE_StopSession_B();
					}
					
				}
				#endif

				#if !CP_ACTIVE
					onStartTransaction_B();   //can add check for fault
				#endif

			}
			else{
				Serial.println("remote start action failed as onStartTransaction_B became NULL");
			}
		} else if (flag_evRequestsCharge_B == true){

		#if CP_ACTIVE
			//flag_evRequestsCharge = false;
				if (getChargePointStatusService_B() != NULL && getChargePointStatusService_B()->getEvDrawsEnergy() == false) {
				
			/***********************Control Pilot @Wamique******************/
					if(EVSE_state == STATE_C || EVSE_state == STATE_D){
						if (getChargePointStatusService_B()->getEmergencyRelayClose() == false) {
							EVSE_StartCharging_B();
						} else if (getChargePointStatusService_B()->getEmergencyRelayClose() == true) {
							Serial.println(F("The voltage / current / Temp is out or range. FAULTY CONDITION DETECTED."));
						}
					}else if(EVSE_state == STATE_SUS){
						EVSE_Suspended_B();
						Serial.println(counter1);
						if(counter1_B++ > 25){    //Have to implement proper timeout
							counter1_B = 0;
							EVSE_B_StopSession();
						}

					}else if(EVSE_state == STATE_DIS || EVSE_state == STATE_E || EVSE_state == STATE_B || EVSE_state == STATE_A){
				
					//	EVSE_StopSession();     // for the very first time cable can be in disconnected state

						//if(txn == true){           // can implement counter > 10 just to remove noise
						EVSE_B_StopSession();
				//	}

					}else{

						Serial.println("[EVSE] STATE Error" +String(EVSE_state));
						delay(2000);

					//	requestLed(RED,START,1);
					}
				} 
				if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){

				//	txn = true;
				
					if(EVSE_state == STATE_C || EVSE_state == STATE_D ){

						if(DEBUG_OUT) Serial.println(F("[EVSE_CP] Drawing Energy"));	

						if(millis() - t_B > 10000){
				 			if(getChargePointStatusService_B()->getEmergencyRelayClose() == false){
				 				requestLed(BLINKYGREEN,START,1);
				 				t_B = millis();
				 			}
				 		}
						/*
						if(blinckCounter++ % 2 == 0){
							requestLed(GREEN,START,1);
						}else{
							requestLed(GREEN,STOP,1);
						}*/
					}else if(EVSE_state == STATE_A || EVSE_state == STATE_E || EVSE_state == STATE_B){//Although CP Inp will never go to A,B state
						if(counter_faultstate_B++ > 5){
						 EVSE_B_StopSession();
						 counter_faultstate_B = 0;
						}
                
					}else if(EVSE_state == STATE_SUS){
						EVSE_B_Suspended();    //pause transaction :update suspended state is considered in charging state

					}else if(EVSE_state == STATE_DIS){

						Serial.println(F("[EVSE] Connect the Connector with EV and Try again"));
						EVSE_B_StopSession();						
				}                      
			}

			 /***Implemented Exit Feature with RFID @Wamique****/ 
		//	  EVSE_B_stopTransactionByRfid();
		#endif

			
			#if !CP_ACTIVE
			if (getChargePointStatusService_B() != NULL && relayTriggered_B == false/*&& getChargePointStatusService_B()->getEvDrawsEnergy() == false*/) {
				if (getChargePointStatusService_B()->getEmergencyRelayClose() == false) {
				//	getChargePointStatusService_B()->startEvDrawsEnergy();
					relayTriggered_B = true;
					
					if (DEBUG_OUT) Serial.print("[EVSE_B] Opening Relays.\n");
					requestForRelay(START,2);
					requestLed(ORANGE,START,1);
    				delay(1200);
    				requestLed(WHITE,START,1);
    				delay(1200);
    				requestLed(GREEN,START,1);
   				 	delay(1000);
					if(DEBUG_OUT) Serial.println("[EVSE_B] Started Drawing Energy");
				} else if (getChargePointStatusService_B()->getEmergencyRelayClose() == true) {
						Serial.println("The voltage or current is out or range. FAULTY CONDITION DETECTED.");
					}
			} 
			if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){
				//delay(250);
            
				 if(DEBUG_OUT) Serial.println("[EVSE_B] Drawing Energy");

				 //blinking green Led
				 if(millis() - t_B > 5000){
				 	// if((WiFi.status() == WL_CONNECTED) && (webSocketConncted == true) && (isInternetConnected == true)&& getChargePointStatusService()->getEmergencyRelayClose() == false){
				 	// 	requestLed(BLINKYGREEN_EINS,START,1);
				 	// 	t = millis();
				 	// }

					if(getChargePointStatusService_B()->getEmergencyRelayClose() == false){
					 		requestLed(BLINKYGREEN,START,1);
					 		t_B = millis();

					 		if(millis() - relay_timer_B > 15000){
						 	
							requestForRelay(START,2);
							relay_timer_B = millis();

						}
					}
					


				}
				 //Current check
				 drawing_current_B = eic.GetLineCurrentB();
				 Serial.println("Current B: "+String(drawing_current_B));
				 if(drawing_current_B <= 0.15){
				 	counter_drawingCurrent_B++;
				 	if(counter_drawingCurrent_B > 120){
				 		counter_drawingCurrent_B = 0;
				 		Serial.println("Stopping session due to No current");
				 		EVSE_B_StopSession();
				 	}

				 }else{
				 	counter_drawingCurrent_B = 0;
				 	Serial.println("counter_drawing Current Reset");

				 }
				
			}
			   //Implemented Exit Feature with RFID @Wamique//
		//	 EVSE_B_stopTransactionByRfid();
			#endif
			//this is the only 'else if' block which is calling next else if block. the control is from this file itself. the control is not changed from any other file. but the variables are required to be present as extern in other file to decide calling of other functions. 
			return;
		} else if (flag_evseStopTransaction_B == true) {
			if (getChargePointStatusService_B() != NULL) {
				
			//	getChargePointStatusService_B()->stopEnergyOffer();

			}
			if (onStopTransaction_B != NULL) {
				// getChargePointStatusService_B()->stopEvDrawsEnergy();
				onStopTransaction_B();
				relayTriggered_B = false;
				#if CP_ACTIVE
				requestforCP_OUT(STOP);  //stop pwm
				#endif
			}
			return;
		} else if (flag_evseUnauthorise_B == true) {
			if (onUnauthorizeUser_B != NULL) {
				onUnauthorizeUser_B();
			}
			return;
		} else if (flag_rebootRequired_B == true && flag_rebootRequired_A == true  /*flag_rebootRequired_C == true*/) {
			//soft reset execution.
			// flag_evseIsBooted_B = false;
			// flag_rebootRequired_B = false;
			// flag_evseSoftReset_B = false;
			if(getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Charging &&
				getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Charging 
			/*	getChargePointStatusService_C()->inferenceStatus() != ChargePointStatus::Charging*/){
					if(DEBUG_OUT) Serial.print("[EVSE] rebooting in 5 seconds...\n");
					delay(5000);
					ESP.restart();
			}
		} else {
			if(DEBUG_OUT) Serial.print("[EVSE_B] waiting for response...\n");
			delay(100);
		}	
	}
}

short EMGCY_counter_B =0;
bool EMGCY_FaultOccured_B = false;

void emergencyRelayClose_Loop_B(){
//	if(millis() - faultTimer_B > 2000){
		// faultTimer_B = millis();
		// bool EMGCY_status_B = requestEmgyStatus();
		// //Serial.println("EMGCY_Status_B: "+String(EMGCY_status_B));
		// if(EMGCY_status_B == true){
		// 	if(EMGCY_counter_B++ > 0){
		// 		requestForRelay(STOP,2);
		// 		requestLed(BLINKYRED,START,1);
		// 		getChargePointStatusService_B()->setEmergencyRelayClose(true);
		// 		EMGCY_FaultOccured_B = true;
		// 		EMGCY_counter_B = 0;
		// 	}
		// }else{
		// 	EMGCY_FaultOccured_B = false;
		// 	EMGCY_counter_B = 0;
		// 	getChargePointStatusService_B()->setEmergencyRelayClose(false);
		// }

		if(EMGCY_GFCI_Fault_Occ == true && getChargePointStatusService_B()->getTransactionId() != -1){

			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = true;
		
		}else if(EMGCY_GFCI_Fault_Occ == false){


			float volt = eic.GetLineVoltageB();
			float current = eic.GetLineCurrentB();
			float temp= eic.GetTemperature();
			Serial.println("Voltage_B: "+String(volt)+", Current_B: "+String(current)+", Temperature: "+String(temp));
			if (getChargePointStatusService_B() != NULL) {
				if(getChargePointStatusService_B()->getOverVoltage() == true ||
					getChargePointStatusService_B()->getUnderVoltage() == true ||
					getChargePointStatusService_B()->getUnderTemperature() == true ||
					getChargePointStatusService_B()->getOverTemperature() == true ||
					getChargePointStatusService_B()->getOverCurrent() == true){
						Serial.println("[EVSE_B] Fault Occurred.");						
						getChargePointStatusService_B()->setEmergencyRelayClose(true);
						if (!timer_initialize_B){
							timeout_start_B = millis();
							timer_initialize_B = true;
						}
					} else if(getChargePointStatusService_B()->getOverVoltage() == false &&
							getChargePointStatusService_B()->getUnderVoltage() == false &&
							getChargePointStatusService_B()->getUnderTemperature() == false &&
							getChargePointStatusService_B()->getOverTemperature() == false &&
							getChargePointStatusService_B()->getOverCurrent() == false){
						Serial.println("[EVSE_B]Not Faulty.");						
						getChargePointStatusService_B()->setEmergencyRelayClose(false);
						//if (!timer_initialize){
							timeout_start_B = 0;
							timer_initialize_B = false;
						//}
					}
					
				if (getChargePointStatusService_B()->getEmergencyRelayClose() == true){
					timeout_active_B = true;
					requestForRelay(STOP,2);
					delay(50);
					#if LED_ENABLED
					requestLed(RED,START,1);
					#endif

					flag_faultOccured_B = true;
				} else if (getChargePointStatusService_B()->getEmergencyRelayClose() == false && flag_faultOccured_B == true){
					timeout_active_B = false;
					if ( (getChargePointStatusService_B()->getTransactionId() != -1)){ //can be buggy
						if(fault_counter_B++ > 1){
							fault_counter_B = 0;
							requestForRelay(START,2);
							delay(50);
							Serial.println("[EmergencyRelay_B] Starting Txn");
							flag_faultOccured_B = false;
						}
					}
				}

				if (timeout_active_B && getChargePointStatusService_B()->getTransactionId() != -1) {
					if (millis() - timeout_start_B >= TIMEOUT_EMERGENCY_RELAY_CLOSE){
						Serial.println("[EVSE_B] Emergency Stop.");
						flag_evRequestsCharge_B = false;
						flag_evseStopTransaction_B = true;
						timeout_active_B = false;
						timer_initialize_B = false;
					}
				}
			}
		}
	}


/*
* @param limit: expects current in amps from 0.0 to 32.0
*/
void EVSE_B_setChargingLimit(float limit) {
	if(DEBUG_OUT) Serial.print(F("[EVSE] New charging limit set. Got "));
	if(DEBUG_OUT) Serial.print(limit);
	if(DEBUG_OUT) Serial.print(F("\n"));
	chargingLimit_B = limit;
}

bool EVSE_B_EvRequestsCharge() {
	return flag_evRequestsCharge_B;
}

bool EVSE_B_EvIsPlugged() {
	return evIsPlugged_B;
}


void EVSE_B_setOnBoot(OnBoot_B onBt_B){
	onBoot_B = onBt_B;
}

void EVSE_B_setOnReadUserId(OnReadUserId_B onReadUsrId_B){
	onReadUserId_B = onReadUsrId_B;
}

void EVSE_B_setOnsendHeartbeat(OnSendHeartbeat_B onSendHeartbt_B){
	onSendHeartbeat_B = onSendHeartbt_B;
}

void EVSE_B_setOnAuthentication(OnAuthentication_B onAuthenticatn_B){
	onAuthentication_B = onAuthenticatn_B;
}

void EVSE_B_setOnStartTransaction(OnStartTransaction_B onStartTransactn_B){
	onStartTransaction_B = onStartTransactn_B;
}

void EVSE_B_setOnStopTransaction(OnStopTransaction_B onStopTransactn_B){
	onStopTransaction_B = onStopTransactn_B;
}

void EVSE_B_setOnUnauthorizeUser(OnUnauthorizeUser_B onUnauthorizeUsr_B){
	onUnauthorizeUser_B = onUnauthorizeUsr_B;
}

// void EVSE_getSsid(String &out) {
// 	out += "Pied Piper";
// }
// void EVSE_getPass(String &out) {
// 	out += "plmzaq123";
// }


// void EVSE_getChargePointSerialNumber(String &out) {


// 	out += preferences.getString("chargepoint","");

// 	/*
// 	#if STEVE
// 	out += "dummyCP002";
// 	#endif

// 	#if EVSECEREBRO
// 	out += "testpodpulkit";
// 	#endif
// 	*/
// }

// char *EVSE_getChargePointVendor() {
// 	return "Amplify Mobility";
// }

// char *EVSE_getChargePointModel() {
// 	return "Wx2";
// }

String EVSE_B_getCurrnetIdTag(MFRC522 * mfrc522) {
    String currentIdTag = "";
	//currentIdTag_B = EVSE_B_readRFID(mfrc522);
	
	if (getChargePointStatusService_B()->getIdTag().isEmpty() == false){
		if(DEBUG_OUT) Serial.println("[EVSE_B] Reading from Charge Point Station Service ID Tag stored.");
		currentIdTag = getChargePointStatusService_B()->getIdTag();
		if(DEBUG_OUT) Serial.print("[EVSE_B] ID Tag: ");
		if(DEBUG_OUT) Serial.println(currentIdTag);
		Serial.flush();
	}
	
	return currentIdTag;
}


String EVSE_B_readRFID(MFRC522 * mfrc522) {
	String currentIdTag_B;	
	currentIdTag_B = readRfidTag(true, mfrc522);
	return currentIdTag_B;
}

/********Added new funtion @Wamique***********************/

// void EVSE_B_stopTransactionByRfid(){

// 	Ext_currentIdTag_B = EVSE_B_readRFID(&mfrc522);
// 	if(currentIdTag_B.equals(Ext_currentIdTag_B) == true){
// 		flag_evRequestsCharge_B = false;
// 		flag_evseStopTransaction_B = true;
// 	}else{
// 			if(Ext_currentIdTag_B.equals("") == false)
// 			if(DEBUG_OUT) Serial.println("\n[EVSE_B] Incorrect ID tag\n");
// 		}
// }

#if CP_ACTIVE
/**************CP Implementation @mwh*************/
void EVSE_B_StartCharging(){

	if(getChargePointStatusService_B()->getEvDrawsEnergy() == false){
		getChargePointStatusService_B()->startEvDrawsEnergy();
	}
    if (DEBUG_OUT) Serial.print(F("[EVSE_B] Opening Relays.\n"));
 //   pinMode(32,OUTPUT);
  //  digitalWrite(32, HIGH); //RELAY_1
    //digitalWrite(RELAY_2, RELAY_HIGH);
    requestForRelay(START,2);
    requestLed(ORANGE,START,1);
    delay(1200);
    requestLed(WHITE,START,1);
    delay(1200);
    requestLed(GREEN,START,1);
    delay(1000);
    Serial.println("[EVS_B] EV is connected and Started charging");
	if(DEBUG_OUT) Serial.println("[EVSE_B] Started Drawing Energy");
	delay(500);
}


void EVSE_B_Suspended(){


	if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){
		getChargePointStatusService_B()->stopEvDrawsEnergy();
	}
		requestLed(BLUE,START,1);     //replace 1 with connector ID
		requestForRelay(STOP,2);
	//	delay(1000);
		Serial.println("[EVSE_B] EV Suspended");


}



/**************************************************/

#endif