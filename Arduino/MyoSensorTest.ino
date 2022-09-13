#define SENSOR A0
#define SAMPLE_DELAY 10

int sensor_value = 0;

void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);
}

void loop() {
  sensor_value = analogRead(SENSOR);
  Serial.println(sensor_value);
  delayMicroseconds(SAMPLE_DELAY);
}
