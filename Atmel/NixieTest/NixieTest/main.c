/*
 * NixieTest.c
 *
 * Created: 27-9-2016 20:20:17
 * Author : Arjan
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include "Timers/timers.h"

//Define IO pins to the Nixies
#define Nixie1_2	PB0
#define Nixie3_4	PB1
#define Nixie5_6	PB2
#define Power		PB3
#define MultiplexTime 35 //00us
#define Blanking_Time 13 //00us

//BCD's for nixie 1,3 and 5
#define BCD1_0		PD0
#define BCD1_1		PD1
#define BCD1_2		PD2
#define BCD1_4		PD3

//BCD's for Nixie 2,4 and 6
#define BCD2_0		PD4
#define BCD2_1		PD5
#define BCD2_2		PD6
#define BCD2_4		PD7

//Prototype's
void Port_Init(void);
void Power_Init(void);
void Power_Loop(void);

uint8_t Nixie_1 = 0;
uint8_t Nixie_2 = 0;
uint8_t Nixie_Count = 1;
uint8_t Mask = 0x0F;
uint8_t Nixie_Timer=MultiplexTime;
uint8_t Blanking_Timer=Blanking_Time;
uint8_t Nixie_Tube[7]={8,4,6,9,2,0,1};

int main(void){
	Port_Init();
	Timer_Init();
	Power_Init();
	sei();

    while (1){
		Timer_loop();
		Power_Loop();
		if (Nixie_Timer==0){
			Nixie_Timer=MultiplexTime;
			Blanking_Timer=Blanking_Time;
			PORTB &= ~((1<<Nixie1_2)|(1<<Nixie3_4)|(1<<Nixie5_6));
			PORTD = 255;
			Nixie_Count++;
			if (Nixie_Count>3){
				Nixie_Count=1;
			}
			switch(Nixie_Count){
				case 1:{
					//PORTD = Nixie_Tube[1]+(Nixie_Tube[2]*16);
					//PORTB |= (1<<Nixie1_2);
				}
				break;
				
				case 2:{
					//PORTD = Nixie_Tube[3]+(Nixie_Tube[4]*16);
					//PORTB |= (1<<Nixie3_4);
				}
				break;
				
				case 3:{
					//PORTD = Nixie_Tube[5]+(Nixie_Tube[6]*16);
					//PORTB |= (1<<Nixie5_6);
				}
				break;
			}
		}
		if(Blanking_Timer==0){
			switch(Nixie_Count){
				case 1:{
					PORTD = Nixie_Tube[1]+(Nixie_Tube[2]*16);
					PORTB |= (1<<Nixie1_2);
				}
				break;
				
				case 2:{
					PORTD = Nixie_Tube[3]+(Nixie_Tube[4]*16);
					PORTB |= (1<<Nixie3_4);
				}
				break;
				
				case 3:{
					PORTD = Nixie_Tube[5]+(Nixie_Tube[6]*16);
					PORTB |= (1<<Nixie5_6);
				}
				break;
			}
		}
		if (Timers.Pulse_1s){
			for (uint8_t x=1; x<7; x++){
				Nixie_Tube[x]++;
				if (Nixie_Tube[x]>9){
					Nixie_Tube[x]=0;
				}
			}
		}
		//PORTD = Nixie_1+(Nixie_2*16);
		Blanking_Timer -= Blanking_Timer && Timers.Pulse_100us;
		Nixie_Timer -= Nixie_Timer && Blanking_Timer==0 && Timers.Pulse_100us;
    }//While 1
}//main

void Port_Init(void){
	DDRD = 255;
	//DDRD |= (1<<BCD1_0)|(1<<BCD1_1)|(1<<BCD1_2)|(1<<BCD1_4);
	DDRB |= (1<<Nixie1_2)|(1<<Nixie3_4)|(1<<Nixie5_6);
}

void Power_Init(void){
	//The required voltage for the Nixies is created by a boost converter. From the website https://learn.adafruit.com/diy-boost-calc/the-calculator I calculated that
	// when you use a 330uH coil you have to generate a 60KHZ block wave to get 170~190V.
	TCCR2 |= (1<<WGM21)|(0<<WGM20)|(0<<CS22)|(0<<CS21)|(1<<CS20);//TCT mode, no prescaler
	OCR2 = 0x42;//8MHZ/66(0x42) = ~121.2121/2 = a block wave of ~60.6060KHZ
	TCCR2 |= (0<<COM21)|(1<<COM20);//Toggle PB3(OC2)
	DDRB |= (1<<Power);
	//Enable the analogue input
	ADMUX |= (1<<ADLAR)|(0<<REFS1)|(0<<REFS0)|(0<<MUX3)|(0<<MUX2)|(0<<MUX1)|(0<<MUX0);//ADLAR= 1 so we only need to get ADCH, reference = AREF(5V), PC0 is connected.
	ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADFR)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0);//Free running mode, 8MHZ/64=125KHZ this is well below the maximum of 200KHZ
}//Power_Init

void Power_Loop(void){
	if (ADCH>0x0F)//if the voltage is to high
	{
		TCCR2 &= ~((1<<COM21)|(1<<COM20));//Disconnect PB3(OC2)
		PORTB &= ~(1<<Power);//And make sure the pin is off
	}
	else{
		//DDRB |= (1<<Power);
		TCCR2 |= (0<<COM21)|(1<<COM20);//Else toggle PB3(OC2)
	}
}//Power_Loop