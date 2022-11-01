#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "pico_tcp_shared.h"

int main() {
	// init stdio
	stdio_init_all();

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

	// TCP has been fully initialized, start the ADC core
	multicore_launch_core1(adc_core_main);

	char buffer[MESSAGE_BUFFER_SIZE];
	uint16_t buffer_length = 0;
	sample_t sample;

	while (1) {
		if (adc_queue) {
			// block until new data becomes available
			queue_remove_blocking(adc_queue, &sample);

			// format into buffer
			buffer_length = snprintf(buffer, MESSAGE_BUFFER_SIZE, MESSAGE_STR,
									 (float)queue_get_level(adc_queue) / (float)MAX_SAMPLES,
									 sample.timestamp, sample.value * CONVERSION_FACTOR);
			// send buffer to client lazily
			tcp_write_immediate(server, buffer, buffer_length);
		} else {
			// do nothing
			tight_loop_contents();
		}
	}

PROGRAM_END:
	// disconnect wifi
	wifi_close();
	return 0;
}

void on_receive(struct tcp_server *server, uint16_t len) {
	printf("Recieved command %s from client.\n", server->rx_buffer);
}