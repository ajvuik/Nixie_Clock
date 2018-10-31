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

//const char *ssid     = "AJVnet";
//const char *password = "R0Z3Npl@ntso3n16";
//const char *ssid      = "WGAccess";               // Set you WiFi SSID
//const char *password  = "WilgengroepX$043";               // Set you WiFi password

//#define DEBUG

WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient timeClient(ntpUDP, "nl.pool.ntp.org", 3600, 60000);


int Calc_Year(const unsigned long _Epoch);
int Calc_Month(const unsigned long _Epoch);
int Calc_Day(const unsigned long _Epoch);
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


/*
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }
*/
  //wifi is ok, start retreiving time
  timeClient.begin();
}

void loop() {

  uint8_t _SerialBuffer[8]={0xAA,0,0,0,0,0,0,0xAA};
  const unsigned long _Epoch=timeClient.getEpochTime();
  timeClient.update();

  _SerialBuffer[1]=timeClient.getHours();
  _SerialBuffer[2]=timeClient.getMinutes();
  _SerialBuffer[3]=timeClient.getSeconds();
  _SerialBuffer[4]=Calc_Day(_Epoch);
  _SerialBuffer[5]=Calc_Month(_Epoch);
  _SerialBuffer[6]=(Calc_Year(_Epoch)-2000);
  Serial.write(_SerialBuffer, 8);
/*
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());
  Serial.print(Calc_Day(_Epoch));
  Serial.print("-");
  Serial.print(Calc_Month(_Epoch));
  Serial.print("-");
  Serial.println(Calc_Year(_Epoch));
  */
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
