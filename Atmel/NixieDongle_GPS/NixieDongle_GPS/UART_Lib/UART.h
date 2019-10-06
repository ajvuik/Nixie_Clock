/*************************************************************************
 Title		:UART.h
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:hardware UART
 
 Description:UART library
			 This library can be used to transmit and receive data from
			 a hardware UART.

 Usage		:Set the F_CPU to the speed to the clock you use in your project
			 and set the baudrate accordingly. It will then automagicly calculate
			 the correct settings for the UBRR registers. Buffersize can also
			 be configured(Default is 50 bytes) 	

***************************************************************************/

#ifndef UART_H
#define UART_H
//Includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
//#include "Timers/timers.h"

//defines

//Library specific
#ifndef F_CPU 
	#define F_CPU 8000000UL
#endif
#define USART_BAUDRATE 9800
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define UART_BUFFER_SIZE 10

//Status defines
#define Status_UART_Idle			0
#define Status_UART_Init			1
#define Status_UART_Write_Wait		2
#define Status_UART_Write			3
#define Status_UART_Read			4

#define Status_Disabled				0
#define Status_Enabled				1

#define INIT_OK					1

//Structs
typedef struct {
	uint8_t *Data_ptr;
	uint8_t Amount;
	uint8_t Status;
} UART_SR_struct;

typedef struct {
	uint8_t Enable;
	uint8_t Init;
	uint8_t Step;
	uint8_t Status;
	uint8_t Timer;
	uint8_t ReceivedByte;
	uint8_t Recv;
	uint8_t Recv_Buff[UART_BUFFER_SIZE];
	uint8_t Recv_Buff_Pos;
	uint8_t Recv_Buff_Read;
	UART_SR_struct Send;
	UART_SR_struct Receive;
} UART_struct;
UART_struct UART;

//Prototypes
void UART_Loop(void);
void UART_init(void);
void UART_Enable(void);
void UART_Disable(void);
void UART_Read(uint8_t *_Data_ptr, uint8_t _Amount);
uint8_t UART_Read_Status(void);
void UART_Write(uint8_t *_Data_ptr, uint8_t _Amount);
uint8_t UART_Write_Status(void);
void UART_Write_Byte(uint8_t _Byte);
uint8_t UART_Waiting(void);
ISR(USART_RX_vect);

#endif