
/*
* I2CPortExpanderX
*/


#include <Wire.h>
#include <avr/io.h>
//#include <Arduino.h>					//	nodig voor de init()
#include <util/delay.h>
#include <string.h>
#include "Plugboard.h"
#include "USART.h"

MY_USART myUsart;
char command[30];
Plugboard plugboard;
char temp[3];

int main(void)
{
	init();								//	van arduino.h
  DDRB |= (1 << PORTB5);
	myUsart.initUSART();				//	Serial.begin(9600);
  
  Wire.begin();
  plugboard = Plugboard();
  
	while (1)
	{
    temp[0] = '\0';
    temp[1] = '\0';
    temp[2] = '\0';
    
		receiveCommand();
   
		// compares the input command to a string
    if(strncmp(command, "getPluggedLetter", 16) == 0){
      PORTB ^= (1 << PORTB5);
      sprintf(temp, "%c/", plugboard.getPluggedLetter(command[strlen(command) - 1]));
      myUsart.println(temp);
    }
	}
 return 0;
}

// Receives a command
void receiveCommand() {
	char key = myUsart.receiveByte();
	// Empties the command string
	emptyCommand();

	// Loops until the command has ended
	while (key > 0) {
		sprintf(command, "%s%c", command, key);
		key = myUsart.receiveByte();

		// Used to break the loop on a character
		if (key == '/') {
      PORTB ^= (1 << PORTB5);
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
	//myUsart.println(command);
}
