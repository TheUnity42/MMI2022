#include "shared.h"

const float CONVERSION_FACTOR = 3.3f / (1 << 12);

const char *ADC_REQUEST = "ADC";

int wifi_init() {
	// setup the module
	if (cyw43_arch_init()) {
		printf("[PicoWIFI] failed to initialise cyw43\n");
		return -1;
	}

	cyw43_arch_enable_sta_mode();

	printf("[PicoWIFI] Connecting to WiFi...\n");
	if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK,
										   30000)) {
		printf("[PicoWIFI] failed to connect.\n");
		return -1;
	} else {
		printf("[PicoWIFI] Connected.\n");
		return 0;
	}
}

int wifi_close() { cyw43_arch_deinit(); }