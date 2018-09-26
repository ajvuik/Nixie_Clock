/*************************************************************************
 Title		:Analog.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:Any AVR device with an AD converter
 
Description:Analog input library
			This library can be used to get the analog value from the AD converter
			uses the timers library

***************************************************************************/

#ifndef ANALOG_H
#define ANALOG_H

#define Analog_Idle 0
#define Analog_Start 1
#define Analog_Wait 2

#define INIT_NOK 0
#define INIT_OK 1
//Define how many channels this chip has(count from 0)
#define Analog_Max_Channel 5 

//Includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../Timers/timers.h"

/*****************************************************************************/

typedef struct{
	uint8_t Conversion_Done;
	uint16_t Value;
	uint16_t Channel_Timeout_SV;
	uint16_t Channel_Timeout_PV;
}Channel_struct;

typedef struct{
	uint8_t Init;
	uint8_t Stap;
	uint8_t Curr_Channel;
	Channel_struct Channel[Analog_Max_Channel+1];
}Analog_struct;

//prototypes
void Analog_Init(void);
void Analog_loop (void);
int8_t Analog_Channel_Init(uint8_t _Channel, uint16_t _Timeout);
uint16_t Analog_Channel_Read(const uint8_t _Channel);
uint8_t Analog_Channel_Ready(const uint8_t _Channel);

#endif
