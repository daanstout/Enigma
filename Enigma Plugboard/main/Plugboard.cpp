#include "Plugboard.h"
#include <Wire.h>
#include <avr/io.h>

Plugboard::Plugboard(){

}

// the main function, this function takes care of the whole operation. Just give it a character and it will 
char Plugboard::getPluggedLetter(char c){
  if(c < 'a' || c > 'z'){         //checks wether c is a small letter, if not, stopt the function
    return 0;
  }else{
    uint8_t shifting = setPort(c);          //sends the given char to the correct port, saves the port that was send low
    int returnPort = getPCF(shifting);      //checks what expander has a port that is low, ignoring the port that was set low by the previous function setPort(), giving 0 if none are found
    if(returnPort == 0){                    //if no ports were found, returns the char that was given. the letter was not bound to another so the given char should be returned.
      return c;
    }else{
      int dataRecieved = recieveData(returnPort, shifting, portSend);   //recieves the data from the port expander and puts the data in an integer
      return letterRead(returnPort, dataRecieved);                      //translates the integer to a char
    }
  }
}

//this function sets all the port expanders to high, so previous ports are reset and new letters can succesfully be encoded
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

//sends the char to the correct port on the correct portexpander
int Plugboard::setPort(char c){
  setAllZero();                         //sets all ports to zero
  int shifter = c - 'a';                //integer that says how many times the 1 needs to get shifted to get the correct spot in the integer, each bit is a different letter of the alfabet
  int shifted = 0;                      //the integer that says what character has to be send according to the correct bit
  int usePort;                          //the port that the data was send to
  if(shifter > 7){                      //if the bit has to be shifted 8 bits or more
    if (shifter > 15){                  //if the bit has to be shifted 16 bits or more
      if (shifter > 23){                //if the bit has to be shifted 24 bits or more
        shifter -= 24;                  //lowers the shifter with 24 so it either shifts not, or 1
        shifted = 1 << shifter;         //shifts a 1 to the left according to how many shifts are needed
        usePort = PCF04;                //says what port needs to be send to
      }else{
        shifter -= 16;                  //same as before but with 16 and another expander
        shifted = 1 << shifter;
        usePort = PCF03;
      }
    }else{
      shifter -= 8;                     //same as before
      shifted = 1 << shifter;
      usePort = PCF02;
    }
  }else{
    usePort = PCF01;                    //same as before
    shifted = 1 << shifter;
  }
  Wire.beginTransmission(usePort);      //opens the Wire with the correct port
  Wire.write(0xFF - shifted);           //sets all ports to high except for the port that connects to the correct char, because the port expander inverts everything
  Wire.endTransmission();               //stops the Wire

  portSend = usePort;                   //saves the port that was send to
  return shifted;                       //returns the integer that contains the char
}

//recieves the data and works it to a usable integer
int Plugboard::recieveData(uint8_t PCFPort, uint8_t port, uint8_t sendPort){
  Wire.requestFrom(PCFPort, 1);         //requests 1 byte from the given expander
  int c;
  while(Wire.available()){
    c = Wire.read();                    //reads the byte
  }
  uint8_t returnPort;
  if(PCFPort == sendPort){              //if the byte was taken from the same expander as the original char was send to, returns 255 - the obtained char and the original data send, else it ignores the original data send
    returnPort = 255 - c - port;
  }else{
    returnPort = 255 - c;
  }
  return returnPort;
}

//translates an integer to a letter
char Plugboard::letterRead(uint8_t PCFPort, int c){
  int count = 0;
  while(!(c & 1)){
    c = c >> 1;                //shifts the 1 to the right and increases count by 1 for everytime it does untill the first bit is 1
    count ++;
  }
  if(PCFPort == 0x26){        //depending on the port it was taken from, increases count by 8, 16 or 24
    count = count + 8;
  }else if(PCFPort == 0x25){
    count = count + 16;
  }else if(PCFPort == 0x24){
    count = count + 24;
  }
  return count + 'a';         //returns count + 'a' so it becomes a character
}

//checks what expander has been linked to the original character
int Plugboard::getPCF(int shift){
  for(int i = 36; i < 40; i++){     //loops through each expander
    Wire.requestFrom(i, 1);
    int c;
    while(Wire.available()){
      c = Wire.read();              //reads the port stands of each expander
    }
    if(c < 255 && c > 0){           //checks whether the obtained byte is not everything high or everything low
      if(i == portSend){            //checks if the port is the port that the original character is originally send to
        if(c != 255 - shift){       //if the above if statement is true, checks if the obtained byte is not the data send to the portexpander previously
          return i;                 //if it is not the same as the data send, returns this expander
        }
      }else{
        return i;                   //returns this expander
      }
    }
  }
  return 0;                         //if no expander has been linked, it returns 0 to signal the character has not been linked.
}
