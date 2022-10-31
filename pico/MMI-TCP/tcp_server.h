#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "lwipopts.h"

#define BUFFER_SIZE 512

struct tcp_server {
	struct tcp_pcb *server_pcb;		// server protocol control block
	struct tcp_pcb *client_pcb;		// client protocol control block
	uint8_t rx_buffer[BUFFER_SIZE]; // receive buffer
	uint8_t tx_buffer[BUFFER_SIZE]; // transmit buffer
	u8_t polling_interval;			// polling interval in seconds

	/**
	 * @brief Called when the server receives data from the client
	 *
	 * @param server the server object that received the data
	 * @param len the length of the data received
	 */
	void (*on_recv)(struct tcp_server *server, uint16_t len);

	/**
	 * @brief Called when the server completes sending data to the client
	 *
	 * @param server the server object that sent the data
	 * @param len the length of the data sent
	 */
	err_t (*on_send_complete)(struct tcp_server *server, uint16_t len);
};

/**
 * @brief Configures a TCP Server
 *
 * @param arg the server object to configure
 * @param newpcb the new protocol control block to configure
 * @param err the error originating from lwip
 * @return err_t error code, 0 if successful
 */
err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);

err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);

void tcp_server_err(void *arg, err_t err);

int tcp_server_open(struct tcp_server *server, u16_t port);

err_t tcp_write_immediate(struct tcp_server *server, const void *data, uint16_t len);

err_t tcp_write_enqueue(struct tcp_server *server, const void *data, uint16_t len);

err_t tcp_write_flush(struct tcp_server *server);

struct tcp_server *init_defaults();

int wifi_init();

int wifi_close();

#endif
