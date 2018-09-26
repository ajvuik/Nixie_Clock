/*************************************************************************
 Title		:NixieClock.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:ATmega8.
 Description:This is a Nixie clock run on an ATmega8 with a I2C RTC to keep
			 the time. The RTC is battery backed so that the time is still
			 correct when the device loses power.

***************************************************************************/

//Includes
#include "NixieClock.h"

Debounce_struct Debounce;
//Power_struct N_Power;
Nixie_struct N_Clock;

//Main program
int main(void)
{
	Timer_Init();
	I2C_Init();
	I2C_Enable();
	DS1307_Init();
	RTC_Init();
	Analog_Init();
	Setting_Init();
	Nixie_Init();
	sei();
		
    while(1)
    {
		Timer_loop();
 		I2C_Loop();
		DS1307_Loop();
		RTC_Loop();
		Analog_loop();
		Setting_Loop();
		Nixie_Loop();
		Power_Loop();
		EEPROM_Loop();
    }
}//main

//Nixie functions
void Nixie_Init(){
	//Set the correct data direction bits
	DDRD |= (1<<BCD1_0)|(1<<BCD1_1)|(1<<BCD1_2)|(1<<BCD1_3)|(1<<BCD2_0)|(1<<BCD2_1)|(1<<BCD2_2)|(1<<BCD2_3);
	DDRB |= (1<<Nixie1_2)|(1<<Nixie3_4)|(1<<Nixie5_6)|(1<<Colon);
	DDRC &= ~((1<<Button1)|(1<<Button2));
	// Set machine states
	N_Clock.Cur_Nixie=0;
	Power_Mode(Power_Mode_OFF);
	N_Clock.Status=Status_Nixie_Start;
	N_Clock.Setting.Blanking_SV = Nixie_Blanking;
	N_Clock.Setting.Time_On_SV = Next_Nixie_Time_on;
	//init power circuit
	Power_Init();
	Setting_Init();
}//Nixie_Init

void Nixie_Loop(void){
	switch (N_Clock.Status){
		
		case Status_Nixie_Start:{
			if (N_Clock.Setting.Init==Nixie_Init_OK){
				Power_Mode(Power_Mode_ON);
				N_Clock.Status=Status_Nixie_Run;
			}
		}//Status_Nixie_Start
		break;
		
		case Status_Nixie_Run:
		case Status_Nixie_Menu:
		case Status_Nixie_Save:{
			Next_Nixie();

			//Get the analogue value for safe mode
			if(Analog_Channel_Ready(Power_Analog_Save_Channel)){
				N_Clock.Setting.Save_AI=Analog_Channel_Read(Power_Analog_Save_Channel);
			}

			//See if it is daytime
			int16_t _Noon = memcmp((uint8_t *)&N_Clock.Setting.Save_Time_ON, (uint8_t *)&N_Clock.Setting.Save_Time_OFF, sizeof(N_Clock.Setting.Save_Time_ON));
			int16_t _Night = memcmp((uint8_t *)&N_Clock.Setting.Save_Time_ON, (uint8_t *)&RTC.Time, sizeof(RTC.Time));
			int16_t _Day = memcmp((uint8_t *)&N_Clock.Setting.Save_Time_OFF, (uint8_t *)&RTC.Time, sizeof(RTC.Time));
			uint8_t _Is_Night = 0;

			if(_Noon>0 && (_Night<0 || _Day>0)){
				_Is_Night = 1;
			}
			else if (_Noon<=0 && (_Night>0 || _Day>0)){
				_Is_Night = 1;
			}

			//Determine if we need to go into power save mode
			switch(N_Clock.Setting.Save_Active){
				case Nixie_Save_AI:{
					if (N_Clock.Setting.Save_On_Threshold<N_Clock.Setting.Save_AI){
						N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
					}
					else{
						N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_SV;
					}
					if (N_Clock.Setting.Save_Timeout_PV==0){
						N_Clock.Status=Status_Nixie_Save;
					}
				}//Nixie_Save_AI
				break;
				case Nixie_Save_Time:{
					if (memcmp((uint8_t *)&RTC.Time,(uint8_t *)&N_Clock.Setting.Save_Time_ON,sizeof(RTC.Time))==0){
						N_Clock.Status=Status_Nixie_Save;
					}
				}//Nixie_Save_Time
				break;
				case Nixie_Save_AI_Time:{
					if(_Is_Night>0){
						if (N_Clock.Setting.Save_On_Threshold<N_Clock.Setting.Save_AI){
							N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
						}
						else{
							N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_SV;
						}
						if (N_Clock.Setting.Save_Timeout_PV==0){
							N_Clock.Status=Status_Nixie_Save;
						}
					}
				}//Nixie_Save_AI_Time
				break;
			}

			//See if we want to go in or out of the menu...
			if ((PINC & (1<<Button1)) || (PINC & (1<<Button2))){
				Debounce.Menu=200;//2s
			}
		
			if (Debounce.Menu==0){
				if (N_Clock.Status=Status_Nixie_Run){
					N_Clock.Status=Status_Nixie_Menu;
					Debounce.Menu=200;//2s
					Debounce.Sub_Menu=100;//1sec
					N_Clock.Menu=1;
				}
				else{
					N_Clock.Status=Status_Nixie_Run;
					Debounce.Menu=200;
					N_Clock.Setting.Write_Settings++;
				}
			}

			switch(N_Clock.Status){
				case Status_Nixie_Run:{
					N_Clock.Menu=0;
					N_Clock.Blink=0;
					
					//Update the Nixies with what we want to show.
					switch(N_Clock.Show){
						case Show_Time:{
							//Alternate between date and time?
							if (N_Clock.Setting.Date_Show){
								if (N_Clock.Setting.Date_Show_SV>0 && N_Clock.Setting.Date_Timeout_PV==0){
									N_Clock.Setting.Date_Show_PV=N_Clock.Setting.Date_Show_SV;
									N_Clock.Show=Show_Date;
								}
							}
						}//Show_Time
						break;
						
						case Show_Date:{
							//After a timeout cycle back to showing the time
							if (N_Clock.Setting.Date_Timeout_SV>0 && N_Clock.Setting.Date_Show_PV==0){
								N_Clock.Setting.Date_Timeout_PV=N_Clock.Setting.Date_Timeout_SV;
								N_Clock.Show=Show_Time;
							}
						}//Show_Date
						break;
					}//N_Clock.Show
					Nixie_Update(N_Clock.Show);

				}//Status_Nixie_Run
				break;
				
				case Status_Nixie_Menu:{

					//Is there a button pressed?					
					if((PINC & (1<<Button1)) && (PINC & (1<<Button2))){
						Debounce.Sub_Menu=10;//100ms debounce
					}
					else if(~(PINC & (1<<Button1)) && (PINC & (1<<Button2)) && Debounce.Sub_Menu==0){
						N_Clock.Menu++;
						if (N_Clock.Menu==4){
							N_Clock.Menu=11;
						}
						else if (N_Clock.Menu==14){
							N_Clock.Menu=20;
						}
						else if (N_Clock.Menu==23){
							N_Clock.Menu=30;
						}
						else if (N_Clock.Menu>36){
							N_Clock.Menu=1;
						}
						Debounce.Sub_Menu=Debounce_Overshoot;//set debounce to 300ms to prevent overshoot
					}
					else if ((PINC & (1<<Button1)) && ~(PINC & (1<<Button2)) && Debounce.Sub_Menu==0){
						RTC_time_struct _Time = RTC.Time;
						RTC_date_struct _Date = RTC.Date;

						switch(N_Clock.Menu){
							case 1:{
								if (++_Time.Second>=60){//protect the seconds from beeing wrong
									_Time.Second=0;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//1
							break;
							case 2:{
								if (++_Time.Minute>=60){//protect the minutes from beeing wrong
									_Time.Minute=0;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//2
							break;
							case 3:{
								_Time.Hour++;//update the hours
								if (_Time.Hour>=24){//protect the hours from beeing wrong
									_Time.Hour=0;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//3
							break;
							case 11:{
								_Date.Year++;//update the years
								if (_Date.Year>99){//protect the year from beeing wrong
									_Date.Year=0;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//4
							break;
							case 12:{
								_Date.Month++;//update the month
								if (_Date.Month>12){//protect the month from beeing wrong
									_Date.Month=1;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//5
							break;
							case 13:{
								_Date.Day++;//update the day
								//protect the day from beeing wrong.
								if ((_Date.Month==2) && (_Date.Day>28) && (RTC_not_leap())){//February and not a leap year
									_Date.Day=1;
								}
								else if ((_Date.Month==2) && (_Date.Day>29)){//February in a leap year
									_Date.Day=1;
								}
								else if ((_Date.Day>30) && ((_Date.Month==4)||(_Date.Month==6)||(_Date.Month==9)||(_Date.Month==11))){//30 day months
									_Date.Day=1;
								}
								else if (_Date.Day>31){//31 day months
									_Date.Day=1;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//13
							break;
							
							case 20:{
								if (N_Clock.Setting.Date_Show){
									N_Clock.Setting.Date_Show=0;
								}
								else {
									N_Clock.Setting.Date_Show++;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//20
							break;
														
							case 21:{
								N_Clock.Setting.Date_Show_SV++;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//21
							break;
							
							case 22:{
								N_Clock.Setting.Date_Timeout_SV++;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//22
							break;
							
							//Save active setting: 0=off, 1=light only, 2=time only, 3=time & light
							case 30:{
								N_Clock.Setting.Save_Active++;
								if (N_Clock.Setting.Save_Active>3){
									N_Clock.Setting.Save_Active=0;
								}
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//30
							break;
							case 31:{
								N_Clock.Setting.Save_On_Threshold=N_Clock.Setting.Save_AI;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//31
							break;
							case 32:{
								N_Clock.Setting.Save_Off_Threshold=N_Clock.Setting.Save_AI;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//32
							break;
							case 33:{
								N_Clock.Setting.Save_Time_ON.Minute++;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//33
							break;
							
							case 34:{
								N_Clock.Setting.Save_Time_ON.Hour++;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//34
							break;
							case 35:{
								N_Clock.Setting.Save_Time_OFF.Minute++;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//23
							break;
							case 36:{
								N_Clock.Setting.Save_Time_OFF.Hour++;
								Debounce.Sub_Menu=Debounce_Overshoot;//Set debounce to 300ms to prevent overshoot.
							}//24
							break;
							
						}//N_Clock.Menu
						
						//Save the time and date
						RTC_Set_DateTime(_Time.Second, _Time.Minute, _Time.Hour, _Date.Day, _Date.Month, _Date.Year);
					}
					
					//What do we need to show on the Nixies?
					switch(N_Clock.Menu){
						case 20:{
							N_Clock.Menu_Nixie3_4=0;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Date_Show;
						}//20
						break;
						case 21:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Date_Show_SV/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Date_Show_SV%100;
						}
						break;
						case 22:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Date_Timeout_SV/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Date_Timeout_SV%100;
						}//22
						break;

						case 30:{
							N_Clock.Menu_Nixie3_4=0;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Active;
						}
						break;
						case 31:
						case 32:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_AI/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_AI%100;		
						}//23 & 24
						break;
						case 33:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Time_ON.Hour;
							if(RTC.Time.Second%2){
								N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_ON.Minute;
							}
							else{
								N_Clock.Menu_Nixie5_6=0xAA;
							}
						}//33
						break;
						case 34:{
							if(RTC.Time.Second%2){
								N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Time_ON.Hour;
							}
							else{
								N_Clock.Menu_Nixie3_4=0xAA;
							}
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_ON.Minute;
						}//34
						break;
						case 35:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Time_OFF.Hour;
							if(RTC.Time.Second%2){
								N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_OFF.Minute;
							}
							else{
								N_Clock.Menu_Nixie5_6=0xAA;
							}
						}//35
						break;
						case 36:{
							if(RTC.Time.Second%2){
								N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Time_OFF.Hour;
							}
							else{
								N_Clock.Menu_Nixie3_4=0xAA;
							}
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_OFF.Minute;
						}//36
						break;
					}//switch
					
					if (N_Clock.Menu<4){
						Nixie_Update(Show_Time);
					}
					else if (N_Clock.Menu>9 && N_Clock.Menu<20){
						Nixie_Update(Show_Date);
					}
					else{
						Nixie_Update(Show_Menu);
					}
				}//Status_Nixie_Menu
				break;
				
				case Status_Nixie_Save:{
					Nixie_Update(Show_Nothing);
					if (N_Clock.Setting.Save_Timeout_PV == 0 || (PINC & (1<<Button1)) || (PINC & (1<<Button2))){
						N_Clock.Show=Show_Time;
					}

					switch(N_Clock.Setting.Save_Active){
						case Nixie_Safe_Off:{
							N_Clock.Status=Status_Nixie_Run;
						}//Nixie_Safe_Off
						break;
						case Nixie_Save_AI:{
							if (N_Clock.Setting.Save_On_Threshold>N_Clock.Setting.Save_AI){
								N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
							}
							else{
								N_Clock.Setting.Save_Timeout_PV = N_Clock.Setting.Save_Timeout_SV;
							}
						}//Nixie_Save_AI
						break;
						case Nixie_Save_Time:{
							if (memcmp((uint8_t *)&RTC.Time,(uint8_t *)&N_Clock.Setting.Save_Time_OFF,sizeof(RTC.Time))==0){
								N_Clock.Status=Status_Nixie_Run;
							}
						}//Nixie_Save_Time
						break;
						case Nixie_Save_AI_Time:{
							//Is there enough light to switch back on again?
							if(_Is_Night==0){
								if (N_Clock.Setting.Save_Off_Threshold>N_Clock.Setting.Save_AI){
									N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
								}
								else{
									N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_SV;
								}
								if (N_Clock.Setting.Save_Timeout_PV==0){
									N_Clock.Status=Status_Nixie_Save;
								}
							}
						}//Nixie_Save_AI_Time
						break;
					}//Switch
				}//Status_Nixie_Save
				break;
			}//N_Clock.Status
			
		}//Status_Nixie_Run & Status_Nixie_Menu
		break;
	}//N_Clock.Status

	//Timers
	//Debounce timers
	uint8_t _Pulse_10ms = Timer_Pulse_10ms();
	Debounce.Menu					-= Debounce.Menu && _Pulse_10ms;
	Debounce.Sub_Menu				-= Debounce.Sub_Menu && _Pulse_10ms;
	Debounce.Timer					-= Debounce.Timer && _Pulse_10ms;

	//Setting timers
	uint8_t _Pulse_1s = Timer_Pulse_1s();
	N_Clock.Setting.Date_Show_PV	-= N_Clock.Setting.Date_Show_PV && _Pulse_1s;
	N_Clock.Setting.Date_Timeout_PV -= N_Clock.Setting.Date_Timeout_PV && _Pulse_1s;

	//Misc timers
	uint8_t _Pulse_1ms = Timer_Pulse_1ms();
	N_Clock.Nixie_Blanking_Timer	-= N_Clock.Nixie_Blanking_Timer && _Pulse_1ms;
	N_Clock.Next_Nixie_Timer		-= N_Clock.Next_Nixie_Timer && N_Clock.Nixie_Blanking_Timer==0 && _Pulse_1ms;
}//Nixie_Loop

void Next_Nixie(void){
	uint8_t _PortB=PORTB;//to prevent flikkering of the nixies we write outputs to a temp value 1st
	if (N_Clock.Blink>0){
		_PortB &= ~((1<<Nixie1_2)|(1<<Nixie3_4)|(1<<Nixie5_6));//disconnect all Nixies
	}
	else{
		if (N_Clock.Next_Nixie_Timer==0){
			N_Clock.Nixie_Blanking_Timer=N_Clock.Setting.Blanking_SV;
			N_Clock.Next_Nixie_Timer=N_Clock.Setting.Time_On_SV;
			_PortB &= ~((1<<Nixie1_2)|(1<<Nixie3_4)|(1<<Nixie5_6));//disconnect all Nixies
			PORTD = 0xFF;
			N_Clock.Cur_Nixie++;
			if (N_Clock.Cur_Nixie>3){
				N_Clock.Cur_Nixie=1;
			}
		}
		if (N_Clock.Nixie_Blanking_Timer==0){
			switch(N_Clock.Cur_Nixie){
				case 1:{
					//set the correct values
					PORTD = N_Clock.BCD_Nixie1_2;//Nixie 1 + 2
					_PortB |= (1<<Nixie1_2);//Connect the right Nixie
				}//1
				break;
				case 2:{
					//set the correct values
					PORTD = N_Clock.BCD_Nixie3_4;//Nixie 3 + 4
					_PortB |= (1<<Nixie3_4);//Connect the right Nixie
				}//2
				break;
				case 3:{
					//set the correct values
					PORTD = N_Clock.BCD_Nixie5_6;//Nixie 5 + 6
					_PortB |= (1<<Nixie5_6);//Connect the right nixie
				}//3
				break;
			}//N_Clock.Cur_Nixie
		}
	}//else
	PORTB = _PortB;//write to the PORT with the temp value.
}//Next_Nixie

void Nixie_Update(const uint8_t _Command){
	switch(_Command){
		case Show_Nothing:{
			Power_Mode(Power_Mode_OFF);
			PORTB &= ~(1<<Colon);//put the colons off
			
			//Write a strange value to the Nixies so that they show nothing
			N_Clock.BCD_Nixie1_2=0xAA;
			N_Clock.BCD_Nixie3_4=0xAA;
			N_Clock.BCD_Nixie5_6=0xAA;
		}//Show_Nothing
		break;
			
		case Show_Time:{//Show the time
			Power_Mode(Power_Mode_ON);
			if(N_Clock.Status==Status_Nixie_Menu){
					if(Timer_Pulse_100ms()){
				PORTB ^= (1<<Colon);//blink colon
				}
			}
			else{
				if(RTC.Time.Second%2){
					PORTB |= (1<<Colon);//put power on the colons
				}
				else{
					PORTB &= ~(1<<Colon);//put the colons off
				}
			}
			//Update the Nixies
			if(N_Clock.Status==Status_Nixie_Menu){
				switch (N_Clock.Menu){
					case 1:{
						N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
						N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
						if(RTC.Time.Second%2){
							N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
						}
						else{
							N_Clock.BCD_Nixie5_6=0xAA;
						}
					}//1
					break;

					case 2:{
						N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
						if(RTC.Time.Second%2){
							N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
						}
						else{
							N_Clock.BCD_Nixie3_4=0xAA;
						}
						N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
					}//2
					break;

					case 3:{
						if(RTC.Time.Second%2){
							N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
						}
						else{
							N_Clock.BCD_Nixie1_2=0xAA;
						}
						N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
						N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
					}
					break;

				}
			}
			else{
				N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
				N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
				N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
			}
		}
		break;
		
		case Show_Date:{//Show the date
			Power_Mode(Power_Mode_ON);
			if(N_Clock.Status==Status_Nixie_Menu){
				if(Timer_Pulse_100ms()){
					PORTB ^= (1<<Colon);//blink colon
				}
			}
			else{
				PORTB &= ~(1<<Colon);//put the colons off
			}

			//Update the Nixies
			if(N_Clock.Status==Status_Nixie_Menu){
				switch(N_Clock.Menu){
					case 11:{
						N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Date.Day);
						N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Month);
						if(RTC.Time.Second%2){
							N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Date.Year);
						}
						else{
							N_Clock.BCD_Nixie5_6=0xAA;
						}
					}//4
					break;

					case 12:{
						N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Date.Day);
						if(RTC.Time.Second%2){
							N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Month);
						}
						else{
							N_Clock.BCD_Nixie3_4=0xAA;
						}
						N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Date.Year);
					}//5
					break;

					case 13:{
						if(RTC.Time.Second%2){
							N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Date.Day);
						}
						else{
							N_Clock.BCD_Nixie1_2=0xAA;
						}
						N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Month);
						N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Date.Year);
					}//6
					break;
					
				}
			}
			else{
				N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Date.Day);
				N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Month);
				N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Date.Year);
			}
		}
		break;
		
		case Show_Menu:{
			Power_Mode(Power_Mode_ON);
			
			if (Timer_Pulse_100ms()){
				PORTB ^= (1<<Colon);//blink colon
			}
			
			//Update the Nixies
			N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Menu);
			
			if (N_Clock.Menu_Nixie3_4>9){
				N_Clock.BCD_Nixie3_4=DecimalToBCD(N_Clock.Menu_Nixie3_4);
			}
			else if (N_Clock.BCD_Nixie3_4>0){
				N_Clock.BCD_Nixie3_4=DecimalToBCD(N_Clock.Menu_Nixie3_4);
				N_Clock.BCD_Nixie3_4+=0xA0;//Mask out the Nixie that don't need to show anything
			}
			else{//Otherwise show nothing
				N_Clock.BCD_Nixie3_4=0xAA;
			}
			
			if (N_Clock.BCD_Nixie5_6>0){//Show the value
				N_Clock.BCD_Nixie5_6=DecimalToBCD(N_Clock.Menu_Nixie5_6);			
			}
			else{//or show only one 0
				N_Clock.BCD_Nixie5_6=0xA0;
			}
		}
		break;
	}	
}

//Setting functions
void Setting_Init(void){
	N_Clock.Setting.Init=INIT_NOK;	
	N_Clock.Setting.EEPROM_File_Size=sizeof(N_Clock.Setting.EEPROM);
}//Setting_Init

void Setting_Loop(void){
	switch (N_Clock.Setting.Init){
		case Nixie_Init_NOK:{
			EEPROM_Read(Nixie_EEPROM_Settings_Adress, N_Clock.Setting.EEPROM_File_Size, (uint8_t *)&N_Clock.Setting.EEPROM);
			N_Clock.Setting.Init=Nixie_Init_Wait;
		}//Nixie_Init_NOK
		break;
		
		case Nixie_Init_Wait:{
			if (EEPROM_Status()==EEPROM_Status_IDLE){
				//Copy the file to the structure
				N_Clock.Setting.Date_Show			=N_Clock.Setting.EEPROM.Date_Show;
				N_Clock.Setting.Date_Show_SV		=N_Clock.Setting.EEPROM.Date_Show_SV;
				N_Clock.Setting.Date_Timeout_SV		=N_Clock.Setting.EEPROM.Date_Timeout_SV;
				N_Clock.Setting.Save_Active			=N_Clock.Setting.EEPROM.Save_Active;
				N_Clock.Setting.Save_Off_Threshold	=N_Clock.Setting.EEPROM.Save_Off_Threshold;
				N_Clock.Setting.Save_On_Threshold	=N_Clock.Setting.EEPROM.Save_On_Threshold;
				N_Clock.Setting.Save_Time_OFF		=N_Clock.Setting.EEPROM.Save_Time_OFF;
				N_Clock.Setting.Save_Time_ON		=N_Clock.Setting.EEPROM.Save_Time_ON;
				N_Clock.Setting.Save_Timeout_SV		=N_Clock.Setting.EEPROM.Save_Timeout_SV;
				N_Clock.Setting.Init=Nixie_Init_OK;
			}
		}//Nixie_Init_Wait
		break;
		
		case Nixie_Init_OK:{
			if (N_Clock.Setting.Write_Settings && EEPROM_Status()==EEPROM_Status_IDLE){
				N_Clock.Setting.Write_Settings				=0;
				N_Clock.Setting.EEPROM.Date_Show			=N_Clock.Setting.Date_Show;
				N_Clock.Setting.EEPROM.Date_Show_SV			=N_Clock.Setting.Date_Show_SV;
				N_Clock.Setting.EEPROM.Date_Timeout_SV		=N_Clock.Setting.Date_Timeout_SV;
				N_Clock.Setting.EEPROM.Save_Active			=N_Clock.Setting.Save_Active;
				N_Clock.Setting.EEPROM.Save_Off_Threshold	=N_Clock.Setting.Save_Off_Threshold;
				N_Clock.Setting.EEPROM.Save_On_Threshold	=N_Clock.Setting.Save_On_Threshold;
				N_Clock.Setting.EEPROM.Save_Time_OFF		=N_Clock.Setting.Save_Time_OFF;
				N_Clock.Setting.EEPROM.Save_Time_ON			=N_Clock.Setting.Save_Time_ON;
				N_Clock.Setting.EEPROM.Save_Timeout_SV		=N_Clock.Setting.Save_Timeout_SV;
				EEPROM_Write(Nixie_EEPROM_Settings_Adress, N_Clock.Setting.EEPROM_File_Size, (uint8_t *) &N_Clock.Setting.EEPROM);
			}
		}//Nixie_Init_OK
		break;
	}
}//Setting_Loop