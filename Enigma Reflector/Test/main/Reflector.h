#include <avr/io.h>

#pragma once
class Reflector
{
private:
	char reflectorConfiguration[26][2];
public:
	Reflector();
	char getLetter(char letter);
};

