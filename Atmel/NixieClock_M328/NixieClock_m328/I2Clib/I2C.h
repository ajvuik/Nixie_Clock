/*************************************************************************
 Title	:   I2C.h
 Author:    Arjan Vuik <ajvuik@hotmail.com>
 Software:  Atmel studio 7.0
 Hardware:  any AVR device with hardware TWI (I2C), correct frequency needs to be set, 
			read data sheet.
Description:Header file for I2C library
			This library can be used to use the hardware TWI interface

***************************************************************************/
#ifndef I2CLIB_H
#define I2CLIB_H
//Includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <avr/cpufunc.h>

//defines
#define TWI_EN		TWEN
#define TWI_INT		TWINT
#define TWI_EA		TWEA
#define TWI_STA		TWSTA
#define TWI_STO		TWSTO
#define TWI_SDA		PC4
#define TWI_SCL		PC5

#define TWI_TWSR	TWSR
#define TWI_TWBR	TWBR
#define TWI_TWCR	TWCR
#define TWI_TWDR	TWDR
#define TWI_PORT	PORTC

//Library specific
#ifndef F_CPU
	//#error
	#define F_CPU			8000000UL
#endif
#define TWI_PRESCALE		0x00
#define TWI_CL				100000UL
#define TWI_TWBR_Value		(0.5*((F_CPU/TWI_CL) - 16))
//#define I2C_BUFFER_SIZE			50
#define I2C_MASTER				1
//#define I2C_10Bit_Adress		30

//Make some machine states for better reading
#define State_I2C_Idle					0
#define State_I2C_Init					1
#define State_I2C_Start					2
#define State_I2C_Start_OK				3
#define State_I2C_Write_SLAW			4
#define State_I2C_Write_SLAW_OK			5
#define State_I2C_Write_Data			10
#define State_I2C_Write_Data_OK			11
#define State_I2C_Read_Data				12
#define State_I2C_Read_Data_OK			13
#define State_I2C_CMD_Start				30
#define State_I2C_Not_Init				99
#define State_I2C_Error					100
#define State_I2C_NO_ACK				101


#define State_Disabled			0
#define State_Enabled			1

#define INIT_NOK				0
#define INIT_OK					1

//how many I2C devices do you need to service?
#define Min_Ident				1
#define Max_Ident				1

//Structs
typedef struct {
	uint8_t Master;
	uint8_t ID;
} Setting_Struct;

typedef struct {
	uint8_t *Send_Data_ptr;
	uint8_t *Receive_Data_ptr;
	uint8_t *Work_Ptr;
	uint8_t Send_Amount;
	uint8_t Receive_Amount;
	bool Send;
	bool Restart;
	uint8_t Adress[2];
	uint8_t Adress_Size;
	uint8_t Status;
	bool Ident_given;
} I2C_SR_struct;

typedef struct{
	uint8_t Enable;
	uint8_t Init;
	uint8_t State;
	uint8_t Status;
	uint8_t Timer;
	uint8_t ReceivedByte;
	uint8_t ACK_Error;
	uint8_t Curr_Ident;
	//uint8_t Recv;
	//uint8_t Recv_Buff[I2C_BUFFER_SIZE];
	//uint8_t Recv_Buff_Pos;
	//uint8_t Recv_Buff_Read;
	uint8_t TWI_Status;
	Setting_Struct Setting;
	I2C_SR_struct Ident[Max_Ident+1];
}I2C;

//prototypes

void I2C_Loop(void);
void I2C_Init(void);
uint8_t I2C_IS_Init(void);
void I2C_Enable(void);
void I2C_Disable(void);
void I2C_Write(uint8_t _Data);
void I2C_Read_ACK(void);
void I2C_Read_NACK(void);
void I2C_Ack_Error(void);

void I2C_Start(void);
void I2C_Stop(void);
uint8_t I2C_Status(void);
uint8_t I2C_ID_State(uint8_t _Ident);
void I2C_ID_Ack_Error(uint8_t _Ident);
uint8_t I2C_Ready(uint8_t _Ident);
uint8_t I2C_Get_Ident(uint8_t _Reg_Size, uint8_t _Register);
void I2C_Free_Ident(uint8_t _Ident);
void I2C_Send_ST(uint8_t *_Data_Ptr, uint8_t _Amount, uint8_t _Ident);
void I2C_Send_STr(uint8_t *_Send_Data_Ptr, uint8_t *_Recieve_Data_Ptr, uint8_t _Send_Amount, uint8_t _Recieve_Amount , uint8_t _Ident);
void I2C_Recieve(uint8_t *_Data_Ptr, uint8_t _Amount, uint8_t _Ident);
//ISR(TWI_vect);

#endif
