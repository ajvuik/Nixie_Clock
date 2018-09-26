/*************************************************************************
 Title		:timers.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:Any AVR device with hardware counter, correct counter, counter settings and 
			reload value needs to be set, read the data sheet for the correct settings.
 Description:Header file for timers library
			This library creates 5 pulses which can be used to create un-obstructive timers.

**************************************************************************

Version:
V1.30.0 08-02-2017
		Timers structure is no longer global and made the pulses available through functions

V1.20.0	17-12-2016
		Moved options to .H file

V1.10.0	5-12-2016
		100us pulse added.

V1.00.0 XX-XX-XXXX
		Working version

V0.00.1	XX-XX-XXXX
		Initial version
*************************************************************************/

#ifndef TIMERS_H
#define TIMERS_H

//Includes
//#include <avr/io.h>

//Defines

/*ATMega 8 datasheet gives the following for pre-scaling:
CS2	CS1	CS1	Discription						Decimal value
0	0	0	Timer disconnected				0
0	0	1	No pre-scaling					1
0	1	0	/8								2
0	1	1	/64								3
1	0	0	/265							4
1	0	1	/1024							5
1	1	0	External clock, falling edge	6
1	1	1	External clock, rising edge		7
*/

#define Timer_Prescale						1 // 001 BIN = 1 DEC = /1
//#define Timer_Prescale					2 // 010 BIN = 2 DEC = /8
//#define Timer_Prescale					3 // 011 BIN = 3 DEC = /64

//Adjust to the CPU specific vector and timer you want to use
#define Timer_Overvlow_Vector			TIMER1_COMPA_vect

#define Timer_TCCR_A_Setting			0
#define Timer_TCCR_B_Setting			Timer_Prescale + (1 << WGM12)
#define Timer_TCCR_C_Setting			0

//Set the correct Timer Counter Control Register (TCCR)
#define Timer_TCCR_A					TCCR1A //timer configuration
#define Timer_TCCR_B					TCCR1B //Timer configuration
#define Timer_TCCR_C					TCCR1C //Timer configuration

//Set the correct Timer Interrupt Mask Register
#define Timer_TIMSK						TIMSK1
#define Timer_Overflow_Interrupt		OCIE1A
#define Timer_TIFR						TIFR1

#define Timer_Mask_Bit1					ICF1
#define Timer_Mask_Bit2					OCF1B

//Set the correct Timer CouNter Register
#define Timer_Counter_Register			OCR1A

/*	To calculate the reload value take the frequency, divide this with the pre-scale setting. 
	Next you divide 1 with value you now have.
	Last step is to divide the time you want with the calculated value*/
//#define Timer_Reload					100 //= 100us/(1/(8mhz/8)))
//#define Timer_Reload					125 //= 1ms/(1/(8mhz/64)))
//#define Timer_Reload					8000 //= 1ms/(1/(8mhz/1)))
#define Timer_Reload					7999 //= 1ms/(1/(8mhz/1)))

/*****************************************************************************/

//Declares
volatile uint8_t Timer_Flank; //Volatile so it doesn't get optimized away 

//Create pulses
typedef struct{
	//uint8_t Pulse_100us;
	uint8_t Pulse_1ms;
	uint8_t Pulse_10ms;
	uint8_t Pulse_100ms;
	uint8_t Pulse_1s;
	//uint8_t Count_1ms;
	uint8_t Count_10ms;
	uint16_t Count_100ms;
	uint16_t Count_1s;
}Timers_struct;

//Timers_struct Timers;

//prototypes
void Timer_Init(void);
void Timer_loop (void);
void Pulse_reset(void);
uint8_t Timer_Pulse_100us(void);
uint8_t Timer_Pulse_1ms(void);
uint8_t Timer_Pulse_10ms(void);
uint8_t Timer_Pulse_100ms(void);
uint8_t Timer_Pulse_1s(void);

//ISR (TIMER0_OVF_vect); This is already defined in the CPU specific .h file

#endif
