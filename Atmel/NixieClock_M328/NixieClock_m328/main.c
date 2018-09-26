/*************************************************************************
 Title		:NixieClock.c
 Author		:Arjan Vuik <ajvuik@hotmail.com>
 Software	:Atmel studio 7.0
 Hardware	:ATmega328.
 Description:This is a Nixie clock run on an ATmega328 with a I2C RTC to keep
			 the time. The RTC is battery backed so that the time is still
			 correct when the device loses power.

***************************************************************************/

/**************************************************************************
		Version History:
		V1.12	20180415
				Bug fix in DS1307 handling. Enable didn't function properly
			
		V1.11	20180410
				Bug fixes in menu items 43~51

		V1.10	20180404
				Added 4 Nixie option
		
		V1.01	XX
				Bug fixes in save options and made it 12/24 time capable

		V1.00	2015XXXX
				1st version 6 Nixies, but had to upgrade to ATMega328
				
**************************************************************************/
#define Version_Def			112

//Includes
#include "NixieClock.h"

Debounce_struct Debounce;
//Power_struct N_Power;
Nixie_struct N_Clock;

//Main program
int main(void){
	Timer_Init();
	I2C_Init();
	I2C_Enable();
	DS1307_Init();
//	RTC_USE_EXT_TIM2();
	RTC_Init();
	Analog_Init();
	Setting_Init();
	Nixie_Init();
	sei();
		
    while(1){
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
	N_Clock.Version=Version_Def;
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
	N_Clock.Setting.Save_Timeout_PV = 300;
	N_Clock.Show=Show_Time;
	//init power circuit
	Power_Init();
	Setting_Init();
}//Nixie_Init

void Nixie_Loop(void){
	uint8_t _Save_On = Power_Safe();

	switch (N_Clock.Status){
		
		case Status_Nixie_Start:{
			if (N_Clock.Setting.Init==Nixie_Init_OK){
				Power_Mode(Power_Mode_ON);
				if (N_Clock.Setting.Nixie_Amount!=6 && N_Clock.Setting.Nixie_Amount!=4){//If the amount is not correct, we assume this is a new chip or something went wrong with the EE-PROM
					N_Clock.Setting.Nixie_Amount=4;
					N_Clock.Status=Status_Nixie_Menu;
					N_Clock.Menu=51;
				}
				else{
					N_Clock.Status=Status_Nixie_Run;
				}
			}
		}//Status_Nixie_Start
		break;
		
		case Status_Nixie_Run:
		case Status_Nixie_Menu:{
			Next_Nixie();

			//See if we want to go in or out of the menu...
			if ((PINC & (1<<Button1)) || (PINC & (1<<Button2))){
				Debounce.Menu=200;//2s
			}
		
			if (Debounce.Menu==0){
				if (N_Clock.Status==Status_Nixie_Run){
					N_Clock.Status=Status_Nixie_Menu;
					Debounce.Menu=200;//2s
					Debounce.Sub_Menu=100;//1sec
					if(N_Clock.Setting.Nixie_Amount<6){
						N_Clock.Menu=2;
					}
					else{
						N_Clock.Menu=1;
					}
				}
				else{
					N_Clock.Status=Status_Nixie_Run;
					Debounce.Menu=200;
					N_Clock.Setting.Write_Settings++;
				}
			}

			switch(N_Clock.Status){
				case Status_Nixie_Run:{
					//N_Clock.Menu=0;
					
					//If enabled, roll the Nixies every minute				
					if(N_Clock.Show==Show_Time || N_Clock.Show==Show_Date){
						if(N_Clock.Setting.Roll_Active && RTC.Time.Second==0){
							N_Clock.ROLL_1=(RTC.Time.Hour/10);
							N_Clock.ROLL_2=(RTC.Time.Hour%10);
							N_Clock.ROLL_3=(RTC.Time.Minute/10);
							N_Clock.ROLL_4=(RTC.Time.Minute%10);
							if(N_Clock.Setting.Nixie_Amount>4){
								N_Clock.ROLL_5=(RTC.Time.Hour%10);
								N_Clock.ROLL_6=(RTC.Time.Minute%10);
							}
							N_Clock.Show=Show_Roll;
						}

						//Determine if we need to go into power save mode
						switch(N_Clock.Setting.Save_Active){
							case Nixie_Save_AI:{
								if (N_Clock.Setting.Save_On_Threshold<N_Clock.Setting.Save_AI){
									N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
								}
								else{
									N_Clock.Setting.Save_Timeout_PV = N_Clock.Setting.Save_Timeout_SV;
								}
							}//Nixie_Save_AI
							break;
							case Nixie_Save_Time:{
								if (_Save_On>0){
									N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
								}
								else{
									N_Clock.Setting.Save_Timeout_PV = N_Clock.Setting.Save_Timeout_SV;
								}
							}//Nixie_Save_Time
							break;
							case Nixie_Save_AI_Time:{
								if(_Save_On>0){
									if (N_Clock.Setting.Save_On_Threshold<N_Clock.Setting.Save_AI){
										N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
									}
									else{
										N_Clock.Setting.Save_Timeout_PV = N_Clock.Setting.Save_Timeout_SV;
									}
								}
							}//Nixie_Save_AI_Time
							break;
						}//switch(N_Clock.Setting.Save_Active)
					}

					//Update the Nixies with what we want to show.
					switch(N_Clock.Show){
						case Show_Time:{
							//Alternate between date and time?
							if (N_Clock.Setting.Save_Timeout_PV==0){
								N_Clock.Setting.Save_Timeout_PV=10;//debouncing
								N_Clock.Show=Show_Nothing;
								break;
							}

							if (N_Clock.Setting.Date_Show){
								if (N_Clock.Setting.Date_Show_SV>0 && N_Clock.Setting.Date_Timeout_PV==0){
									N_Clock.Setting.Date_Show_PV=N_Clock.Setting.Date_Show_SV;
									N_Clock.Show=Show_Date;
								}
							}
						}//Show_Time
						break;
						
						case Show_Date:{
							//After a timeout, cycle back to showing the time
							if (N_Clock.Setting.Date_Timeout_SV>0 && N_Clock.Setting.Date_Show_PV==0){
								N_Clock.Setting.Date_Timeout_PV=N_Clock.Setting.Date_Timeout_SV;
								N_Clock.Show=Show_Time;
							}
						}//Show_Date
						break;

						case Show_Nothing:{
							if (N_Clock.Setting.Save_AI<N_Clock.Setting.Save_Off_Threshold){
								N_Clock.Setting.Save_Timeout_PV -= N_Clock.Setting.Save_Timeout_PV && Timer_Pulse_1s();
							}
							else{
								N_Clock.Setting.Save_Timeout_PV = N_Clock.Setting.Save_Timeout_SV;
							}

							//User wants to see the time for a short while
							if (!(PINC & (1<<Button1)) || !(PINC & (1<<Button2))){
								N_Clock.Show=Show_Time;//return to where we came from
								break;
							}
							
							//Check if we need to go to normal operation
							if(N_Clock.Setting.Save_Active==Nixie_Save_AI){
								if (N_Clock.Setting.Save_Timeout_PV==0){
									N_Clock.Show=Show_Time;//return to where we came from
									break;
								}
							}//Nixie_Save_AI
							if (N_Clock.Setting.Save_Active==Nixie_Save_Time){
								if (_Save_On==0){
									N_Clock.Show=Show_Time;//return to where we came from
									break;
								}
							}//Nixie_Save_Time
							if(N_Clock.Setting.Save_Active==Nixie_Save_AI_Time){
								if (N_Clock.Setting.Save_Timeout_PV==0 && _Save_On==0){
									N_Clock.Show=Show_Time;//return to where we came from
								}
							}//Nixie_Save_AI_Time
							
						}//Show_Nothing
						break;
						case Show_Roll:{
							if (N_Clock.ROLL_Count>10){
								N_Clock.ROLL_Count=0;
								N_Clock.Show=Show_Time;
							}
						}//Show_Roll
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
						if(N_Clock.Setting.Nixie_Amount<6){
							N_Clock.Menu_Enter=2;
						}
						//Walk through the menu.
						N_Clock.Menu++;
						if (N_Clock.Menu==1 && N_Clock.Setting.Nixie_Amount<6){
							N_Clock.Menu=2;
						}
						else if (N_Clock.Menu==4){
							N_Clock.Menu=11;
						}
						else if (N_Clock.Menu==14){
							N_Clock.Menu=20;
						}
						else if (N_Clock.Menu==23){
							N_Clock.Menu=30;
						}
						else if (N_Clock.Menu==31){
							if(N_Clock.Setting.Save_Active==2){
								N_Clock.Menu=33;
							}
							else if(N_Clock.Setting.Save_Active==0){
								N_Clock.Menu=40;
							}
						}
						else if (N_Clock.Menu==38){
							N_Clock.Menu=40;
						}
						else if (N_Clock.Menu==43){
							N_Clock.Menu=50;
						}
						else if (N_Clock.Menu==52){
							N_Clock.Menu=0;
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
							}//1
							break;
							case 2:{
								if (++_Time.Minute>=60){//protect the minutes from beeing wrong
									_Time.Minute=0;
								}
							}//2
							break;
							case 3:{
								_Time.Hour++;//update the hours
									if(RTC_Get_TWLF()){
										if (RTC_Get_PM() && _Time.Hour>11){
											RTC_Set_TWLF(0);
											RTC_Set_PM(0);
											_Time.Hour=0;
										}
										else if (_Time.Hour>12){
											RTC_Set_PM(1);
											_Time.Hour=1;
										}
									}
									else{
										if (_Time.Hour>=24){//protect the hours from beeing wrong
											RTC_Set_TWLF(1);
											RTC_Set_PM(0);
											_Time.Hour=0;
										}
									}
							}//3
							break;
							case 11:{
								_Date.Year++;//update the years
								if (_Date.Year>99){//protect the year from beeing wrong
									_Date.Year=0;
								}
							}//4
							break;
							case 12:{
								_Date.Month++;//update the month
								if (_Date.Month>12){//protect the month from beeing wrong
									_Date.Month=1;
								}
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
							}//13
							break;
							
							case 20:{
								if (N_Clock.Setting.Date_Show){
									N_Clock.Setting.Date_Show=0;
								}
								else {
									N_Clock.Setting.Date_Show++;
								}
							}//20
							break;
														
							case 21:{
								N_Clock.Setting.Date_Show_SV++;
							}//21
							break;
							case 22:{
								N_Clock.Setting.Date_Timeout_SV++;
							}//22
							break;
							
							//Save active setting: 0=off, 1=light only, 2=time only, 3=time & light
							case 30:{
								N_Clock.Setting.Save_Active++;
								if (N_Clock.Setting.Save_Active>3){
									N_Clock.Setting.Save_Active=0;
								}
							}//30
							break;
							
							//Value from the LDR to go activate the Nixie's
							case 31:{
								N_Clock.Setting.Save_On_Threshold=N_Clock.Setting.Save_AI;
							}//31
							break;
							
							//Value from the LDR to go to sleep
							case 32:{
								N_Clock.Setting.Save_Off_Threshold=N_Clock.Setting.Save_AI;
							}//32
							break;
							
							//Time at which the Nixie's should be active again
							case 33:{
								N_Clock.Setting.Save_Time_ON.Minute++;
								if(N_Clock.Setting.Save_Time_ON.Minute>59){
									N_Clock.Setting.Save_Time_ON.Minute=0;
								}
							}//33
							break;
							case 34:{
								N_Clock.Setting.Save_Time_ON.Hour++;
								if (RTC.IS_TWLF){
									if (RTC.IS_PM && N_Clock.Setting.Save_Time_ON.Hour>11){
										N_Clock.Setting.Save_Time_ON.Hour=0;
									}
									else if (N_Clock.Setting.Save_Time_ON.Hour>12){
										N_Clock.Setting.Save_Time_ON.Hour=0;
									}
								}
								else if(N_Clock.Setting.Save_Time_ON.Hour>23){
									N_Clock.Setting.Save_Time_ON.Hour=0;
								}
							}//34
							break;
							
							//Time at which we go to sleep
							case 35:{
								N_Clock.Setting.Save_Time_OFF.Minute++;
								if (N_Clock.Setting.Save_Time_OFF.Minute>59){
									N_Clock.Setting.Save_Time_OFF.Minute=0;
								}
							}//35
							break;
							case 36:{
								N_Clock.Setting.Save_Time_OFF.Hour++;
								if (RTC.IS_TWLF){
									if (RTC.IS_PM && N_Clock.Setting.Save_Time_OFF.Hour>11){
										N_Clock.Setting.Save_Time_OFF.Hour=0;
									}
									else if (N_Clock.Setting.Save_Time_OFF.Hour>12){
										N_Clock.Setting.Save_Time_OFF.Hour=0;
									}
								}
								else if(N_Clock.Setting.Save_Time_OFF.Hour>23){
									N_Clock.Setting.Save_Time_OFF.Hour=0;
								}
							}//36
							break;

							//Timeout before we go into or out of save mode. To prevent the clock to go on and off during thressholds
							case 37:{
								N_Clock.Setting.Save_Timeout_SV++;
								if (N_Clock.Setting.Save_Timeout_SV>100){
									N_Clock.Setting.Save_Timeout_SV=0;
								}
							}//37
							break;
							
							case 40:{
								N_Clock.Setting.Roll_Active++;
								if (N_Clock.Setting.Roll_Active>1){
									N_Clock.Setting.Roll_Active=0;
								}
							}//40
							break;
							case 41:{
								N_Clock.Setting.Fade++;
								if (N_Clock.Setting.Fade>2){
									N_Clock.Setting.Fade=0;
								}
							}//41
							break;
							case 42:{
								N_Clock.Setting.Fade_Time++;
								if (N_Clock.Setting.Fade_Time>200){
									N_Clock.Setting.Fade_Time=0;
								}
							}//42
							break;
							
							case 50:{
								N_Clock.Setting.Colon_Active++;
								if (N_Clock.Setting.Colon_Active>1){
									N_Clock.Setting.Colon_Active=0;
								}
							}//50
							break;
							case 51:{
								if(N_Clock.Setting.Nixie_Amount>4){
									N_Clock.Setting.Nixie_Amount=4;
								}
								else{
									N_Clock.Setting.Nixie_Amount=6;
								}
							}//51
							break;
						}//N_Clock.Menu
						
						Debounce.Sub_Menu=Debounce_Overshoot;
						//Save the time and date
						RTC_Set_DateTime(_Time.Second, _Time.Minute, _Time.Hour, _Date.Day, _Date.Month, _Date.Year);
					}
					
					//What do we need to show on the Nixies?
					switch(N_Clock.Menu){
						case 0:{
							N_Clock.Menu_Nixie3_4=N_Clock.Version/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Version%100;
						}
						break;
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
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_ON.Minute;
						}//33
						break;
						case 34:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Time_ON.Hour;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_ON.Minute;
						}//34
						break;
						case 35:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Time_OFF.Hour;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_OFF.Minute;
						}//35
						break;
						case 36:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Time_OFF.Hour;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Time_OFF.Minute;
						}//36
						break;
						case 37:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Save_Timeout_SV/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Save_Timeout_SV%100;
						}//37
						break;
						case 40:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Roll_Active/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Roll_Active%100;
						}//40
						break;
						case 41:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Fade/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Fade%100;
						}//41
						break;
						case 42:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Fade_Time/100;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Fade_Time%100;
						}//42
						break;
						case 50:{
							N_Clock.Menu_Nixie3_4=0;
							N_Clock.Menu_Nixie5_6=N_Clock.Setting.Colon_Active;
						}//50
						break;
						case 51:{
							N_Clock.Menu_Nixie3_4=N_Clock.Setting.Nixie_Amount;
							N_Clock.Menu_Nixie5_6=0;
						}
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
			}//N_Clock.Status
			
		}//Status_Nixie_Run & Status_Nixie_Menu
		break;
	}//N_Clock.Status

	//Timers
	//10ms timers
	uint8_t _Pulse_10ms = Timer_Pulse_10ms();
	Debounce.Menu					-= Debounce.Menu && _Pulse_10ms;
	Debounce.Sub_Menu				-= Debounce.Sub_Menu && _Pulse_10ms;
	Debounce.Timer					-= Debounce.Timer && _Pulse_10ms;

	//Setting timers
	uint8_t _Pulse_1s = Timer_Pulse_1s();
	N_Clock.Setting.Date_Show_PV	-= N_Clock.Setting.Date_Show_PV && _Pulse_1s;
	N_Clock.Setting.Date_Timeout_PV -= N_Clock.Setting.Date_Timeout_PV && _Pulse_1s;
	N_Clock.Menu_Enter				-= N_Clock.Menu_Enter && N_Clock.Status==Status_Nixie_Menu && _Pulse_1s;

	//Misc timers
	uint8_t _Pulse_1ms = Timer_Pulse_1ms();
	N_Clock.Nixie_Blanking_Timer	-= N_Clock.Nixie_Blanking_Timer && _Pulse_1ms;
	N_Clock.Next_Nixie_Timer		-= N_Clock.Next_Nixie_Timer && N_Clock.Nixie_Blanking_Timer==0 && _Pulse_1ms;
	N_Clock.Fade_Timer_On			-= N_Clock.Fade_Timer_On && _Pulse_1ms;
	N_Clock.Fade_Timer_Off			-= N_Clock.Fade_Timer_Off && N_Clock.Fade_Timer_On==0 && _Pulse_1ms;
}//Nixie_Loop

void Next_Nixie(void){
	uint8_t _PortB=PORTB;//to prevent flikkering of the nixies we write outputs to a temp value 1st
	if (N_Clock.Next_Nixie_Timer==0){
		N_Clock.Nixie_Blanking_Timer=N_Clock.Setting.Blanking_SV;
		N_Clock.Next_Nixie_Timer=N_Clock.Setting.Time_On_SV;
		_PortB &= ~((1<<Nixie1_2)|(1<<Nixie3_4)|(1<<Nixie5_6));//disconnect all Nixies
		//PORTD = 0xFF;
		N_Clock.Cur_Nixie++;
		if ((N_Clock.Setting.Nixie_Amount<6 && N_Clock.Cur_Nixie==3) || N_Clock.Cur_Nixie>3){
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
				if (RTC.IS_TWLF){
					if (RTC.IS_PM){
						PORTB &= (1<<Colon);//put the colons on
					}
					else{
						PORTB &= (1<<Colon);//Put the colons off
					}
				}
				else if(Timer_Pulse_100ms()){
					PORTB ^= (1<<Colon);//blink colon
				}
			}
			else{
				if(RTC.Time.Second%2 && N_Clock.Setting.Colon_Active){
					PORTB |= (1<<Colon);//put power on the colons
				}
				else{
					PORTB &= ~(1<<Colon);//put the colons off
				}
			}
			//Update the Nixies
			if(N_Clock.Status==Status_Nixie_Menu){
				if(N_Clock.Setting.Nixie_Amount<6 && N_Clock.Menu_Enter>0){
					N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Menu);
				}//N_Clock.Setting.Nixie_Amount
				else{
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
							if(N_Clock.Setting.Nixie_Amount>4){
								N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
							}
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
							if(N_Clock.Setting.Nixie_Amount>4){
								N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
							}
						}
						break;
					}//N_Clock.Menu
				}//else
			}
			else{
				if(N_Clock.Setting.Fade==0){
					N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
					N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
					N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
				}
				else{
					switch (N_Clock.Fade_Step){
						case 0:{
							N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
							N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
							N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
							if (memcmp(&N_Clock.Time_Old, &RTC.Time, sizeof(N_Clock.Time_Old))){
								N_Clock.Fade_Step++;
								N_Clock.Faded_time=0;
							}
						}//0
						break;

						case 1:{
							if(N_Clock.Setting.Fade==1){//crossfade
								//fade hour
								if(N_Clock.Time_Old.Hour!=RTC.Time.Hour && N_Clock.Fade_Timer_On>0){
									N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Time_Old.Hour);
								}
								else{
									N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
								}
								//fade minutes
								if(N_Clock.Time_Old.Minute!=RTC.Time.Minute && N_Clock.Fade_Timer_On>0){
									N_Clock.BCD_Nixie3_4=DecimalToBCD(N_Clock.Time_Old.Minute);
								}
								else{
									N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
								}
								//fade seconds
								if (N_Clock.Fade_Timer_On>0){
									N_Clock.BCD_Nixie5_6=DecimalToBCD(N_Clock.Time_Old.Second);
								}
								else{
									N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
								}
							}
							else{//fade in/out
								if(N_Clock.Faded_time<6){
									//fade hour out
									if(N_Clock.Time_Old.Hour!=RTC.Time.Hour && N_Clock.Fade_Timer_On>0){
										N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Time_Old.Hour);
									}
									else{
										if(N_Clock.Time_Old.Hour/10!=RTC.Time.Hour/10){
											N_Clock.BCD_Nixie1_2=0xAA;
										}
										else{
											N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Time_Old.Hour);
											N_Clock.BCD_Nixie1_2|=0x0F;
										}
									}
									//fade minutes out
									if(N_Clock.Time_Old.Minute!=RTC.Time.Minute && N_Clock.Fade_Timer_On>0){
										N_Clock.BCD_Nixie3_4=DecimalToBCD(N_Clock.Time_Old.Minute);
									}
									else{
										if(N_Clock.Time_Old.Minute/10!=RTC.Time.Minute/10){
											N_Clock.BCD_Nixie3_4=0xAA;
										}
										else{
											N_Clock.BCD_Nixie3_4=DecimalToBCD(N_Clock.Time_Old.Minute);
											N_Clock.BCD_Nixie3_4=0x0F;
										}
									}
									//fade seconds out
									if (N_Clock.Fade_Timer_On>0){
										N_Clock.BCD_Nixie5_6=DecimalToBCD(N_Clock.Time_Old.Second);
									}
									else{
										if(N_Clock.Time_Old.Second/10!=RTC.Time.Second/10){
											N_Clock.BCD_Nixie5_6=0xAA;
										}
										else{
											N_Clock.BCD_Nixie5_6=DecimalToBCD(N_Clock.Time_Old.Second);
											N_Clock.BCD_Nixie5_6=0x0F;
										}
									}
								}
								else{
									//fade hour in
									if(N_Clock.Time_Old.Hour!=RTC.Time.Hour && N_Clock.Fade_Timer_On>0){
										if (N_Clock.Time_Old.Hour/10!=RTC.Time.Hour/10){
											N_Clock.BCD_Nixie1_2=0xAA;
										}
										else{
											N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
											N_Clock.BCD_Nixie1_2|=0x0F;
										}
									}
									else{
										N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Time.Hour);
									}
									//fade minutes in
									if(N_Clock.Time_Old.Minute!=RTC.Time.Minute && N_Clock.Fade_Timer_On>0){
										if(N_Clock.Time_Old.Minute/10!=RTC.Time.Minute/10){
											N_Clock.BCD_Nixie3_4=0xAA;
										}
										else{
											N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
											N_Clock.BCD_Nixie3_4|=0x0F;
										}
									}
									else{
										N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Time.Minute);
									}
									//fade seconds in
									if (N_Clock.Fade_Timer_On>0){
										if(N_Clock.Time_Old.Second/10!=RTC.Time.Second/10){
											N_Clock.BCD_Nixie5_6=0xAA;
										}
										else{
											N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
											N_Clock.BCD_Nixie5_6|=0x0F;
										}
									}
									else{
										N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Time.Second);
									}
								}
								
							}
							if (N_Clock.Fade_Timer_Off==0 && N_Clock.Fade_Timer_On==0){
								N_Clock.Fade_Timer_On=N_Clock.Setting.Fade_Time-N_Clock.Faded_time;
								N_Clock.Fade_Timer_Off=N_Clock.Faded_time;
								N_Clock.Faded_time++;
							}
							if (N_Clock.Faded_time>N_Clock.Setting.Fade_Time){
								N_Clock.Fade_Step++;
							}
						}//1
						break;

						//done fading return to start
						case 2:{
							memcpy(&N_Clock.Time_Old, &RTC.Time, sizeof(N_Clock.Time_Old));
							N_Clock.Fade_Step=0;
						}//2
						break;
					}
				}
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
				if(N_Clock.Setting.Nixie_Amount<6 && N_Clock.Menu_Enter>0){
					N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Menu);
				}//N_Clock.Setting.Nixie_Amount
				else{	
					switch(N_Clock.Menu){
						case 11:{
							if(N_Clock.Setting.Nixie_Amount>4){
								N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Date.Day);
								N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Month);
								if(RTC.Time.Second%2){
									N_Clock.BCD_Nixie5_6=0xAA;
								}
								else{
									N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Date.Year);
								}
							}
							else{
								N_Clock.BCD_Nixie1_2=DecimalToBCD(20);
								N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Year);
							}
						}//11
						break;

						case 12:{
							N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Date.Day);
							if(RTC.Time.Second%2){
								N_Clock.BCD_Nixie3_4=0xAA;
							}
							else{
								N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Month);
							}
							if(N_Clock.Setting.Nixie_Amount>4){
								N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Date.Year);
							}
						}//12
						break;

						case 13:{
							if(RTC.Time.Second%2){
								N_Clock.BCD_Nixie1_2=0xAA;
							}
							else{
								N_Clock.BCD_Nixie1_2=DecimalToBCD(RTC.Date.Day);
							}
							N_Clock.BCD_Nixie3_4=DecimalToBCD(RTC.Date.Month);
							if(N_Clock.Setting.Nixie_Amount>4){
								N_Clock.BCD_Nixie5_6=DecimalToBCD(RTC.Date.Year);
							}
						}//6
						break;
					}//N_Clock.Menu
				}//else
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
			if(N_Clock.Setting.Nixie_Amount>4 || N_Clock.Menu==51){
				N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Menu);
			
				if (N_Clock.Menu_Nixie3_4>9){
					N_Clock.BCD_Nixie3_4=DecimalToBCD(N_Clock.Menu_Nixie3_4);
				}
				else if (N_Clock.BCD_Nixie3_4>0){
					N_Clock.BCD_Nixie3_4 = DecimalToBCD(N_Clock.Menu_Nixie3_4);
					N_Clock.BCD_Nixie3_4 += 0xA0;//Mask out the Nixie that doesn't need to show anything
				}
				else{//Otherwise show nothing
					N_Clock.BCD_Nixie3_4 = 0xAA;
				}
			
				if (N_Clock.BCD_Nixie5_6>0){//Show the value
					N_Clock.BCD_Nixie5_6 = DecimalToBCD(N_Clock.Menu_Nixie5_6);			
				}
				else{//or show only one 0
					N_Clock.BCD_Nixie5_6 = 0xA0;
				}
			}
			else{
				//Determen what to show on menu enter
				if (N_Clock.Menu_Enter>0){
					N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Menu);
				}
				else{
					if (N_Clock.Menu_Nixie3_4>9){
						N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Menu_Nixie3_4);
					}
					else if (N_Clock.Menu_Nixie3_4>0){
						N_Clock.BCD_Nixie1_2=DecimalToBCD(N_Clock.Menu_Nixie3_4);
						N_Clock.BCD_Nixie1_2+=0xA0;
					}
					else{
						N_Clock.BCD_Nixie1_2=0xAA;
					}
					
					if (N_Clock.Menu_Nixie5_6>0){
						N_Clock.BCD_Nixie3_4=DecimalToBCD(N_Clock.Menu_Nixie5_6);
					}
					else{
						N_Clock.BCD_Nixie3_4=0xA0;
					}
				}
			}
		}
		break;

		case Show_Roll:{
			if (Timer_Pulse_100ms()){
				N_Clock.ROLL_Count++;
				N_Clock.ROLL_1++;
				if (N_Clock.ROLL_1>9){
					N_Clock.ROLL_1=0;
				}
				N_Clock.ROLL_2++;
				if (N_Clock.ROLL_2>9){
					N_Clock.ROLL_2=0;
				}
				N_Clock.ROLL_3++;
				if (N_Clock.ROLL_3>9){
					N_Clock.ROLL_3=0;
				}
				N_Clock.ROLL_4++;
				if (N_Clock.ROLL_4>9){
					N_Clock.ROLL_4=0;
				}
				N_Clock.ROLL_5++;
				if (N_Clock.ROLL_5>9){
					N_Clock.ROLL_5=0;
				}
				N_Clock.ROLL_6++;
				if (N_Clock.ROLL_6>9){
					N_Clock.ROLL_6=0;
				}
			}
			N_Clock.BCD_Nixie1_2=DecimalToBCD((N_Clock.ROLL_1*10)+N_Clock.ROLL_2);
			N_Clock.BCD_Nixie3_4=DecimalToBCD((N_Clock.ROLL_3*10)+N_Clock.ROLL_4);
			N_Clock.BCD_Nixie5_6=DecimalToBCD((N_Clock.ROLL_5*10)+N_Clock.ROLL_6);
		}
		break;
	}	
}

//Setting functions
void Setting_Init(void){
	N_Clock.Setting.Init=INIT_NOK;	
	N_Clock.Setting.EEPROM_File_Size = sizeof(N_Clock.Setting.EEPROM);
}//Setting_Init

void Setting_Loop(void){
	switch (N_Clock.Setting.Init){
		case Nixie_Init_NOK:{
			EEPROM_Read(Nixie_EEPROM_Settings_Adress, N_Clock.Setting.EEPROM_File_Size, (uint8_t *)&N_Clock.Setting.EEPROM);
			N_Clock.Setting.Init = Nixie_Init_Wait;
		}//Nixie_Init_NOK
		break;
		
		case Nixie_Init_Wait:{
			if (EEPROM_Status()==EEPROM_Status_IDLE){
				//Copy the file to the structure
				N_Clock.Setting.Date_Show			=N_Clock.Setting.EEPROM.eDate_Show;
				N_Clock.Setting.Date_Show_SV		=N_Clock.Setting.EEPROM.eDate_Show_SV;
				N_Clock.Setting.Date_Timeout_SV		=N_Clock.Setting.EEPROM.eDate_Timeout_SV;
				N_Clock.Setting.Save_Active			=N_Clock.Setting.EEPROM.eSave_Active;
				N_Clock.Setting.Save_Off_Threshold	=N_Clock.Setting.EEPROM.eSave_Off_Threshold;
				N_Clock.Setting.Save_On_Threshold	=N_Clock.Setting.EEPROM.eSave_On_Threshold;
				N_Clock.Setting.Save_Time_OFF		=N_Clock.Setting.EEPROM.eSave_Time_OFF;
				N_Clock.Setting.Save_Time_ON		=N_Clock.Setting.EEPROM.eSave_Time_ON;
				N_Clock.Setting.Save_Timeout_SV		=N_Clock.Setting.EEPROM.eSave_Timeout_SV;
				N_Clock.Setting.Save_IS_PM			=N_Clock.Setting.EEPROM.eSave_IS_PM;
				N_Clock.Setting.Roll_Active			=N_Clock.Setting.EEPROM.eRoll_Active;
				N_Clock.Setting.Fade				=N_Clock.Setting.EEPROM.eFade;
				N_Clock.Setting.Fade_Time			=N_Clock.Setting.EEPROM.eFade_Time;
				N_Clock.Setting.Colon_Active		=N_Clock.Setting.EEPROM.eColon_Active;
				N_Clock.Setting.Nixie_Amount		=N_Clock.Setting.EEPROM.eNixie_Amount;
				N_Clock.Setting.Init=Nixie_Init_OK;
			}
		}//Nixie_Init_Wait
		break;
		
		case Nixie_Init_OK:{
			if (N_Clock.Setting.Write_Settings && EEPROM_Status()==EEPROM_Status_IDLE){
				N_Clock.Setting.Write_Settings				=0;
				N_Clock.Setting.EEPROM.eDate_Show			=N_Clock.Setting.Date_Show;
				N_Clock.Setting.EEPROM.eDate_Show_SV		=N_Clock.Setting.Date_Show_SV;
				N_Clock.Setting.EEPROM.eDate_Timeout_SV		=N_Clock.Setting.Date_Timeout_SV;
				N_Clock.Setting.EEPROM.eSave_Active			=N_Clock.Setting.Save_Active;
				N_Clock.Setting.EEPROM.eSave_Off_Threshold	=N_Clock.Setting.Save_Off_Threshold;
				N_Clock.Setting.EEPROM.eSave_On_Threshold	=N_Clock.Setting.Save_On_Threshold;
				N_Clock.Setting.EEPROM.eSave_Time_OFF		=N_Clock.Setting.Save_Time_OFF;
				N_Clock.Setting.EEPROM.eSave_Time_ON		=N_Clock.Setting.Save_Time_ON;
				N_Clock.Setting.EEPROM.eSave_Timeout_SV		=N_Clock.Setting.Save_Timeout_SV;
				N_Clock.Setting.EEPROM.eSave_IS_PM			=N_Clock.Setting.Save_IS_PM;
				N_Clock.Setting.EEPROM.eRoll_Active			=N_Clock.Setting.Roll_Active;
				N_Clock.Setting.EEPROM.eFade				=N_Clock.Setting.Fade;
				N_Clock.Setting.EEPROM.eFade_Time			=N_Clock.Setting.Fade_Time;
				N_Clock.Setting.EEPROM.eColon_Active		=N_Clock.Setting.Colon_Active;
				N_Clock.Setting.EEPROM.eNixie_Amount		=N_Clock.Setting.Nixie_Amount;
				EEPROM_Write(Nixie_EEPROM_Settings_Adress, N_Clock.Setting.EEPROM_File_Size, (uint8_t *) &N_Clock.Setting.EEPROM);
			}
		}//Nixie_Init_OK
		break;
	}
}//Setting_Loop

uint8_t Power_Safe(void){
	//Nixie isn't running, no need to do this yet
	if (N_Clock.Status!=Status_Nixie_Run || N_Clock.Setting.Save_Active==0){
		return 0;
	}
	//Get the analogue value for safe mode
	if(Analog_Channel_Ready(Power_Analog_Save_Channel)){
		N_Clock.Setting.Save_AI	= Analog_Channel_Read(Power_Analog_Save_Channel);
	}

	//RTC is not ready or we are not saving on time settings, go back
	if (RTC_Ready()==0 || N_Clock.Setting.Save_Active==1){
		return 0;
	}

	//See if it is daytime
	RTC_time_struct _Save_Time_ON	= N_Clock.Setting.Save_Time_ON;
	RTC_time_struct _Save_Time_OFF	= N_Clock.Setting.Save_Time_OFF;
	RTC_time_struct _RTC_Time		= RTC.Time;

	//If we use 12 hour format, we convert it to 24 hour for easy calculating.
	//NOTE: After 12:00:00 IS_PM is set, take this into account
	if(RTC.IS_TWLF){
		if (N_Clock.Setting.Save_Time_ON_PM && _Save_Time_ON.Hour!=12){
			_Save_Time_ON.Hour	+= 12;
		}
		if(N_Clock.Setting.Save_Time_OFF_PM && _Save_Time_OFF.Hour!=12){
			_Save_Time_OFF.Hour	+= 12;
		}
		if(RTC.IS_PM && _RTC_Time.Hour!=12){
			_RTC_Time.Hour		+= 12;
		}
	}

	
	//Compare the times
	//Old method, this doesn't work because of a bug in memcmp
	//int _Midnight		= memcmp((uint8_t *)&_Save_Time_ON,(uint8_t *)&_Save_Time_OFF, sizeof(_Save_Time_ON));
	//int _On_Time		= memcmp((uint8_t *)&_RTC_Time, (uint8_t *)&_Save_Time_ON, sizeof(_RTC_Time));
	//int _Off_Time		= memcmp((uint8_t *)&_RTC_Time, (uint8_t *)&_Save_Time_OFF, sizeof(_RTC_Time));

	//Determen if we are within the zone in which the Nixies must be inactive 
	//if(_Midnight>0 && (_On_Time<0 || _Off_Time>0)){
	//	return 1;
	//}
	//else if (_Midnight<=0 && (_On_Time>0 || _Off_Time>0)){
	//	return 1;
	//}
	
	//New method, not as elegant but,  ce la vie
	if (_Save_Time_OFF.Hour>_Save_Time_ON.Hour){
		if (_RTC_Time.Hour>_Save_Time_OFF.Hour && _RTC_Time.Hour<_Save_Time_ON.Hour){
			if(_RTC_Time.Minute>_Save_Time_OFF.Minute && _RTC_Time.Minute<_Save_Time_ON.Minute){
				if (_RTC_Time.Second>_Save_Time_OFF.Second && _RTC_Time.Second<_Save_Time_ON.Second){
					return 1;
				}
			}
		}
	}
	else if (_Save_Time_OFF.Hour<_Save_Time_ON.Hour){
		if (_RTC_Time.Hour<_Save_Time_OFF.Hour && _RTC_Time.Hour>_Save_Time_ON.Hour){
			if(_RTC_Time.Minute<_Save_Time_OFF.Minute && _RTC_Time.Minute>_Save_Time_ON.Minute){
				if (_RTC_Time.Second<_Save_Time_OFF.Second && _RTC_Time.Second>_Save_Time_ON.Second){
					return 1;
				}
			}
		}
	}
	
	return 0;	
}//Power_Safe