/****************************************************************
* Project   :Nixie_Clock
* Title     :ESP_Arduino
* Author		:Arjan Vuik <ajvuik@hotmail.com>
* Software	:PlatformIO
* Hardware	:ESP8266
* Discription: This is the ESP version of a 2 part system to create an
*              I2C dongle for the Nixie_Clock project
*
*******************************************************************/
#include "Arduino.h"
#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>
#include <math.h>
//#include <ArduinoOTA.h>

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

//#define DEBUG

WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "nl.pool.ntp.org", 3600, 60000);


int Calc_Year(const unsigned long _Epoch);
int Calc_Month(const unsigned long _Epoch);
int Calc_Day(const unsigned long _Epoch);
int dow(int y, int m, int d);
//uint8_t DecimalToBCD(const uint8_t _Decimal);

void setup(){
  Serial.begin(19200);

  //create a local instance, because when we are done, we don't need it anymore
  WiFiManager wifiManager;
#ifndef DEBUG
  wifiManager.setDebugOutput(false);
#endif
  //Call the portal, this is blocking
  wifiManager.autoConnect("Nixie_ClockDongleAP");
  //wifi is ok, start retreiving time
  timeClient.begin();
}

void loop() {

  uint8_t _SerialBuffer[8]={0x55,0,0,0,0,0,0,0xAA};
  timeClient.update();
  const unsigned long _Epoch=timeClient.getEpochTime();

  uint8_t Hour=timeClient.getHours();
  uint8_t Minute=timeClient.getMinutes();
  uint8_t Second=timeClient.getSeconds();
  uint8_t Day=Calc_Day(_Epoch);
  uint8_t Month=Calc_Month(_Epoch);
  uint8_t Year=(Calc_Year(_Epoch)-2000);
  uint8_t Dow=dow(Year, Month, Day);

  if((Month==3 && Dow==0 && Day+7>31 && Hour>2)||
    (Month==3 && Dow>0 && Day+7>31)||
    (Month==10 && Dow==0 && Day+7<31 && Hour<2)||
    (Month==10 && Dow>0 && Day+7<31)||
    (Month>3 && Month<10)){
    if(Hour<24){
      Hour+=1;
    }
    else{
      Hour=1;
    }
  }
 
  _SerialBuffer[1]=Hour;
  _SerialBuffer[2]=Minute;
  _SerialBuffer[3]=Second;
  _SerialBuffer[4]=Day;
  _SerialBuffer[5]=Month;
  _SerialBuffer[6]=Year;
  Serial.write(_SerialBuffer, 8);
  //update every second. We don't have anything else todo, so we use the ugly 'delay'
  delay(1000);
}

int Calc_Year(const unsigned long _Epoch){
  int _Days = (int)_Epoch/60/60/24;
  int _Years = (int) _Days/365.25;
  return 1970+_Years;
}

int Calc_Month(const unsigned long _Epoch){
  int _MonthDays[2][13] = {
    {0,31,28,31,30,31,30,31,31,30,31,30,31},
    {0,31,29,31,30,31,30,31,31,30,31,30,31}
  };

  int _Year = Calc_Year(_Epoch);
  int _Days = (int)_Epoch/60/60/24;
  _Days = _Days - (int)(_Days/365.25) * 365 - ceil((_Year-1970)/4)+1;
  int _Month = 1;
  int _Ly = 1;
  if(_Year%4 || _Year%100 || _Year%400){
    _Ly=0;
  }
  while(1){
    if(_Days>_MonthDays[_Ly][_Month]){
      _Days=_Days-_MonthDays[_Ly][_Month];
    }
    else{
      return _Month;
    }
    _Month++;
  }
}

int Calc_Day(const unsigned long _Epoch){
  int _MonthDays[2][13] = {
    {0,31,28,31,30,31,30,31,31,30,31,30,31},
    {0,31,29,31,30,31,30,31,31,30,31,30,31}
  };

  int _Year = Calc_Year(_Epoch);
  int _Days = (int)_Epoch/60/60/24;
  _Days = _Days - (int)(_Days/365.25) * 365 - ceil((_Year-1970)/4)+1;
  int _Month = 1;
  int _Ly = 1;
  if(_Year%4 || _Year%100 || _Year%400){
    _Ly=0;
  }
  while(1){
    if(_Days>_MonthDays[_Ly][_Month]){
      _Days=_Days-_MonthDays[_Ly][_Month];
    }
    else{
      return _Days;
    }
    _Month++;
  }

}

int dow(int y, int m, int d){
      static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
      y -= m < 3;
      return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
  }

/*
uint8_t DecimalToBCD(const uint8_t _Decimal){
	if(_Decimal<100){
		uint8_t _x = _Decimal%10;
		uint8_t _y = (((_Decimal-_x)/10)<<4) + _x;
		return _y;
	}
	return 0;
}
*/
