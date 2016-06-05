#include "Rotor.h"
#include <avr/io.h>

Rotor::Rotor()
{
	// Puts the full alphabet in the first row of the array
	for (int i = 0; i < 26; i++) {
		this->rotorConfiguration[i][0] = 'a' + i;
	}

	char configuration[27] = "ekmflgdqvzntowyhxuspaibrcj"; // Rotor I configuration
	//char configuration[27] = "ajdksiruxblhwtmcqgznpyfvoe"; // Rotor II configuration
	//char configuration[27] = "bdfhjlcprtxvznyeiwgakmusqo"; // Rotor III configuration
	//char configuration[27] = "esovpzjayquirhxlnftgkdcmwb"; // Rotor IV configuration
	//char configuration[27] = "vzbrgityupsdnhlxawmjqofeck"; // Rotor V configuration
	//char configuration[27] = "jpgvoumfyqbenhzrdkasxlictw"; // Rotor VI configuration
	//char configuration[27] = "nzjhgrcxmyswboufaivlpekqdt"; // Rotor VII configuration
	//char configuration[27] = "fkqhtlxocbjspdzramewniuygv"; // Rotor VIII configuration
	//char configuration[27] = "enkqauywjicopblmdxzvfthrgs"; // Rotor Beta configuration
	//char configuration[27] = "rdobjntkvehmlfcwzaxgyipsuq"; // Rotor Gamma configuration

	// Fils the array with the corresponding rotor configuration
	for (int i = 0; i < 26; i++) {
		this->rotorConfiguration[i][1] = configuration[i];
	}

	// Set standard values
	this->ringConfiguration = 0;
	this->rotorPosition = 0;
	this->rotorLetter = 'a'; // Start position of the rotor letter displayed

	// The rotors I - V only have one triggerLetter, the rotors VI - VIII have two triggerLetters
	this->triggerLetter = 'q'; // Rotor I trigger
	//this->triggerLetter = 'e'; // Rotor II trigger
	//this->triggerLetter = 'v'; // Rotor III trigger
	//this->triggerLetter = 'j'; // Rotor IV trigger
	//this->triggerLetter = 'z'; // Rotor V trigger
	//this->triggerLetter = 'z' // Rotor VI - VIII trigger

	// The rotors VI - VIII have two triggers so when these rotors are used their triggerLetterTwo also has to be used
	// Otherwise the triggerLetterTwo of the rotors I - V has to be used, which never will be triggered because the rotorLetter will never be '0'
	//this->triggerLetterTwo = 'm' // Rotor VI - VIII second trigger
	this->triggerLetterTwo = '0' // Rotor I - V second trigger (will never be triggered)
}

// Makes the rotor rotate forward
void Rotor::rotateForward()
{
	// Sets the rotor Letter
	if ((this->rotorLetter + 1) > 'z') {
		this->rotorLetter = 'a';
	}
	else {
		this->rotorLetter += 1;
	}

	// Sets the rotor Position
	if ((this->rotorPosition + 1) > 25) {
		this->rotorPosition = 0;
	}
	else {
		this->rotorPosition += 1;
	}
}

// Makes the rotor rotate backwards
void Rotor::rotateBackwards()
{
	// Sets the rotor Letter
	if ((this->rotorLetter - 1) < 'a') {
		this->rotorLetter = 'z';
	}
	else {
		this->rotorLetter -= 1;
	}

	// Sets the rotor Position
	if ((this->rotorPosition - 1) < 0) {
		this->rotorPosition = 25;
	}
	else {
		this->rotorPosition -= 1;
	}
}

// Changes the ring configuration of the rotor
void Rotor::changeRingConfiguration(uint8_t change)
{
	// WARNING the change has to be one lower than the original ringConfiguration
	// For Example: if the original ringConfiguration is 20 the change has to be 19

	// Sets the rotor Position
	uint8_t originalPosition = this->rotorPosition + this->ringConfiguration;

	// Prevents that the rotorPosition gets lower than zero
	if ((originalPosition - change) < 0) {
		this->rotorPosition = (originalPosition + 26) - change;
	}
	else {
		this->rotorPosition = originalPosition - change;
	}
	this->ringConfiguration = change;
}

// Gets the encrypted letter from the right side of the rotor
char Rotor::getLetter(char letter)
{
	return this->getShiftedReverseLetter(this->getInternalLetter(this->getShiftedLetter(letter)));
}

// Gets the encrypted letter from the left side of the rotor
char Rotor::getReverseLetter(char letter)
{
	return this->getShiftedReverseLetter(this->getInternalReverseLetter(this->getShiftedLetter(letter)));
}

// Gets the shifted letter from the inner ring of contacts from the outside
char Rotor::getShiftedLetter(char letter)
{
	// If rotorPosition is bigger than 'z' start over at 'a'
	if ((letter + this->rotorPosition) > 'z') {
		return (letter + this->rotorPosition - 'z') + 'a' - 1;
	}
	else {
		return letter + this->rotorPosition;
	}
}

// Gets the shifted letter from the inner ring of contacts from the inside
char Rotor::getShiftedReverseLetter(char letter)
{
	// If rotorPosition is smaller than 'a' start over at 'z'
	if ((letter - this->rotorPosition) < 'a') {
		return (letter - this->rotorPosition - 'a') + 'z' + 1;
	}
	else {
		return letter - this->rotorPosition;
	}
}

// Gets the letter that is being displayed
char Rotor::getRotorLetter() {
	return this->rotorLetter;
}

// Gets the other letter of the pair from the right side of the rotor
char Rotor::getInternalLetter(char letter)
{
	// Searches for the letter the letter is connected to
	for (int i = 0; i < 26; i++) {
		if (letter == rotorConfiguration[i][0]) {
			return rotorConfiguration[i][1];
		}
	}
}

// Gets the other letter of the pair from the left side of the rotor
char Rotor::getInternalReverseLetter(char letter)
{
	// Searches for the letter the letter is connected to
	for (int i = 0; i < 26; i++) {
		if (letter == rotorConfiguration[i][1]) {
			return rotorConfiguration[i][0];
		}
	}
}

// Gets the current trigger status of the rotor
bool Rotor::getTriggered()
{
	if ((this->triggerLetter == this->rotorLetter) || (this->triggerLetterTwo == this->rotorLetter)) {
		return true;
	}
	else {
		return false;
	}
}

