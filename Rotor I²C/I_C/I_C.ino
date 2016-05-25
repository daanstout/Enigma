#include <Wire.h>
#include <avr/io.h>
#include <util/delay.h>
#include "String.h"

uint8_t inD2, inD3 = 0;
uint8_t portAddress;
char binnen[40] = "";

ISR(INT1_vect, ISR_ALIASOF(INT0_vect));

ISR(INT0_vect){
  inD2 = ((PIND & B00000100) >> 2);
  inD3 = ((PIND & B00001000) >> 2);
  portAddress = (B00001100 | inD2 | inD3);
  Wire.end();
  Wire.begin(portAddress);
}

int main(){
  init();

  DDRD |= B01100000;

  sei();
  EICRA |= (1 << ISC10) | (1 << ISC00);
  EIMSK |= (1 << INT1) | (1 << INT0);

  inD2 = ((PIND & B00000100) >> 2);
  inD3 = ((PIND & B00001000) >> 2);

  portAddress = (B00001100 | inD2 | inD3);
  
  Wire.begin(portAddress);
  Wire.onReceive(receiveEvent);
  Serial.begin(9600);
  
  while(1){
    //Serial.println(portAddress, BIN);
    //Serial.println(portAddress);
	  Serial.println(binnen);
    _delay_ms(2500);
  }

  return 0;
}

void receiveEvent(int howMany){
  //Serial.println("hi");
	//char theString[40] = "";
	//binnen = "";
	for (int i = 0; i < 40; i++) {
		binnen[i] = "";
	}
	uint8_t t = 0;
	bool loop = true;
	while(loop) {
		if (Wire.available()) {
			char c = Wire.read();
			//Serial.println(c);
			if (c == '/') {
				loop = false;
			}
			else {
				binnen[t] = c;
				t++;
			}
		}
	}
  //char c = Wire.read();
  //Serial.println(theString);
}

