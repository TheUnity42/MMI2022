#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "pico_tcp_shared.h"

// https://www.nongnu.org/lwip/2_0_x/group__udp__raw.html

int main() {
	// init stdio
	stdio_init_all();

	// init wifi
	int wifi_up = wifi_init();

	sleep_ms(2000);
	printf("Waking up...\n");

	// init UDP
	struct udp_pcb *pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		printf("[UDPServer] Failed to create PCB\n");
		return -1;
	}

	err_t err = udp_bind(pcb, NULL, 4200);
	if (err) {
		printf("[UDPServer] Failed to bind to port 4200\n");
		return -1;
	}

	printf("[UDPServer] Ready on port 4200\n");
	printf("[UDPServer] Attempting to resolve host...\n");

	ip_addr_t server_ip;
	uint8_t resolved = 0;
	err = dns_gethostbyname("fedora", &server_ip, NULL, &resolved);
	
	while (!resolved) {
		printf("[UDPServer] Waiting for DNS resolution...\n");
		sleep_ms(1000);
	}

	printf("[UDPServer] Resolved host to %s\n", ip4addr_ntoa(&server_ip));

	// connect to resolved host
	err = udp_connect(pcb, &server_ip, 4200);

	if (err) {
		printf("[UDPServer] Failed to connect to host\n");
		return -1;
	}

	// UDP has been fully initialized, start the ADC core
	multicore_launch_core1(adc_core_main);

	char buffer[MESSAGE_BUFFER_SIZE];
	uint16_t buffer_length = 0;
	sample_t sample;

	while(1) {
		if (adc_queue) {
			// block until new data becomes available
			queue_remove_blocking(adc_queue, &sample);

			// format into buffer
			buffer_length = snprintf(buffer, MESSAGE_BUFFER_SIZE, MESSAGE_STR,
									 (float)queue_get_level(adc_queue), sample.timestamp,
									 sample.value * CONVERSION_FACTOR);
			// send buffer to server
			struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, buffer_length, PBUF_RAM);
			memcpy(p->payload, buffer, buffer_length);
			udp_send(pcb, p);
		} else {
			// do nothing
			sleep_ms(10);
			tight_loop_contents();
		}
	}



PROGRAM_END:
	// disconnect wifi
	wifi_close();
	return 0;
}

void on_receive(struct tcp_server *server, uint16_t len) {
	int length = strlen(server->rx_buffer);
	for (; length > 0 && isspace(server->rx_buffer[length]); length--);
	server->rx_buffer[length] = '\0';
	printf("Recieved command %s from client.\n", server->rx_buffer);
}