# Nixie Clock

This is a nixie clock based on an Atmel Atmega328 running on a 8Mhz external Xtal. It is programmed in [Atmel Studio 7.0](http://www.microchip.com/mplab/avr-support/atmel-studio-7)
and the PCB is designed in [Fritzing](http://fritzing.org/home/).<br />
Currently it uses 2 buttons to go through an extensive menu structure to configure the clock.<br />
Power loss time keeping is done through an DS1307 module which can be bought in the more famous chinese on-line stores.<br />
The Nixies are multiplexed in pairs, so counting from left to right: nixies 1&2 are active, then nixies 3&4 and, if configured and connected, nixies 5&6.<br />

**Do not use PCB's lower than V1.3, these have a bug that shorts out the powersupply!**

### Features:
* 12/24H
* 4/6 nixies
* Nixie saving by rolling the nixies every minute
* Seconds colons on/of
* Tube saving by time, light or both
* Time restoring after powerloss by adding an DS1307 time keeper
* Settings are stored into EE-prom
* 180V is generated by the Atmega328, but can be adjusted via a variable resistor. So no need for external HV source
* Hex file is provided, you only need to program the Atmega328 with it.
* Gerber files provided, so you can order the PCB's yourself
	
### Known bugs:
* Clock runs to fast(about a minute per week), external recievers are beeing created to remedy this problem(See WIP)
* Fading doesn't work
	
### Work in progress:
* I2C based Dekatron controller as a seconds counter
* I2C based DCF77 reciever
* I2C based NTP reciever (ESP8266 or ESP32)
* I2C based GPS reciever
	
### Todo:
- [ ] Order and test V1.3 PCB
- [ ] Manual for the menu/clock
- [ ] Fix fading
- [ ] Replace the TLP627-4 optocoupler since it is obsolete

### License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
