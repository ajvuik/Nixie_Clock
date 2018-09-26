/*************************************************************************
 Title		:NixieClock.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:ATmega8.
 Description:Include file for NixieClock.c
			 This is a Nixie clock run on an ATmega8 with a I2C RTC to keep
			 the time. The RTC is battery backed so that the time is still
			 correct when the device loses power.

***************************************************************************/
#ifndef Nixie_H
#define Nixie_H
//This is here so that all the includes can use it
#ifndef F_CPU
#define F_CPU				8000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "Timers/timers.h"
#include "I2Clib/I2C.h"
#include "DS1307lib/DS1307.h"
#include "RTClib/RTC.h"
#include "Convertlib/Convert.h"
#include "Analoglib/Analog.h"
#include "Nixie_Power.h"
#include "EEPROMlib/EEPROMlib.h"

//Defines

//Define IO pins to the Nixies
#define Nixie1_2			PB0
#define Nixie3_4			PB1
#define Nixie5_6			PB2
#define Colon				PB4

//BCD's for Nixie 1,3 and 5
#define BCD1_0				PD0
#define BCD1_1				PD1
#define BCD1_2				PD2
#define BCD1_3				PD3

//BCD's for Nixie 2,4 and 6
#define BCD2_0				PD4
#define BCD2_1				PD5
#define BCD2_2				PD6
#define BCD2_3				PD7

#define Button1				PC1 //Advance in menu structure
#define Button2				PC2 //Advance digit/save setting in current menu

#define Debounce_Overshoot	30

//What do we show on the Nixies
#define Show_Nothing		0
#define Show_Time			1
#define Show_Date			2
#define Show_Menu			3

//Machine states
#define Status_Nixie_Idle	0
#define Status_Nixie_Start	1
#define Status_Nixie_Run	2
#define Status_Nixie_Menu	3
#define Status_Nixie_Save	4

#define Nixie_Blanking		3//ms
#define Next_Nixie_Time_on	5//ms

#define Nixie_Init_NOK		0
#define Nixie_Init_Wait		1
#define Nixie_Init_OK		2

#define Nixie_Safe_Off		0
#define Nixie_Save_AI		1
#define Nixie_Save_Time		2
#define Nixie_Save_AI_Time	3

//#define Setting_Menu_Size sizeof(N_Clock.Setting)
#define Nixie_File_ID		1

//Define an adress where to put the settings in the internal EEPROM
#define Nixie_EEPROM_Settings_Adress	0

//Prototypes for Nixie functions
void Nixie_Init(void);
void Nixie_Loop(void);
void Next_Nixie(void);
void Nixie_Update(const uint8_t _Command);

//Prototypes for setting functions
void Setting_Init(void);
void Setting_Loop(void);
void Safe_Settings(void);
void Read_Settings(void);

//Declares

//Struct for AT24C32 file
typedef struct{
	uint8_t Date_Show;
	uint8_t Date_Timeout_SV;
	uint8_t Date_Show_SV;
	uint8_t Save_Active;
	uint16_t Save_Off_Threshold;
	uint16_t Save_On_Threshold;
	uint16_t Save_AI;
	RTC_time_struct Save_Time_OFF;
	RTC_time_struct Save_Time_ON;
	uint16_t Save_Timeout_SV;
}Nixie_EEPROM_STruct;

//Nixie setting structure
typedef struct{
	uint8_t Init;
	uint8_t Write_Settings;
	uint8_t Date_Show;
	uint8_t Date_Timeout_SV;
	uint8_t Date_Timeout_PV;
	uint8_t Date_Show_SV;
	uint8_t Date_Show_PV;
	uint8_t Save_Active;
	uint16_t Save_Off_Threshold;
	uint16_t Save_On_Threshold;
	uint16_t Save_AI;
	RTC_time_struct Save_Time_OFF;
	RTC_time_struct Save_Time_ON;
	uint16_t Save_Timeout_SV;
	uint16_t Save_Timeout_PV;
	uint16_t EEPROM_File_Size;
	Nixie_EEPROM_STruct EEPROM;
	uint8_t Blanking_SV;
	uint8_t Time_On_SV;
}Nixie_Setting_struct;

//Nixie menu structure
typedef struct{
	uint8_t Menu;
	uint8_t Sub_Menu;
	uint8_t Timer;
}Debounce_struct;

//Nixie Clock structure
typedef struct{
	uint8_t Cur_Nixie;
	uint8_t BCD_Nixie1_2;
	uint8_t BCD_Nixie3_4;
	uint8_t BCD_Nixie5_6;
	uint8_t Menu_Nixie3_4;
	uint8_t Menu_Nixie5_6;
	uint8_t Status;
	uint8_t Show;
	uint8_t Menu;
	uint8_t Blink;
	uint8_t Nixie_Blanking_Timer;
	uint8_t Next_Nixie_Timer;
	Nixie_Setting_struct Setting;
}Nixie_struct;

#endif