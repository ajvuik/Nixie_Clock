/*************************************************************************
 Title		:USI_TWI.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:Any AVR device with an USI interface.
 Description:Header file for using the USI in TWI mode.

**************************************************************************

Version:
V0.00.1	XX-XX-XXXX
		Initial version
*************************************************************************/

#ifndef USI_TWI_H
#define USI_TWI_H

void USI_TWI_INIT();
void USI_TWI_LOOP();

//ISR (TIMER0_OVF_vect); This is already defined in the CPU specific .h file

#endif
