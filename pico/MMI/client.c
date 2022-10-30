#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define HANDLE_ERROR(err, msg)                                                                     \
	if (err) {                                                                                     \
		perror(msg);                                                                               \
		return EXIT_FAILURE;                                                                       \
	}

#define SOFT_HANDLE_ERROR(err, msg)                                                                \
	if (err) {                                                                                     \
		perror(msg);                                                                               \
		break;                                                                                     \
	}

#define BUFFER_SIZE 256

const char ADC_QUERY = 40;

struct adc_sample {
	uint16_t adc_value;
	uint64_t timestamp;
};

int configure_pico_usb(int port_descriptor, struct termios *tty);

int main(int argc, char **argv) {
	// read input file
	if (argc < 2) {
		printf("Usage: %s <Serial Port> [<log file>]\n", argv[0]);
		return 1;
	}

	const char *port = argv[1];
	FILE *log = NULL;
	if (argc > 2) {
		log = fopen(argv[2], "w+");
		HANDLE_ERROR(log == NULL, "Unable to open/create log file");
		fprintf(log, "Timestamp,Value\n");
	}

	printf("Starting client, listening to %s\n", port);

	if (log) {
		printf("Logging to file %s\n", argv[2]);
	}

	// open serial port descriptor
	int port_descriptor = open(port, O_RDWR);

	HANDLE_ERROR(port_descriptor < 0, "Unable to open port");

	// setup options
	struct termios tty;

	HANDLE_ERROR(configure_pico_usb(port_descriptor, &tty) != 0,
				 "Unable to configure serial port for pico");

	// read data
	char buffer[BUFFER_SIZE];
	struct adc_sample sample;
	int ws = 0;
	int n = 1;

	while (n) {
		// query for ADC value
		ws = write(port_descriptor, &ADC_QUERY, 1);
		// Handle missed write
		SOFT_HANDLE_ERROR(ws < 1, "Unable to write to port");

		// read incoming bytes. Waits 5s for data, else returns 0, which will exit the loop
		n = read(port_descriptor, &buffer, BUFFER_SIZE);

		SOFT_HANDLE_ERROR(n < 0, "Unable to read from port");
		printf("[bytes read: %d] ", n);

		if (n > 0) {
			buffer[n] = '\0';
			printf("%s", buffer);

            if(log) {
                char *s = strtok(buffer, "<,");
                char *v = strtok(NULL, ",");
                char *t = strtok(NULL, ",>");
				fprintf(log, "%s,%s\n", t, v);
			}
		}

		printf("\n");
		usleep(10);
	}

	// close connection
	printf("Closing connection\n");
	close(port_descriptor);
    if(log)
		fclose(log);
	return 0;
}

int configure_pico_usb(int port_descriptor, struct termios *tty) {
	// set existing options
	HANDLE_ERROR(tcgetattr(port_descriptor, tty) != 0, "Unable to get port attributes");

	// parity OFF
	tty->c_cflag &= ~PARENB;
	// stop bits 1
	tty->c_cflag &= ~CSTOPB;
	// 8 bits
	tty->c_cflag &= ~CSIZE;
	tty->c_cflag |= CS8;
	// RTS/CTS OFF
	tty->c_cflag &= ~CRTSCTS;
	// local and read mode
	tty->c_cflag |= CLOCAL | CREAD;

	// disable canonical mode
	tty->c_lflag &= ~ICANON;
	// disable echo, erasure, and nl echo
	tty->c_lflag &= ~ECHO;
	tty->c_lflag &= ~ECHOE;
	tty->c_lflag &= ~ECHONL;
	// disable special characters
	tty->c_lflag &= ~ISIG;

	// disable sw flow control
	tty->c_iflag &= ~(IXON | IXOFF | IXANY);
	// disable special handling
	tty->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

	// prevent output processing
	tty->c_oflag &= ~OPOST;
	tty->c_oflag &= ~ONLCR;

	// wait 5s for data, or ADC_SAMPLE_SIZE bytes
	tty->c_cc[VTIME] = 50; // 5s
	tty->c_cc[VMIN] = 0;   // Any bytes

	// set speed
	cfsetspeed(tty, B115200);

	// save options
	HANDLE_ERROR(tcsetattr(port_descriptor, TCSANOW, tty) != 0, "Unable to set port attributes");

	return 0;
}