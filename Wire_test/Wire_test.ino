#include <Wire.h>
#include <avr/io.h>
//#include <Arduino.h>
#include <util/delay.h>
#include "Plugboard.h"

#define PCF01 39  // 39
#define PCF02 38  // 38
#define PCF03 37  // 37
#define PCF04 36  // 36

Plugboard plugboard;

int main(){
  init();
  
  Wire.begin();
  Serial.begin(9600);
  plugboard = Plugboard();
  while(1){
    Serial.println(plugboard.getPluggedLetter('c'));
    Serial.println(plugboard.getPluggedLetter('z'));
    delay(2000);
  }
  return 0;
}








