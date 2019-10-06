/*
 * NixieDongle_GPS.c
 *
 * Created: 29-7-2018 12:25:55
 * Author : Arjan en Margriet
 */ 

#include <avr/io.h>
#include "UART_Lib/UART.h"
#include "USI_I2C_Slave/usi_i2c_slave.h"

#define SerialBufSize 90

uint8_t SerialBuffer[SerialBufSize];
uint8_t SerialBufPos = 0;
bool NewData = false;
bool StartDet = false;
bool EndDet = false;
char startMarker = '$';
char endMarker = '\n';


int main(void){
	USI_I2C_Init(11);
	UART_init();
	UART_Enable();
	memset(SerialBuffer, 0, sizeof(SerialBuffer));
	sei();
	
	while (1){
		UART_Loop();
		uint8_t _UARTAmount = UART_Waiting();
		if(_UARTAmount>0 && UART.Receive.Status=Status_UART_Idle){
			UART_Read((uint8_t *)SerialBuffer[SerialBufPos], _UARTAmount);
			if(startMarker==(char)SerialBuffer[0]){
				StartDet=true;
			}
			if(StartDet==true){
				SerialBufPos=+_UARTAmount;
			}
		}
		for(int x=0; x<SerialBufSize; x++){
		}
		
		
	}
}
