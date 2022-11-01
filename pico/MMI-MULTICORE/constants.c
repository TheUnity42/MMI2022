#include "pico_tcp_shared.h"

const float CONVERSION_FACTOR = 3.3f / (1 << 12);

const char *ADC_REQUEST = "ADC";