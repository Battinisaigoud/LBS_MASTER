#include "ControlPilot.h"
#include "Master.h"
#include "display.h"

#if EVSE_3S_ADC_VALUES
volatile uint32_t evse_state_a_upper_threshold = 4096;
volatile uint32_t evse_state_a_lower_threshold = 3900;
volatile uint32_t evse_state_b_upper_threshold = 3500;
volatile uint32_t evse_state_b_lower_threshold = 2600;
volatile uint32_t evse_state_c_upper_threshold = 747;
volatile uint32_t evse_state_c_lower_threshold = 626;
volatile uint32_t evse_state_d_upper_threshold = 625;
volatile uint32_t evse_state_d_lower_threshold = 500;
volatile uint32_t evse_state_e_threshold = 499;
volatile uint32_t evse_state_sus_upper_threshold = 877;
volatile uint32_t evse_state_sus_lower_threshold = 748;
volatile uint32_t evse_state_e2_upper_threshold = 2399;
volatile uint32_t evse_state_e2_lower_threshold = 2100;
volatile uint32_t evse_state_dis_upper_threshold = 1050;
volatile uint32_t evse_state_dis_lower_threshold = 878;

#endif

#if EVSE_6_6_ADC_VALUES

volatile uint32_t evse_state_a_upper_threshold_6 = 4096;
volatile uint32_t evse_state_a_lower_threshold_6 = 3900;
volatile uint32_t evse_state_b_upper_threshold_6 = 3450;
volatile uint32_t evse_state_b_lower_threshold_6 = 2600;
volatile uint32_t evse_state_c_upper_threshold_6 = 1300;
volatile uint32_t evse_state_c_lower_threshold_6 = 1140;
volatile uint32_t evse_state_d_upper_threshold_6 = 625;
volatile uint32_t evse_state_d_lower_threshold_6 = 500;
volatile uint32_t evse_state_e_threshold_6 = 499;
volatile uint32_t evse_state_sus_upper_threshold_6 = 1520;
volatile uint32_t evse_state_sus_lower_threshold_6 = 1360;
volatile uint32_t evse_state_e2_upper_threshold_6 = 2399;
volatile uint32_t evse_state_e2_lower_threshold_6 = 2100;
volatile uint32_t evse_state_dis_upper_threshold_6 = 1770;
volatile uint32_t evse_state_dis_lower_threshold_6 = 1600;



#endif

#if EVSE_7S_ADC_VALUES

volatile uint32_t evse_state_a_upper_threshold_7 = 4096;
volatile uint32_t evse_state_a_lower_threshold_7 = 3500;
volatile uint32_t evse_state_b_upper_threshold_7 = 3500;
volatile uint32_t evse_state_b_lower_threshold_7 = 2900;
volatile uint32_t evse_state_c_upper_threshold_7 = 1551;
volatile uint32_t evse_state_c_lower_threshold_7 = 1150;
volatile uint32_t evse_state_d_upper_threshold_7 = 1150;
volatile uint32_t evse_state_d_lower_threshold_7 = 850;
volatile uint32_t evse_state_e_threshold_7 = 850;
volatile uint32_t evse_state_sus_upper_threshold_7 = 1850;
volatile uint32_t evse_state_sus_lower_threshold_7 = 1550;
volatile uint32_t evse_state_e2_upper_threshold_7 = 2800;
volatile uint32_t evse_state_e2_lower_threshold_7 = 2100;
volatile uint32_t evse_state_dis_upper_threshold_7 = 2100;
volatile uint32_t evse_state_dis_lower_threshold_7 = 1850;




#endif



/*
const int pwm_pin 		 	= PWM_PIN;
const int pwm_freq 		 	= PWM_FREQUENCY;
const int pwm_channel 	 	= PWM_CHANNEL;
const int pwm_resolution   	= PWM_RESOLUTION;

const int potPin = 34;    //Pilot In  GPIO35

*/
bool flag_controlPAuthorise_A = false;
extern volatile float device_load;

// volatile float device_load = 7.40f;
// int dutyCycle = 64; //Hard coded for 15Amps
float defaultCurrentLimit = 32.0f; // Take care for every new Charge point //To slave Hardcode @anesh
extern float chargingLimit;
// bool flag_pwm = false;
EVSE_states_enum EVSE_state;
extern bool SessionStop;
PILOT_readings_t PILOT_reading;
extern uint8_t gu8_finishing_state;

/*
void SetPwmOn(){

	if((EVSE_state == STATE_B || EVSE_state == STATE_C || EVSE_state == STATE_D)){
		//start pwm
		int dutyC = CalculateDutyCycle(chargingLimit);
		Serial.println("[ControlPilot]Set maximum Current limit->" + String(chargingLimit));
		ledcWrite(pwm_channel,dutyC);     //To Slave
		Serial.println("[ControlPilot] Pwm ON");
	}else{

		Serial.print("[ControlPilot] Incorrect state for Turning ON Pwm: " + String(EVSE_state));
	}
}


void SetPwmOff(){  // testing phase as its not working the way it has to.

	//	ledcWrite(pwm_channel,0);
		ledcWrite(pwm_channel,255);                //To Slave
		Serial.println("[ControlPilot] Pwm OFF");

}
*/
/*
void SetStateA(){
	ledcWrite(pwm_channel,255);              //To slave
//	digitalWrite(pwm_pin,HIGH);
	Serial.println("[ControlPilot] STATE_A is Set");

}

void ControlPSetup(){             //To slave
	//pinmode(pwm_pin,OUTPUT);
	ledcSetup(pwm_channel, pwm_freq, pwm_resolution); //configured functionalities

	ledcAttachPin(pwm_pin , pwm_channel);

	analogReadResolution(12);
}
*/
/*
void ControlPBegin(){
	ledcWrite(pwm_channel,255);     //to slave
	EVSE_state = STATE_A;
	Serial.println("[ControlPilot] EVSE_state is STATE_A");

}
*/

void ControlPRead()
{

	int ADC_Result = 0;
	/*
	for(int i=0 ; i<50 ;i++){

		ADC_Result += analogRead(potPin);//Took 5 samples to remove any noise  //To slave
	}

	ADC_Result = ADC_Result/50;

	Serial.println("[ControlPilot] ADC Value : ");
	Serial.println(ADC_Result);
	*/

	ADC_Result = requestforCP_IN();
	Serial.println("[ControlPilot] ADC Value : ");
	Serial.println(ADC_Result);
	/* ADC result is between 3.2 and 2.8 volts */
	if (device_load == 3.30f)
	{
		if ((ADC_Result < evse_state_a_upper_threshold) && /*(ADC_Result > 935)*/ (ADC_Result > evse_state_a_lower_threshold))
		{
			PILOT_reading = V_12;
			gu8_finishing_state = 0;
			if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
			{
				// EVSE_transaction_status = EVSE_NO_TRANSACTION;
				evse_ChargePointStatus = Available;
				// requestLed(GREEN,START,1);
			}

		}
		else if ((ADC_Result < evse_state_b_upper_threshold) && (ADC_Result > evse_state_b_lower_threshold)) // defualt 2800
		{
			PILOT_reading = V_9;
			if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
			{
				// requestLed(BLUE,START,1);
				if (gu8_finishing_state == 1)
				{
					if (SessionStop == true)
					{
						evse_ChargePointStatus = Preparing;
					}
					else
					{
						evse_ChargePointStatus = Finishing;
					}
				}
				else
				{
					evse_ChargePointStatus = Preparing;
				}
			}

		}
		else if ((ADC_Result < evse_state_sus_upper_threshold) && (ADC_Result > evse_state_sus_lower_threshold))
		{
			PILOT_reading = V_SUS;
		}
		else if ((ADC_Result < evse_state_dis_upper_threshold) && (ADC_Result > evse_state_dis_lower_threshold))
		{
			PILOT_reading = V_DIS;

		}
		else if ((ADC_Result < evse_state_c_upper_threshold) && (ADC_Result > evse_state_c_lower_threshold))
		{
			PILOT_reading = V_6;
			evse_ChargePointStatus = Charging;

		}
		else if ((ADC_Result < evse_state_d_upper_threshold) && (ADC_Result > evse_state_d_lower_threshold)) // testing)
		{
			PILOT_reading = V_3;

		}
		else if ((ADC_Result < evse_state_e_threshold)) // 0.4V
		{
			PILOT_reading = V_UNKNOWN;

		}
		else
		{
			PILOT_reading = V_DEFAULT;

			Serial.println("ADC values are not in range");
		}
	}
	if (device_load == 6.60f)
	{
		if ((ADC_Result < evse_state_a_upper_threshold_6) && /*(ADC_Result > 935)*/ (ADC_Result > evse_state_a_lower_threshold_6))
		{
			PILOT_reading = V_12;
			gu8_finishing_state = 0;
			if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
			{
				// EVSE_transaction_status = EVSE_NO_TRANSACTION;
				evse_ChargePointStatus = Available;
				// requestLed(GREEN,START,1);
			}

		}
		else if ((ADC_Result < evse_state_b_upper_threshold_6) && (ADC_Result > evse_state_b_lower_threshold_6)) // defualt 2800
		{
			PILOT_reading = V_9;
			if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
			{
				// requestLed(BLUE,START,1);
				if (gu8_finishing_state == 1)
				{
					if (SessionStop == true)
					{
						evse_ChargePointStatus = Preparing;
					}
					else
					{
						evse_ChargePointStatus = Finishing;
					}
				}
				else
				{
					evse_ChargePointStatus = Preparing;
				}
			}

		}
		else if ((ADC_Result < evse_state_sus_upper_threshold_6) && (ADC_Result > evse_state_sus_lower_threshold_6))
		{
			PILOT_reading = V_SUS;
		}
		else if ((ADC_Result < evse_state_dis_upper_threshold_6) && (ADC_Result > evse_state_dis_lower_threshold_6))
		{
			PILOT_reading = V_DIS;

		}
		else if ((ADC_Result < evse_state_c_upper_threshold_6) && (ADC_Result > evse_state_c_lower_threshold_6))
		{
			PILOT_reading = V_6;
			evse_ChargePointStatus = Charging;

		}
		else if ((ADC_Result < evse_state_d_upper_threshold_6) && (ADC_Result > evse_state_d_lower_threshold_6)) // testing)
		{
			PILOT_reading = V_3;

		}
		else if ((ADC_Result < evse_state_e_threshold_6)) // 0.4V
		{
			PILOT_reading = V_UNKNOWN;

		}
		else
		{
			PILOT_reading = V_DEFAULT;

			Serial.println("ADC values are not in range");
		}


	}
	if (device_load == 7.40f)
	{
		if ((ADC_Result < evse_state_a_upper_threshold_7) && /*(ADC_Result > 935)*/ (ADC_Result > evse_state_a_lower_threshold_7))
		{
			PILOT_reading = V_12;
			gu8_finishing_state = 0;
			if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
			{
				// EVSE_transaction_status = EVSE_NO_TRANSACTION;
				evse_ChargePointStatus = Available;
				// requestLed(GREEN,START,1);
			}

		}
		else if ((ADC_Result < evse_state_b_upper_threshold_7) && (ADC_Result > evse_state_b_lower_threshold_7)) // defualt 2800
		{
			PILOT_reading = V_9;
			if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
			{
				// requestLed(BLUE,START,1);
				if (gu8_finishing_state == 1)
				{
					if (SessionStop == true)
					{
						evse_ChargePointStatus = Preparing;
					}
					else
					{
						evse_ChargePointStatus = Finishing;
					}
				}
				else
				{
					evse_ChargePointStatus = Preparing;
				}
			}

		}
		else if ((ADC_Result < evse_state_sus_upper_threshold_7) && (ADC_Result > evse_state_sus_lower_threshold_7))
		{
			PILOT_reading = V_SUS;
		}
		else if ((ADC_Result < evse_state_dis_upper_threshold_7) && (ADC_Result > evse_state_dis_lower_threshold_7))
		{
			PILOT_reading = V_DIS;

		}
		else if ((ADC_Result < evse_state_c_upper_threshold_7) && (ADC_Result > evse_state_c_lower_threshold_7))
		{
			PILOT_reading = V_6;
			evse_ChargePointStatus = Charging;

		}
		else if ((ADC_Result < evse_state_d_upper_threshold_7) && (ADC_Result > evse_state_d_lower_threshold_7)) // testing)
		{
			PILOT_reading = V_3;

		}
		else if ((ADC_Result < evse_state_e_threshold_7)) // 0.4V
		{
			PILOT_reading = V_UNKNOWN;

		}
		else
		{
			PILOT_reading = V_DEFAULT;

			Serial.println("ADC values are not in range");
		}

	}
#if 0
	if ((ADC_Result < 4096) && /*(ADC_Result > 935)*/ (ADC_Result > 3500))
	{
		PILOT_reading = V_12;
		gu8_finishing_state = 0;
		if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
		{
			// EVSE_transaction_status = EVSE_NO_TRANSACTION;
			evse_ChargePointStatus = Available;
			// requestLed(GREEN,START,1);
		}
	}

	/* ADC result is between 2.8 and 2.33 volts */
	else if ((ADC_Result < 3500) && (ADC_Result > 2800))
	{
		PILOT_reading = V_9;
		if ((evse_ChargePointStatus != Faulted) && (evse_ChargePointStatus != Unavailable))
		{
			// requestLed(BLUE,START,1);
			if (gu8_finishing_state == 1)
			{
				if (SessionStop == true)
				{
					evse_ChargePointStatus = Preparing;
				}
				else
				{
					evse_ChargePointStatus = Finishing;
				}
			}
			else
			{
				evse_ChargePointStatus = Preparing;
			}
		}
	}

	/* ADC result is between 1.49 and 1.32 volts */ // implemented based on observation
#if V_charge_lite1_4
	else if ((ADC_Result < 1850) && (ADC_Result > 1550))
	{
		PILOT_reading = V_SUS;
	}
#else
	else if ((ADC_Result < 1750) && (ADC_Result > 1550))
	{
		PILOT_reading = V_SUS;
	}
#endif

	/* ADC result is between 1.73 and 1.49 volts */
#if V_charge_lite1_4
	else if ((ADC_Result < 2100) && (ADC_Result > 1850))
	{
		PILOT_reading = V_DIS;
	}
#else
	else if ((ADC_Result < 2100) && (ADC_Result > 1750))
	{
		PILOT_reading = V_DIS;
	}
#endif

	/* ADC result is between 1.32 and 1.08 volts */
	else if ((ADC_Result < 1550) && (ADC_Result > 1150))
	{
		PILOT_reading = V_6;
		evse_ChargePointStatus = Charging;
	}

	/* ADC result is between 1.08 and 0.60 volts */
	else if ((ADC_Result < 1150) && (ADC_Result > 850)) // testing)
	{
		PILOT_reading = V_3;
	}

	else if ((ADC_Result < 850)) // 0.4V
	{
		PILOT_reading = V_UNKNOWN;
	}

	else
	{
		PILOT_reading = V_DEFAULT;

		Serial.println("ADC values are not in range");
	}
#endif
}
/*

int CalculateDutyCycle(float chargingLimit){

	int dutycycle_l = 0;

	if((chargingLimit <= 51) &&(chargingLimit > 5)){

		dutycycle_l = ((chargingLimit / 0.6) * 2.55);
		Serial.println("[ControlPilot] Duty Cycle is = " + String(dutycycle_l));

	}else if((chargingLimit < 80) &&(chargingLimit > 51)){

		dutycycle_l = (((chargingLimit / 2.5) + 64 ) * 2.55 );
		Serial.println("[ControlPilot] Duty Cycle is = " + String(dutycycle_l));

	}else{

		Serial.println("[ControlPilot] chargingLimit is not in range");
	}

	return dutycycle_l;
}

*/
void ControlP_loop()
{

	/* Future Implementation @mwh*/
	/*	if(defaultCurrentLimit != chargingLimit)
		{

			if(EVSE_state == STATE_C || EVSE_state == STATE_D){
				int newDutyCyle = CalculateDutyCycle(chargingLimit);
				Serial.println("Charging Limit is changed to " + String(chargingLimit) + "\n Starting new PWM Signal");
				ledcWrite(pwm_channel,newDutyCyle); //setting new pwm

				defaultCurrentLimit = chargingLimit;
			}else{
				defaultCurrentLimit = chargingLimit;
				Serial.println("[ControlPilot] Charging limit is changed");
			}
		}*/
		/******************************************/
	ControlPRead();
	// delay(100);

	if (PILOT_reading == V_12)
	{

		EVSE_state = STATE_A;
		Serial.println("[CP] State A");
	}
	else if (PILOT_reading == V_9)
	{

		EVSE_state = STATE_B;
		Serial.println("[CP] State B");
	}
	else if (PILOT_reading == V_SUS)
	{

		EVSE_state = STATE_SUS;
		evse_ChargePointStatus = SuspendedEV;
		evse_ChargePointStatus = SuspendedEVSE;
		requestLed(BLINKYBLUE, START, 1);
		Serial.println("[CP] State SUSPENDED");
	}
	else if (PILOT_reading == V_DIS)
	{

		EVSE_state = STATE_DIS;
		Serial.println("[CP] State DISCONNECTED");
	}
	else if (PILOT_reading == V_6)
	{

		EVSE_state = STATE_C;
		Serial.println("[CP] State C");
	}
	else if (PILOT_reading == V_3)
	{

		EVSE_state = STATE_D;
		Serial.println("[CP] State D");
	}
	else if (PILOT_reading == V_UNKNOWN)
	{ // can Implement here defaults state

		EVSE_state = STATE_E; // error
		Serial.println("[CP] State E");
	}
	else
	{

		Serial.println("[CP] Unknown State :ADC value not in range");
	}

	/*	while(EVSE_state == STATE_E){
			delay(1000);
			ControlPRead();
		}
	*/
	if (EVSE_state == STATE_B)
	{
		if (flag_controlPAuthorise_A == true)
		{
			// SetPwmOn();
			// Serial.println(flag_controlPAuthorise);
			// requestforCP_OUT(START);
			// delay(500);
			flag_controlPAuthorise_A = false;
		}
	}
}