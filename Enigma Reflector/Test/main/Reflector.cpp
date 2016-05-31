#include "Reflector.h"
#include <avr/io.h>

Reflector::Reflector()
{
	// Puts the full alphabet in the first row of the array
	for (int i = 0; i < 26; i++) {
		this->reflectorConfiguration[i][0] = 'a' + i;
	}

	char configuration[27] = "enkqauywjicopblmdxzvfthrgs"; // Reflector B configuration
	//char configuration[27] = "rdobjntkvehmlfcwzaxgyipsuq"; // Reflector C configuration


	// Fils the array with the corresponding rotor configuration
	for (int i = 0; i < 26; i++) {
		this->reflectorConfiguration[i][1] = configuration[i];
	}
}

// Gets the encrypted letter from the right side of the rotor
char Reflector::getLetter(char letter)
{
	// Searches for the letter the letter is connected to
	for (int i = 0; i < 26; i++) {
		if (letter == reflectorConfiguration[i][0]) {
			return reflectorConfiguration[i][1];
		}
	}
}