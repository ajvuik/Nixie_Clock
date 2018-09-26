/*************************************************************************
 Title		:Analog.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:Any AVR device with an AD converter
 
 Description:Analog input library
			 This library can be used to get the analog value from the AD converter
			 uses the timers library

***************************************************************************/

#include "Analoglib/Analog.h"

Analog_struct Analog;

void Analog_Init(void){
	//Enable the analogue input
	ADMUX |= (0<<ADLAR)|(0<<REFS1)|(0<<REFS0)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);//ADLAR=0, first read ADCL then ADCH, reference = AREF(5V).
	ADCSRA |= (1<<ADEN)|(0<<ADSC)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);//Single conversion mode, 8MHZ/64=125KHZ this is well below the maximum of 200KHZ
	Analog.Curr_Channel=0;
	Analog.Init=INIT_OK;
	Analog.Stap=Analog_Idle;
}

void Analog_loop(void){
	if(Analog.Init){
		switch(Analog.Stap){
			case Analog_Idle:{
				//Check if something needs to be done
				if (Analog.Channel[Analog.Curr_Channel].Channel_Timeout_PV==0 && Analog.Channel[Analog.Curr_Channel].Channel_Timeout_SV>0){
					Analog.Stap=Analog_Start;
				}
				else{//Next channel
					Analog.Curr_Channel++;
					if (Analog.Curr_Channel>Analog_Max_Channel){
						Analog.Curr_Channel=0;
					}
				}
			}//Analog_Idle
			break;
			
			case Analog_Start:{
				uint8_t _Temp_MUX = ADMUX;
				_Temp_MUX &= 0xF0;
				_Temp_MUX += Analog.Curr_Channel;
				ADMUX = _Temp_MUX;
				ADCSRA |= (1<<ADSC);
				Analog.Stap=Analog_Wait;
			}//Analog_Start
			break;
			
			case Analog_Wait:{
				if (!(ADCSRA & (1<<ADSC))){
					Analog.Channel[Analog.Curr_Channel].Value=ADCL+(ADCH*256);
					//Analog.Channel[Analog.Curr_Channel].Value=(ADCH*256);
					Analog.Channel[Analog.Curr_Channel].Conversion_Done=1;
					Analog.Channel[Analog.Curr_Channel].Channel_Timeout_PV=Analog.Channel[Analog.Curr_Channel].Channel_Timeout_SV;
					Analog.Curr_Channel++;
					if (Analog.Curr_Channel>Analog_Max_Channel){
						Analog.Curr_Channel=0;
					}
					Analog.Stap=Analog_Idle;
				}
			}//Analog_Wait
			break;
		}//Switch
		//Timers
		uint8_t _Pulse_10ms = Timer_Pulse_10ms();
		for (uint8_t x=0; x<=Analog_Max_Channel; x++){
			Analog.Channel[x].Channel_Timeout_PV -= Analog.Channel[x].Channel_Timeout_PV && _Pulse_10ms;
		}
	}//If(Analog.Init)
}//Loop

int8_t Analog_Channel_Init(uint8_t _Channel, uint16_t _Timeout){
	if (_Channel<=Analog_Max_Channel){
		Analog.Channel[_Channel].Channel_Timeout_SV=_Timeout;
		return 1;
	}
	return -1;
}

uint16_t Analog_Channel_Read(const uint8_t _Channel){
	if(	Analog.Channel[_Channel].Conversion_Done){
		Analog.Channel[_Channel].Conversion_Done=0;
	}
	return Analog.Channel[_Channel].Value;
}

uint8_t Analog_Channel_Ready(const uint8_t _Channel){
	return Analog.Channel[_Channel].Conversion_Done;
}