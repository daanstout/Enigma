#include "Rotor.h"
#include <avr/io.h>

Rotor::Rotor()
{
	// Puts the full alphabet in the first row of the array
	for (int i = 0; i < 26; i++) {
		this->rotorConfiguration[i][0] = 'a' + i;
	}

	char configuration[27] = "ekmflgdqvzntowyhxuspaibrcj";

	for (int i = 0; i < 26; i++) {
		this->rotorConfiguration[i][1] = configuration[i];
	}

	// Set standard values
	this->ringConfiguration = 0;
	this->rotorPosition = 0;
	this->rotorLetter = 'a';
	this->triggerLetter = 'c';
}

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

void Rotor::changeRingConfiguration(uint8_t change)
{
	// Sets the rotor Position
	uint8_t originalPosition = this->rotorPosition + this->ringConfiguration;
	if ((originalPosition - change) < 0) {
		this->rotorPosition = (originalPosition + 26) - change;
	}
	else {
		this->rotorPosition = originalPosition - change;
	}
	this->ringConfiguration = change;
}

char Rotor::getLetter(char letter)
{
	return this->getShiftedReverseLetter(this->getInternalLetter(this->getShiftedLetter(letter)));
}

char Rotor::getReverseLetter(char letter)
{
	return this->getShiftedReverseLetter(this->getInternalReverseLetter(this->getShiftedLetter(letter)));
}

char Rotor::getShiftedLetter(char letter)
{
	// If rotorPosition is bigger than 'z'start over at 'a'
	if ((letter + this->rotorPosition) > 'z') {
		return (letter + this->rotorPosition - 'z') + 'a';
	}
	else {
		return letter + this->rotorPosition;
	}
}

char Rotor::getShiftedReverseLetter(char letter)
{

	if (this->ringConfiguration > 0) {
		letter += 1;
	}

	// If rotorPosition is smaller than 'a'start over at 'z'
	if ((letter - this->rotorPosition) < 'a') {
		return (letter - this->rotorPosition - 'a') + 'z';
	}
	else {
		return letter - this->rotorPosition;
	}
}

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

bool Rotor::getTriggered()
{
	if (this->triggerLetter == this->rotorLetter) {
		return true;
	}
	else {
		return false;
	}
}

