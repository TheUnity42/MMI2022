#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    float x = 0.1;
    while (true) {
        printf("%f\n", x);
        x += 0.1;
        sleep_ms(1000);
    }
    return 0;
}
