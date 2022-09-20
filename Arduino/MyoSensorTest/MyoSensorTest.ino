#define SENSOR A0
#define SAMPLE_DELAY 1000


int sensor_value = 0;

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
}

void loop() {
  sensor_value = analogRead(SENSOR);
//  Serial.print(0);
//  Serial.print(" ");
//  Serial.print(1024);
//  Serial.print(" ");
  Serial.println(sensor_value);
  delayMicroseconds(SAMPLE_DELAY);
}
