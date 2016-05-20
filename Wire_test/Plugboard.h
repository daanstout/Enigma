#include <Wire.h>
#include <avr/io.h>

#define PCF01 39  // 39
#define PCF02 38  // 38
#define PCF03 37  // 37
#define PCF04 36  // 36

#pragma once
class Plugboard{
private:
	int portSend;
public:
  Plugboard();
	char getPluggedLetter(char c);
	void setAllZero();
	int setPort(char c);
	int recieveData(uint8_t PCFPort, uint8_t port, uint8_t sendPort);
	char letterRead(uint8_t PCFPort, int c);
	int getPCF(int shift);
};
