/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdbool.h>
#include <wiringPi.h>


const int pwmPin = 18; // PWM LED - Broadcom pin 18, P1 pin 12
const int ledPin = 23; // Regular LED - Broadcom pin 23, P1 pin 16
const int butPin = 17; // Active-low button - Broadcom pin 17, P1 pin 11

const int pwmValue = 1; // Use this to set an LED brightness

bool ledBright = false;

void initializeGpio()
{
	wiringPiSetupGpio();
	
	//pinMode(pwmPin, PWM_OUTPUT); // Set PWM LED as PWM output
    pinMode(ledPin, OUTPUT);     // Set regular LED as output
    pinMode(butPin, INPUT);      // Set button as INPUT
    pullUpDnControl(butPin, PUD_UP); // Enable pull-up resistor on button
}

int gpio_testing(GtkWidget *widget, bool *turnOn)
{
    
	// Regular pin out
	int state = digitalRead(ledPin);
		
	if(state == 0)
		digitalWrite(ledPin, HIGH); // Turn LED ON
	else
		digitalWrite(ledPin, LOW); // Turn LED ON

/*
	// PWM led
	if(ledBright)
		pwmWrite(pwmPin, 1024 - pwmValue);
	else 
		pwmWrite(pwmPin, pwmValue);
	
	ledBright = !ledBright;
*/
/*
    // Loop (while(1)):
    while(1)
    {
        if (digitalRead(butPin)) // Button is released if this returns 1
        {
            pwmWrite(pwmPin, pwmValue); // PWM LED at bright setting
            digitalWrite(ledPin, LOW);     // Regular LED off
        }
        else // If digitalRead returns 0, button is pressed
        {
            pwmWrite(pwmPin, 1024 - pwmValue); // PWM LED at dim setting
            // Do some blinking on the ledPin:
            digitalWrite(ledPin, HIGH); // Turn LED ON
            delay(75); // Wait 75ms
            digitalWrite(ledPin, LOW); // Turn LED OFF
            delay(75); // Wait 75ms again
        }
    }
*/
	return 0;
}
