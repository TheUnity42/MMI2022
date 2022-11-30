#include <stdlib.h>
#include <string.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico_tcp_shared.h"

queue_t *adc_queue;

void adc_core_main() {
	// init adc
	adc_initialize();

	// setup an spinlocked queue to hold up to MAX_SAMPLES samples
	adc_queue = (queue_t *) malloc(sizeof(queue_t));
	queue_init(adc_queue, sizeof(sample_t), MAX_SAMPLES);

	// setup the repeating timer to call the adc every 10ms
	struct repeating_timer adc_timer;
	add_repeating_timer_ms(5, adc_callback, (void *)NULL, &adc_timer);

	// lock up this core as its only needed for interrupts now
	while(1) {
		// do nothing
		tight_loop_contents();
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

void adc_initialize() {
	adc_init();
	// Make sure GPIO is high-impedance, no pullups etc
	adc_gpio_init(26);
	// Select ADC input 0 (GPIO26)
	adc_select_input(0);
}