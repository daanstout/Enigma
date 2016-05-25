
/*
* I2CPortExpanderX
*/


#include <avr/io.h>
#include "USART.h"
#include <util/delay.h>
#include <Arduino.h>					//	nodig voor de init()
#include <string.h>
#include "Rotor.h"

MY_USART myUsart;
char command[30];
Rotor rotor = Rotor();

int main(void)
{
	init();								//	van arduino.h
	_delay_ms(100);

	myUsart.initUSART();				//	Serial.begin(9600);
	_delay_ms(100);

	DDRB |= (1 << PORTB5);
	PORTB |= (1 << PORTB5);
	//Test code
	myUsart.println("hallo/\n");
	while (1)
	{
		receiveCommand();
		// compares the input command to a string
		if (strcmp(command, "test") == 0) {
			PORTB ^= (1 << PORTB5);
			myUsart.println("toggled/");
		} else if(strcmp(command, "rotateForward") == 0){
			rotor.rotateForward();
		} else if (strcmp(command, "getRotorLetter") == 0) {
			myUsart.transmitByte(rotor.getRotorLetter());
			myUsart.transmitByte('\n');
		} else if (strcmp(command, "rotateBackwards") == 0) {
			rotor.rotateBackwards();
		} else if (strncmp(command, "getShiftedLetter", 16) == 0) {
			myUsart.transmitByte(rotor.getShiftedLetter(command[strlen(command) - 1]));
			myUsart.transmitByte('\n');
		} else if (strncmp(command, "getLetter", 9) == 0) {
			myUsart.transmitByte(rotor.getLetter(command[strlen(command) - 1]));
			myUsart.transmitByte('\n');
		} else if (strncmp(command, "getReverseLetter", 16) == 0) {
			myUsart.transmitByte(rotor.getReverseLetter(command[strlen(command) - 1]));
			myUsart.transmitByte('\n');
		} else if (strncmp(command, "changeRingCfg", 13) == 0) {
			rotor.changeRingConfiguration((uint8_t)command[strlen(command) - 1] - 48);
		} else if (strncmp(command, "getReverseShiftedLetter", 23) == 0) {
			myUsart.transmitByte(rotor.getShiftedReverseLetter(command[strlen(command) - 1]));
			myUsart.transmitByte('\n');
		} else if (strncmp(command, "getTriggered", 12) == 0) {
			if(rotor.getTriggered()) {
				myUsart.println("De rotor is in de trigger positie");
			}
			else {
				myUsart.println("De rotor is niet in de trigger positie");
			}
			myUsart.transmitByte('\n');
		}
		else if (strncmp(command, "a", 1) == 0) {
			myUsart.println("Ontvangen");
			PORTB ^= (1 << PORTB5);
		}
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