#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/gpio.h>

#include "define.h"

#define LISTEN_BACKLOG 5
#define MAX_RBUF 256
#define MAX_WBUF 256

#define ERROR_MSG "no such ID or CMD"

typedef void (*sighandler_t)(int);

pid_t pid;

#define GPIODEV "/dev/gpiochip4"

#define GPIO_LED 12
#define GPIO_KEY 21

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

#if 0
        for (int i = 0; i < chip_info.lines; i++) {

                struct gpioline_info line_info;

                line_info.line_offset = i;

                if (ioctl(fd_gpio, GPIO_GET_LINEINFO_IOCTL, &line_info) == -1) {
                        printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
                } else {
                        printf("[%d] %d: %s\n", pid, i, line_info.name);
                }
        }
#endif

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
		return -1;
        }

        return hdata.values[0];
}

int control_led(int on)
{
        struct gpiohandle_data hdata;

        if(on == 1) {
                /* LED ON */
                hdata.values[0] = 1;
                if(ioctl(handle_request_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &hdata) == -1) {
                        printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return -1;
                }

        } else {
                hdata.values[0] = 0;
                if(ioctl(handle_request_led.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &hdata) == -1) {
                        printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return -1;
                }
        }
	return 0;
}

int do_led_on(void)
{
	return control_led(1);
}

int do_led_off(void)
{
	return control_led(0);
}

int get_key_status(void)
{
	return read_key();
}

void process_command(int sfd_client)
{
	char rbuf[MAX_RBUF];
	char wbuf[MAX_WBUF];
	int rlen;
	unsigned int id, cmd;
	int result;

	for(;;) {
		rlen = read(sfd_client, rbuf, MAX_RBUF);
		if (rlen <= 0) return;

		if (rlen != 8) continue;
		/* Implement code */
		id = ntohl(*(unsigned int *)&rbuf[0]);
		cmd = ntohl(*(unsigned int *)&rbuf[4]);
		result = RES_OK;
		if (id == ID_LED)
		{
			if (cmd == CMD_LED_OFF)
			{
				result = do_led_on();
			}
			else if (cmd == CMD_LED_ON)
			{
				result = do_led_off();
			}
			else
			{
				result = RES_ERROR;
			}
		}
		else if (id == ID_KEY)
		{	
			if (cmd == CMD_KEY_STATUS)
			{
				result = get_key_status();
			}
			else
			{
				result = RES_ERROR;
			}
		}
		else
		{
			result = RES_ERROR;
		}

	}
	return (result);
}

int main(int argc, char **argv)
{
	int ret;
	int sfd_server, sfd_client;
	struct sockaddr_in addr_server;
	struct sockaddr_in addr_client;
	socklen_t addr_client_len;
	pid_t pid_temp;
	sighandler_t sig_ret;
	int optval = 1;

	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("[%d] running %s\n", pid = getpid(), argv[0]);

	/* to prevent child process from becoming zombie */
	sig_ret = signal(SIGCHLD, SIG_IGN);
	if(sig_ret == SIG_ERR) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	sfd_server = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd_server == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	/* to prevent "Address already in use" error */
	ret = setsockopt(sfd_server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	memset(&addr_server, 0, sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_server.sin_port = htons(SERVER_PORT);
	ret = bind(sfd_server, (struct sockaddr *)&addr_server, sizeof(addr_server));
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	ret = listen(sfd_server, LISTEN_BACKLOG);
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	init_gpio();

	for(;;) {
		printf("[%d] waiting for client ...\n", pid);
		addr_client_len = sizeof(addr_client);
		sfd_client = accept(sfd_server, (struct sockaddr *)&addr_client, &addr_client_len);
		if(sfd_client == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return EXIT_FAILURE;
		}
		printf("[%d] connected\n", pid);

		pid_temp = fork();

		if(pid_temp == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return EXIT_FAILURE;
		}
		else if(pid_temp == 0) {
			pid = getpid();

			close(sfd_server);

			process_command(sfd_client);

			close(sfd_client);
			printf("[%d] closed\n", pid);

			return EXIT_SUCCESS;
		}
		else {
			close(sfd_client);
		}
	}

	close(sfd_server);

	printf("[%d] terminated\n", pid);

	return EXIT_SUCCESS;
}

