#ifndef MAIN_H
#define MAIN_H

#define PORT 6001
#define HOST "nate-Blade-15"
#define MAX_SAMPLES 32
#define RESPONSE_PORT 6001

#define DEBUG_printf printf

#define MESSAGE_STR "(%f,%llu,%f,%f,%f)\n"
#define MSG_BUFFER_SIZE 128

const inline float CONVERSION_FACTOR = 3.3f / (1 << 12);

struct dns_callback {
	uint8_t valid;
	ip_addr_t *addr;
};

typedef struct SAMPLE_T_ {
	uint16_t value0;
	uint16_t value1;
	uint16_t value2;
	uint16_t temperature;
	uint64_t timestamp;
} sample_t;

// https://www.nongnu.org/lwip/2_0_x/group__udp__raw.html

// udp functions
static int64_t set_send_UDP(alarm_id_t id, void *userdata);
void send_UDP(ip_addr_t *destAddr, int port, void *data, int data_size);
void recv_from_UDP(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);
// dns functions
void on_dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg);
// adc functions
bool adc_callback(struct repeating_timer *t);
void adc_core_main();
void adc_initialize();

#endif // MAIN_H