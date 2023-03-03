#include <string.h>
#include <time.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define PORT 6001
#define HOST "fedora"
#define MAX_SAMPLES 32

#define DEBUG_printf printf

#define MESSAGE_STR "PICO_W[QUEUE_HEALTH(%f\%), ADC_0(%llu, %f)]\n"
#define MESSAGE_STR "(%f,%llu,%f)\n"
#define MESSAGE_BUFFER_SIZE 128

const inline float CONVERSION_FACTOR = 3.3f / (1 << 12);

unsigned int loop = 0;
alarm_pool_t *alarm_pool;
repeating_timer_t repeating_timer;
struct udp_pcb *upcb;
struct udp_pcb *spcb;

queue_t *adc_queue;

bool timer_flag = false;

static int64_t set_send_UDP(alarm_id_t id, void *userdata) { timer_flag = true; }

void send_UDP(ip_addr_t* destAddr, int port, void *data, int data_size) {
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, data_size, PBUF_RAM);
	memcpy(p->payload, data, data_size);
	cyw43_arch_lwip_begin();
	udp_sendto(upcb, p, destAddr, port);
	cyw43_arch_lwip_end();
	pbuf_free(p);
}

void recv_from_UDP(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port) {

	printf("received from %d.%d.%d.%d port=%d\n", addr->addr & 0xff, (addr->addr >> 8) & 0xff,
		   (addr->addr >> 16) & 0xff, addr->addr >> 24, port);

	printf("Length = %d , Total Length = %d\n", p->len, p->tot_len);
	printf("payload -> %s\n", (char *)p->payload);
}

struct dns_callback {
	uint8_t valid;
	ip_addr_t *addr;
};

typedef struct SAMPLE_T_ {
	uint16_t value;
	uint64_t timestamp;
} sample_t;

// https://www.nongnu.org/lwip/2_0_x/group__udp__raw.html

void on_dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
	struct dns_callback *callback_data = (struct dns_callback *)callback_arg;

	char *ip_name = ip4addr_ntoa(ipaddr);

	// printf("[UDPServer] DNS Callback %s, %s\n", name, ip_name);

	if (ip_name) {
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
	sample.value = adc_read();
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

	// setup the repeating timer to call the adc every 10ms
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
	// Select ADC input 0 (GPIO26)
	adc_select_input(0);
}

int main() {
	
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

	err_t err = udp_bind(spcb, IP_ADDR_ANY, 6002);

	udp_recv(spcb, recv_from_UDP, NULL);

	ip_addr_t destination_ip;
	// ip4addr_aton(RECEIVER_IP, &destination_ip);

	printf("[UDPServer] DNS Lookup in progress...\n");
	struct dns_callback callback_struct = {0};
	while (!callback_struct.valid) {
		err_t err = dns_gethostbyname(HOST, &destination_ip, on_dns_found, &callback_struct);
		if (err == ERR_INPROGRESS) {
			printf("[UDPServer] DNS lookup in progress...\n");
		} else if (err == ERR_ARG) {
			printf("[UDPServer] DNS lookup failure ARG\n");
		} else if (err == ERR_OK) {
			printf("[UDPServer] DNS lookup OK\n");
			break;
		}
		sleep_ms(10);
	}

	printf("[UDPServer] DNS Resolved to: %d.%d.%d.%d\n", (callback_struct.addr->addr & 0xff),
		   (callback_struct.addr->addr >> 8) & 0xff, (callback_struct.addr->addr >> 16) & 0xff,
		   callback_struct.addr->addr >> 24);

	// UDP has been fully initialized, start the ADC core
	multicore_launch_core1(adc_core_main);

	char buffer[32];
	uint16_t buffer_length = 0;
	sample_t sample;

	while (1) {
		// if (timer_flag) {
		// 	memset(buffer, 0, BUF_SIZE);
		// 	sprintf(buffer, "%u\n", loop++);
		// 	send_UDP(callback_struct.addr, RECEIVER_PORT, buffer, BUF_SIZE);
		// 	timer_flag = false;
		// }
		// cyw43_arch_poll();
		if (adc_queue && timer_flag) {
			// send all data we have available
			while(queue_try_remove(adc_queue, &sample)) {
				// format into buffer
				buffer_length = snprintf(buffer, MESSAGE_BUFFER_SIZE, MESSAGE_STR,
										 (float)queue_get_level(adc_queue), sample.timestamp,
										 sample.value * CONVERSION_FACTOR);
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