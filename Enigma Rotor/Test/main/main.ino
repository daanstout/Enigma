
/*
* I2CPortExpanderX
*/


#include <avr/io.h>
#include "USART.h"
#include <util/delay.h>
#include <Arduino.h>					//	nodig voor de init()
#include <string.h>

MY_USART myUsart;
char command[30];

int main(void)
{
	init();								//	van arduino.h
	_delay_ms(100);

	myUsart.initUSART();				//	Serial.begin(9600);
	_delay_ms(100);

	

	while (1)
	{
    char temp[] = "test";
		sendCommand(temp);
		
		_delay_ms(100);
	}
}

// Receives a command
char receiveCommand() {
	char key = myUsart.receiveByte();
	// Empties the command string
	emptyCommand();

	// Loops until the command has ended
	while (key > 0) {
		sprintf(command, "%s%c", command, key);
		key = myUsart.receiveByte();

		// Used to break the loop on a character
		if (key == '/') {
			key = 0;
		}
	}
}

// Sends a command
void sendCommand(char string[]) {
	myUsart.println(string);
}

// Empties the command string
void emptyCommand() {
	// sets all characters of the string to '\0'
	for (int i = 0; command[i] != '\0'; i++) {
		command[i] = '\0';
	}
}

// Prints the command
void printCommand() {
	myUsart.println(command);
}
