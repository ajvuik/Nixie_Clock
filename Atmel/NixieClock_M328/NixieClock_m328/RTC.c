/*************************************************************************
 Title		:RTC.C
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7
 Hardware	:Any AVR
 
 Description:RTC library
			 This library can be used to keep the time and date.

***************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include "Timers/timers.h"
#include "RTClib/RTC.h"


void RTC_Loop(void){
	uint8_t _Pulse_1S;
	//uint8_t _Pulse_1S = Timers.Pulse_1s;
	if (RTC.Ext_Tim2){
		if(RTC.Ext_Tim2_1s){
			_Pulse_1S		= 1;
			RTC.Ext_Tim2_1s	= 0;
		}
		else{
			_Pulse_1S		= 0;
		} 
	}
	else{
		_Pulse_1S = Timer_Pulse_1s();
	}
		
	switch(RTC.Status){
		case Status_RTC_Idle:{
			//no init yet, do nothing
		}
		break;
		
		case Status_RTC_Init:{
			RTC.Init=RTC_Init_OK;
			RTC.WorkInterval=1;
			RTC.IS_Sync=0;
			RTC.Sync_Type=RTC_GetSync;
			RTC.Status=Status_RTC_Sync;
		}//RTC_Init
		break;
		
		case Status_RTC_Run:{
			//1st see if we need to sync with an external time keeper
			if (RTC.Sync_Type==RTC_SetSync){
				RTC.Status=Status_RTC_Sync;
				break;
			}
			else if (RTC.WorkInterval==0){
				if (RTC.Interval>0){
					RTC.WorkInterval=RTC.Interval;
				}
				else if (timeout){
					RTC.WorkInterval=timeout;
				}
				RTC.Sync_Type=RTC_GetSync;
				RTC.Status=Status_RTC_Sync;
				break;
			}
			else if (RTC.Ext_Sync_Ready_Ptr() && !RTC.IS_Sync){
				RTC.IS_Sync=1;
				RTC.IS_TWLF=RTC.Get_Ext_IS_TWLF_Function_Ptr()>0;
				if (RTC.IS_TWLF){
					RTC.IS_PM=RTC.Get_Ext_IS_PM_Function_Ptr()>0;
				}
				
			}

			RTC.Time.Second += _Pulse_1S;
			//RTC.Time.Second += _Pulse_1S;
			//Update the time
			//if( _Pulse_1S>0 && RTC.Init>0){
				//RTC.Time.Second++;
				if (RTC.Time.Second>59){
					RTC.Time.Second=0;
					if (++RTC.Time.Minute>59){
						RTC.Time.Minute=0;
						RTC.Time.Hour++;
						uint8_t _Next_Day=0;
						if (RTC.IS_TWLF){
							if (RTC.IS_PM && RTC.Time.Hour>11){
								RTC.Time.Hour=0;
								RTC.IS_PM=0;
								_Next_Day++;
							}
							else if (RTC.Time.Hour>12){
								RTC.Time.Hour=1;
								RTC.IS_PM++;
							}
						}
						else if (RTC.Time.Hour>23){
							RTC.Time.Hour=0;
							_Next_Day++;
						}
						if (_Next_Day){
							uint8_t _Next_Month=0;
							RTC.Date.Day++;
							if (RTC.Date.Month==2){
								if (RTC.Date.Day>=28 && RTC_not_leap()){
									RTC.Date.Day=1;
									_Next_Month++;
								}
								else if (RTC.Date.Day>=29){
									RTC.Date.Day=1;
									_Next_Month++;
								}
							}
							else if (RTC.Date.Day>=30 && (RTC.Date.Month==4 || RTC.Date.Month==7 || RTC.Date.Month==9 || RTC.Date.Month==11)){
									RTC.Date.Day=1;
									_Next_Month++;
							}
							else if (RTC.Date.Day>=31){
									RTC.Date.Day=1;
									_Next_Month++;
							}
							if (_Next_Month){
								if (++RTC.Date.Month>12){
									RTC.Date.Month=1;
									if (++RTC.Date.Year>99){
										RTC.Date.Year=0;
									}
								}
							}//_Next_Month
						}//_Next_Day
					}//minutes
				}//Seconds
			//}//if(Timers.Pulse_1s && RTC.Init)
		}//RTC_Run
		break;
		
		case Status_RTC_Sync:{//we need to sync with whatever we need to sync with.
			if(RTC.Ext_Sync_Ready_Ptr>0 && RTC.Ext_Sync_Ready_Ptr()){
				if (RTC.Get_Ext_IS_Enable_Function_Ptr>0 && RTC.Get_Ext_IS_Enable_Function_Ptr()==0 && RTC.Set_Ext_Enable_Function_Ptr>0){
					RTC.Set_Ext_Enable_Function_Ptr();
				}
				if (RTC.Set_Ext_Time_Function_Ptr>0 && RTC.Sync_Type==RTC_SetSync){
					RTC.Set_Ext_Time_Function_Ptr((uint8_t *) &RTC.Time.Second);
					RTC.Set_Ext_IS_PM_Function_Ptr(RTC.IS_PM);
					RTC.Set_Ext_IS_TWLF_Function_Ptr(RTC.IS_TWLF);
				}
				else if (RTC.Get_Ext_Time_Function_Ptr>0 && RTC.Sync_Type==RTC_GetSync){
					RTC.Get_Ext_Time_Function_Ptr((uint8_t *) &RTC.Time.Second);
					RTC.IS_PM=RTC.Get_Ext_IS_PM_Function_Ptr();
					RTC.IS_TWLF=RTC.Get_Ext_IS_TWLF_Function_Ptr();
				}
			}
			//Even if the external sync fails, keep running the RTC
			RTC.Sync_Type=RTC_SyncIdle;
			RTC.IS_Sync=0;
			RTC.Status=Status_RTC_Run;
		}//RTC_Sync
		break;
		
		default:{//something went wrong
			RTC.Status=Status_RTC_Idle;
		}//Default
		break;
	}//switch(RTC.Status)
	RTC.WorkInterval -= RTC.WorkInterval && _Pulse_1S;
}//RTC_Loop()

void RTC_Set_DateTime(uint8_t _Second, uint8_t _Minute, uint8_t _Hour, uint8_t _Day, uint8_t _Month, uint8_t _Year){
	//Time values
	//Set the given seconds
	if(_Second<60){//Check if the value is not out of bound.
		RTC.Time.Second=_Second;
	}
	//Set the given minutes
	if (_Minute<60){//Check if the value is not out of bound.
		RTC.Time.Minute=_Minute;
	}
	//Set the given hour
	if (!RTC.IS_TWLF && _Hour<24){//Check if the value is not out of bound.
		RTC.Time.Hour=_Hour;
	}
	else if (RTC.IS_PM && _Hour<12){
		RTC.Time.Hour=_Hour;
	}
	else if(!RTC.IS_PM && _Hour<13){
		RTC.Time.Hour=_Hour;
	}
	
	//Date values
	//Set the given year
	if (_Year<100){
		RTC.Date.Year=_Year;
	}
	//Set the given month
	if (_Month<13){
		RTC.Date.Month=_Month;
	}
	//set the given day
	if (RTC.Date.Month==2){
		if (RTC_not_leap() && _Day<29){
			RTC.Date.Day=_Day;
		}
		else if (_Day<30){
			RTC.Date.Day=_Day;
		}
	}
	else if (_Day<31 && (RTC.Date.Month==4 || RTC.Date.Month==7 || RTC.Date.Month==9 || RTC.Date.Month==11)){
		RTC.Date.Day=_Day;
	}
	else if (_Day<32){
		RTC.Date.Day=_Day;
	}
	//RTC has been updated, sync it with an external device if needed
	RTC.Sync_Type=RTC_SetSync;
	//RTC.Status=Status_RTC_Sync;
}//RTC_Set_DateTime

void RTC_Init(void){
	RTC_Set_Ext_Sync_Ready_Function(DEF_Ext_Sync_Ready);
	RTC_Set_Sync_Function(DEF_RTC_Set_Sync_Function);
	RTC_Get_Sync_Function(DEF_RTC_Get_Sync_Function);
	RTC_Set_IS_TWLF_Function(DEF_RTC_Set_IS_TWLF_Function);
	RTC_Get_IS_TWLF_Function(DEF_RTC_Get_IS_TWLF_Function);
	RTC_Set_IS_PM_Function(DEF_RTC_Set_IS_PM_Function);
	RTC_Get_IS_PM_Function(DEF_RTC_Get_IS_PM_Function);
	RTC_Get_IS_Enable_Function(DEF_RTC_IS_Enable_Function);
	RTC_Set_Enable_Function(DEF_RTC_Enable_Function);
	RTC_Set_Disable_Function(DEF_RTC_Disable_Function);
	RTC.Status=Status_RTC_Init;
}

void RTC_USE_EXT_TIM2(void){
	RTC.Ext_Tim2=1;
	ASSR	= (1<<AS2);
	TCNT2	= 0;
	TIMSK2	= (1<<TOIE2);
	TCCR2A	= 0;
	TCCR2B	= 5;
}

void RTC_Set_TWLF(uint8_t _TWLF){
	if (RTC.Set_Ext_IS_TWLF_Function_Ptr>0){
		RTC.Set_Ext_IS_TWLF_Function_Ptr(_TWLF);
		RTC.IS_TWLF=_TWLF;
	}
}

uint8_t RTC_Get_TWLF(){
	if(RTC.Get_Ext_IS_TWLF_Function_Ptr>0){
		return RTC.Get_Ext_IS_TWLF_Function_Ptr();
	}
	else{
		return RTC.IS_TWLF;
	}
}

void RTC_Set_PM(uint8_t _PM){
	if (RTC.Set_Ext_IS_PM_Function_Ptr>0){
		RTC.Set_Ext_IS_PM_Function_Ptr(_PM);
		RTC.IS_PM=_PM;
	}
}

uint8_t RTC_Get_PM(){
	if (RTC.Get_Ext_IS_PM_Function_Ptr>0){
		return RTC.Get_Ext_IS_PM_Function_Ptr();
	}
	else{
		return RTC.IS_PM;
	}
}


uint8_t RTC_Ready(void){
	return RTC.Status==Status_RTC_Run && RTC.Sync_Type==RTC_SyncIdle;
}
void RTC_Set_Ext_Sync_Ready_Function(Ext_Sync_Ready _Function_Ptr){
	RTC.Ext_Sync_Ready_Ptr = _Function_Ptr;	
}

void RTC_Set_Sync_Function(Set_Ext_Time_Function _Function_Ptr){
	RTC.Set_Ext_Time_Function_Ptr = _Function_Ptr;	
}

void RTC_Get_Sync_Function(Get_Ext_Time_Function _Function_Ptr){
	RTC.Get_Ext_Time_Function_Ptr = _Function_Ptr;
}

void RTC_Set_IS_TWLF_Function(Set_Ext_IS_TWLF_Function _Function_Ptr){
	RTC.Set_Ext_IS_TWLF_Function_Ptr=_Function_Ptr;
}

void RTC_Get_IS_TWLF_Function(Get_Ext_IS_TWLF_Function _Function_Ptr){
	RTC.Get_Ext_IS_TWLF_Function_Ptr=_Function_Ptr;
}

void RTC_Set_IS_PM_Function(Set_Ext_IS_PM_Function _Function_Ptr){
	RTC.Set_Ext_IS_PM_Function_Ptr=_Function_Ptr;
}

void RTC_Get_IS_PM_Function(Get_Ext_IS_PM_Function _Function_Ptr){
	RTC.Get_Ext_IS_PM_Function_Ptr=_Function_Ptr;
}

void RTC_Get_IS_Enable_Function(Get_Ext_IS_Enable_Function _Function_Ptr){
	RTC.Get_Ext_IS_Enable_Function_Ptr=_Function_Ptr;
}

void RTC_Set_Enable_Function(Set_Ext_Enable_Function _Function_Ptr){
	RTC.Set_Ext_Enable_Function_Ptr=_Function_Ptr;
}

void RTC_Set_Disable_Function(Set_Ext_Disable_Function _Function_Ptr){
	RTC.Set_Ext_Disable_Function_Ptr=_Function_Ptr;
}

char RTC_not_leap(void){      //check for leap year
	int year = RTC.Date.Year + 2000;
	if (!(year%100))
	return (char)(year%400);
	else
	return (char)(year%4);
}

ISR(TIMER2_OVF_vect){
	RTC.Ext_Tim2_1s++;
}

