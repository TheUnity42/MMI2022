#define SENSOR A0
#define SAMPLE_RATE 1000

#define HANDSHAKE_CODE 0x7E
#define HANDSHAKE_RESPONSE 0x7F
#define SAMPLE_START 0x80

int sensor_value = 0;
unsigned long duration = 0;
unsigned long runtime = 0;

typedef enum { WAIT_FOR_HANDSHAKE, WAIT_FOR_START, SAMPLE } State;

// initialize state machine at WAIT_FOR_HANDSHAKE
State state = WAIT_FOR_HANDSHAKE;

void setup() {
	Serial.begin(115200);
	pinMode(A0, INPUT);
}

void loop() {
	switch (state) {
	case WAIT_FOR_HANDSHAKE:
		if (Serial.available() > 0) {
			if (Serial.read() == HANDSHAKE_CODE) {
				Serial.write(HANDSHAKE_RESPONSE);
				state = WAIT_FOR_START;
			}
		}
		break;
	case WAIT_FOR_START:
		if (Serial.available() > 0) {
			if (Serial.read() == SAMPLE_START) {
				// read 4 byte duration
				duration = (long) Serial.read() << 24;
				duration |= (long) Serial.read() << 16;
				duration |= (long) Serial.read() << 8;
				duration |= (long) Serial.read();
				runtime = 0;
				state = SAMPLE;
			}
		}
		break;
	case SAMPLE:
		sensor_value = analogRead(SENSOR);
		Serial.write((byte *)&sensor_value, sizeof(sensor_value));
		delayMicroseconds(1000 / SAMPLE_RATE);

    // check if we have reached the end of the sampling interval
    if (runtime >= duration) {
      state = WAIT_FOR_HANDSHAKE;
    } else {
      runtime += 1000 / SAMPLE_RATE;
    }

		break;
	}
}
