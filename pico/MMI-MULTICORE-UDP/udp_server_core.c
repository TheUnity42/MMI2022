#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "shared.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

struct dns_callback {
	uint8_t valid;
	ip_addr_t* addr;
};

// https://www.nongnu.org/lwip/2_0_x/group__udp__raw.html

void on_dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
	struct dns_callback* callback_data = (struct dns_callback*)callback_arg;

	char *ip_name = ip4addr_ntoa(ipaddr);

	// printf("[UDPServer] DNS Callback %s, %s\n", name, ip_name);

	if (ip_name) {
		callback_data->addr = ipaddr;
		callback_data->valid = 1;
	} else {
		printf("[UDPServer] Failed to resolve host\n");
	}
}

int main() {
	// init stdio
	stdio_init_all();

	// init wifi
	int wifi_up = wifi_init();

	sleep_ms(2000);
	printf("Waking up...\n");

	// init UDP
	struct udp_pcb *pcb = udp_new_ip_type(IPADDR_TYPE_V4);
	if (!pcb) {
		printf("[UDPServer] Failed to create PCB\n");
		return -1;
	}

	printf("Created pcb\n");
	sleep_ms(1000);

	err_t err = 0;
	//udp_bind(pcb, netif_ip4_addr(netif_list), 1000);
	if (err) {
		printf("[UDPServer] Failed to bind to port 4200\n");
		return -1;
	}

	printf("[UDPServer] Attempting to resolve host...\n");
	sleep_ms(1000);

	ip_addr_t server_ip;
	struct dns_callback callback_struct = {0};
	callback_struct.valid = 1;
	callback_struct.addr = NULL;

	IP_ADDR4(callback_struct.addr, 192, 168, 86, 50);

	// ip4addr_aton("192.168.86.50", callback_struct.addr);

	while (!callback_struct.valid) {
		err = dns_gethostbyname(HOST, &server_ip, on_dns_found, &callback_struct);
		if (err == ERR_INPROGRESS) {
			printf("[UDPServer] DNS lookup in progress...\n");
		} else if (err == ERR_ARG) {
			printf("[UDPServer] DNS lookup failure ARG\n");
		} else if (err == ERR_OK) {
			printf("[UDPServer] DNS lookup OK\n");
			break;
		}
		sleep_ms(1000);
	}

	// printf("[UDPServer] DNS Resolved host to %s\n", ip4addr_ntoa(callback_struct.addr));
	printf("UDP Connect\n");
	// connect to resolved host
	err = udp_connect(pcb, callback_struct.addr, 4200);
	// sleep_ms(1000);

	if (err) {
		printf("[UDPServer] Failed to connect to host. Error = %d\n", err);
		return -1;
	} else {
		printf("[UDPServer] Connected to host\n");
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
			pbuf_free(p);
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