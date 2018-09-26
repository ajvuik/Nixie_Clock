/*************************************************************************
 Title		:DS1307.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 6.2
 Hardware	:Any AVR device with I2C/TWI
 
 Description:Header file for DS1307 library
			 This library this library can be used to Set and Get the time from
			 a DS1307 type I2C RTC

***************************************************************************/

#ifndef DS1307_H
#define DS1307_H

#include <avr/io.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../Timers/timers.h"
#include "../I2Clib/I2C.h"
#include "../Convertlib/Convert.h"

//defines

//Adjust the timeout on which the time should be updated, in ms.
#define Retry_Timeout				10//s
//Here you can define the address of the RTC. Put in the real RTC address, the function shifts 1 left
//and makes the read or write bit according to the I2C protocol.
#define DS1307_Adress				0x68

//Don't change this unless you know what your are doing
//Registers of the DS1307
#define Second_Reg					0x00
#define Minute_Reg					0x01
#define Hour_Reg					0x02
#define DOW_Reg						0x03
#define Day_Reg						0x04
#define Month_Reg					0x05
#define Year_Reg					0x06
#define Setting_Reg					0x07
#define DS1307_RAM_Begin			0x08
#define DS1307_RAM_End				0x3F

//Masks for time registers
#define Second_Mask					0b01111111
#define Minute_Mask					0b01111111
#define Hour_Mask_12				0b00011111
#define Hour_Mask_24				0b00111111
#define Day_Mask					0b00111111
#define Month_Mask					0b00011111
//#define Year_Mask

//Bit shifts for the register bits
#define CH							7
#define PM							5
#define TWLF						6
#define RS0							0
#define RS1							1
#define	SQWE						4
#define OUT							7

//Status defines
#define Status_DS1307_Idle			0
#define Status_DS1307_Init			1

//Get status defines
#define Status_DS1307_GetTime		2
#define Status_DS1307_GetDate		3
#define Status_DS1307_GetDateTime	4
#define Status_DS1307_GetSettings	5	

//Set status defines
#define Status_DS1307_SetTime		11
#define Status_DS1307_SetDate		12
#define Status_DS1307_SetDateTime	13
#define Status_DS1307_SetSettings	14

#define Status_DS1307_Enable		21
#define Status_DS1307_Disable		22

//Rest
#define Status_DS1307_Busy			99

//includes
//#include "I2Clib/I2Clib.h"

//Declares
typedef struct{
	uint8_t Second;
	uint8_t Minute;
	uint8_t Hour;
}DS1307_time_struct;

typedef struct{
	uint8_t Day;
	uint8_t Month;
	uint8_t Year;
}DS1307_date_struct;	

typedef struct{
	uint8_t sCH;
	//uint8_t sAM;
	uint8_t sTWLF;
	uint8_t sRS0;
	uint8_t sRS1;
	uint8_t sSQWE;
	uint8_t sOUT;
}DS1307_setting_struct;

typedef struct{
	DS1307_time_struct Time;
	DS1307_date_struct Date;
	DS1307_setting_struct Setting;
	uint8_t Init;
	uint8_t Status;
	uint8_t Timer;
	uint8_t Is_PM;
	uint8_t Done;
	uint8_t I2C_Ident;
	uint8_t Old_status;
	uint8_t Command;
	uint8_t *DataPtr;
	uint8_t buffer[8];
}DS1307_struct;

//Prototypes
void DS1307_Loop(void);
void DS1307_Init(void);
uint8_t DS1307_Ready(void);

void DS1307_Get_Time(uint8_t *_Time_Ptr);
void DS1307_Get_Date(uint8_t *_Date_Ptr);
void DS1307_Get_DateTime(uint8_t *_DateTime_Ptr);
void DS1307_Get_Settings(void);

uint8_t DS1307_Get_TWLF(void);
uint8_t DS1307_Get_PM(void);

void DS1307_Set_Time(uint8_t *_Time_Ptr);
void DS1307_Set_Date(uint8_t *_Date_Ptr);
void DS1307_Set_DateTime(uint8_t *_DateTime_Ptr);
void DS1307_Set_Settings(uint8_t _OUT, const uint8_t _SQWE, const uint8_t _RS1, const uint8_t _RS0);
void DS1307_Set_TWLF(uint8_t _TWLF);
void DS1307_Set_PM(uint8_t _PM);

void DS1307_Enable(void);
void DS1307_Disable(void);
uint8_t DS1307_Get_Enable();

char DS1307_not_leap(void);

#endif