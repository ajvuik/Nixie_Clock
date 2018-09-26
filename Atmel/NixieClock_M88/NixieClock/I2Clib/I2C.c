/*************************************************************************
 Title	:   timers.c
 Author:    Arjan Vuik <ajvuik@hotmail.com>
 Software:  Atmel studio 7.0
 Hardware:  Any AVR device with hardware I2C(TWI), correct frequency needs to be set, 
			read datasheet.
Description:I2C library
			This library can be used to use the hardware TWI interface.
			The aim of this library is that it can be used without halting 
			the program while waiting for response ie. an Ack.
			

***************************************************************************/

#include "I2Clib/I2C.h"

I2C TWI;

void I2C_Loop(void){
	//TWI.TWI_Status=I2C_Status();
	
	if (TWI.State<100){
		TWI.Status=TWI.State;
	}
	
	switch(TWI.State){
		//TWI is IDLE. There is nothing to do.
		case State_I2C_Idle:{
			if (TWI.Enable && TWI.Init){
				for (uint8_t x=Min_Ident; x<=Max_Ident; x++){
					if((TWI.Ident[x].Send_Amount || TWI.Ident[x].Receive_Amount) && TWI.Ident[x].Ident_given==true){
						TWI.State=State_I2C_Start;
						TWI.Curr_Ident = x;
						//TWI.Ident[x].Status=State_I2C_Start;
						break;
					}
				}
			}
		}//State_I2C_Idle
		break;
		
		case State_I2C_Init:{
			//Setup the I2C
			TWI_TWSR = TWI_PRESCALE;
			TWI_TWBR = TWI_TWBR_Value;
			TWI_TWCR = (1<<TWI_EN);
			TWI_PORT |= ((1<<TWI_SCL)|(1<<TWI_SDA));
			for (uint8_t x=Min_Ident; x<=Max_Ident; x++){
				TWI.Ident[x].Ident_given=false;
			}
			TWI.Init=INIT_OK;
			TWI.State=State_I2C_Idle;
		}
		break;//State_I2C_Init
		
//*******************TWI needs to access the bus. ********************************************
		case State_I2C_Start:{//1st claim the bus
			TWI.State=State_I2C_Start_OK;
			I2C_Start();	
		}//State_I2C_Start
		break;
		
		//wait for the start to be completed
		case State_I2C_Start_OK:{
			if (!((TWCR & (1<<TWINT))==0)){
				TWI.TWI_Status=I2C_Status();
				if(TWI.TWI_Status==0x08 || TWI.TWI_Status==0x10){
					TWI.State=State_I2C_Write_SLAW;
				}
				else{
					TWI.State=State_I2C_Error;
					TWI.Status=TWI.TWI_Status;
				}
			}
		}//State_I2C_Start_OK
		break;
		
//*******************TWI needs to send SLAW to the bus. ********************************************
		case State_I2C_Write_SLAW:{
			if (TWI.Ident[TWI.Curr_Ident].Send_Amount){//Data 0 is the ID of the I2C chip
				TWI.State=State_I2C_Write_SLAW_OK;
				I2C_Write(TWI.Ident[TWI.Curr_Ident].Adress[0]<<1);//Left shift the address with 1 and leave the last bit zero to indicate we want to write.
			}
			else{
				TWI.State=State_I2C_Write_SLAW_OK;
				I2C_Write((TWI.Ident[TWI.Curr_Ident].Adress[0]<<1)+1);//Left shift the address with 1 and write the last bit one to indicate we want to read.
			}
		}//State_I2C_Write_SLAW
		break;
		
		//TWI is sending SLAW, wait for it to finish.
		case State_I2C_Write_SLAW_OK:{
			if (!((TWCR & (1<<TWINT))==0)){
				TWI.TWI_Status=I2C_Status();
				if(TWI.TWI_Status==0x18 || TWI.TWI_Status==0x40){
					if (TWI.Ident[TWI.Curr_Ident].Send_Amount){
						TWI.State=State_I2C_Write_Data;
					}
					else{
						TWI.State=State_I2C_Read_Data;
					}
					
				}
				else if (TWI.TWI_Status==0x20){
					TWI.State=State_I2C_Idle;
					TWI.Ident[TWI.Curr_Ident].Send_Amount=0;
					TWI.Ident[TWI.Curr_Ident].Receive_Amount=0;
					TWI.Ident[TWI.Curr_Ident].Status=State_I2C_NO_ACK;
					TWI.Curr_Ident=0;
					I2C_Stop();
				}
				else if(TWI.TWI_Status==0x38){
					TWI.State=State_I2C_Start;
				}
				else{
					TWI.Status=TWI.TWI_Status;
					TWI.State=State_I2C_Error;
				}
			}
		}
		break;//State_I2C_Write_SLAW_OK

//*******************TWI needs to send data to the bus. ********************************************
		case State_I2C_Write_Data:{
			if (TWI.Ident[TWI.Curr_Ident].Send_Amount){
					I2C_Write(*TWI.Ident[TWI.Curr_Ident].Send_Data_ptr);
					TWI.State=State_I2C_Write_Data_OK;
				}
			else{
					if (TWI.Ident[TWI.Curr_Ident].Receive_Amount){
						TWI.State=State_I2C_Start;
					}
					else{
						TWI.Ident[TWI.Curr_Ident].Status=State_I2C_Idle;
						I2C_Stop();
						TWI.State=State_I2C_Idle;
					}
			}
		}//State_I2C_Write_Data
		break;
		
		case State_I2C_Write_Data_OK:{
			if (!((TWCR & (1<<TWINT))==0)){
				TWI.TWI_Status=I2C_Status();
				if(TWI.TWI_Status==0x28){//I2C succesfully written something
					TWI.Ident[TWI.Curr_Ident].Send_Data_ptr++;
					TWI.Ident[TWI.Curr_Ident].Send_Amount--;
					TWI.State=State_I2C_Write_Data;
				}
				else{//Something went wrong. Release the bus and let the above structure know what went wrong.
					I2C_Stop();
					TWI.Ident[TWI.Curr_Ident].Status=TWI.TWI_Status;
					TWI.Ident[TWI.Curr_Ident].Send_Data_ptr=0;
					TWI.Ident[TWI.Curr_Ident].Send_Amount=0;
					TWI.Ident[TWI.Curr_Ident].Receive_Data_ptr=0;
					TWI.Ident[TWI.Curr_Ident].Receive_Amount=0;
					TWI.State=State_I2C_Idle;
				}
			}
		}//State_I2C_Write_Data_OK
		break;
//************TWI needs to read data from the bus********************************************
		case State_I2C_Read_Data:{
			if (TWI.Ident[TWI.Curr_Ident].Receive_Amount>1){
				I2C_Read_ACK();
				TWI.State=State_I2C_Read_Data_OK;
			}
			else if (TWI.Ident[TWI.Curr_Ident].Receive_Amount==1){
				I2C_Read_NACK();
				TWI.State=State_I2C_Read_Data_OK;
			}
			else{
				TWI.Ident[TWI.Curr_Ident].Status=TWI.State=State_I2C_Idle;
				I2C_Stop();
			}
		}
		break;//State_I2C_Read_Data
		
		case State_I2C_Read_Data_OK:{
			if (!((TWCR & (1<<TWINT))==0)){
				TWI.TWI_Status=I2C_Status();
				if(TWI.TWI_Status==0x50 || TWI.TWI_Status==0x58){//I2C succesfully recieved something
					*TWI.Ident[TWI.Curr_Ident].Receive_Data_ptr=TWDR;
					TWI.Ident[TWI.Curr_Ident].Receive_Data_ptr++;
					TWI.Ident[TWI.Curr_Ident].Receive_Amount--;
					TWI.State=State_I2C_Read_Data;
				}
				else{//Something went wrong. Release the bus and let the above structure know what went wrong.
					I2C_Stop();
					TWI.Ident[TWI.Curr_Ident].Status=TWI.TWI_Status;
					TWI.Ident[TWI.Curr_Ident].Receive_Data_ptr=0;
					TWI.Ident[TWI.Curr_Ident].Receive_Amount=0;
					TWI.State=State_I2C_Idle;
				}
			}
		}
		break;//State_I2C_Read_Data_OK
		
//************We entered an error state***************************************************
		case State_I2C_Error:{
			//Error state is removed in I2C_Ack_Error() function.
			_NOP();
		}//State_I2C_Error
		break;
		
//************Unknown TWI state, go to idle***********************************************
		default:{
			TWI.State=State_I2C_Idle;
		}
		break;
		
	}//Switch(TWI.State)
}//I2C_Loop

void I2C_Init(void){
//	if(TWI.State==State_I2C_Not_Init){
		TWI.Setting.Master=I2C_MASTER;
		TWI.State=State_I2C_Init;
		TWI.Init=INIT_NOK;
//	}
}

void I2C_Enable(void){
	//enable TWI
	TWI.Enable=State_Enabled;
}

void I2C_Disable(void){
	//disable TWI
	TWI.Enable=State_Disabled;
}


void I2C_Write(uint8_t _Data){
	/*
	Write a single byte (8bit) on the I2C bus
	*/
  TWI_TWDR = _Data;
  TWI_TWCR = ((1<<TWI_INT)|(1<<TWI_EN));
}
void I2C_Read_ACK(void){
	/*
	Read a byte from the I2C bus with ACK
	*/
    TWI_TWCR = ((1<<TWI_INT)|(1<<TWI_EN)|(1<<TWI_EA));
}
void I2C_Read_NACK(void){
	/*
	Read a byte from the I2C bus without ACK(Mostly used to end transmission)
	*/
    TWI_TWCR = ((1<<TWI_INT)|(1<<TWI_EN));
}

void I2C_Start(void){
   /*
    Send start condition
    */
    //TWDR = 0;
    TWI_TWCR = ((1<<TWI_INT)|(1<<TWI_STA)|(1<<TWI_EN));
}
void I2C_Stop(void){
	/*
	Send stop condition.
	*/
	TWI_TWCR = ((1<<TWI_INT)|(1<<TWI_EN)|(1<<TWI_STO));
}

uint8_t I2C_Status(void){
    uint8_t _Status;
    //mask Status
    _Status = TWI_TWSR & 0xF8;
    return _Status;
}

uint8_t I2C_ID_State(uint8_t _Ident){
	return TWI.Ident[_Ident].Status;
}

void I2C_ID_Ack_Error(uint8_t _Ident){
	TWI.Ident[_Ident].Status=State_I2C_Idle;
}

uint8_t I2C_Ready(uint8_t _Ident){
	if (TWI.Ident[_Ident].Status==State_I2C_Idle && TWI.Init){
		return 1;
	}
	return 0;
}

//Send something on the bus and then stop
void I2C_Send_ST(uint8_t *_Data_Ptr, uint8_t _Amount, uint8_t _Ident){
	if ((int)_Data_Ptr > RAMSTART && (int)_Data_Ptr < RAMEND){
		TWI.Ident[_Ident].Send_Data_ptr=_Data_Ptr;
		TWI.Ident[_Ident].Receive_Data_ptr=0;
		TWI.Ident[_Ident].Send_Amount=_Amount;
		TWI.Ident[_Ident].Receive_Amount=0;
		TWI.Ident[_Ident].Send=true;
		TWI.Ident[_Ident].Status=State_I2C_Start;
	}
}

//Send something on the bus with a restart and receive something
void I2C_Send_STr(uint8_t *_Send_Data_Ptr, uint8_t *_Recieve_Data_Ptr, uint8_t _Send_Amount, uint8_t _Recieve_Amount , uint8_t _Ident){
	if (((int)_Send_Data_Ptr > RAMSTART && (int)_Send_Data_Ptr < RAMEND)&&
	((int)_Recieve_Data_Ptr > RAMSTART && (int)_Recieve_Data_Ptr < RAMEND)){//Make sure the pointer points somewhere valid
		TWI.Ident[_Ident].Send_Data_ptr=_Send_Data_Ptr;
		TWI.Ident[_Ident].Receive_Data_ptr=_Recieve_Data_Ptr;
		TWI.Ident[_Ident].Send_Amount=_Send_Amount;
		TWI.Ident[_Ident].Receive_Amount=_Recieve_Amount;
		TWI.Ident[_Ident].Send=true;
		TWI.Ident[_Ident].Status=State_I2C_Start;
	}
}

//Receive something
void I2C_Recieve(uint8_t *_Data_Ptr, uint8_t _Amount, uint8_t _Ident){
	if ((int)_Data_Ptr > RAMSTART && (int)_Data_Ptr < RAMEND){
		TWI.Ident[_Ident].Receive_Data_ptr=_Data_Ptr;
		TWI.Ident[_Ident].Receive_Amount=_Amount;
		TWI.Ident[_Ident].Send=false;
		TWI.Ident[_Ident].Status=State_I2C_Start;
	}
}

uint8_t I2C_Get_Ident(uint8_t _Adress_Size, uint8_t _Register){
	if (TWI.Init==INIT_OK){
		for (uint8_t x = Min_Ident; x < Max_Ident; x++){
			if (TWI.Ident[x].Ident_given==false){
				TWI.Ident[x].Ident_given=true;
				TWI.Ident[x].Adress_Size=_Adress_Size;
				TWI.Ident[x].Adress[0]=_Register;
				return x;
			}
		}
	}
	return 0;
}

void I2C_Free_Ident(uint8_t _Ident){
	TWI.Ident[_Ident].Ident_given=false;
}

void I2C_Ack_Error(void){
	TWI.State=State_I2C_Idle;
	TWI.Ident[TWI.Curr_Ident].Status=State_I2C_Idle;
}

//ISR(TWI_vect){
	////TWCR |= (1<< TWINT);//Acknowledge the interrupt
	//switch(TWI.TWI_Status){
		//case State_I2C_Start:{
			//TWCR |= (1<< TWINT);//Acknowledge the interrupt
			//TWI.TWI_Status=State_I2C_Start_OK;
		//}
		//break;
		//case State_I2C_Write:{
			//TWI.TWI_Status=State_I2C_Write_OK;
		//}
		//break;
		//case State_I2C_Read_Ack:
		//case State_I2C_Read_Nack:{
			//TWI.ReceivedByte=TWDR;
			//TWI.TWI_Status=State_I2C_Read_OK;
		//}
		//break;
		//case State_I2C_Stop:{
			//TWI.TWI_Status=State_I2C_Stop_OK;
		//}
		//break;
	//}
	//
//}

