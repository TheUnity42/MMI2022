#ifndef PICO_TCP_SHARED_H
#define PICO_TCP_SHARED_H

#include "pico/util/queue.h"
#include "tcp_server.h"

#define TCP_PORT 4242
#define MAX_SAMPLES 32

#define SETUP_ERROR_MSG "Unable to setup TCP server\n"
#define HANDLE_SETUP_ERROR(cond)                                                                   \
	if (cond) {                                                                                    \
		printf(SETUP_ERROR_MSG);                                                                   \
		goto PROGRAM_END;                                                                          \
	}

#define MESSAGE_STR "PICO_W[QUEUE_HEALTH(%f\%), ADC_0(%llu, %f)]\n"
#define MESSAGE_STR "(%f,%llu,%f)\n"
#define MESSAGE_BUFFER_SIZE 128

typedef struct SAMPLE_T_ {
	uint16_t value;
	uint64_t timestamp;
} sample_t;

// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
extern const float CONVERSION_FACTOR;

extern const char *ADC_REQUEST;

extern queue_t *adc_queue;

void on_receive(struct tcp_server *server, uint16_t len);

bool adc_callback(struct repeating_timer *t);

void adc_initialize();

void adc_core_main();

#endif /* PICOTCP_POLL_SENSOR_H */