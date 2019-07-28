/*************************************************************************
 Title		:UART.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:hardware UART
 Version	:V0.01.0
 
 Changes
 V0.01.0	: 1st functional version.
 V0.00.1	: Initial setup
 
 Dependencies: None
 
 Description:UART library
			 This library can be used to transmit and receive data from
			 a hardware UART.

 Usage		:In the header file you have to set the F_CPU to the speed of 
			 the clock you use in your project and set the baudrate accordingly. 
			 It will then automagicly calculate the correct settings for the 
			 UBRR register.
			 At this point the library does not rename the register names of the
			 different AVR chips. In the following functions the correct register names
			 must be set(See datasheet of used AVR):
			 Status_UART_Init	:Option registers needs to be set/checked.
			 UART_Write_Byte	:Correct UDR needs to be set
			 USART_RX_vect		:Correct UDR needs to be set
			 
			 To use the library add the .c file to your solution explorer and include 
			 the .h file in your main.c.
			 The library can read and write to the UART simultaneously. This
			 has some considerations you have to be aware of while using this
			 library.
			 
			 In your main the UART_init() function must be called 1st and only once.
			 After that the UART_Loop() function must be called continuously for the 
			 library to function. The UART_init() function will not set the global interrupt.
			 This must be done manualy(By calling the sei() function).
			 
			 You can use the UART_Write() function to send data
			 through the UART port of the AVR. The function takes 2 arguments.
			 The 1st argument must be a pointer from which the function can
			 read from. To conserve memory the function reads directly from your variables or buffer!
			 If a buffer is used, please be aware not to alter it while the function is 
			 sending the data, this will corrupt the message that is being send.
			 
			 With the UART_Write_Status() function the current status of the send function
			 can be determent. The function returns Status_UART_Idle (0) if the function
			 is in not sending any data. It will return Status_UART_Write (3) if it is still
			 working on the last write command. The function returns a uint8_t
			 
			 The function UART_Waiting() is used to see if any bytes are waiting in the 
			 buffer of the function. It returns the number of bytes as an uint_8t.
			 If the number of received bytes is bigger then the configured buffer(Default = 50 bytes)
			 the received bytes are lost.
			 
			 To retrieve the bytes from the receive buffer the UART_Read() function is
			 to be used. The function takes 2 arguments. The 1st is a pointer to
			 where the data is to be transfered to, the second is the amount of bytes
			 to read. The function will always begin to read from the 1st location
			 in the buffer and transfer the amount of bytes given to the location
			 from the provided pointer onwards and will shift the amount of bytes
			 up in the the buffer that has been read. 
			 For example, if 20 bytes are waiting in the read buffer and 10 bytes are read
			 from it. The function will remove the 10 bytes that are read and move 
			 the remain 10 bytes up to position 0. Any incoming bytes will be placed 
			 behind the moved bytes. During the move of the buffer all interrupts 
			 are disabled to ensure buffer integrity.
			 
			 The UART_Read_Status() function can be used to see if the function is
			 finished with reading the the amount of bytes from the buffer. The
			 function returns Status_UART_Idle(0) if the function is doing nothing
			 and it will return Status_UART_Read(4) if it is still working. The function
			 returns a uint8_t.
			 
***************************************************************************/
#include "UART_Lib/UART.h"


void UART_Loop(void){
	//Always read a received byte
	if(UART.Recv){
		UART.Recv=0;
		if (UART.Recv_Buff_Pos < sizeof(UART.Recv_Buff))
		{
			UART.Recv_Buff[UART.Recv_Buff_Pos]=UART.ReceivedByte;
			UART.Recv_Buff_Pos++;
		}
	}

	//main UART function
	switch(UART.Step){
		case Status_UART_Idle:{
			if(UART.Init && UART.Enable){
				if (UART.Send.Amount>0)			{
					UART.Step=Status_UART_Write_Wait;
				}
				if (UART.Receive.Amount>0)
				{
					UART.Step=Status_UART_Read;
				}
			}
			
		}
		break;//idle

		case Status_UART_Init:{
			UART.Recv_Buff_Pos=0;
			UCSR0B = ((1<<TXEN0)|(1<<RXEN0) | (1<<RXCIE0));   // Turn the transmission and reception circuitry on
			//UCSRC = ((1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1)); // Use 8-bit character sizes
			UBRR0H = (BAUD_PRESCALE >> 8); // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
			UBRR0L = BAUD_PRESCALE; // Load lower 8-bits of the baud rate value into the low byte of the UBRR register
			//UBRR0=0x01FF;
			UART.Step=Status_UART_Idle;
			UART.Init=INIT_OK;
		}
		break; //init
		
		case Status_UART_Write_Wait:{
			if ((UCSR0A &(1<<UDRE0)) != 0)
			{
				UART.Step=Status_UART_Write;
			}
			else{
				UART.Step=Status_UART_Read;
			}
		}
		break;//Write_Wait

		case Status_UART_Write:{
			UART_Write_Byte(*UART.Send.Data_ptr);
			UART.Send.Data_ptr++;
			UART.Send.Amount--;
			if(UART.Receive.Amount>0){
				UART.Step=Status_UART_Read;
			}
			else if (UART.Send.Amount>0){
				UART.Step=Status_UART_Write_Wait;
			}
			else{
				UART.Step=Status_UART_Idle;
			}
		}
		break;//Write
		
		case Status_UART_Read:{
			UART.Recv_Buff_Read=0;//always begin reading at buffer position 0
			while (UART.Receive.Amount>0){//keep reading until we reach the amount we need to read
				*UART.Receive.Data_ptr=UART.Recv_Buff[UART.Recv_Buff_Read];
				UART.Receive.Data_ptr++;
				UART.Recv_Buff_Read++;
				UART.Receive.Amount--;
			}
			//remove the remaining bytes to the beginning of the buffer
			cli();
			for (int x=0; x<sizeof(UART.Recv_Buff)-1; x++)
			{
				UART.Recv_Buff[x]=UART.Recv_Buff[x+UART.Recv_Buff_Read];
			}
			UART.Recv_Buff_Pos -= UART.Recv_Buff_Read;//Adjust the buffer position pointer
			sei();
			if (UART.Send.Amount>0){
				UART.Step=Status_UART_Write_Wait;
			}
			else{
				UART.Step=Status_UART_Idle;
			}
		}
		break;//UART_Read
	}//UART.Step

	if (UART.Send.Amount==0 && ((UCSR0A &(1<<UDRE0))!=0)){
		UART.Send.Status=Status_UART_Idle;
	}
	
	if (UART.Receive.Amount==0 ){
		UART.Receive.Status=Status_UART_Idle;
	}
	
}//UART_Loop

void UART_init(void){
	UART.Step=Status_UART_Init;
}

void UART_Enable(void){
	UART.Enable=Status_Enabled;
}

void UART_Disable(void){
	UART.Enable=Status_Disabled;
}

void UART_Read(uint8_t *_Data_ptr, uint8_t _Amount){
	if ((int)_Data_ptr > RAMSTART && (int)_Data_ptr < RAMEND){//Make sure the pointer points somewhere valid
		UART.Receive.Data_ptr=_Data_ptr;
	}
	if(_Amount < UART_BUFFER_SIZE){//protect from reading beyond the buffer limit
		UART.Receive.Amount=_Amount;
	}
	UART.Receive.Status=Status_UART_Read;	
}
uint8_t UART_Read_Status(void){
	return UART.Receive.Status;
}

void UART_Write(uint8_t *_Data_ptr, uint8_t _Amount){
	if ((int)_Data_ptr > RAMSTART && (int)_Data_ptr < RAMEND){//Make sure the pointer points somewhere valid
		UART.Send.Data_ptr=_Data_ptr;
	}
	UART.Send.Amount=_Amount;
	UART.Send.Status=Status_UART_Write;
}

uint8_t UART_Write_Status(void){
	return UART.Send.Status;
}

void UART_Write_Byte(uint8_t _Byte){
	UDR0 = _Byte;
}

uint8_t UART_Waiting(void){
		return UART.Recv_Buff_Pos;
}

ISR(USART_RX_vect){
	UART.ReceivedByte = UDR0;             //read UART register into value
	UART.Recv++;
}
