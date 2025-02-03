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
#define MAX_RBUF 256
#define MAX_WBUF 256
#define MAX_TBUF (256-16)

pid_t pid;

int get_pwd(char *buf)
{
	/* Implement code */
	buf = getcwd(0, sizeof(char) * 1024);
	if (buf)
		return (0);
	else
		return (1);

}

void process_command(int sfd_client)
{
	char rbuf[MAX_RBUF];
	char wbuf[MAX_WBUF];
	char tbuf[MAX_TBUF];
	int len;

	for(;;) {
		/* Implement code: call read() */
		len = read(sfd_client, rbuf, MAX_BUF-1);
		if (len <= 0) {
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

		printf("[%d] received: %s", pid, rbuf);

		if(strcmp(rbuf, "QUIT\r\n") == 0) {
			/* Implement code: call write() */
			write(sfd_client, "OK", 2);

			return;
		}
		else if(strcmp(rbuf, "PWD\r\n") == 0) {
			/* Implement code: call write() */

			get_pwd(tbuf);
			wbuf = strcat("ok", tbuf);
			write(sfd_client, wbuf, strlen(wbuf));
		}
	}
}

int main(int argc, char **argv)
{
	int ret;
	int sfd_server, sfd_client;
	struct sockaddr_in addr_server;
	struct sockaddr_in addr_client;
	socklen_t addr_client_len;
	int optval = 1;

	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("[%d] running %s\n", pid = getpid(), argv[0]);

	/* Implement code: call socket() */
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

	/* Implement code: call bind() */
	memset(&addr_server, 0, sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_server.sin_port = htons(SERVER_PORT);
	ret = bind(sfd_server, (struct sockaddr *)&addr_server, sizeof(addr_server));
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	/* Implement code: call listen() */
	ret = listen(sfd_server, LISTEN_BACKLOG);
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	for(;;) {
		printf("[%d] waiting for client ...\n", pid);
		
		/* Implement code: call accept() */
		addr_client_len = sizeof(addr_client);
		sfd_client = accept(sfd_server, (struct sockaddr *)&addr_client, &addr_client_len);
		if(sfd_client == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return EXIT_FAILURE;
		}

		printf("[%d] connected\n", pid);

		process_command(sfd_client);

		/* Implement code: call close() */
		close(sfd_client);

		printf("[%d] closed\n", pid);
	}

	close(sfd_server);

	printf("[%d] terminated\n", pid);

	return EXIT_SUCCESS;
}

