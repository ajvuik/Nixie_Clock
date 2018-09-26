/*************************************************************************
 Title	:   EEPROMlib.h
 Author:    Arjan Vuik <ajvuik@hotmail.com>
 Software:  Atmel studio 7.0
 Hardware:  any AVR device with EEPROM memory
 Description:Header file for EEPROM library
			This library can be used to use the EEPROM memory

***************************************************************************/
#ifndef EEPROMLIB_H
#define EEPROMLIB_H

#include <avr/interrupt.h>

#define EEPROM_Status_IDLE			0
#define EEPROM_Status_Write			1
#define EEPROM_Status_Write_Wait	2
#define EEPROM_Status_Read			3
#define EEPROM_Status_Read_Wait		4

#define EEPROM_Status_Busy			99

//Registers
#define EEPROM_EECR					EECR
#define EEPROM_EEDR					EEDR
#define EEPROM_EEAR					EEAR
#define EEPROM_EEARH				EEARH
#define EEPROM_EEARL				EEARL
//Bits
#define EEPROM_EERIE				EERIE
#define EEPROM_EEMPE				EEMPE
#define EEPROM_EEPE					EEPE
#define EEPROM_EERE					EERE

typedef struct{
	uint8_t Status;
	uint16_t Address;
	uint16_t Size;
	uint8_t	Write;
	uint8_t *Data_ptr;
} EEPROMLib_Struct;

//prototypes
void EEPROM_Loop(void);
uint8_t EEPROM_Status(void);
void EEPROM_Write(uint16_t _Address, uint16_t _Size, uint8_t *_Data_ptr);
void EEPROM_Read(uint16_t _Address, uint16_t _Size, uint8_t *_Data_ptr);

#endif
