/*************************************************************************
 Title		:timers.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:Any AVR device with external hardware interrupt.
 Description:DCF77 library.

***************************************************************************
Version:
		see .h file
		
***************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "DCF77lib/DCF77.h"
#include "../NixieDongle_DCF77/USI_I2C_Slave/usi_i2c_slave.h"

DCF_struct DCF77;

void DCF_INIT(){
	Timer_Flank=0;
	DCF77.InSync=0;
	DCF77.Timer=0;	
	DCF77.StartSignal=0;
	GIMSK |= (1<<INT1);//Enable Interrupt
	MCUCR |= (1<<ISC10);//Up or down
	DDRD &= ~(1<<PORTD3);//Input
	PORTD |= (1<<PORTD3);//With pullup enabled
	USI_Slave_register_buffer[1] = &DCF77.Minute;
	USI_Slave_register_buffer[2] = &DCF77.Hour;
	USI_Slave_register_buffer[4] = &DCF77.Day;
	USI_Slave_register_buffer[5] = &DCF77.Month;
	USI_Slave_register_buffer[6] = &DCF77.Year;
}

void DCF_LOOP(){
    static const uint8_t xBCD[] = { 1, 2, 4, 8, 10, 20, 40, 80 };
		  static uint8_t xParity = 0;
				 uint8_t x10ms = Timer_Pulse_10ms();
				 uint8_t xCurrBit = 0;

	DCF77.Timer=DCF77.Timer+(x10ms&&DCF77_Flank);
	
	if (DCF77_Flank==0 && DCF77.Timer>0){
		if (DCF77.Timer>100){
			DCF77.StartSignal++;
			DCF77.SecondCounter=0;
		}
		else if (DCF77.Timer>16){
			xCurrBit=1;
			DCF77.SecondCounter++;
		}
		else{
			DCF77.SecondCounter++;
		}
		DCF77.Timer=0;
	}
	if (DCF77.SecondCounter!=DCF77.SecondCounterOld){
		DCF77.SecondCounterOld=DCF77.SecondCounter;
		
		xParity ^= xCurrBit;
		switch(DCF77.SecondCounter){
			case 0:{
				if (xCurrBit>0){
					DCF77.InSync=0;
				}
				else{
					if(DCF77.StartSignal>1){
						DCF77.InSync=1;
						DCF77.StartSignal=0;
					}
				}
			}//0
			break;
			case 17:{
				DCF77.SummerTime=xCurrBit;
			}//17
			break;
			case 18:{
				if (DCF77.SummerTime && xCurrBit){
					DCF77.InSync=0;
					DCF77.SecondCounter=0;
				}
			}
			break;
			case 19:{
				if (xCurrBit>0){
					DCF77.LeapSecond=1;
				}
			}//19
			break;
			case 20:{
				if(xCurrBit==0){
					DCF77.InSync=0;
					DCF77.SecondCounter=0;
				}
			}//20
			break;
			
			//get the minute
			case 21:xParity = xCurrBit; DCF77.Minute=0;
			case 22:
			case 23:
			case 24:
			case 25:
			case 26:
			case 27:if(xCurrBit)DCF77.Minute+=xBCD[DCF77.SecondCounter-21];
			break;
			case 28:{
				if(xParity){
					DCF77.InSync=0;
					DCF77.SecondCounter=0;
				}
			}
			break;
			
			//get hour
			case 29:xParity = xCurrBit; DCF77.Hour=0;
			case 30:
			case 31:
			case 32:
			case 33:
			case 34:if(xCurrBit)DCF77.Hour+=xBCD[DCF77.SecondCounter-29];
			break;
			case 35:{
				if(xParity){
					DCF77.InSync=0;
					DCF77.SecondCounter=0;
				}
			}
			break;
			
			//day of the month
			case 36:xParity = xCurrBit; DCF77.Day=0;
			case 37:
			case 38:
			case 39:
			case 40:
			case 41:if(xCurrBit)DCF77.Day+=xBCD[DCF77.SecondCounter-36];
			break;
			
			//Get the month
			case 45:DCF77.Month=0;
			case 46:
			case 47:
			case 48:
			case 49:if(xCurrBit)DCF77.Month+=xBCD[DCF77.SecondCounter-45];
			break;
			
			//Get the year
			case 50:DCF77.Year=0;
			case 51:
			case 52:
			case 53:
			case 54:
			case 55:
			case 56:
			case 57:if(xCurrBit)DCF77.Year+=xBCD[DCF77.SecondCounter-50];
			break;
			case 58:{
				if(xParity){
					DCF77.InSync=0;
					DCF77.SecondCounter=0;
				}
			}
			break;
			
			
		}//DCF77.SecondCounter
	}
	
		
}

//ISR(INT0_vect){
//}

ISR(INT1_vect){
	if (PIND & (1<<PORTD3)){
		DCF77_Flank=1;
	}
	else{
		DCF77_Flank=0;
	}
}

