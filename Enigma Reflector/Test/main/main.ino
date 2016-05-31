#include <avr/io.h>
#include <Wire.h>
#include <util/delay.h>
#include <Arduino.h>					//	nodig voor de init()
#include <string.h>
#include "Reflector.h"
//#include "USART.h"

char command[30];
char result;
uint8_t portAddress;
Reflector reflector = Reflector();


int main(void)
{
	init();								//	van arduino.h
	// Sets the address of the slave
	portAddress = 16;

	Wire.begin(portAddress);
	Wire.onReceive(receiveCommand);
	Wire.onRequest(sendCommand);

	DDRB |= (1 << PORTB5);

	while (1)
	{
		char string[40];
		sprintf(string, "Test: %s\n", command);

		// compares the input command to a string and executes a task and clears the command
		if (strncmp(command, "getReflector", 12) == 0) {
			PORTB ^= (1 << PORTB5);
			result = reflector.getLetter(command[strlen(command) - 1]);
			emptyCommand();
			PORTB ^= (1 << PORTB5);
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