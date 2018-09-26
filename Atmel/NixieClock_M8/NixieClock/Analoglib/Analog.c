/*************************************************************************
 Title	:   timers.c
 Author:    Arjan Vuik <ajvuik@hotmail.com>
 Software:  Atmel studio 6.1
 Hardware:  any AVR device, correct counter needs to be set, 
			read datasheet.
Description:Timers library
			This library creates 4 pulses which can be used to create unintrusive timers.

***************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
//#include "timers.h"

uint8_t Count_10ms;
uint8_t Count_100ms;
int Count_1s;

void Timers (void)
{
	//First reset the timers
	Pulse_reset();
	
	if(Tim0_FL > 0)
	{
		Tim0_FL = 0;
		Pulse_1ms = 1;
		Count_10ms ++;
		Count_100ms++;
		Count_1s++;

		if(Count_10ms > 9)
		{
			Count_10ms = 0;
			Pulse_10ms = 1;
		}
		if(Count_100ms > 99)
		{
			Count_100ms = 0;
			Pulse_100ms = 1;
		}
		if(Count_1s > 999)
		{
			Count_1s = 0;
			Pulse_1s = 1;
		}
	}

}

void Timer_Init(void)
{
	TCCR0A	|=	(1 << WGM01);//CTC mode
	TCCR0B 	|= (1 << CS02 )|(0 << CS01)|(0<<CS00);//CLK/128
	TIMSK 	|= (1 << OCIE0A);//Enable Compare A interrupt
	OCR0A	+=	0x4D; //Overflow value
}

void Pulse_reset(void)
{
	Pulse_1ms = 0;
	Pulse_10ms = 0;
	Pulse_100ms = 0;
	Pulse_1s = 0;
}

ISR (TIMER0_COMPA_vect)
{
	Tim0_FL++;
}

