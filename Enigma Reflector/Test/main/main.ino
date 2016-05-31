#include <avr/io.h>
#include <Wire.h>
#include <util/delay.h>
#include <Arduino.h>					//	nodig voor de init()
#include <string.h>
#include "Reflector.h"

char command[30];
char result;
uint8_t portAddress;
Reflector reflector = Reflector();


int main(void)
{
	init();								//	van arduino.h

	// Sets the address of the slave
	portAddress = B00010000;

	Wire.begin(portAddress);
	Wire.onReceive(receiveCommand);
	Wire.onRequest(sendCommand);

	while (1)
	{
		// compares the input command to a string and executes a task and clears the command
		if (strncmp(command, "getReflector", 12) == 0) {
			result = reflector.getLetter(command[strlen(command) - 1]);
			emptyCommand();
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