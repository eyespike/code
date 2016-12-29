/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdbool.h>
#include <wiringPi.h>


//const int pwmPin = 18; // PWM LED - Broadcom pin 18, P1 pin 12
const int pin12 = 12; // Regular LED - Broadcom pin 23, P1 pin 16
const int pin16 = 16; // Regular LED - Broadcom pin 23, P1 pin 16
//const int butPin = 17; // Active-low button - Broadcom pin 17, P1 pin 11

//const int pwmValue = 1; // Use this to set an LED brightness

//bool ledBright = false;

void initializeGpio()
{
	wiringPiSetupGpio();

    pinMode(pin12, OUTPUT);     // Set regular LED as output
	pinMode(pin16, OUTPUT);     // Set regular LED as output
	
	// make sure they are off
	digitalWrite(pin12, LOW);
	digitalWrite(pin16, LOW);
}

int set_gpio_12(int state)
{
    
	// Regular pin out
	//int state = digitalRead(pin12);
		
	if(state == 1)
		digitalWrite(pin12, HIGH); // Turn LED ON
	else
		digitalWrite(pin12, LOW); // Turn LED ON


	return 0;
}

int set_gpio_16(int state)
{
    
	// Regular pin out
	//int state = digitalRead(pin16);
		
	if(state == 1)
		digitalWrite(pin16, HIGH); // Turn LED ON
	else
		digitalWrite(pin16, LOW); // Turn LED ON


	return 0;
}