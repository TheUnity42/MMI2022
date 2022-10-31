#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico_tcp_poll_sensor.h"

int main() {
	// init stdio
	stdio_init_all();
	// init adc
	adc_initialize();
	// init wifi
	int wifi_up = wifi_init();

	sleep_ms(2000);
	printf("Waking up...\n");

	// init tcp server
	struct tcp_server *server = init_defaults();

	// set on receive callback
	server->on_recv = &on_receive;

	// error if server is null
	HANDLE_SETUP_ERROR(!server)

	// error if tcp server fails to open
	HANDLE_SETUP_ERROR(tcp_server_open(server, 4242))

	// setup an adc queue to hold up to MAX_SAMPLES samples
	sample_queue = (Queue *)malloc(sizeof(Queue));
	queue_initialize(sample_queue, MAX_SAMPLES);

	// setup the repeating timer to call the adc every 10ms
	struct repeating_timer adc_timer;
	add_repeating_timer_ms(10, adc_callback, (void *)sample_queue, &adc_timer);

	char buffer[MESSAGE_BUFFER_SIZE];
	uint16_t buffer_length = 0;
	Sample sample;

	while (1) {
		// enqueue any data that is available to the tcp server
		uint16_t length = sample_queue->size;

		for (uint16_t i = 0; i < length; i++) {
			if(!queue_dequeue(sample_queue, &sample)) {
				// format into buffer
				buffer_length = snprintf(buffer, MESSAGE_BUFFER_SIZE, MESSAGE_STR,
										(float) length / (float) MAX_SAMPLES,
										sample.timestamp, sample.value * CONVERSION_FACTOR);
				// send buffer to client lazily
				tcp_write_enqueue(server, buffer, buffer_length);
			} else {
				break; // we've run out of data
			}
		}
		// flush the tcp write queue
		tcp_write_flush(server);

		// do nothing
		sleep_ms(100);
	}

PROGRAM_END:
	// disconnect wifi
	wifi_close();
	return 0;
}

void on_receive(struct tcp_server *server, uint16_t len) {
	printf("Recieved command %s from client.\n", server->rx_buffer);
	// compare rx_buffer to the ADC_REQUEST string
	if ((char)server->rx_buffer[0] == 'A') {
		// the client request ADC data. Send all available samples
		// to the client
		int current_length = sample_queue->size;

		printf("Writing %d samples to client\n", current_length);

		char buffer[64];
		uint16_t buffer_length = 0;
		Sample sample;
		for (uint16_t i = 0; i < current_length; i++) {
			if (!queue_dequeue(sample_queue, &sample)) {
				printf("Sample %llu, %u\n", sample.timestamp, sample.value);
				// format into buffer
				buffer_length =
					snprintf(buffer, 64, "<%llu: %u>\n", sample.timestamp, sample.value);
				// send buffer to client lazily
				printf("Writing: %s", buffer);
				tcp_write_enqueue(server, buffer, buffer_length);
			} else {
				// we hit the end
				break;
			}
		}
		// flush the tcp write queue
		tcp_write_flush(server);
	}
}

bool adc_callback(struct repeating_timer *t) {
	// make a new sample and read data into it
	Sample sample;
	// read the adc
	sample.value = adc_read();
	// timestamp the sample
	sample.timestamp = to_us_since_boot(get_absolute_time());
	// enqueue the sample (throw away the error, we jsut wont write beyond the max)
	uint16_t success = queue_enqueue((Queue *)t->user_data, sample);
	// printf("Enqueued sample [value: %u, timestamp: %lu, status %u]\n", sample.value,
	// 	   sample.timestamp, success);
	return true;
}

void adc_initialize() {
	adc_init();
	// Make sure GPIO is high-impedance, no pullups etc
	adc_gpio_init(26);
	// Select ADC input 0 (GPIO26)
	adc_select_input(0);
}