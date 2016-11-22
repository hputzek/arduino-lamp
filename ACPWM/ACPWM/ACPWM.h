/****** ACPWM LIBRARY *****/
/* Created by Dimitris El. Vassis.
/* 2013 - Distribution under the GNU Public Lisence.
/* This library performs AC power control with PWM (Pulse Width Modulation), using the Arduino microcontroller.
/* It is designed for use with the Atmel Atmega8, Atmega168 and Atmega328 AVR microcontrollers and the Arduino platform.
/* It is kept as simple and as minimal as possible, in order to be memory efficient.
/* See examples of use and visit http://www.pamvotis.org/e-sulotions for more details */

#ifndef ACPWM_h
#define ACPWM_h

#include <arduino.h>

class ACPWM
{
	private:
		int zeroControlPn;	//Input pin of zero cross detector
		int pwmPn;		//Output pin to switching component (TRIAC, SSR, e.t.c.)
		int dutyCycle;	// % ON PWM interval
		int period; //Half AC main period in microseconds
		long offTime; //The off time of the cycle
		long onTime; //The on time of the cycle
		int scale; //The scaling of the duty cycle.
		int onLatch; //The ON latching interval of the switching unit, when pulse goes from off to on.
		int basicLatch; //The basic latching interval of the switching unit, when AC mains crosses zero.
		static void mainInterruptHandler();	//Handler for the mainInterrupt function (see source file for more details).	
		void mainInterrupt(); //Implements the main cycle interrupt
	
	public:
		ACPWM();
		~ACPWM();
		//frequency: Frequency of AC power. We hope we know what you are doing.
		//If not, see here: http://www.school-for-champions.com/science/ac_world_volt_freq_list.htm
		//zeroControl: Input pin of zero cross detector
		//zeroPulseMode: Defines how the zero cross point interrupt should be triggered.
		//Valid values: LOW, CHANGE, RISING, FALLING. See documentation for Arduino's attachInterrupt function.
		//CHANGE is the default value.
		//pwmPin: Output pin to switching component (TRIAC, SSR, e.t.c.)
		//scale: The scaling of the duty cycle. We recommend setting it up to 255.
		//basicLatch: The basic latching interval of the switching unit, when AC mains crosses zero.
		//onLatch: The ON latching interval of the switching unit, when pulse goes from off to on.
		void initialize(int frequency, int zeroControlPin, int zeroPulseMode, int pwmPin, int scale);

		//Set the latching intervals (if any).
		//basicLatch: The basic latching interval of the switching unit, when AC mains crosses zero.
		//onLatch: The ON latching interval of the switching unit, when pulse goes from off to on.
		void setLatch(int basicLatch, int onLatch);

		//dutyCycle: % ON PWM interval
		void setDutyCycle(int dutyCycle);
		int getPwmPin(); //Not useful. Only for internal use.
		int getZeroControlPin(); //Not useful. Only for internal use.
		long getBasicLatch(); //Not useful. Only for internal use.
		long getOnLatch(); //Not useful. Only for internal use.
		long getOnTime(); //Not useful. Only for internal use.
		long getOffTime(); //Not useful. Only for internal use.
};
extern ACPWM ACpwm;	//Use this object to access methods from the main program
#endif