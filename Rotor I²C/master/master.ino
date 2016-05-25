#include <Wire.h>
#include <avr/io.h>
#include <util/delay.h>

int main(){

  Wire.begin();
  Serial.begin(9600);
  while(1){
    char wireSend = 'a';
    for(int i = 12; i < 16; i++){
      Serial.println(i);
      //Wire.beginTransmission(15);
      //Wire.write(i);
      //Wire.endTransmission();
      //wireSend++;  
    }
    _delay_ms(1000);
  }

  return 0;
}

