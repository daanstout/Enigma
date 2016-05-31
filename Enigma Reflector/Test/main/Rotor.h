#include <avr/io.h>

#pragma once
class Rotor
{
private:
	char rotorConfiguration[26][2];
	uint8_t rotorPosition;
	char rotorLetter;
	uint8_t ringConfiguration;
	char triggerLetter;
public:
	Rotor();
	void rotateForward();
	void rotateBackwards();
	void changeRingConfiguration(uint8_t change);
	char getLetter(char letter);
	char getShiftedLetter(char letter);
	char getInternalLetter(char letter);
	char getReverseLetter(char letter);
	char getShiftedReverseLetter(char letter);
	char getInternalReverseLetter(char letter);
	char getRotorLetter();
	bool getTriggered();
};

