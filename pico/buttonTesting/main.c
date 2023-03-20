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

int main() {
    
    stdio_init_all();
    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_down(BUTTON);
    while (true){
        printf("%d\n",gpio_get(BUTTON));
        sleep_ms(1000);
    }
    return 0;
}