#include <stdlib.h>
#include <string.h>

#include "tcp_server.h"

#define TCP_PORT 4242
#define SETUP_ERROR_MSG "Unable to setup TCP server\n"
#define HANDLE_SETUP_ERROR(cond)                                                                   \
	if (cond) {                                                                                    \
		printf(SETUP_ERROR_MSG);                                                                   \
		goto prog_end;                                                                             \
	}

void on_recieve(struct tcp_server *server, uint16_t len);

int main() {
	stdio_init_all();

	int wifi_up = wifi_init();

	sleep_ms(2000);
	printf("Waking up...\n");

	struct tcp_server *server = init_defaults();

	server->on_recv = &on_recieve;

	HANDLE_SETUP_ERROR(!server)

	HANDLE_SETUP_ERROR(tcp_server_open(server, 4242))

	while (1) {
		// do nothing
		sleep_ms(1000);
	}

prog_end:
	wifi_close();
	return 0;
}

void on_recieve(struct tcp_server *server, uint16_t len) {
	// log message
	printf("Server recieved: %s\n", (char *)server->rx_buffer);
	// echo message
	tcp_write_immediate(server, server->rx_buffer, len);
}