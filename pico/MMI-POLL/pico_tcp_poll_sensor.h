#ifndef PICOTCP_POLL_SENSOR_H
#define PICOTCP_POLL_SENSOR_H

#include "adc_queue.h"
#include "tcp_server.h"

#define TCP_PORT 4242
#define MAX_SAMPLES 256

#define SETUP_ERROR_MSG "Unable to setup TCP server\n"
#define HANDLE_SETUP_ERROR(cond)                                                                   \
	if (cond) {                                                                                    \
		printf(SETUP_ERROR_MSG);                                                                   \
		goto PROGRAM_END;                                                                          \
	}

#define MESSAGE_STR "PICO_W[QUEUE_UTILIZATION(%f\%), ADC_0(%llu, %f)]\n"
#define MESSAGE_BUFFER_SIZE 128

// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
const float CONVERSION_FACTOR = 3.3f / (1 << 12);

const char *ADC_REQUEST = "ADC";

Queue *sample_queue;

void on_receive(struct tcp_server *server, uint16_t len);

bool adc_callback(struct repeating_timer *t);

void adc_initialize();

#endif /* PICOTCP_POLL_SENSOR_H */