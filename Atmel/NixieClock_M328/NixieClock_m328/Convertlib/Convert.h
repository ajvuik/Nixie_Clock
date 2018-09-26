/*************************************************************************
 Title	:   Convert.h
 Author:    Arjan Vuik <ajvuik@hotmail.com>
 Software:  Atmel studio 7.0
 Hardware:  N/A
 Description:Header file for convert library
			This library can be used to use for converting the following:
			From and to BCD

***************************************************************************/

#ifndef Convert_H
#define Convert_H

#include <avr/io.h>
#include <stdlib.h>


uint8_t BCDToDecimal(const uint8_t _BCD);
uint8_t DecimalToBCD(const uint8_t _Decimal);
unsigned char swapNibbles(const unsigned char x);

#endif