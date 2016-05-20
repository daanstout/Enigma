#include <Wire.h>
#include <avr/io.h>
//#include <Arduino.h>
#include <util/delay.h>

#define A 0x01 // P0
#define B 0x02 // P1
#define C 0x04 // P2
#define D 0x08 // P3
#define E 0x10 // P4
#define F 0x20 // P5
#define G 0x40 // P6
#define H 0x80 // P7

#define PCF01 0x27  // 39



int main(){
  init();
  
  Wire.begin();
  Serial.begin(9600);

  while(1){
    Wire.beginTransmission(PCF01);
    Wire.write(0xFF - E);
    Wire.endTransmission();
    _delay_ms(1000);
    Serial.println(letterRead(PCF01 ,recieveData(PCF01, 7, E)));
    _delay_ms(1000);
    Wire.beginTransmission(PCF01);
    Wire.write(0);
    Wire.endTransmission();
    _delay_ms(1000);
  }
  return 0;
}

int recieveData(uint8_t PCFPort, int checkBit, uint8_t port){
  Wire.requestFrom(PCFPort, 1);
  int c;
  while(Wire.available()){
    c = Wire.read();
  }
  uint8_t returnPort = 255 - c - port;
  return returnPort; //(c >> (checkBit - 1)) & 1;
}

char letterRead(uint8_t PCFPort, uint8_t c){
  if(PCFPort == 0x26){
    c = c << 8;
  }else if(PCFPort == 0x25){
    c = c << 16;
  }
  int count = 0;
  while(!(c & 1)){
    c = c >> 1;
    count ++;
  }
  return count + 'a';
}

