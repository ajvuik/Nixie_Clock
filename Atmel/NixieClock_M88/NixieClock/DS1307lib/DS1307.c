/*************************************************************************
 Title		:DS1307.C
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7
 Hardware	:Any AVR device with I2C/TWI
 
 Description:DS1307 library
			 This library can be used to set and get the time from
			 a DS1307 type I2C RTC
			 Uses I2Clib.

***************************************************************************/

#include "DS1307lib/DS1307.h"

DS1307_struct DS1307;

void DS1307_Loop(void){
	switch(DS1307.Status){
		case Status_DS1307_Idle:{
			if (DS1307.Init){
				if (DS1307.Command){
					DS1307.Status=DS1307.Command;
				}
			}
			else if (DS1307.Timer==0){
				DS1307.Status=DS1307.Command=Status_DS1307_Init;
			}

		}//Status_DS1307_Idle
		break;
		
		case Status_DS1307_Init:{
			if (DS1307.I2C_Ident==0){
				DS1307.I2C_Ident=I2C_Get_Ident(7, DS1307_Adress);
			}
			else if (DS1307.I2C_Ident>0 && I2C_Ready(DS1307.I2C_Ident)){
				DS1307.Status=DS1307.Command=Status_DS1307_GetSettings;
			}
		}//Status_DS1307_Init
		break;
		
		//Get functions
		//Get time
		case Status_DS1307_GetTime:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				DS1307.buffer[0]=Second_Reg;
				I2C_Send_STr(&DS1307.buffer[0], &DS1307.buffer[1], sizeof(DS1307.buffer[0]),sizeof(DS1307.Time), DS1307.I2C_Ident);
				DS1307.Old_status=DS1307.Status;
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				
				//Get the seconds from the BCD code
				DS1307.Time.Second = BCDToDecimal(DS1307.buffer[1]&Second_Mask);
				//Get the minutes from the BCD code
				DS1307.Time.Minute = BCDToDecimal(DS1307.buffer[2]&Minute_Mask);
				//Get the hours from the BCD code
				if (DS1307.Setting.sTWLF){
					DS1307.Time.Hour = BCDToDecimal(DS1307.buffer[3]&Hour_Mask_12);
					DS1307.Is_PM = DS1307.buffer[3]&(1<<PM);
				}
				else{
					DS1307.Time.Hour = BCDToDecimal(DS1307.buffer[3]&Hour_Mask_24);
				}
				if (DS1307.DataPtr>0){
					memcpy(DS1307.DataPtr, &DS1307.Time, sizeof(DS1307.Time));
				}

				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}//Status_DS1307_GetTime
		break;
		
		//Get Date
		case Status_DS1307_GetDate:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				DS1307.buffer[0]=Day_Reg;
				I2C_Send_STr(&DS1307.buffer[0], &DS1307.buffer[1], sizeof(DS1307.buffer[0]),sizeof(DS1307.Time)+sizeof(DS1307.Date)+1, DS1307.I2C_Ident);
				DS1307.Old_status=DS1307.Status;
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				
				//Get the day from the BCD code
				DS1307.Date.Day = BCDToDecimal(DS1307.buffer[1]&Day_Mask);
				//Get the month from the BCD code
				DS1307.Date.Month = BCDToDecimal(DS1307.buffer[2]&Month_Mask);
				//Get the year from the BCD code
				DS1307.Date.Year = BCDToDecimal(DS1307.buffer[3]);
				if (DS1307.DataPtr>0){
					memcpy(DS1307.DataPtr, &DS1307.Date, sizeof(DS1307.Date));
				}
				
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}//Status_DS1307_GetTime
		break;

		//Get time and date
		case Status_DS1307_GetDateTime:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				DS1307.buffer[0]=Second_Reg;
				I2C_Send_STr(&DS1307.buffer[0], &DS1307.buffer[1], sizeof(DS1307.buffer[0]),sizeof(DS1307.Time)+sizeof(DS1307.Date)+1, DS1307.I2C_Ident);
				DS1307.Old_status=DS1307.Status;
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				if (DS1307.Init==INIT_NOK){
					//check if the connected DS1307 is running
					if (DS1307.buffer[1] & (1<<CH)){
						DS1307.Setting.sCH=1;
					}
					DS1307.Init=INIT_OK;
					DS1307.Setting.sTWLF=DS1307.buffer[3]&(1<<TWLF);
				}
				
				//Get the seconds from the BCD code
				DS1307.Time.Second = BCDToDecimal(DS1307.buffer[1]&Second_Mask);
				//Get the minutes from the BCD code
				DS1307.Time.Minute = BCDToDecimal(DS1307.buffer[2]&Minute_Mask);
				//Get the hours from the BCD code
				if (DS1307.Setting.sTWLF){
					DS1307.Time.Hour = BCDToDecimal(DS1307.buffer[3]&Hour_Mask_12);
					DS1307.Is_PM = DS1307.buffer[3]&(1<<PM);
				}
				else{
					DS1307.Time.Hour = BCDToDecimal(DS1307.buffer[3]&Hour_Mask_24);
				}
				
				//Get the day from the BCD code
				DS1307.Date.Day = BCDToDecimal(DS1307.buffer[5]&Day_Mask);
				//Get the month from the BCD code
				DS1307.Date.Month = BCDToDecimal(DS1307.buffer[6]&Month_Mask);
				//Get the year from the BCD code
				DS1307.Date.Year = BCDToDecimal(DS1307.buffer[7]);
				if (DS1307.DataPtr>0){
					memcpy(DS1307.DataPtr, &DS1307.Time, sizeof(DS1307.Time) + sizeof(DS1307.Date));
				}
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}//Status_DS1307_GetDateTime
		break;
		
		//Get settings from the DS1307
		case Status_DS1307_GetSettings:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){
				DS1307.buffer[0]=Setting_Reg;
				I2C_Send_STr(&DS1307.buffer[0], &DS1307.buffer[1], sizeof(DS1307.buffer[0]), sizeof(DS1307.buffer[1]), DS1307.I2C_Ident);
				DS1307.Old_status=DS1307.Status;
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				//get the settings from the received register
				DS1307.Setting.sOUT=DS1307.buffer[1]&(1<<OUT);
				DS1307.Setting.sSQWE=DS1307.buffer[1]&(1<<SQWE);
				DS1307.Setting.sRS1=DS1307.buffer[1]&(1<<RS1);
				DS1307.Setting.sRS0=DS1307.buffer[1]&(1<<RS0);
				if (DS1307.Init==INIT_NOK){
					DS1307.Command=DS1307.Status=Status_DS1307_GetDateTime;
				}
				else{
					//Done return to idle
					DS1307.Command=DS1307.Status=Status_DS1307_Idle;
				}
			}
		}//Status_DS1307_GetSettings
		break;
		
		//Here are the functions to set the time from DS1307
		case Status_DS1307_SetTime:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				//Clear the buffer to remove any stray data
				memset(DS1307.buffer,0,sizeof(DS1307.buffer));
				//Set the correct register
				DS1307.buffer[0] = Second_Reg;
				//Convert time to BCD and add the correct setting bits
				DS1307.buffer[1]=DecimalToBCD(DS1307.Time.Second) | (DS1307.Setting.sCH<<CH);
				DS1307.buffer[2]=DecimalToBCD(DS1307.Time.Minute) & Minute_Mask;
				if(DS1307.Setting.sTWLF){
					DS1307.buffer[3]=DecimalToBCD(DS1307.Time.Hour) | (DS1307.Setting.sTWLF<<TWLF) | (DS1307.Is_PM<<PM);
				}
				else{
					DS1307.buffer[3]=DecimalToBCD(DS1307.Time.Hour) | (DS1307.Setting.sTWLF<<TWLF);
				}
				//Send the time to the DS1307
				I2C_Send_ST(&DS1307.buffer[0], sizeof(DS1307.Time)+sizeof(DS1307.buffer[0]), DS1307.I2C_Ident);
				//Remember where we came from
				DS1307.Old_status=DS1307.Status;
				//Tell the world we are busy
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}//Status_DS1307_SetTime
		break;	

		//Here are the functions to set the date from the DS1307
		case Status_DS1307_SetDate:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				//Clear the buffer to remove any stray data
				memset(DS1307.buffer,0,sizeof(DS1307.buffer));
				//Set the correct register
				DS1307.buffer[0] = Day_Reg;
				//Convert date to BCD and remove any stray bits
				DS1307.buffer[1]=DecimalToBCD(DS1307.Date.Day) & Day_Mask;
				DS1307.buffer[2]=DecimalToBCD(DS1307.Date.Month) & Month_Mask;
				DS1307.buffer[3]=DecimalToBCD(DS1307.Date.Year);

				I2C_Send_ST(&DS1307.buffer[0], sizeof(DS1307.Time)+sizeof(DS1307.buffer[0]), DS1307.I2C_Ident);
				//Remember where we came from
				DS1307.Old_status=DS1307.Status;
				//Tell the world we are busy
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}//Status_DS1307_SetDate
		break;
		
		//Here are the functions to set the time from DS1307
		case Status_DS1307_SetDateTime:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				//Clear the buffer to remove any stray data
				memset(DS1307.buffer,0,sizeof(DS1307.buffer));
				memcpy(&DS1307.Time.Second, DS1307.DataPtr, sizeof(DS1307.Time)+sizeof(DS1307.Date)+1);
				//Set the correct register
				DS1307.buffer[0] = Second_Reg;
				//Convert time to BCD and add the correct setting bits
				DS1307.buffer[1]=DecimalToBCD(DS1307.Time.Second) | (DS1307.Setting.sCH<<CH);
				DS1307.buffer[2]=DecimalToBCD(DS1307.Time.Minute) & Minute_Mask;
				if (DS1307.Setting.sTWLF){
					DS1307.buffer[3]=DecimalToBCD(DS1307.Time.Hour) | (DS1307.Setting.sTWLF<<TWLF) | (DS1307.Is_PM<<PM);
				}
				else{
					DS1307.buffer[3]=DecimalToBCD(DS1307.Time.Hour) | (DS1307.Setting.sTWLF<<TWLF);
				}
				//Convert date to BCD and add the correct setting bits
				DS1307.buffer[5]=DecimalToBCD(DS1307.Date.Day) & Day_Mask;
				DS1307.buffer[6]=DecimalToBCD(DS1307.Date.Month) & Month_Mask;
				DS1307.buffer[7]=DecimalToBCD(DS1307.Date.Year);
				//Send the time to the DS1307
				I2C_Send_ST(&DS1307.buffer[0], sizeof(DS1307.buffer[0])+sizeof(DS1307.Time)+sizeof(DS1307.Date)+1, DS1307.I2C_Ident);
				//Remember where we came from
				DS1307.Old_status=DS1307.Status;
				//Tell the world we are busy
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}//Status_DS1307_SetTime
		break;

		//Set settings is they are changed
		case Status_DS1307_SetSettings:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				//Clear the buffer to remove any stray data
				memset(DS1307.buffer,0,sizeof(DS1307.buffer));
				//Set the correct register
				DS1307.buffer[0] = Setting_Reg;
				DS1307.buffer[1] = (DS1307.Setting.sOUT<<OUT) | (DS1307.Setting.sSQWE<<SQWE) | (DS1307.Setting.sRS1<<RS1) | (DS1307.Setting.sRS0<<RS0);
				I2C_Send_ST(&DS1307.buffer[0], sizeof(DS1307.buffer[0])+sizeof(DS1307.buffer[1]), DS1307.I2C_Ident);
				//Remember where we came from
				DS1307.Old_status=DS1307.Status;
				//Tell the world we are busy
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}//Status_DS1307_SetSettings
		break;
		
		case Status_DS1307_Enable:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				//Clear the buffer to remove any stray data
				memset(DS1307.buffer,0,sizeof(DS1307.buffer));
				//Set the correct register
				DS1307.buffer[0] = Second_Reg;
				DS1307.buffer[1] = DecimalToBCD(DS1307.Time.Second) & ~(1<<CH);
				I2C_Send_ST(&DS1307.buffer[0], sizeof(DS1307.buffer[0])+sizeof(DS1307.buffer[1]), DS1307.I2C_Ident);
				//Remember where we came from
				DS1307.Old_status=DS1307.Status;
				//Tell the world we are busy
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
		}
		break;//Status_DS1307_Enable
		
		case Status_DS1307_Disable:{
			if (I2C_Ready(DS1307.I2C_Ident) && DS1307.Done==0){//check if the I2C isn't busy
				//Clear the buffer to remove any stray data
				memset(DS1307.buffer,0,sizeof(DS1307.buffer));
				//Set the correct register
				DS1307.buffer[0] = Second_Reg;
				DS1307.buffer[1] = DecimalToBCD(DS1307.Time.Second) | (1<<CH);
				I2C_Send_ST(&DS1307.buffer[0], sizeof(DS1307.buffer[0])+sizeof(DS1307.buffer[1]), DS1307.I2C_Ident);
				//Remember where we came from
				DS1307.Old_status=DS1307.Status;
				//Tell the world we are busy
				DS1307.Status=Status_DS1307_Busy;
			}
			else if (DS1307.Done){
				DS1307.Done=0;
				//Done return to idle
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;
			}
			
		}
		break;//Status_DS1307_Disable
		
		//The status is set to busy if the 
		case Status_DS1307_Busy:{
			if (I2C_ID_State(DS1307.I2C_Ident)==State_I2C_NO_ACK){//No ACK received from the chip, we assume it's not there (anymore)
				DS1307.Command=DS1307.Status=Status_DS1307_Idle;//clear the command and return to idle
				I2C_ID_Ack_Error(DS1307.I2C_Ident);
				DS1307.Timer=Retry_Timeout;
			}
			else if (I2C_Ready(DS1307.I2C_Ident)){//Check if the I2C is finished
				DS1307.Status=DS1307.Old_status;
				DS1307.Done++;
			}
		}
		break;
	}
	DS1307.Timer -= DS1307.Timer && Timers.Pulse_1s;
}

void DS1307_Init(void){
	memset(DS1307.buffer, 0, sizeof(DS1307.buffer));
	DS1307.Done=0;
	DS1307.Init=INIT_NOK;
	if (DS1307.I2C_Ident){
		I2C_Free_Ident(DS1307.I2C_Ident);
	}
	DS1307.I2C_Ident=0;
	DS1307.Timer=0;
	DS1307.Command=Status_DS1307_Init;
}

uint8_t DS1307_Ready(void){
	return DS1307.Status==Status_DS1307_Idle && DS1307.Init==INIT_OK && DS1307.Command==Status_DS1307_Idle;
}

//Get functions
void DS1307_Get_Time(uint8_t *_Time_Ptr){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.DataPtr=_Time_Ptr;
		DS1307.Command=Status_DS1307_GetTime;
	}
}

void DS1307_Get_Date(uint8_t *_Date_Ptr){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.DataPtr=_Date_Ptr;
		DS1307.Command=Status_DS1307_GetDate;
	}
}

void DS1307_Get_DateTime(uint8_t *_DateTime_Ptr){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.DataPtr=_DateTime_Ptr;
		DS1307.Command=Status_DS1307_GetDateTime;
	}
}

void DS1307_Get_Settings(void){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.Command=Status_DS1307_GetSettings;
	}
}

uint8_t DS1307_Get_TWLF(void){
	return DS1307.Setting.sTWLF;
}
uint8_t DS1307_Get_PM(void){
	return DS1307.Is_PM;
}

//Set functions
void DS1307_Set_Time(uint8_t *_Time_Ptr){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.DataPtr=_Time_Ptr;
		DS1307.Command=Status_DS1307_SetTime;
	}
}
void DS1307_Set_Date(uint8_t *_Date_Ptr){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.DataPtr=_Date_Ptr;
		DS1307.Command=Status_DS1307_SetDate;
	}
}

void DS1307_Set_DateTime(uint8_t *_DateTime_Ptr){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.DataPtr=_DateTime_Ptr;
		DS1307.Command=Status_DS1307_SetDateTime;	
	}
}

void DS1307_Set_Settings(uint8_t _OUT, uint8_t _SQWE, uint8_t _RS1, uint8_t _RS0){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.Setting.sOUT=_OUT>0;
		DS1307.Setting.sSQWE=_SQWE>0;
		DS1307.Setting.sRS1=_RS1>0;
		DS1307.Setting.sRS0=_RS0>0;
		DS1307.Command=Status_DS1307_SetSettings;
	}
}

void DS1307_Set_TWLF(uint8_t _TWLF){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.Setting.sTWLF=_TWLF>0;
		if (DS1307.Time.Hour>12 && _TWLF){
			DS1307.Time.Hour-=12;
			DS1307.Is_PM=1;
		}
		else if (DS1307.Is_PM && !_TWLF){
			DS1307.Time.Hour+=12;
			DS1307.Is_PM=0;
		}
		DS1307.Command=Status_DS1307_SetTime;
	}
}

void DS1307_Set_PM(uint8_t _PM){
	if (DS1307.Setting.sTWLF){
		DS1307.Is_PM=_PM;
	}
}

void DS1307_Enable(void){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.Setting.sCH=0;
		DS1307.Command=Status_DS1307_Enable;
	}
}

void DS1307_Disable(void){
	if (DS1307.Command==Status_DS1307_Idle){
		DS1307.Setting.sCH=1;
		DS1307.Command=Status_DS1307_Disable;
	}
}
