/*************************************************************************
 Title		:timers.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:Any AVR device with hardware counter, correct counter, counter settings and
			 reload value needs to be set, read the data sheet for the correct settings.
 Description:Header file for timers library
					This library creates 5 pulses which can be used to create un-obstructive timers.

***************************************************************************
Version:
		see .h file
		
***************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Timers/timers.h"

Timers_struct Timers;

void Timer_loop (void)
{
	//First reset the timers
	Pulse_reset();
	
	if(Timer_Flank > 0)
	{
		Timer_Flank = 0;
		Timers.Pulse_1ms=1;
		//Timers.Count_1ms++;
		Timers.Count_10ms++;
		Timers.Count_100ms++;
		Timers.Count_1s++;

		//if (Timers.Count_1ms > 9){
		//	Timers.Count_1ms=0;
		//	Timers.Pulse_1ms=1;
		//}
		if(Timers.Count_10ms > 9){
			Timers.Count_10ms = 0;
			Timers.Pulse_10ms = 1;
		}
		if(Timers.Count_100ms > 99){
			Timers.Count_100ms = 0;
			Timers.Pulse_100ms = 1;
		}
		if(Timers.Count_1s > 999){
			Timers.Count_1s = 0;
			Timers.Pulse_1s = 1;
		}
	}

}

void Timer_Init(void){
	//TCCR0	|=	(1 << WGM01);//CTC mode
	Timer_TCCR_A			= Timer_TCCR_A_Setting;//See timers.h file for settings
	#ifdef Timer_TCCR_B
	Timer_TCCR_B			= Timer_TCCR_B_Setting;
	#endif
	Timer_TIMSK 			|= (1 << Timer_Overflow_Interrupt);//Enable interrupt
	Timer_Counter_Register	=  Timer_Reload; //Overflow value
	TIFR0 |= (1<<OCF0B)|(1<<OCF0B);
}

void Pulse_reset(void){
	//Timers.Pulse_100us = 0;
	Timers.Pulse_1ms = 0;
	Timers.Pulse_10ms = 0;
	Timers.Pulse_100ms = 0;
	Timers.Pulse_1s = 0;
}

//uint8_t Timer_Pulse_100us(void){
//	return Timers.Pulse_100us;
//}

uint8_t Timer_Pulse_1ms(void){
	return Timers.Pulse_1ms;
}

uint8_t Timer_Pulse_10ms(void){
	return Timers.Pulse_10ms;
}

uint8_t Timer_Pulse_100ms(void){
	return Timers.Pulse_100ms;
}

uint8_t Timer_Pulse_1s(void){
	return Timers.Pulse_1s;
}

ISR(Timer_Overvlow_Vector){
	//Timer_Counter_Register=Timer_Reload; //Overflow value
	Timer_Flank++;
	//TIFR0 |= (1<<OCF0A)|(1<<OCF0B)|(1<<TOV0);
}

