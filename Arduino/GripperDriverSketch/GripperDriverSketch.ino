#include "oldGripper.h"

int detectorPin = 11;
int label;
long long time;
bool state = false, flipper = false;
bool incremental = true;

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
  setup_gripper();
//  openGripper();
//  closeGripperTest();
//  openGripperTest();
  pinMode(detectorPin, INPUT);
  digitalWrite(detectorPin, HIGH);

  openSpeed(100);
  delay(2000);
  neutral();
  delay(500);
  closeSpeed(100);
  delay(2000);
  neutral();

  
  Serial.begin(9600);
  time = millis();
}



void loop() {
  // stop the code until we get something
  //  while(Serial.available() == 0){
  //  }
  //  label = Serial.read();
//  label = '0';

  // Ignore newline characters and other non-numeric characters
  while (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '\n' || ch == '\r') {
      break;
    } else {
      label = ch;
    }
//    Serial.println(label);
  }

  
  // put your main code here, to run repeatedly:


  //motor.setPosition( flipper ? 6 : 0);

  if (label == '1')
  {
    //Serial.print("close\n");
    //          openGripper();
    //          openSpeed
    openSpeed(100);
//    if (incremental)
//    {
//      delay(100);
//      stopGripper();
//    }
    //Uncomment for incremental

  }
  else if (label == '2')
  {
    closeSpeed(100);
    //Serial.print("open\n");
//    closeGripper();
//    //Uncomment for incremental
//    if (incremental)
//    {
//      delay(100);
//      stopGripper();
//    }
  } else {
    neutral();
  }

  if(millis() - time > 1000) {
    time = millis();
    neutral();
    label = '0';
  }

  delay(10);

}
