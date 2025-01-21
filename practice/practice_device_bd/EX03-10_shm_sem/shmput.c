#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/sem.h>

#include "shm.h"
#include "sem.h"

pid_t pid;

int main(int argc, char **argv)
{
	int ret;
	int ret_sem;
	int id_shm;
	struct shm_buf *shmbuf;
	int i;
	struct timeval tv;
	time_t start_time;
	int first = 1;
	unsigned long cs;

	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("[%d] running %s\n", pid = getpid(), argv[0]);

	id_shm = shmget((key_t)KEY_SHM, sizeof(struct shm_buf), 0666|IPC_CREAT);
	if(id_shm == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	shmbuf = shmat(id_shm, (void *)0, 0);
	if(shmbuf == (struct shm_buf *)-1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	
	int id_sem;
	struct sembuf sops_dec[1] = {{0, -1, SEM_UNDO}};
	struct sembuf sops_inc[1] = {{0, 1, SEM_UNDO}};
	id_sem = semget((key_t)KEY_SEM, 1, 0666|IPC_CREAT);

	for (;;)
	{
		ret_sem = semop(id_sem, sops_dec, 1);
		if (ret_sem == -1)
		{
			printf("semop fail\n");
			return EXIT_FAILURE;
		}
	
		if(gettimeofday(&tv, NULL) == -1) {
			printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
			return EXIT_FAILURE;
		}

		if(first) {
			first = 0;
			start_time = tv.tv_sec;
		}
		else {
			if(tv.tv_sec - start_time > TIMEOUT) {
				shmbuf->status = STATUS_INVALID;
				break;
			}
		}

		sprintf(shmbuf->buf, "%lu.%06lu", tv.tv_sec, tv.tv_usec);
		shmbuf->status = STATUS_VALID;
		for(i=0, cs=0; i<sizeof(struct shm_buf)-sizeof(unsigned long); i++) {
			cs += ((unsigned char *)shmbuf)[i];
		}
		shmbuf->cs = cs;

		//usleep(100000);
		ret_sem = semop(id_sem, sops_inc, 1);
	}

	ret = shmdt(shmbuf);
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	printf("[%d] terminted\n", pid);

	return EXIT_SUCCESS;
}

