#include <Wire.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "USART.h"

MY_USART myUsart;
char command[30];
char slaveCommand[30];

int main(){
  init();

  myUsart.initUSART();
  
  Wire.begin();
  //Serial.begin(9600);

  char result[2];
  
  while(1){
    receiveCommand();
    emptySlaveCommand();
    result[0] = '\0';
    result[1] = '\0';
    if(strncmp(command, "rotateForward", 13) == 0){
      Wire.beginTransmission(command[14] - 37);
      Wire.write("rotateForward/");
      Wire.endTransmission();
    }else if(strncmp(command, "getRotorLetter", 14) == 0){
      Wire.beginTransmission(command[15] - 37);
      Wire.write("getRotorLetter/");
      Wire.endTransmission();
      //ONTVANG
      _delay_ms(15);
      Wire.requestFrom(command[15] - 37, 1);
      result[0] = Wire.read();
      //DO STUFF
      myUsart.println(result);
    }else if(strncmp(command, "rotateBackwards", 15) == 0){
      Wire.beginTransmission(command[16] - 37);
      Wire.write("rotateBackwards/");
      Wire.endTransmission();
    }else if(strncmp(command, "getLetter", 9) == 0){
      Wire.beginTransmission(command[10] - 37);
      Wire.write("getLetter");
      Wire.write(command[strlen(command) - 1]);
      Wire.write("/");
      Wire.endTransmission();
      //ONTVANG
      _delay_ms(15);
      Wire.requestFrom(command[10] - 37, 1);
      result[0] = Wire.read();
      //DO STUFF
      myUsart.println(result);
    }else if(strncmp(command, "getReverseLetter", 16) == 0){
      Wire.beginTransmission(command[17] - 37);
      Wire.write("getReverseLetter");
      Wire.write(command[strlen(command) - 1]);
      Wire.write("/");
      Wire.endTransmission();
      //ONTVANG
      _delay_ms(15);
      Wire.requestFrom(command[17] - 37, 1);
      result[0] = Wire.read();
      //DO STUFF
      myUsart.println(result);
    }else if(strncmp(command, "changeRingCfg", 13) == 0){
      Wire.beginTransmission(command[14] - 37);
      Wire.write("changeRingCfg");
      Wire.write(command[strlen(command) - 1]);
      Wire.write("/");
      Wire.endTransmission();
    }else if(strncmp(command, "getTriggered", 12) == 0){
      Wire.beginTransmission(command[13] - 37);
      Wire.write("getTriggered/");
      Wire.endTransmission();
      //ONTVANG
      _delay_ms(15);
      Wire.requestFrom(command[13] - 37, 1);
      result[0] = Wire.read();
      //DO STUFF
      myUsart.println(result);
    }
//    _delay_ms(100);
  }

  return 0;
}

// Receives a command
char receiveCommand() {
  char key = myUsart.receiveByte();
  // Empties the command string
  emptyCommand();

  // Loops until the command has ended
  while (key > 0) {
    sprintf(command, "%s%c", command, key);
    key = myUsart.receiveByte();

    // Used to break the loop on a character
    if (key == '/') {
      key = 0;
    }
  }
}

// Sends a command
void sendCommand(char string[]) {
  myUsart.println(string);
}

// Empties the command string
void emptyCommand() {
  // sets all characters of the string to '\0'
  for (int i = 0; command[i] != '\0'; i++) {
    command[i] = '\0';
  }
}

void emptySlaveCommand() {
  // sets all characters of the string to '\0'
  for (int i = 0; slaveCommand[i] != '\0'; i++) {
    slaveCommand[i] = '\0';
  }
}

// Prints the command
void printCommand() {
  myUsart.println(command);
}
