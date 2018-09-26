/*************************************************************************
 Title		:Nixie_Power.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:ATmega8.
 Description:Include file for Nixie_Power.h
				This is the power function to control the power for
				the Nixie Clock

***************************************************************************/
#ifndef Nixie_Power_H
#define Nixie_Power_H

#ifndef F_CPU
	#define F_CPU			8000000UL
#endif

#include <avr/io.h>
#include <stdlib.h>
#include <stdbool.h>
#include "Timers/timers.h"
#include "Analoglib/Analog.h"

//Defines
#define Power_Threshold				555

//Chip specific defines
#define Power_TCCRA					TCCR2A
#define Power_TCCRB					TCCR2B
#define Power_OCR					OCR2A

#define Power_WGMx0					WGM20
#define Power_WGMx1					WGM21
#define Power_CSx0					CS20
#define Power_CSx1					CS21
#define Power_CSx2					CS22
#define Power_COMx0					COM2A0
#define Power_COMx1					COM2A1

#ifdef Power_TCCRB
	#define Power_TCCRA_Setting (1<<Power_WGMx1)//TCT mode, no prescaler
	#define Power_TCCRB_Setting (1<<Power_CSx0)
#else
	#define Power_TCCRA_Setting (1<<Power_WGMx1)|(1<<Power_CSx0)//TCT mode, no prescaler
#endif
//Define IO pins to for the power
#define Power_Pin					PB3

//Define ports
#define Power_Port					PORTB
#define Power_DDR					DDRB

#define Power_Mode_OFF				0
#define Power_Mode_ON				1
#define Power_Analog_Channel		0 //Channel 0 = PC0
#define Power_Analog_Timeout		1 //0ms
#define Power_Analog_Save_Channel	1 //Channel 1 = PC1
#define Power_Analog_Save_Timeout	600 // only once every 6 seconds
//#define Power_Save_Timeout			300 //5 minutes


//Nixie power supply structure
typedef struct{
	uint8_t mode;
	uint16_t Analog_In;
	uint16_t Power_Threshold_SV;
}Power_struct;

//Prototypes for power functions
void Power_Init(void);
void Power_Loop(void);
void Power_Mode(const uint8_t _Command);
#endif