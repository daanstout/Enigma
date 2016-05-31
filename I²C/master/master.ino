#include <Wire.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "USART.h"
#include "Plugboard.h"

MY_USART myUsart;
Plugboard plugboard;
char command[30];

int main(){
  init();

  myUsart.initUSART();        //initialiseert de USART
  plugboard = Plugboard();

  DDRB |= (1 << PORTB5);
  
  Wire.begin();               //initialiseert de Wire
  //Serial.begin(9600);

  PORTB |= (1 << PORTB5);
  
  char result[3];
  
  while(1){
    receiveCommand();         //haalt een command op van het DE2 board
    result[0] = '\0';         //leegt result
    result[1] = '\0';
    result[2] = '\0';
    //printCommand();
    if(strncmp(command, "rotateForward", 13) == 0){           //als het commando van DE2 rotateForward is
      slaveSendCommand(command[14] - 37, "rotateForward/");
    }else if(strncmp(command, "getRotorLetter", 14) == 0){
      slaveSendCommand(command[15] - 37, "getRotorLetter/");
      result[0] = slaveReceiveCommand(command[15] - 37);
      result[1] = '/';
      myUsart.println(result);                                //stuurt de byte naar het DE2 board
    }else if(strncmp(command, "rotateBackwards", 15) == 0){
      slaveSendCommand(command[16] - 37, "rotateBackwards/");
    }else if(strncmp(command, "getLetter", 9) == 0){
      slaveSendCommand(command[10] - 37, "getLetter", command[strlen(command) - 1]);
      result[0] = slaveReceiveCommand(command[10] - 37);
      result[1] = '/';
      myUsart.println(result);
      //PORTB ^= (1 << PORTB5);
    }else if(strncmp(command, "getReverseLetter", 16) == 0){
      slaveSendCommand(command[17] - 37, "getReverseLetter", command[strlen(command) - 1]);
      result[0] = slaveReceiveCommand(command[17] - 37);
      result[1] = '/';
      myUsart.println(result);
    }else if(strncmp(command, "changeRingCfg", 13) == 0){
      slaveSendCommand(command[14] - 37, "changeRingCfg", command[strlen(command) - 1]);
    }else if(strncmp(command, "getTriggered", 12) == 0){
      slaveSendCommand(command[13] - 37, "getTriggered/");
      result[0] = slaveReceiveCommand(command[13] - 37) + 48;
      result[1] = '/';
      myUsart.println(result);
    }else if(strncmp(command, "getPluggedLetter", 16) == 0){
      sprintf(result, "%c/", plugboard.getPluggedLetter(command[strlen(command) - 1]));
      myUsart.println(result);
    }else if(strncmp(command, "getReflector", 12) == 0){
      slaveSendCommand(16, "getReflector", command[strlen(command) - 1]);
      result[0] = slaveReceiveCommand(16);
      result[1] = '/';
      myUsart.println(result);
    }
  }

  return 0;
}

// I2C send function that sends just a string
void slaveSendCommand(uint8_t adres, char sendCommand[30]){
  Wire.beginTransmission(adres);        //open naar slave op gegeven adres
  Wire.write(sendCommand);              //stuurt de gegeven command naar de slave
  Wire.endTransmission();               //sluit het kanaal
}

// I2C send function that sends a string + an aditional byte, for example, and extra char
void slaveSendCommand(uint8_t adres, char sendCommand[30], char data){
  Wire.beginTransmission(adres);
  Wire.write(sendCommand);
  Wire.write(data);
  Wire.write("/");
  Wire.endTransmission();
}

char slaveReceiveCommand(uint8_t adres){
  _delay_ms(15);                        //wacht 15 millieseconden om de slave zijn werk te laten doen
  Wire.requestFrom(adres, 1);           //vraag 1 byte van slave op gegeven adres
  char f = Wire.read();                 //return de ontvangen byte
  return f;
}

// Receives a command
char receiveCommand() {
  PORTB ^= (1 << PORTB5);
  char key = myUsart.receiveByte();
  // Empties the command string
  emptyCommand();
  PORTB ^= (1 << PORTB5);
  // Loops until the command has ended
  while (key > 0) {
    sprintf(command, "%s%c", command, key);
    key = myUsart.receiveByte();

    // PORTB ^= (1 << PORTB5);
    // Used to break the loop on a character
    if (key == '/') {
      key = 0;
      
      PORTB ^= (1 << PORTB5);
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

// Prints the command
void printCommand() {
  myUsart.println(command);
}
