#include "EVSE_C.h"
#include "Master.h"
#include "ControlPilot.h"

OnBoot_C onBoot_C;
OnReadUserId_C onReadUserId_C;
OnSendHeartbeat_C onSendHeartbeat_C;
OnAuthentication_C onAuthentication_C;
OnStartTransaction_C onStartTransaction_C;
OnStopTransaction_C onStopTransaction_C;
OnUnauthorizeUser_C onUnauthorizeUser_C;

bool evIsPlugged_C; 
bool flag_evseIsBooted_C;
bool flag_evseReadIdTag_C;
bool flag_evseAuthenticate_C;
bool flag_evseStartTransaction_C;
bool flag_evRequestsCharge_C;
bool flag_evseStopTransaction_C;
bool flag_evseUnauthorise_C;
bool flag_rebootRequired_C;
bool flag_evseSoftReset_C; 

extern bool flag_rebootRequired_A;
extern bool flag_rebootRequired_B;

float chargingLimit_C = 32.0f;
String Ext_currentIdTag_C = "";
int prevTxnId_C = -1;
long int blinckCounter_C =0;
int counter1_C =0;

ulong t_C;
int connectorDis_counter_C=0;
short int counter_drawingCurrent_C = 0;
float drawing_current_C = 0;

String currentIdTag_C="";
extern WebSocketsClient webSocket;

extern EVSE_states_enum EVSE_state;
extern Preferences preferences;
extern MFRC522 mfrc522;
extern String currentIdTag;
extern bool webSocketConncted;
extern bool isInternetConnected;
extern bool wifi_connect;
extern bool gsm_connect;

Preferences resumeTxn_C;
extern TinyGsmClient client;

bool notFaulty_C = false;

bool disp_evse_C = false;
extern bool disp_evse_A;
extern bool disp_evse_B;

bool ongoingTxn_C;
String idTagData_C = "";

bool timeout_active_C=false;
bool timer_initialize_C = false;
ulong timeout_start_C=0;
short int fault_counter_C = 0;
bool flag_faultOccured_C = false;
ulong relay_timer_C = 0;
ulong faultTimer_C =0;

extern bool wifi_reconnected_flag;

/**********************************************************/
void EVSE_C_StopSession(){

	if(getChargePointStatusService_C()->getEvDrawsEnergy() == true){
		getChargePointStatusService_C()->stopEvDrawsEnergy();
	}
	
	//digitalWrite(32, LOW);
    requestForRelay(STOP,3); 
    delay(500);           	
    flag_evseReadIdTag_C = false;
	flag_evseAuthenticate_C = false;
	flag_evseStartTransaction_C = false;
	flag_evRequestsCharge_C = false;
	flag_evseStopTransaction_C = true;
	flag_evseUnauthorise_C = false;
    Serial.println("[EVSE_C] Stopping Session : " +String(EVSE_state));
}
/**************************************************************************/


/**************************************************************************/

void EVSE_C_initialize() {
	if(DEBUG_OUT) Serial.print("[EVSE_C] EVSE_C Starting Boot.\n");
	//onBoot_C();
	flag_evseIsBooted_C = true; //Exit condition for booting. 	
	flag_evseReadIdTag_C = true; //Entry condition for reading ID Tag.
	faultTimer_C = millis();
}

/**************************SetUp********************************************/
void EVSE_C_setup(){

	resumeTxn_C.begin("resume_C",false); //opening preferences in R/W mode
  	idTagData_C = resumeTxn_C.getString("idTagData_C","");
  	ongoingTxn_C = resumeTxn_C.getBool("ongoingTxn_C",false);

  	Serial.println("Stored ID:"+String(idTagData_C));
  	Serial.println("Ongoing Txn: "+String(ongoingTxn_C));

	EVSE_C_setOnBoot([]() {
		//this is not in loop, that is why we need not update the flag immediately to avoid multiple copies of bootNotification.
		OcppOperation *bootNotification = makeOcppOperation(&webSocket,	new BootNotification());
		initiateOcppOperation(bootNotification);
		bootNotification->setOnReceiveConfListener([](JsonObject payload) {

      	// if( flag_MeteringIsInitialised == false){
      	// 	Serial.println("[SetOnBooT] Initializing metering services");
      	// //	meteringService->init(meteringService);
      	// }

      if (DEBUG_OUT) Serial.print("EVSE_C Metering Services Init finished.\n");

			flag_evseIsBooted_C = true; //Exit condition for booting. 	
			flag_evseReadIdTag_C = true; //Entry condition for reading ID Tag.
			flag_evseAuthenticate_C = false;
			flag_evseStartTransaction_C = false;
			flag_evRequestsCharge_C = false;
			flag_evseStopTransaction_C = false;
			flag_evseUnauthorise_C = false;
      if (DEBUG_OUT) Serial.print("EVSE_C Closing Relays.\n");

			if (DEBUG_OUT) Serial.print("EVSE_C Boot successful.\n");
		});
	});

	EVSE_C_setOnReadUserId([] () {
		if (DEBUG_OUT) Serial.print("EVSE_C EVSE waiting for User ID read...\n");
		static ulong timerForRfid = millis();
		currentIdTag_C = ""; 
		idTagData_C = resumeTxn_C.getString("idTagData_C","");
  		ongoingTxn_C = resumeTxn_C.getBool("ongoingTxn_C",false);

  		if((wifi_connect == true)&& 														//Wifi Block
  			(getChargePointStatusService_C()->getEmergencyRelayClose() == false)&&
  			(webSocketConncted == true)&&
  			(WiFi.status() == WL_CONNECTED)&&
  			(isInternetConnected == true)){

  			if((idTagData_C != "") && (ongoingTxn_C == 1)){
  				currentIdTag_C = resumeTxn_C.getString("idTagData_C","");
  				getChargePointStatusService_C()->authorize(currentIdTag_C); // so that Starttxn can easily fetch this data
  				Serial.println("[EVSE_C] Resuming Session");
  				requestLed(BLUE, START, 3);
  			}else{
				if(millis() - timerForRfid > 10000){ //timer for sending led request
	    		  	requestLed(GREEN,START,3);
					wifi_reconnected_flag = true;
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_C = EVSE_C_getCurrnetIdTag(&mfrc522);
				Serial.println("[Wifi]*RFID C*");
  			}

  		}else if((gsm_connect == true)&&													//GSM Block
  				(getChargePointStatusService_C()->getEmergencyRelayClose() == false)&&
  				(client.connected() == true)){

  			if((idTagData_C != "") && (ongoingTxn_C == 1)){
  				currentIdTag_C = resumeTxn_C.getString("idTagData_C","");
  				getChargePointStatusService_C()->authorize(currentIdTag_C); // so that Starttxn can easily fetch this data
  				Serial.println("[EVSE_C] Resuming Session");
  				requestLed(BLUE, START, 3);
  			}else{
				if(millis() - timerForRfid > 10000){ //timer for sending led request
	    		  	requestLed(GREEN,START,3);
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_C = EVSE_C_getCurrnetIdTag(&mfrc522);
				Serial.println("[GSM]**RFID C**");
  			}


  		} 
		/*	
		idTagData_m = resumeTxn.getString("idTagData","");
  		ongoingTxn_m = resumeTxn.getBool("ongoingTxn",false);

		if((ongoingTxn_m == 1) && (idTagData_m != "") && 
		      (getChargePointStatusService_C()->getEmergencyRelayClose() == false) &&
		      (WiFi.status() == WL_CONNECTED)&&
		      (webSocketConncted == true)&&
		      (isInternetConnected == true)){   //giving priority to stored data
			currentIdTag_C = resumeTxn.getString("idTagData","");
			Serial.println("[EVSE_setOnReadUserId] Resuming Session");
      		requestLed(BLUE,START,1);
         
		}else*/ 
		// if((getChargePointStatusService_C()->getEmergencyRelayClose() == false) &&
		//           (WiFi.status() == WL_CONNECTED) &&
		//           (webSocketConncted == true) && 
		//           (isInternetConnected == true)){
		// 	  #if LED_ENABLED
		// 	  if(millis() - timerForRfid > 5000){ //timer for sending led request
  //   		  requestLed(GREEN,START,3);
  //   		  timerForRfid = millis();
  //   		  }
  //   		  #endif
		// 	currentIdTag_C = EVSE_C_getCurrnetIdTag(&mfrc522);
		// 	Serial.println("********RFID C**********");
		// }

		if (currentIdTag_C.equals("") == true) {
			flag_evseReadIdTag_C = true; //Looping back read block as no ID tag present.
			flag_evseAuthenticate_C = false;
			flag_evseStartTransaction_C = false;
			flag_evRequestsCharge_C = false;
			flag_evseStopTransaction_C = false;
			flag_evseUnauthorise_C = false;
		} else {
			flag_evseReadIdTag_C = false;
			flag_evseAuthenticate_C = true; //Entry condition for authentication block.
			flag_evseStartTransaction_C = false;
			flag_evRequestsCharge_C = false;
			flag_evseStopTransaction_C = false;
			flag_evseUnauthorise_C = false;
			if (DEBUG_OUT) Serial.print("EVSE_C Successful User ID Read\n");
		}	
	});
	
	EVSE_C_setOnsendHeartbeat([] () {
    if (DEBUG_OUT) Serial.print("EVSE_Sending heartbeat signal...\n");
    OcppOperation *heartbeat = makeOcppOperation(&webSocket, new Heartbeat());
    initiateOcppOperation(heartbeat); 
    heartbeat->setOnReceiveConfListener([](JsonObject payload) {
        const char* currentTime = payload["currentTime"] | "Invalid";
        if (strcmp(currentTime, "Invalid")) {
          if (setTimeFromJsonDateString(currentTime)) {
            if (DEBUG_OUT) Serial.print("EVSE_Request has been accepted!\n");
          } else {
            Serial.print("EVSE_ Request accepted.\n");
          }
        } else {
          Serial.print("EVSE_: Request denied. \n");
        }
    });   
  });
 
	EVSE_C_setOnAuthentication([] () {
		if (DEBUG_OUT) Serial.print("EVSE_C Authenticating...\n");
		flag_evseAuthenticate_C = false;
		OcppOperation *authorize = makeOcppOperation(&webSocket, new Authorize(currentIdTag_C));
		initiateOcppOperation(authorize);
		/*chargePointStatusService->authorize(currentIdTag_C, connectorId_C);  */    //have to edit
		authorize->setOnReceiveConfListener([](JsonObject payload) {
			const char* status = payload["idTagInfo"]["status"] | "Invalid";
			if (!strcmp(status, "Accepted")) {
				flag_evseReadIdTag_C = false;
				flag_evseAuthenticate_C = false;
				flag_evseStartTransaction_C = true; //Entry condition for starting transaction.
				flag_evRequestsCharge_C = false;
				flag_evseStopTransaction_C = false;
				flag_evseUnauthorise_C = false;
				requestLed(BLUE,START,3);
				if (DEBUG_OUT) Serial.print("EVSE_C Authorize request has been accepted! \n");
				#if CP_ACTIVE 
				flag_controlPAuthorise = true;
				#endif

			} else {
				flag_evseReadIdTag_C = false;
				flag_evseAuthenticate_C = false;
				flag_evseStartTransaction_C = false;
				flag_evRequestsCharge_C = false;
				flag_evseStopTransaction_C = false;
				flag_evseUnauthorise_C = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
				if (DEBUG_OUT) Serial.print("EVSE_C_ Authorize request has been denied! \n");
			}  
		});
	});
	
	EVSE_C_setOnStartTransaction([] () {
		flag_evseStartTransaction_C = false;
		String idTag = "";
		int connectorId = 0;
		if (getChargePointStatusService_C() != NULL) {
			if (!getChargePointStatusService_C()->getIdTag().isEmpty()) {
				idTag = String(getChargePointStatusService_C()->getIdTag());
				connectorId = getChargePointStatusService_C()->getConnectorId();
			}
		}
		OcppOperation *startTransaction = makeOcppOperation(&webSocket, new StartTransaction(idTag,connectorId));
		initiateOcppOperation(startTransaction);
		startTransaction->setOnReceiveConfListener([](JsonObject payload) {
    		const char* status = payload["idTagInfo"]["status"] | "Invalid";
      if (!strcmp(status, "Accepted")) { 

      flag_evseReadIdTag_C = false;
      flag_evseAuthenticate_C = false;
      flag_evseStartTransaction_C = false;
      flag_evRequestsCharge_C = true;
      flag_evseStopTransaction_C = false;
      flag_evseUnauthorise_C = false;
      if (DEBUG_OUT) Serial.print("EVSE_C StartTransaction was successful\n");

      /************************************/
      int transactionId = payload["transactionId"] | -1;
      getChargePointStatusService_C()->startTransaction(transactionId);        //new block for Three connector
      getChargePointStatusService_C()->startEnergyOffer();
      resumeTxn_C.putString("idTagData_C",getChargePointStatusService_C()->getIdTag());
      resumeTxn_C.putBool("ongoingTxn_C",true);

      //*****Storing tag data in EEPROM****//
      /*
	  resumeTxn.putString("idTagData",currentIdTag);
      resumeTxn.putBool("ongoingTxn",true);*/
      //***********************************//

      } else {
        flag_evseReadIdTag_C = false;
        flag_evseAuthenticate_C = false;
        flag_evseStartTransaction_C = false;
        flag_evRequestsCharge_C = false;
        flag_evseStopTransaction_C = false;
        flag_evseUnauthorise_C = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
        if (DEBUG_OUT) Serial.print("EVSE_C StartTransaction was unsuccessful\n");

    	#if CP_ACTIVE
		requestforCP_OUT(STOP);  //stop pwm
		#endif
		/*resume namespace
		resumeTxn.putBool("ongoingTxn",false);
		resumeTxn.putString("idTagData","");
		*/

      }
		});
	});
			
	EVSE_C_setOnStopTransaction([] () {
		flag_evseStopTransaction_C = false;
		if (getChargePointStatusService_C() != NULL) {
          getChargePointStatusService_C()->stopEnergyOffer();
        }
        int txnId = getChargePointStatusService_C()->getTransactionId();
        int connector = getChargePointStatusService_C()->getConnectorId();
		if(txnId !=prevTxnId_C)
		{
		OcppOperation *stopTransaction = makeOcppOperation(&webSocket, new StopTransaction(currentIdTag_C, txnId, connector));
		initiateOcppOperation(stopTransaction);
		prevTxnId_C = txnId;
    	if (DEBUG_OUT) Serial.print("EVSE_C_ Closing Relays.\n");

    	/**********************Until Offline functionality is implemented***********/
    	//Resume namespace(Preferences)
    	if(getChargePointStatusService_C()->getEmergencyRelayClose() == false){
    		requestLed(GREEN,START,1);   //temp fix
    	}
    	resumeTxn_C.putBool("ongoingTxn_C",false);
    	resumeTxn_C.putString("idTagData_C","");
    	if(wifi_connect == true){
	    	if(!webSocketConncted || WiFi.status() != WL_CONNECTED || isInternetConnected == false){
		        getChargePointStatusService_C()->stopTransaction();
		    	getChargePointStatusService_C()->unauthorize();  //can be buggy 
		        flag_evseReadIdTag_C = true;
		        flag_evseAuthenticate_C = false;
		        flag_evseStartTransaction_C = false;
		        flag_evRequestsCharge_C = false;
		        flag_evseStopTransaction_C = false;
		        flag_evseUnauthorise_C = false;
		    	Serial.println("Clearing Stored ID tag in StopTransaction()");
	    	}
    	}else if(gsm_connect == true){
    		if(client.connected() == false){
    			getChargePointStatusService_C()->stopTransaction();
		    	getChargePointStatusService_C()->unauthorize();  //can be buggy 
		        flag_evseReadIdTag_C = true;
		        flag_evseAuthenticate_C = false;
		        flag_evseStartTransaction_C = false;
		        flag_evRequestsCharge_C = false;
		        flag_evseStopTransaction_C = false;
		        flag_evseUnauthorise_C = false;
		    	Serial.println("Clearing Stored ID tag in StopTransaction()");
    		}
    	}

    	requestForRelay(STOP,3);
    
    	delay(500);
		stopTransaction->setOnReceiveConfListener([](JsonObject payload) {
			flag_evseReadIdTag_C = false;
			flag_evseAuthenticate_C = false;
			flag_evseStartTransaction_C = false;
			flag_evRequestsCharge_C = false;
			flag_evseStopTransaction_C = false;
			flag_evseUnauthorise_C = true;
			getChargePointStatusService_C()->stopTransaction();
      		if (DEBUG_OUT) Serial.print("EVSE_ Closing Relays.\n");
			if (DEBUG_OUT) Serial.print("EVSE_ StopTransaction was successful\n");
			if (DEBUG_OUT) Serial.print("EVSE_C Reinitializing for new transaction. \n");
		});
		}
	else
	{
		Serial.println("[EVSE_C] EVSE_setOnStopTransaction already called. ");
	}
	});

	EVSE_C_setOnUnauthorizeUser([] () {
		if(flag_evseSoftReset_C == true){
			//This 'if' block is developed by @Wamique.
			flag_evseReadIdTag_C = false;
			flag_evseAuthenticate_C = false;
			flag_evseStartTransaction_C = false;
			flag_evRequestsCharge_C = false;
			flag_evseStopTransaction_C = false;
			flag_evseUnauthorise_C = false;
			flag_rebootRequired_C = true;
			getChargePointStatusService_C()->unauthorize();
			if (DEBUG_OUT) Serial.println("EVSE_C_ Initiating Soft reset");
		} else if(flag_evseSoftReset_C == false){
			flag_evseReadIdTag_C = true;
			flag_evseAuthenticate_C = false;
			flag_evseStartTransaction_C = false;
			flag_evRequestsCharge_C = false;
			flag_evseStopTransaction_C = false;
			flag_evseUnauthorise_C = false;
			if (DEBUG_OUT) Serial.print("EVSE_Unauthorizing user and setting up for new user ID read.\n");
			getChargePointStatusService_C()->unauthorize();
			
		}
	});
}

/*********************************************************************/

void EVSE_C_loop() {
	
	if (flag_evseIsBooted_C == false){
		if(DEBUG_OUT) Serial.println(F("[EVSE] Booting..."));
		delay(1000);
		//onBoot();
		t_C = millis();
		return;
	} else if (flag_evseIsBooted_C == true) {
		if (flag_evseReadIdTag_C == true) {
			if (onReadUserId_C != NULL) {
				onReadUserId_C();
			}
			return;
		} else if (flag_evseAuthenticate_C == true) {
			if (onAuthentication_C != NULL) {
				onAuthentication_C();
				
			}
			return;
		} else if (flag_evseStartTransaction_C == true) {
			if (onStartTransaction_C != NULL) {
				#if CP_CCTIVE
				if((EVSE_state == STATE_C || EVSE_state == STATE_D) && getChargePointStatusService()->getEmergencyRelayClose() == false){
					onStartTransaction_C();
				}else{
					Serial.println(F("Connect the Connector to EV / Or fault exist"));     //here have to add timeout of 30 sec
					connectorDis_counter_C++;
					//EVSE_stopTransactionByRfid();
					if(connectorDis_counter_C > 25){
						connectorDis_counter_C = 0;

						EVSE_C_StopSession();
					}
					
				}
				#endif

				#if !CP_ACTIVE
					onStartTransaction_C();   //can add check for fault
				#endif

			}
		} else if (flag_evRequestsCharge_C == true){

		#if CP_ACTIVE
			//flag_evRequestsCharge = false;
				if (getChargePointStatusService_C() != NULL && getChargePointStatusService_C()->getEvDrawsEnergy() == false) {
				
			/***********************Control Pilot @Wamique******************/
					if(EVSE_state == STATE_C || EVSE_state == STATE_D){
						if (getChargePointStatusService_C()->getEmergencyRelayClose() == false) {
							EVSE_C_StartCharging();
						} else if (getChargePointStatusService_C()->getEmergencyRelayClose() == true) {
							Serial.println(F("The voltage / current / Temp is out or range. FAULTY CONDITION DETECTED."));
						}
					}else if(EVSE_state == STATE_SUS){
						EVSE_C_Suspended();
						Serial.println(counter1);
						if(counter1_C++ > 25){    //Have to implement proper timeout
							counter1_C = 0;
							EVSE_C_StopSession();
						}

					}else if(EVSE_state == STATE_DIS || EVSE_state == STATE_E || EVSE_state == STATE_B || EVSE_state == STATE_A){
				
					//	EVSE_StopSession();     // for the very first time cable can be in disconnected state

						//if(txn == true){           // can implement counter > 10 just to remove noise
						EVSE_C_StopSession();
				//	}

					}else{

						Serial.println(F("[EVSE] STATE Error" +String(EVSE_state)));
						delay(2000);

					//	requestLed(RED,START,1);
					}
				} 
				if(getChargePointStatusService_C()->getEvDrawsEnergy() == true){

				//	txn = true;
				
					if(EVSE_state == STATE_C || EVSE_state == STATE_D ){

						if(DEBUG_OUT) Serial.println(F("[EVSE_CP] Drawing Energy"));	

						if(millis() - t_C > 10000){
				 			if(getChargePointStatusService_C()->getEmergencyRelayClose() == false){
				 				requestLed(BLINKYGREEN,START,3);
				 				t_C = millis();
				 			}
				 		}
						/*
						if(blinckCounter++ % 2 == 0){
							requestLed(GREEN,START,1);
						}else{
							requestLed(GREEN,STOP,1);
						}*/
					}else if(EVSE_state == STATE_A || EVSE_state == STATE_E || EVSE_state == STATE_B){//Although CP Inp will never go to A,B state
						if(counter_faultstate_C++ > 5){
						 EVSE_C_StopSession();
						 counter_faultstate_C = 0;
						}
                
					}else if(EVSE_state == STATE_SUS){
						EVSE_C_Suspended();    //pause transaction :update suspended state is considered in charging state

					}else if(EVSE_state == STATE_DIS){

						Serial.println(F("[EVSE] Connect the Connector with EV and Try again"));
						EVSE_C_StopSession();						
				}                      
			}

			 /***Implemented Exit Feature with RFID @Wamique****/ 
			//  EVSE_C_stopTransactionByRfid();
		#endif

			
			#if !CP_ACTIVE
			if (getChargePointStatusService_C() != NULL && getChargePointStatusService_C()->getEvDrawsEnergy() == false) {
				if (getChargePointStatusService_C()->getEmergencyRelayClose() == false) {
					getChargePointStatusService_C()->startEvDrawsEnergy();
					
					if (DEBUG_OUT) Serial.print("[EVSE_C] Opening Relays.\n");
					requestForRelay(START,3);
					requestLed(ORANGE,START,3);
    				delay(1200);
    				requestLed(WHITE,START,3);
    				delay(1200);
    				requestLed(GREEN,START,3);
   				 	delay(1000);
					if(DEBUG_OUT) Serial.println("[EVSE_C] Started Drawing Energy");
				} else if (getChargePointStatusService_C()->getEmergencyRelayClose() == true) {
						Serial.println("The voltage or current is out or range. FAULTY CONDITION DETECTED.");
					}
			} 
			if(getChargePointStatusService_C()->getEvDrawsEnergy() == true){
				//delay(250);
            
				 if(DEBUG_OUT) Serial.println(F("[EVSE_C] Drawing Energy"));

				 //blinking green Led
				 if(millis() - t_C > 5000){
				 	// if((WiFi.status() == WL_CONNECTED) && (webSocketConncted == true) && (isInternetConnected == true)&& getChargePointStatusService()->getEmergencyRelayClose() == false){
				 	// 	requestLed(BLINKYGREEN_EINS,START,1);
				 	// 	t = millis();
				 	// }

						if(getChargePointStatusService_C()->getEmergencyRelayClose() == false){
						 		requestLed(BLINKYGREEN,START,3);
						 		t_C = millis();
						 }
						if(millis() - relay_timer_C > 15000){
							 	
							requestForRelay(START,3);
							relay_timer_C = millis();

						}


				 }
				 //Current check
				 drawing_current_C = eic.GetLineCurrentC();
				 Serial.println("Current C: "+String(drawing_current_C));
				 if(drawing_current_C <= 0.15){
				 	counter_drawingCurrent_C++;
				 	if(counter_drawingCurrent_C > 120){
				 		Serial.println("Stopping session due to No current");

				 		counter_drawingCurrent_C = 0;
				 		EVSE_C_StopSession();
				 	}

				 }else{
				 	counter_drawingCurrent_C = 0;
				 	Serial.println("counter_drawingCurrent Reset");

				 }
				
			}
			   //Implemented Exit Feature with RFID @Wamique//
			// EVSE_C_stopTransactionByRfid();
			#endif
			//this is the only 'else if' block which is calling next else if block. the control is from this file itself. the control is not changed from any other file. but the variables are required to be present as extern in other file to decide calling of other functions. 
			return;
		} else if (flag_evseStopTransaction_C == true) {
			if (getChargePointStatusService_C() != NULL) {
				getChargePointStatusService_C()->stopEvDrawsEnergy();

			}
			if (onStopTransaction_C != NULL) {
				onStopTransaction_C();
				#if CP_ACTIVE
				requestforCP_OUT(STOP);  //stop pwm
				#endif
			}
			return;
		} else if (flag_evseUnauthorise_C == true) {
			if (onUnauthorizeUser_C != NULL) {
				onUnauthorizeUser_C();
			}
			return;
		} else if (flag_rebootRequired_C == true && flag_rebootRequired_A == true && flag_rebootRequired_B == true) {
			//soft reset execution.
			// flag_evseIsBooted_C = false;
			// flag_rebootRequired_C = false;
			// flag_evseSoftReset_C = false;
			if(getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Charging &&
				getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Charging &&
				getChargePointStatusService_C()->inferenceStatus() != ChargePointStatus::Charging){
					if(DEBUG_OUT) Serial.print("[EVSE_C] rebooting in 5 seconds...\n");
					delay(5000);
					ESP.restart();
			}
		} else {
			if(DEBUG_OUT) Serial.print("[EVSE_C] waiting for response...\n");
			delay(100);
		}	
	}
}

bool EMGCY_FaultOccured_C = false;
short EMGCY_counter_C = 0;


void emergencyRelayClose_Loop_C(){
	if(millis() - faultTimer_C > 2000){
		faultTimer_C = millis();
		bool EMGCY_status_C = requestEmgyStatus();
		//Serial.println("EMGCY_Status_C: "+String(EMGCY_Status_C));
		if(EMGCY_status_C == true){
			if(EMGCY_counter_C++ > 0){
				requestForRelay(STOP,3);
				requestLed(BLINKYRED,START,3);
				getChargePointStatusService_C()->setEmergencyRelayClose(true);
				EMGCY_FaultOccured_C = true;
				EMGCY_counter_C = 0;
			}
		}else{
			EMGCY_FaultOccured_C = false;
			EMGCY_counter_C = 0;
			getChargePointStatusService_C()->setEmergencyRelayClose(false);
		}

		if(EMGCY_FaultOccured_C == true && getChargePointStatusService_C()->getTransactionId() != -1){

			flag_evseReadIdTag_C = false;
			flag_evseAuthenticate_C = false;
			flag_evseStartTransaction_C = false;
			flag_evRequestsCharge_C = false;
			flag_evseStopTransaction_C = true;
		
		}else if(EMGCY_FaultOccured_C == false){

				float volt = eic.GetLineVoltageC();
				float current = eic.GetLineCurrentC();
				float temp= eic.GetTemperature();
				Serial.println("Voltage_C: "+String(volt)+", Current_C: "+String(current)+", Temperature: "+String(temp));
				if (getChargePointStatusService_C() != NULL) {
					if(getChargePointStatusService_C()->getOverVoltage() == true ||
						getChargePointStatusService_C()->getUnderVoltage() == true ||
						getChargePointStatusService_C()->getUnderTemperature() == true ||
						getChargePointStatusService_C()->getOverTemperature() == true ||
						getChargePointStatusService_C()->getOverCurrent() == true){
							Serial.println("[EVSE_C] Fault Occurred.");						
							getChargePointStatusService_C()->setEmergencyRelayClose(true);
							if (!timer_initialize_C){
								timeout_start_C = millis();
								timer_initialize_C = true;
							}
						} else if(getChargePointStatusService_C()->getOverVoltage() == false &&
								getChargePointStatusService_C()->getUnderVoltage() == false &&
								getChargePointStatusService_C()->getUnderTemperature() == false &&
								getChargePointStatusService_C()->getOverTemperature() == false &&
								getChargePointStatusService_C()->getOverCurrent() == false){
							Serial.println("[EVSE_C] Not Faulty.");						
							getChargePointStatusService_C()->setEmergencyRelayClose(false);
							//if (!timer_initialize){
								timeout_start_C = 0;
								timer_initialize_C = false;
							//}
						}
						
					if (getChargePointStatusService_C()->getEmergencyRelayClose() == true){
						timeout_active_C = true;
						requestForRelay(STOP,3);
						delay(50);
						#if LED_ENABLED
						requestLed(RED,START,3);
						#endif

						flag_faultOccured_C = true;
					} else if (getChargePointStatusService_C()->getEmergencyRelayClose() == false && flag_faultOccured_C == true){
						timeout_active_C = false;
						if ( (getChargePointStatusService_C()->getTransactionId() != -1)){ //can be buggy
							if(fault_counter_C++ > 1){
								fault_counter_C = 0;
								requestForRelay(START,3);
								delay(50);
								Serial.println("[EmergencyRelay_C] Starting Txn");
								flag_faultOccured_C = false;
							}
						}
					}

					if (timeout_active_C && getChargePointStatusService_C()->getTransactionId() != -1) {
						if (millis() - timeout_start_C >= TIMEOUT_EMERGENCY_RELAY_CLOSE){
							Serial.println("[EVSE_C] Emergency Stop.");
							flag_evRequestsCharge_C = false;
							flag_evseStopTransaction_C = true;
							timeout_active_C = false;
							timer_initialize_C = false;
						}
					}
				}
			}
	}
}

/*
* @param limit: expects current in amps from 0.0 to 32.0
*/
void EVSE_C_setChargingLimit(float limit) {
	if(DEBUG_OUT) Serial.print("[EVSE] New charging limit set. Got ");
	if(DEBUG_OUT) Serial.print(limit);
	if(DEBUG_OUT) Serial.print("\n");
	chargingLimit_C = limit;
}

bool EVSE_C_EvRequestsCharge() {
	return flag_evRequestsCharge_C;
}

bool EVSE_C_EvIsPlugged() {
	return evIsPlugged_C;
}


void EVSE_C_setOnBoot(OnBoot_C onBt_C){
	onBoot_C = onBt_C;
}

void EVSE_C_setOnReadUserId(OnReadUserId_C onReadUsrId_C){
	onReadUserId_C = onReadUsrId_C;
}

void EVSE_C_setOnsendHeartbeat(OnSendHeartbeat_C onSendHeartbt_C){
	onSendHeartbeat_C = onSendHeartbt_C;
}

void EVSE_C_setOnAuthentication(OnAuthentication_C onAuthenticatn_C){
	onAuthentication_C = onAuthenticatn_C;
}

void EVSE_C_setOnStartTransaction(OnStartTransaction_C onStartTransactn_C){
	onStartTransaction_C = onStartTransactn_C;
}

void EVSE_C_setOnStopTransaction(OnStopTransaction_C onStopTransactn_C){
	onStopTransaction_C = onStopTransactn_C;
}

void EVSE_C_setOnUnauthorizeUser(OnUnauthorizeUser_C onUnauthorizeUsr_C){
	onUnauthorizeUser_C = onUnauthorizeUsr_C;
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

String EVSE_C_getCurrnetIdTag(MFRC522 * mfrc522) {
    String currentIdTag = "";
	//currentIdTag_C = EVSE_C_readRFID(mfrc522);
	
	if (getChargePointStatusService_C()->getIdTag().isEmpty() == false){
		if(DEBUG_OUT) Serial.println("[EVSE_C] Reading from Charge Point Station Service ID Tag stored.");
		currentIdTag = getChargePointStatusService_C()->getIdTag();
		if(DEBUG_OUT) Serial.print("[EVSE_C] ID Tag: ");
		if(DEBUG_OUT) Serial.println(currentIdTag);
		Serial.flush();
	}
	
	return currentIdTag;
}


String EVSE_C_readRFID(MFRC522 * mfrc522) {
	String currentIdTag_C;	
	currentIdTag_C = readRfidTag(true, mfrc522);
	return currentIdTag_C;
}

/********Added new funtion @Wamique***********************/

// void EVSE_C_stopTransactionByRfid(){

// 	Ext_currentIdTag_C = EVSE_C_readRFID(&mfrc522);
// 	if(currentIdTag_C.equals(Ext_currentIdTag_C) == true){
// 		flag_evRequestsCharge_C = false;
// 		flag_evseStopTransaction_C = true;
// 	}else{
// 			if(Ext_currentIdTag_C.equals("") == false)
// 			if(DEBUG_OUT) Serial.println("\n[EVSE_C] Incorrect ID tag\n");
// 		}
// }

#if CP_ACTIVE
/**************CP Implementation @mwh*************/
void EVSE_C_StartCharging(){

	if(getChargePointStatusService_C()->getEvDrawsEnergy() == false){
		getChargePointStatusService_C()->startEvDrawsEnergy();
	}
    if (DEBUG_OUT) Serial.print(F("[EVSE_C] Opening Relays.\n"));
 //   pinMode(32,OUTPUT);
  //  digitalWrite(32, HIGH); //RELAY_1
    //digitalWrite(RELAY_2, RELAY_HIGH);
    requestForRelay(START,3);
    requestLed(ORANGE,START,3);
    delay(1200);
    requestLed(WHITE,START,3);
    delay(1200);
    requestLed(GREEN,START,3);
    delay(1000);
    Serial.println("[EVSE_C] EV is connected and Started charging");
	if(DEBUG_OUT) Serial.println("[EVSE_C] Started Drawing Energy");
	delay(500);
}


void EVSE_C_Suspended(){


	if(getChargePointStatusService_C()->getEvDrawsEnergy() == true){
		getChargePointStatusService_C()->stopEvDrawsEnergy();
	}
		requestLed(BLUE,START,3);     //replace 1 with connector ID
		requestForRelay(STOP,3);
	//	delay(1000);
		Serial.printf("[EVSE_C] EV Suspended");


}



/**************************************************/

#endif