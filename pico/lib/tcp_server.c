#include <stdlib.h>
#include <string.h>

#include "tcp_server.h"

int tcp_server_open(struct tcp_server *server, u16_t port) {
	// log
	printf("[TCPServer] Opening TCP Server at %s on %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)),
		   port);

	// create a protocol control block for any IP address
	struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		printf("[TCPServer] Failed to create PCB\n");
		return -1;
	}

	// bind pcb to port
	err_t err = tcp_bind(pcb, NULL, port);
	if (err) {
		printf("[TCPServer] Failed to bind to port %u\n", port);
		return -1;
	}

	// set to listen on port
	server->server_pcb = tcp_listen_with_backlog(pcb, 1);
	if (!server->server_pcb) {
		printf("[TCPServer] Failed to listen\n");
		if (pcb) {
			tcp_close(pcb);
		}
		return -1;
	}

	// set callback arguments
	tcp_arg(server->server_pcb, server);
	tcp_accept(server->server_pcb, tcp_server_accept);

	return 0;
}

err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
	// check that the accept did not fail
	if (err != ERR_OK || newpcb == NULL) {
		printf("[TCPServer] Failure in accept\n");
		return ERR_VAL;
	}
	// log new connection
	printf("[TCPServer] Client connected\n");

	// set callback arguments
	((struct tcp_server *)arg)->client_pcb = newpcb;
	tcp_arg(newpcb, arg);
	tcp_sent(newpcb, tcp_server_sent);
	tcp_recv(newpcb, tcp_server_recv);
	tcp_poll(newpcb, tcp_server_poll, ((struct tcp_server *)arg)->polling_interval * 2);
	tcp_err(newpcb, tcp_server_err);

	return 0;
}

err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
	if (((struct tcp_server *)arg)->on_send_complete) {
		((struct tcp_server *)arg)->on_send_complete(arg, len);
	}
	return 0;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
	struct tcp_server *server = (struct tcp_server *)arg;
	// check if the buffer exists
	if (!p) {
		return ERR_VAL;
	}
	// checks
	cyw43_arch_lwip_check();
	// check if the message contains any data
	if (p->tot_len > 0) {
		printf("[TCPServer] tcp_server_recv %d\n", p->tot_len);
		// read in the coming data into the rx buffer
		uint16_t amt_read = pbuf_copy_partial(p, (void *)(server->rx_buffer), BUFFER_SIZE, 0);
		// report that we have read in the data
		tcp_recved(tpcb, amt_read);
		// call user callback recv function
		if (server->on_recv)
			server->on_recv(server, amt_read);
	}
	// free the buffer
	pbuf_free(p);
	// return
	return ERR_OK;
}

err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) { return 0; }

void tcp_server_err(void *arg, err_t err) {
	// log any errors except abort
	if (err != ERR_ABRT) {
		printf("[TCPServer] TCP Server Error: %d\n", err);
	}
}

err_t tcp_write_immediate(struct tcp_server *server, const void *data, uint16_t len) {
	err_t err = ERR_OK;
	err = tcp_write_enqueue(server, data, len);
	if(!err) {
		err = tcp_output(server->client_pcb);
	}
	return err;
}

err_t tcp_write_enqueue(struct tcp_server *server, const void *data, uint16_t len) {
	if(!server || !data || len <= 0) {
		printf("[TCPServer] Unable to enqueue empty data\n");
		return ERR_VAL;
	}
	return tcp_write(server->client_pcb, data, len, TCP_WRITE_FLAG_COPY);
}

err_t tcp_write_flush(struct tcp_server *server) {
	return tcp_output(server->client_pcb);
}

struct tcp_server *init_defaults() {
	struct tcp_server *server = (struct tcp_server *)calloc(1, sizeof(struct tcp_server));
	if (!server)
		printf("[TCPServer] Unable to allocate space for server instance\n");
	// set defaults
	server->polling_interval = 1; // 1 sec default polling rate
	// return
	return server;
}

int wifi_init() {
	// setup the module
	if (cyw43_arch_init()) {
		printf("[TCPServer] failed to initialise cyw43\n");
		return -1;
	}

	cyw43_arch_enable_sta_mode();

	printf("[TCPServer] Connecting to WiFi...\n");
	if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK,
										   30000)) {
		printf("[TCPServer] failed to connect.\n");
		return -1;
	} else {
		printf("[TCPServer] Connected.\n");
		return 0;
	}
}

int wifi_close() { cyw43_arch_deinit(); }