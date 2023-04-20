#ifndef MAIN_H
#define MAIN_H

// Configuration for boards
#define PORT 6000
#define HOST "mmi-host"
#define MAX_SAMPLES 32
#define RESPONSE_PORT 6000
#define SAMPLE_PERIOD 1

// Message format
#define MESSAGE_STR "(%f,%llu,%f,%f,%f,%d)\n"
#define MSG_BUFFER_SIZE 136

// Pin definitions
// Button for labelling data
#define BUTTON 5

// conversion from ADC to voltage
const inline float CONVERSION_FACTOR = 3.3f / (1 << 12);

/**
 * @brief Callback struct used for DNS lookup
 * 
 */
struct dns_callback {
	uint8_t valid;
	ip_addr_t *addr;
};

/**
 * @brief Struct used for storing samples
 * Each sample contains all ADC values, a label, and the timestamp
 * 
 */
typedef struct SAMPLE_T_ {
	uint16_t value0;
	uint16_t value1;
	uint16_t value2;
	uint8_t label;
	
	uint64_t timestamp;
} sample_t;

// https://www.nongnu.org/lwip/2_0_x/group__udp__raw.html

// udp functions
/**
 * @brief configures timer interrupt to repeat indefinitely
 * 
 * @param id timer id
 * @param userdata userdata structure (not used)
 * @return int64_t error code
 */
static int64_t set_send_UDP(alarm_id_t id, void *userdata);

/**
 * @brief sends UDP packet to specified address
 * 
 * @param destAddr destination address
 * @param port destination port
 * @param data data to send
 * @param data_size the size of the data to send
 */
void send_UDP(ip_addr_t *destAddr, int port, void *data, int data_size);

/**
 * @brief callback function to receive UDP packets
 * 
 * @param arg user argument (not used)
 * @param pcb receiving protocol control block
 * @param p sending packet buffer
 * @param addr sending address
 * @param port sending port
 */
void recv_from_UDP(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);

// dns functions
/**
 * @brief callback function for DNS lookup
 * 
 * @param name hostname found
 * @param ipaddr ip address of hostname
 * @param callback_arg user argument (not used)
 */
void on_dns_found(const char *name, const ip_addr_t *ipaddr, void *callback_arg);

// adc functions
/**
 * @brief Callback function to read ADC values
 * 
 * @param t interrupt timer struct
 * @return true to continue timer
 * @return false to stop timer
 */
bool adc_callback(struct repeating_timer *t);

/**
 * @brief Main function for ADC core
 * 
 */
void adc_core_main();

/**
 * @brief Initializes ADC core
 * 
 */
void adc_initialize();

#endif // MAIN_H
