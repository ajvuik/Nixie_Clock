/*
 * NixieDongle_DCF77.c
 *
 * Created		: 29-7-2018 12:25:55
 * Author		: Arjan Vuik
 * Description	: Time synchronization dongle for the Nixie_Clock
 * Version		:V1.00
 *
 * History:
 * V1.00		:Initial version
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Timers/timers.h"
#include "DCF77lib/DCF77.h"
#include "USI_I2C_Slave/usi_i2c_slave.h"

//prototypes

int main(void){
	cli();
	Timer_Init();
	DCF_INIT();
	USI_I2C_Init(0x78);
	sei();

    while (1){
		Timer_loop();
		DCF_LOOP();
    }
}
