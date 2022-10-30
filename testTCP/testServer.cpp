#include <cstdio>
#include <unistd.h>

int main() {
    // send a test message every half second until 500 messages are sent
    for (int i = 0; i < 500; i++) {
        printf("Test Packet\n");
        fflush(stdout);
        usleep(500000);
    }

	return 0;
}