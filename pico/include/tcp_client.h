#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/dns.h"

#include "lwipopts.h"

#define BUFFER_SIZE 512

struct tcp_client {
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

/**
 * @brief Called after the server finishes sending data to the client
 *
 * @param arg the server object that sent the data
 * @param tpcb the protocol control block that sent the data
 * @param len the length of the data sent
 * @return err_t
 */
err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);

/**
 * @brief Called when the server receives data from the client
 *
 * @param arg the server object that received the data
 * @param tpcb the protocol control block that received the data
 * @param p the buffer containing the recieved data
 * @param err the error encountered (0 if no error)
 * @return err_t if an error occurs processing data
 */
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

/**
 * @brief Called when the server polls the client
 *
 * @param arg the server object that is polling the client
 * @param tpcb the protocol control block that is polling the client
 * @return err_t if any error occurs while attempting to poll the client
 */
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);

/**
 * @brief Handles any critical errors that occur while the server is running
 *
 * @param arg the server that encountered the error
 * @param err the error encountered
 */
void tcp_server_err(void *arg, err_t err);

/**
 * @brief Opens a TCP Server on the specified port
 *
 * @param server the server object to bind to this port
 * @param port the port to bind to
 * @return int 0 if successful, else an error
 */
int tcp_server_open(struct tcp_server *server, u16_t port);

/**
 * @brief Writes data to the client and send it immediately.
 * This may be less efficient overall. tcp_write_enqueue will allow the module to bundle data
 * together and improve efficiency
 *
 * @param server the server object sending the data
 * @param data the data to write
 * @param len the length of the data to write
 * @return err_t 0 if successful, else an error
 */
err_t tcp_write_immediate(struct tcp_server *server, const void *data, uint16_t len);

/**
 * @brief enqueues data to be written to the client.
 * This method allows the module to group data to improve efficiency
 *
 * @param server the server object sending the data
 * @param data the data to write
 * @param len the length of the data to write
 * @return err_t 0 if successful, else an error
 */
err_t tcp_write_enqueue(struct tcp_server *server, const void *data, uint16_t len);

/**
 * @brief Flushes any enqueued data by sending it to the client immediately
 *
 * @param server the server sending the data
 * @return err_t 0 if successful, else an error
 */
err_t tcp_write_flush(struct tcp_server *server);

/**
 * @brief Allocated memory for a new server object and sets default parameters
 *
 * @return struct tcp_server* a pointer to the newly created server
 */
struct tcp_server *init_defaults();

/**
 * @brief Initializes the Pico W's wifi module and attempts to connect to the network specified by
 * CMake
 *
 * @return int 0 if successful, else an error
 */
int wifi_init();

/**
 * @brief Deactivates the wifi module
 *
 * @return int 0 if successful, else an error
 */
int wifi_close();

#endif
