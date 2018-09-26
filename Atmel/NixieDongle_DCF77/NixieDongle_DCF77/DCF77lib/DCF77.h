/*************************************************************************
 Title		:DCF77.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:Any AVR device with external hardware interrupt.
 Description:Header file for DCF77 library

**************************************************************************

Version:
V0.00.1	XX-XX-XXXX
		Initial version
*************************************************************************/

#ifndef DCF_H
#define DCF_H

#include "../Timers/timers.h"
volatile uint8_t DCF77_Flank; //Volatile so it doesn't get optimized away

typedef struct {
	uint8_t Timer;
	uint8_t SummerTime;
	uint8_t	Day;
	uint8_t Month;
	uint8_t Year;
	uint8_t LeapSecond;
	uint8_t	Minute;
	uint8_t	Hour;
	uint8_t SecondCounter;
	uint8_t SecondCounterOld;
	uint8_t InSync;
	uint8_t StartSignal;	 
} DCF_struct;
void DCF_INIT();
void DCF_LOOP();
//ISR (TIMER0_OVF_vect); This is already defined in the CPU specific .h file

#endif
