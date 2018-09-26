/*************************************************************************
 Title		:RTC.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7
 Hardware	:Any AVR device
 
 Description:Header file for RTC library
			 This library can be used to keep the time and date.

***************************************************************************/

#ifndef RTC_H
#define RTC_H
//includes
#include "../DS1307lib/DS1307.h"

//defines
#define Status_RTC_Idle		0
#define Status_RTC_Init		1
#define Status_RTC_Run		2
#define Status_RTC_Sync		3

#define RTC_Init_NOK		0
#define RTC_Init_OK			1

#define RTC_SyncIdle		0
#define RTC_SetSync			1
#define RTC_GetSync			2

//defines for functions
#define DEF_Ext_Sync_Ready				DS1307_Ready
#define DEF_RTC_Set_Sync_Function		DS1307_Set_DateTime
#define	DEF_RTC_Get_Sync_Function		DS1307_Get_DateTime
#define DEF_RTC_Set_IS_TWLF_Function	DS1307_Set_TWLF
#define DEF_RTC_Get_IS_TWLF_Function	DS1307_Get_TWLF
#define DEF_RTC_Set_IS_PM_Function		DS1307_Set_PM
#define DEF_RTC_Get_IS_PM_Function		DS1307_Get_PM
#define DEF_RTC_IS_Enable_Function		DS1307_Get_Enable
#define DEF_RTC_Enable_Function			DS1307_Enable
#define DEF_RTC_Disable_Function		DS1307_Disable

//Adjust the timeout on which the time should be updated, in s. This can be changed in program to.
#define timeout			10

//Declares
typedef uint8_t (*Ext_Sync_Ready)();
typedef void (*Set_Ext_Time_Function)();
typedef void (*Get_Ext_Time_Function)();
typedef uint8_t (*Get_Ext_IS_TWLF_Function)();
typedef void (*Set_Ext_IS_TWLF_Function)(uint8_t);
typedef uint8_t (*Get_Ext_IS_PM_Function)();
typedef void (*Set_Ext_IS_PM_Function)(uint8_t);
typedef uint8_t(*Get_Ext_IS_Enable_Function)();
typedef void(*Set_Ext_Enable_Function)();
typedef void(*Set_Ext_Disable_Function)();

typedef struct{
	uint8_t Second;
	uint8_t Minute;
	uint8_t Hour;
}RTC_time_struct;

typedef struct{
	uint8_t Day;
	uint8_t Month;
	uint8_t Year;
}RTC_date_struct;	

typedef struct{
	RTC_time_struct Time;
	RTC_date_struct Date;
	uint8_t Init;
	//uint8_t Run;
	uint8_t Status;
	uint8_t IS_TWLF;
	uint8_t IS_PM;
	uint8_t Sync_Type;
	uint8_t IS_Sync;
	uint8_t Ext_Tim2;
	uint8_t Ext_Tim2_1s;
	Ext_Sync_Ready Ext_Sync_Ready_Ptr;
	Set_Ext_Time_Function Set_Ext_Time_Function_Ptr;
	Get_Ext_Time_Function Get_Ext_Time_Function_Ptr;
	Set_Ext_IS_TWLF_Function Set_Ext_IS_TWLF_Function_Ptr;
	Get_Ext_IS_TWLF_Function Get_Ext_IS_TWLF_Function_Ptr;
	Set_Ext_IS_PM_Function Set_Ext_IS_PM_Function_Ptr;
	Get_Ext_IS_PM_Function Get_Ext_IS_PM_Function_Ptr;
	Get_Ext_IS_Enable_Function Get_Ext_IS_Enable_Function_Ptr;
	Set_Ext_Enable_Function Set_Ext_Enable_Function_Ptr;
	Set_Ext_Disable_Function Set_Ext_Disable_Function_Ptr;
	int Interval;
	int WorkInterval;
}RTC_struct;
RTC_struct RTC;//Structure is used by external functions

//Prototypes
void RTC_Loop(void);
void RTC_Init(void);
void RTC_USE_EXT_TIM2(void);

void RTC_Set_DateTime(uint8_t _Second, uint8_t _Minute, uint8_t _Hour, uint8_t _Day, uint8_t _Month, uint8_t _Year);
void RTC_Set_TWLF(uint8_t _TWLF);
uint8_t RTC_Get_TWLF();
void RTC_Set_PM(uint8_t _PM);
uint8_t RTC_Get_PM();
uint8_t RTC_Ready(void);

void RTC_Set_Ext_Sync_Ready_Function(Ext_Sync_Ready _Function_Ptr);
void RTC_Set_Sync_Function(Set_Ext_Time_Function _Function_Ptr);
void RTC_Get_Sync_Function(Get_Ext_Time_Function _Function_Ptr);
void RTC_Set_IS_TWLF_Function(Set_Ext_IS_TWLF_Function _Function_Ptr);
void RTC_Get_IS_TWLF_Function(Get_Ext_IS_TWLF_Function _Function_Ptr);
void RTC_Set_IS_PM_Function(Set_Ext_IS_PM_Function _Function_Ptr);
void RTC_Get_IS_PM_Function(Get_Ext_IS_PM_Function _Function_Ptr);
void RTC_Get_IS_Enable_Function(Get_Ext_IS_Enable_Function _Function_Ptr);
void RTC_Set_Enable_Function(Set_Ext_Enable_Function _Function_Ptr);
void RTC_Set_Disable_Function(Set_Ext_Disable_Function _Function_Ptr);

char RTC_not_leap(void);

#endif