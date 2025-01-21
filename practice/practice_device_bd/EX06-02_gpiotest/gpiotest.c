#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>

#define GPIODEV	"/dev/gpiochip4"

#define GPIO_LED 12
#define GPIO_KEY 21

pid_t pid;

struct gpiochip_info chip_info;
struct gpiohandle_request handle_request_led;
struct gpiohandle_request handle_request_key;
int fd_gpio;

void init_gpio(void)
{
	fd_gpio = open(GPIODEV, O_RDONLY);
	if(fd_gpio == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}
	printf("[%d] %s was opened successfully\n", pid, GPIODEV);

	if(ioctl(fd_gpio, GPIO_GET_CHIPINFO_IOCTL, &chip_info) == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}
	printf("[%d] GPIO chip: name=%s, lines=%d\n", pid, chip_info.name, chip_info.lines);

	for (int i = 0; i < chip_info.lines; i++) {

		struct gpioline_info line_info;

		line_info.line_offset = i;

		if (ioctl(fd_gpio, GPIO_GET_LINEINFO_IOCTL, &line_info) == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		} else {
			printf("[%d] %d: %s\n", pid, i, line_info.name);
		}
	}

	handle_request_led.lineoffsets[0] = GPIO_LED;
	handle_request_led.flags = GPIOHANDLE_REQUEST_OUTPUT | GPIOHANDLE_REQUEST_BIAS_DISABLE;
	handle_request_led.lines = 1;

	if(ioctl(fd_gpio, GPIO_GET_LINEHANDLE_IOCTL, &handle_request_led) == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	handle_request_key.lineoffsets[0] = GPIO_KEY;
	handle_request_key.flags = GPIOHANDLE_REQUEST_INPUT | GPIOHANDLE_REQUEST_BIAS_DISABLE;
	handle_request_key.lines = 1;

	if(ioctl(fd_gpio, GPIO_GET_LINEHANDLE_IOCTL, &handle_request_key) == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}
}

int read_key(void)
{
	struct gpiohandle_data hdata;

	if(ioctl(handle_request_key.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &hdata) == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		exit(EXIT_FAILURE);
	}

	return hdata.values[0];
}

void control_led(int on)
{
	struct gpiohandle_data hdata;

	if(on == 1) {
		/* LED ON */
		hdata.values[0] = 1;
		if(ioctl(handle_request_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &hdata) == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			exit(EXIT_FAILURE);
		}

	} else {
		hdata.values[0] = 0;
		if(ioctl(handle_request_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &hdata) == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			exit(EXIT_FAILURE);
		}
	}
}

int main(int argc, char **argv)
{
	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("[%d] running %s\n", pid = getpid(), argv[0]);

	init_gpio();

	for(;;) {
		if(read_key() == 0) { /* Key pressed */
			control_led(1); /* LED ON */

		} else { /* Key released */
			control_led(0); /* LED OFF */
		}
		usleep(1000);
	}

	return 0;
}


