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

#define LISTEN_BACKLOG 5
#define MAX_BUF 256

pid_t pid;

int main(int argc, char **argv)
{
	int ret;
	int len;
	int sfd_server, sfd_client;
	struct sockaddr_in addr_server;
	struct sockaddr_in addr_client;
	socklen_t addr_client_len;
	char buf[MAX_BUF];
	int optval = 1;

	struct timeval tv;
	fd_set rfd, tfd;

	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("[%d] running %s\n", pid = getpid(), argv[0]);

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

	FD_ZERO(&rfd);
	FD_SET(sfd_server, &rfd);

	for(;;) {
		printf("[%d] waiting for client ...\n", pid);

		tv.tv_sec = 3;
		tv.tv_usec = 0;
		tfd = rfd;
		ret = select(FD_SETSIZE, &tfd, NULL, NULL, &tv);
		if(ret == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return EXIT_FAILURE;
		}
		else if(ret == 0) {
			printf("[%d] timeout\n", pid);
			continue;
		}

		addr_client_len = sizeof(addr_client);
		sfd_client = accept(sfd_server, (struct sockaddr *)&addr_client, &addr_client_len);
		if(sfd_client == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return EXIT_FAILURE;
		}
		printf("[%d] connected\n", pid);

		for(;;) {
			len = read(sfd_client, buf, MAX_BUF-1);
			if(len <= 0) {
				close(sfd_client);
				printf("[%d] closed\n", pid);
				break;
			}
			else {
				buf[len] = 0;
				printf("[%d] received: %s\n", pid, buf);

				/* echo back */
				write(sfd_client, buf, len);
			}
		}
	}

	close(sfd_server);

	printf("[%d] terminated\n", pid);

	return EXIT_SUCCESS;
}

