/*************************************************************************
 Title		:timers.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 6.2
 Hardware	:Any AVR device, correct counter needs to be set, 
			 read data sheet.
Description:Header file for timers library
			This library creates 4 pulses which can be used to create unobtrusive timers.

***************************************************************************/

#ifndef TIMERS_H
#define TIMERS_H

//Includes
//#include <avr/io.h>

/*****************************************************************************/


volatile uint8_t Tim0_FL;

//Create pulses
typedef struct{
	uint8_t Pulse_100us;
	uint8_t Pulse_1ms;
	uint8_t Pulse_10ms;
	uint8_t Pulse_100ms;
	uint8_t Pulse_1s;
	uint8_t Count_1ms;
	uint8_t Count_10ms;
	uint16_t Count_100ms;
	uint16_t Count_1s;
}Timers_struct;
Timers_struct Timers;

//prototypes
void Timer_Init(void);
void Timer_loop (void);
void Pulse_reset(void);
//ISR (TIMER0_OVF_vect);


#endif
