#include "Plugboard.h"
#include <Wire.h>
#include <avr/io.h>

Plugboard::Plugboard(){

}

char Plugboard::getPluggedLetter(char c){
  uint8_t shifting = setPort(c);
  int returnPort = getPCF(shifting);
  if(returnPort == 0){
    return c;
  }else{
    int dataRecieved = recieveData(returnPort, shifting, portSend);
    return letterRead(returnPort, dataRecieved);
  }
}


void Plugboard::setAllZero(){
  Wire.beginTransmission(PCF01);
  Wire.write(0xFF);
  Wire.endTransmission();
  Wire.beginTransmission(PCF02);
  Wire.write(0xFF);
  Wire.endTransmission();
  Wire.beginTransmission(PCF03);
  Wire.write(0xFF);
  Wire.endTransmission();
  Wire.beginTransmission(PCF04);
  Wire.write(0xFF);
  Wire.endTransmission();
}

int Plugboard::setPort(char c){
  setAllZero();
  int shifter = c - 'a';
  int shifted = 0;
  int usePort;
  if(shifter > 7){
    if (shifter > 15){
      if (shifter > 23){
        shifter -= 24;
        shifted = 1 << shifter;
        usePort = PCF04;
      }else{
        shifter -= 16;
        shifted = 1 << shifter;
        usePort = PCF03;
      }
    }else{
      shifter -= 8;
      shifted = 1 << shifter;
      usePort = PCF02;
    }
  }else{
    usePort = PCF01;
    shifted = 1 << shifter;
  }
  Wire.beginTransmission(usePort);
  Wire.write(0xFF - shifted);
  Wire.endTransmission();

  portSend = usePort;
  return shifted;
}

int Plugboard::recieveData(uint8_t PCFPort, uint8_t port, uint8_t sendPort){
  Wire.requestFrom(PCFPort, 1);
  int c;
  while(Wire.available()){
    c = Wire.read();
  }
  uint8_t returnPort;
  if(PCFPort == sendPort){
    returnPort = 255 - c - port;
  }else{
    returnPort = 255 - c;
  }
  return returnPort;
}


char Plugboard::letterRead(uint8_t PCFPort, int c){
  //uint32_t c = e;
  int count = 0;
  while(!(c & 1)){
    c = c >> 1;
    count ++;
  }
  //Serial.println(PCFPort);
  if(PCFPort == 0x26){
    count = count + 8;
  }else if(PCFPort == 0x25){
    count = count + 16;
  }else if(PCFPort == 0x24){
    count = count + 24;
  }
  return count + 'a';
}

int Plugboard::getPCF(int shift){
  for(int i = 36; i < 40; i++){
    Wire.requestFrom(i, 1);
    int c;
    while(Wire.available()){
      c = Wire.read();
    }
    //Serial.println(c);
    if(c < 255 && c > 0){
      if(i == portSend){
        if(c != 255 - shift){
          return i;
        }
      }else{
        return i;
      }
    }
  }
  return 0;
}
