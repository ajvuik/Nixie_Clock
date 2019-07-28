/*
 * NixieDongle_ESP.c
 *
 * Created: 29-7-2018 12:25:55
 * Author : Arjan en Margriet
 */ 

#include <avr/io.h>
#include "UART_Lib/UART.h"
#include "USI_I2C_Slave/usi_i2c_slave.h"

uint8_t _SerialBuffer[8];

int main(void){
	USI_I2C_Init(11);
	UART_init();
	UART_Enable();
	sei();
	
    while (1){
		UART_Loop();
		if(UART_Waiting()){
			
		}
		
    }
}

