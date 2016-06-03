#include <Wire.h>
#include <avr/io.h>
#include <util/delay.h>

uint8_t inD2, inD3 = 0;
uint8_t portAddress;

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
    Serial.println(portAddress, BIN);
    Serial.println(portAddress);
    _delay_ms(2500);
  }

  return 0;
}

void receiveEvent(int howMany){
  Serial.println("hi");
  Serial.println(Wire.read());
}

