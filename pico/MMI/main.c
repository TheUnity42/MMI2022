#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>

#define OUTPUT_BUFFER 256

const uint16_t ADC_QUERY = 40;

bool timer_callback(struct repeating_timer *t);

// 12-bit conversion, assume max value == ADC_VREF == 3.3 V
const float conversion_factor = 3.3f / (1 << 12);

struct adc_sample {
	uint16_t adc_value;
	uint64_t timestamp;
};

int main() {
	stdio_init_all();

	adc_init();

	// Make sure GPIO is high-impedance, no pullups etc
	adc_gpio_init(26);
	// Select ADC input 0 (GPIO26)
	adc_select_input(0);

	// initialize empty callback struct
	struct repeating_timer timer;

	// initialize data struct
	struct adc_sample sample;

	// add_repeating_timer_ms(-10, timer_callback, current_sample, &timer);

	char output[OUTPUT_BUFFER];
	uint16_t output_length = 0;

	// run indefinitely
	while (1) {
		// wait 100ms for a query
		uint16_t rc = getchar_timeout_us(100000);
		// if we recieved an adc query, print the value
		if (rc == ADC_QUERY) {
			sample.adc_value = adc_read();
			sample.timestamp = to_us_since_boot(get_absolute_time());

			output_length = snprintf(output, OUTPUT_BUFFER, ",%1.6f,%d",
									 sample.adc_value * conversion_factor, sample.timestamp);

			printf("<%03d%s>", output_length + 5, output);
		}
	}
}

// bool timer_callback(struct repeating_timer *t) {
// 	// take a sample
// 	adctmp->adc_value = adc_read();
// 	adctmp->timestamp = to_us_since_boot(get_absolute_time());

// 	struct adc_sample *out = (struct adc_sample *)t->user_data;

// 	// store the output
// 	memcpy(out, adctmp, sizeof(struct adc_sample));
// }
