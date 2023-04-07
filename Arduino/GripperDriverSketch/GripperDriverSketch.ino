#include "oldGripper.h"

int detectorPin = 11;
char label;
long long time; 
bool state = false, flipper = false;
/*
int RPWM=5;
int LPWM=6;
// timer 0
int L_EN=7;
int R_EN=8;
*/

void setup() {
  // put your setup code here, to run once:
  /*
  for(int i=5;i<9;i++){
   pinMode(i,OUTPUT);
  }
   for(int i=5;i<9;i++){
   digitalWrite(i,LOW);
  }
   delay(1000);
    Serial.begin(9600);
    */
    openGripper();
    closeGripper();
    pinMode(detectorPin, INPUT);
    digitalWrite(detectorPin, HIGH);
    Serial.begin(9600);
  }



void loop() {
  // stop the code untill we get something 
  while(Serial.available() == 0){
  }
  label = Serial.read();
  
  // put your main code here, to run repeatedly:
  
    
      //motor.setPosition( flipper ? 6 : 0);
      if(label > 0)
      {
        Serial.print("close\n");
        closeGripper();
      }
      else
      {
        Serial.print("open\n");
        openGripper();
      }
      
    
    
  
}
