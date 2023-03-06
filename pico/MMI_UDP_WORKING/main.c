#include <string.h>
#include <time.h>

#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "main.h"

struct udp_pcb *upcb;

// volatile variables
queue_t *adc_queue;
bool timer_flag = false;

int main() {
	// create local vars
	alarm_pool_t *alarm_pool;
	repeating_timer_t repeating_timer;
	struct udp_pcb *spcb;

	stdio_init_all();

	if (cyw43_arch_init()) {
		DEBUG_printf("[UDPServer] failed to initialise\n");
		return 1;
	}
	cyw43_arch_enable_sta_mode();

	printf("[UDPServer] Connecting to WiFi...\n");
	if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK,
										   30000)) {
		printf("[UDPServer] failed to connect.\n");
		return 1;
	} else {
		printf("[UDPServer] Connected.\n");
	}

	alarm_pool_init_default();
	alarm_pool = alarm_pool_get_default();

	alarm_pool_add_repeating_timer_ms(alarm_pool, 100, set_send_UDP, NULL, &repeating_timer);

	upcb = udp_new();
	spcb = udp_new();

	err_t err = udp_bind(spcb, IP_ADDR_ANY, RESPONSE_PORT);

	udp_recv(spcb, recv_from_UDP, NULL);

	ip_addr_t destination_ip;

	struct dns_callback callback_struct = {0};
	while (!callback_struct.valid) {
		err_t err = dns_gethostbyname(HOST, &destination_ip, on_dns_found, &callback_struct);
		if (err == ERR_INPROGRESS) {
			printf("[UDPServer] DNS lookup for %s in progress...\n", HOST);
		} else if (err == ERR_ARG) {
			printf("[UDPServer] DNS lookup failure ARG\n");
		} else if (err == ERR_OK) {
			printf("[UDPServer] DNS lookup OK\n");
			break;
		}
		sleep_ms(10);
	}

	printf("[UDPServer] DNS for %s Resolved to: %d.%d.%d.%d\n", HOST, (callback_struct.addr->addr & 0xff),
		   (callback_struct.addr->addr >> 8) & 0xff, (callback_struct.addr->addr >> 16) & 0xff,
		   callback_struct.addr->addr >> 24);

	// UDP has been fully initialized, start the ADC core
	multicore_launch_core1(adc_core_main);

	char buffer[MSG_BUFFER_SIZE];
	uint16_t buffer_length = 0;
	sample_t sample;

	while (1) {
		if (adc_queue && timer_flag) {
			// send all data we have available
			while(queue_try_remove(adc_queue, &sample)) {
				// format into buffer
				buffer_length = snprintf(
					buffer, MSG_BUFFER_SIZE, MESSAGE_STR, (float)queue_get_level(adc_queue),
					sample.timestamp, sample.value0 * CONVERSION_FACTOR,
					sample.value1 * CONVERSION_FACTOR, sample.value2 * CONVERSION_FACTOR,
					27.0 - (sample.temperature * CONVERSION_FACTOR - 0.706) / 0.001721);
				// send buffer to server
				send_UDP(callback_struct.addr, PORT, buffer, buffer_length);
			}			
			timer_flag = false;
		} else {
			// poll wifi module
			cyw43_arch_poll();
		}
	}

	udp_remove(upcb);
	udp_remove(spcb);
	cyw43_arch_deinit();
	return 0;
}

static int64_t set_send_UDP(alarm_id_t id, void *userdata) { timer_flag = true; }

void send_UDP(ip_addr_t *destAddr, int port, void *data, int data_size) {
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, data_size, PBUF_RAM);
	memcpy(p->payload, data, data_size);
	cyw43_arch_lwip_begin();
	udp_sendto(upcb, p, destAddr, port);
	cyw43_arch_lwip_end();
	pbuf_free(p);
}

void recv_from_UDP(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port) {
	printf("[UDPServer] Received from %d.%d.%d.%d port=%d\n", addr->addr & 0xff,
		   (addr->addr >> 8) & 0xff, (addr->addr >> 16) & 0xff, addr->addr >> 24, port);

	printf("[UDPServer] Length = %d , Total Length = %d\n", p->len, p->tot_len);
	printf("[UDPServer] payload -> %s\n", (char *)p->payload);
}

void on_dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
	struct dns_callback *callback_data = (struct dns_callback *)callback_arg;

	if (ipaddr) {
		callback_data->addr = ipaddr;
		callback_data->valid = 1;
	} else {
		printf("[UDPServer] Failed to resolve host\n");
	}
}

bool adc_callback(struct repeating_timer *t) {
	// make a new sample and read data into it
	sample_t sample;
	// read the adc
	adc_select_input(0);
	sample.value0 = adc_read();
	adc_select_input(1);
	sample.value1 = adc_read();
	adc_select_input(2);
	sample.value2 = adc_read();
	adc_select_input(3);
	sample.temperature = adc_read();
	// timestamp the sample
	sample.timestamp = to_us_since_boot(get_absolute_time());
	// enqueue the sample (throw away the error, we jsut wont write beyond the max)
	bool success = queue_try_add(adc_queue, &sample);
	// return true to keep the timer going
	return true;
}

void adc_core_main() {
	// init adc
	adc_initialize();

	// setup an spinlocked queue to hold up to MAX_SAMPLES samples
	adc_queue = (queue_t *)malloc(sizeof(queue_t));
	queue_init(adc_queue, sizeof(sample_t), MAX_SAMPLES);

	// setup the repeating timer to call the adc
	struct repeating_timer adc_timer;
	add_repeating_timer_ms(5, adc_callback, (void *)NULL, &adc_timer);

	// lock up this core as its only needed for interrupts now
	while (1) {
		// do nothing
		tight_loop_contents();
	}
}

void adc_initialize() {
	adc_init();
	// Make sure GPIO is high-impedance, no pullups etc
	adc_gpio_init(26);
	adc_gpio_init(27);
	adc_gpio_init(28);
	// adc_gpio_init(29);
	// Select ADC input 0 (GPIO26)
	adc_select_input(0);
}