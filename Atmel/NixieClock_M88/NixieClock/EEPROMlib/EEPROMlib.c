/*************************************************************************
 Title	:   EEPROMlib.c
 Author:    Arjan Vuik <ajvuik@hotmail.com>
 Software:  Atmel studio 7.0
  Hardware:  any AVR device with EEPROM memory
 Description:Header file for EEPROM library
			This library can be used to use the EEPROM memory

***************************************************************************/
#include "EEPROMlib/EEPROMlib.h"

EEPROMLib_Struct EEPROM;

void EEPROM_Loop(){
	switch(EEPROM.Status){
		case EEPROM_Status_IDLE:{
			if ((EEPROM_EECR & (1<<EEPROM_EEPE))==0){
				if (EEPROM.Size){
					if (EEPROM.Write){
						EEPROM.Status=EEPROM_Status_Write;
					}
					else{
						EEPROM.Status=EEPROM_Status_Read;
					}
				}
			}
						
		}//case EEPROM_Status_IDLE
		break;
		
//////////////////////////////////Write date to the EEPROM////////////////////////////////////////
		case EEPROM_Status_Write:{
			if (EEPROM.Size){
				//Prepare to write to the EEPROM
				EEPROM.Size--;
				cli();
				EEPROM_EEAR		=EEPROM.Address++;
				EEPROM_EEDR		=*EEPROM.Data_ptr++;
				EEPROM_EECR		|=(1<<EEPROM_EEMPE);//Do we want to write to the EEPROM?
				EEPROM_EECR		|=(1<<EEPROM_EEPE);//Yes we do!
				sei();
				EEPROM.Status	=EEPROM_Status_Write_Wait;//Wait for the writing to finish
			}
			else{
				EEPROM.Status	=EEPROM_Status_IDLE;
			}
		}//case EEPROM_Status_Write
		break;
		
		case EEPROM_Status_Write_Wait:{
			if ((EEPROM_EECR & (1<<EEPROM_EEPE))==0){
				EEPROM.Status	=EEPROM_Status_Write;
			}
		}//case EEPROM_Status_Read_Wait
		break;

//////////////////////////////////Read data from the EEPROM////////////////////////////////////////
		case EEPROM_Status_Read:{
			if (EEPROM.Size>0){
				//Prepare to read from the EEPROM
				EEPROM.Size--;
				EEPROM_EEAR		=EEPROM.Address++;
				EEPROM_EECR		|=(1<<EEPROM_EERE);//Read from the EEPROM
				*EEPROM.Data_ptr++ =EEPROM_EEDR;
				EEPROM.Status	=EEPROM_Status_Read_Wait;//Wait for the writing to finish
			}
			else{
				EEPROM.Status	=EEPROM_Status_IDLE;
			}
		}//case EEPROM_Status_Read
		break;
		
		case EEPROM_Status_Read_Wait:{
			if ((EEPROM_EECR & (1<<EEPROM_EEPE))==0){
				EEPROM.Status	=EEPROM_Status_Read;
			}
		}//case EEPROM_Status_Read_Wait
		break;

		default:{
			EEPROM.Status=EEPROM_Status_IDLE;
		}
	}//switch(EEPROM.Status)
}//EEPROM_Loop

uint8_t EEPROM_Status(){
	if (EEPROM.Size>0){
		return EEPROM_Status_Busy;
	}
	return EEPROM.Status;
}

void EEPROM_Write(uint16_t _Address, uint16_t _Size, uint8_t *_Data_ptr){
	EEPROM.Address	=_Address;
	EEPROM.Size		=_Size;
	EEPROM.Data_ptr	=_Data_ptr;
	EEPROM.Write++;
}

void EEPROM_Read(uint16_t _Address, uint16_t _Size, uint8_t *_Data_ptr){
	EEPROM.Address	=_Address;
	EEPROM.Size		=_Size;
	EEPROM.Data_ptr	=_Data_ptr;
	EEPROM.Write	=0;
}


