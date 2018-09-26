/*************************************************************************
 Title	:   Convert.c
 Author:    Arjan Vuik <ajvuik@hotmail.com>
 Software:  Atmel studio 7.0
 Hardware:  N/A
 Description:This library can be used to use for converting the following:
			Decimal from and to BCD

***************************************************************************/
#include "Convertlib/Convert.h"

uint8_t BCDToDecimal(const uint8_t _BCD){
    if((_BCD&0xF0)<0xA0 && (_BCD&0x0F)<0x0A){
	    return (_BCD>>4)*10 + (_BCD & 0x0F);
	    //return _x;
    }
   return 0;
}

uint8_t DecimalToBCD(const uint8_t _Decimal){
	if(_Decimal<100){
		uint8_t _x = _Decimal%10;
		uint8_t _y = (((_Decimal-_x)/10)<<4) + _x;
		return _y;
	}
	return 0;
}

unsigned char swapNibbles(const unsigned char x){
	return x<<4 | x>>4;
}