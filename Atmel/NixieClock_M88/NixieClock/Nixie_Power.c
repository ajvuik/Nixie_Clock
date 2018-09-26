/*************************************************************************
 Title		:Nixie_Power.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:ATmega8.
 Description:This is the power function to control the power for
				the Nixie Clock

***************************************************************************/

//Includes
#include "Nixie_Power.h"

Power_struct N_Power;

//Power functions
void Power_Init(void){
	//The required voltage for the Nixies is created by a boost converter. From the website https://learn.adafruit.com/diy-boost-calc/the-calculator I calculated that
	// when you use a 330uH coil you have to generate a 60KHZ block wave to get 170~190V.
	Power_TCCRA = Power_TCCRA_Setting;//See .h file
	#ifdef Power_TCCRB
	Power_TCCRB = Power_TCCRB_Setting;//See .h file
	#endif
	Power_OCR = 0x78;//8MHZ/66(0x42) = ~121.2121/2 = a block wave of ~60.6060KHZ
	Power_DDR |= (1<<Power_Pin);//Make the powerpin output
	N_Power.Power_Threshold_SV = Power_Threshold;
	Analog_Channel_Init(Power_Analog_Channel, Power_Analog_Timeout);
	Analog_Channel_Init(Power_Analog_Save_Channel, Power_Analog_Save_Timeout);
}//Power_Init

void Power_Loop(void){
	if (Analog_Channel_Ready(Power_Analog_Channel)){
		N_Power.Analog_In=Analog_Channel_Read(Power_Analog_Channel);
	}
	switch(N_Power.mode){	
		case Power_Mode_OFF:{
			Power_TCCRA &= ~((1<<Power_COMx1)|(1<<Power_COMx0));//Disconnect the output pin
			Power_Port &= ~(1<<Power_Pin);//Make sure it is really low
		}//Mode_Power_OFF
		break;
		case Power_Mode_ON:{//This has to be tested. Will shutting the frequency on and off work or do I need to change the OCR2 value?
			if (N_Power.Analog_In>N_Power.Power_Threshold_SV)//if the voltage is to high
			{
				Power_TCCRA &= ~((1<<Power_COMx1)|(1<<Power_COMx0));//Disconnect the output pin
				Power_Port &= ~(1<<Power_Pin);//Make sure it is really low
			}
			else{
				Power_TCCRA |= (1<<Power_COMx0);//Else toggle PB3(OC2)
			}
		}//Mode_Power_ON
		break;
	}//N_Power.mode
}//Power_Loop

void Power_Mode(const uint8_t _Command){
	N_Power.mode=_Command;
}