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

#include "define.h"

#define MAX_BUF 256

pid_t pid;

int write_log(char *str, char *filename)
{
	FILE *fp_w;
	int ret;
	char cmd[64];

	sprintf(cmd, "cat >> %s", filename);
	fp_w = popen(cmd, "w");
	if(fp_w == NULL) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return -1;
	}
	fwrite(str, 1, strlen(str), fp_w);

	ret = pclose(fp_w);
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret;
	int len;
	int sfd;
	struct sockaddr_in addr_server;
	struct sockaddr_in addr_client;
	socklen_t addr_client_len;
	char buf[MAX_BUF];
	info_t * p_info = (info_t *)buf;
	//char tbuf[MAX_BUF];

	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("[%d] running %s\n", pid = getpid(), argv[0]);

	/* Implement code: call socket() */
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sfd == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	/* Implement code: call bind() */
	memset(&addr_server, 0, sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_server.sin_port = htons(SERVER_PORT);
	ret = bind(sfd, (struct sockaddr *)&addr_server, sizeof(addr_server));
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	printf("ok before for loop\n");
	for(;;) {
		/* Implement code: call recvfrom(), write_log(), sendto() */
		addr_client_len = sizeof(addr_client);
		len = recvfrom(sfd, buf, MAX_BUF-1, 0, (struct sockaddr *)&addr_client, &addr_client_len);
		if(len > 0) {
			buf[len] = 0;
			
			char string[64];
			sprintf(string, "[%d] received: date = %s, msg = %s\n", pid, p_info->date, p_info->msg);
			printf("%s\n", string);

			write_log(string, "my.log");

			/* send ok */
			char sendStr[3] = "OK\0";
			sendto(sfd, sendStr, len, 0,  (struct sockaddr *)&addr_client, sizeof(addr_client));
		}

	}

	/* Implement code: call close() */
	close(sfd);

	printf("[%d] terminated\n", pid);

	return EXIT_SUCCESS;
}

