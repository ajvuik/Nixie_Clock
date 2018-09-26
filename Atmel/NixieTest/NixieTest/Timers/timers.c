/*************************************************************************
 Title		:timers.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 6.2
 Hardware	:Any AVR device, correct counter needs to be set, 
			 read data sheet.
 Description:Timers library
			 This library creates 4 pulses which can be used to create unobtrusive timers.

***************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Timers/timers.h"


void Timer_loop (void)
{
	//First reset the timers
	Pulse_reset();
	
	if(Tim0_FL > 0)
	{
		Tim0_FL = 0;
		Timers.Pulse_100us=1;
		Timers.Count_1ms++;
		Timers.Count_10ms++;
		Timers.Count_100ms++;
		Timers.Count_1s++;

		if (Timers.Count_1ms > 9){
			Timers.Count_1ms=0;
			Timers.Pulse_1ms=1;
		}
		if(Timers.Count_10ms > 99)
		{
			Timers.Count_10ms = 0;
			Timers.Pulse_10ms = 1;
		}
		if(Timers.Count_100ms > 999)
		{
			Timers.Count_100ms = 0;
			Timers.Pulse_100ms = 1;
		}
		if(Timers.Count_1s > 9999)
		{
			Timers.Count_1s = 0;
			Timers.Pulse_1s = 1;
		}
	}

}

void Timer_Init(void)
{
	//TCCR0	|=	(1 << WGM01);//CTC mode
	TCCR0 	|= (0 << CS02 )|(1 << CS01)|(0<<CS00);//CLK/8
	TIMSK 	|= (1 << TOIE0);//Enable Compare A interrupt
	TCNT0	=	0x100; //Overflow value
}

void Pulse_reset(void)
{
	Timers.Pulse_100us = 0;
	Timers.Pulse_1ms = 0;
	Timers.Pulse_10ms = 0;
	Timers.Pulse_100ms = 0;
	Timers.Pulse_1s = 0;
}

ISR(TIMER0_OVF_vect) {
	TCNT0=0x100; //Overflow value
	Tim0_FL++;
}

