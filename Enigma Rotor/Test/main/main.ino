#include <avr/io.h>
#include <Wire.h>
#include <util/delay.h>
#include <Arduino.h>					//	nodig voor de init()
#include <string.h>
#include "Rotor.h"

char command[30];
char result;
uint8_t inD2, inD3 = 0;
uint8_t portAddress;
Rotor rotor = Rotor();

// Makes the INT1_vect use the ISR of INT0_vect
ISR(INT1_vect, ISR_ALIASOF(INT0_vect));

// The interrupt changes the address
ISR(INT0_vect) {
	inD2 = ((PIND & B00000100) >> 2);
	inD3 = ((PIND & B00001000) >> 2);
	portAddress = (B00001100 | inD2 | inD3);
	Wire.end();
	Wire.begin(portAddress);
}

int main(void)
{
	init();								//	van arduino.h

	// Initializes the interrupts
	sei();
	EICRA |= (1 << ISC10) | (1 << ISC00);
	EIMSK |= (1 << INT1) | (1 << INT0);

	// Gets the address of the slave
	inD2 = ((PIND & B00000100) >> 2);
	inD3 = ((PIND & B00001000) >> 2);

	// Sets the address of the slave
	portAddress = (B00001100 | inD2 | inD3);

	Wire.begin(portAddress);
	Wire.onReceive(receiveCommand);
	Wire.onRequest(sendCommand);

	while (1)
	{
		// compares the input command to a string
		if(strcmp(command, "rotateForward") == 0){
			rotor.rotateForward();
		} else if (strcmp(command, "getRotorLetter") == 0) {
			result = rotor.getRotorLetter();
		} else if (strcmp(command, "rotateBackwards") == 0) {
			rotor.rotateBackwards();
		} else if (strncmp(command, "getShiftedLetter", 16) == 0) {
			result = rotor.getShiftedLetter(command[strlen(command) - 1]);
		} else if (strncmp(command, "getLetter", 9) == 0) {
			result = rotor.getLetter(command[strlen(command) - 1]);
		} else if (strncmp(command, "getReverseLetter", 16) == 0) {
			result = rotor.getReverseLetter(command[strlen(command) - 1]);
		} else if (strncmp(command, "changeRingCfg", 13) == 0) {
			rotor.changeRingConfiguration((uint8_t)command[strlen(command) - 1] - 48);
		} else if (strncmp(command, "getReverseShiftedLetter", 23) == 0) {
			result = rotor.getShiftedReverseLetter(command[strlen(command) - 1]);
		} else if (strncmp(command, "getTriggered", 12) == 0) {
			result = rotor.getTriggered();
		}
	}
}

// Receives a command
void receiveCommand(int amount) {
	// Empties the command string
	emptyCommand();
	char key;
	if (Wire.available()) {
		key = Wire.read();
	}

	// Loops until the end character is received
	while (key > 0) {
			if (key == '/') {
				key = 0;
			}
			else {
				sprintf(command, "%s%c", command, key);
			}
			key = Wire.read();
	}
}

// Sends a command
void sendCommand() {
		Wire.write(result);
		// clears the result and the command
		result = 0;
		emptyCommand();
}

// Empties the command string
void emptyCommand() {
	// sets all characters of the string to '\0'
	for (int i = 0; command[i] != '\0'; i++) {
		command[i] = '\0';
	}
}