#ifndef oldGripper_h
#define oldGropper_h
int RPWM=5;
int LPWM=6;
// timer 0
int L_EN=7;
int R_EN=8;

//int spd = 100;

void setup_gripper() {
  for(int i=5;i<9;i++){
   pinMode(i,OUTPUT);
  }
  for(int i=5;i<9;i++){
   digitalWrite(i,LOW);
  }
  digitalWrite(R_EN,HIGH);
  digitalWrite(L_EN,HIGH);
}

void closeSpeed(int spd) {
  analogWrite(RPWM,0);
  analogWrite(LPWM,spd);
}

void openSpeed(int spd) {
  analogWrite(LPWM,0);
  analogWrite(RPWM,spd);
}

void neutral() {
  analogWrite(LPWM,0);
  analogWrite(RPWM,0);
}

void openGripper()
{
//  Serial.begin(9600);

  
  

  
//  for(int i=0;i<256;i++){
  for(int i=0;i<100;i++){
//    analogWrite(RPWM,i);
    analogWrite(LPWM,100-i);
    delay(50);
  }
  
//  Serial.print("open finished\n");

//  Serial.print("\n");
}

void closeGripper()
{


  /*
  for(int i=0;i<100;){
    analogWrite(RPWM,i);
//  analogWrite(LPWM,255-i);
    delay(50);
    i = i + 15;
    
  }
  */
  analogWrite(RPWM, 0);
  analogWrite(RPWM, 100);
//  delay(3000);

//  Serial.print("close finished\n");

//  Serial.print("\n");
}
#endif
