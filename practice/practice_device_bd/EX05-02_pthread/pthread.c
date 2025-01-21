#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/syscall.h>

pid_t pid;
pid_t tid;
int g_count;

void *thread_func(void *arg)
{
	int i;

	tid = syscall(__NR_gettid);
	printf("[%d] thread started with arg \"%s\"\n", tid, (char *)arg);
	for(i=0; i<5; i++) {
		g_count++;
		printf("[%d] thread running (g_count = %d)\n", tid, g_count);
		sleep(1);
	}

	pthread_exit("Goodbye Thread");
}

int main(int argc, char **argv)
{
	pthread_t thread_id;
	void *thread_ret;
	int ret;

	printf("[%d] running %s\n", pid = getpid(), argv[0]);

	printf("[%d] creating thread (g_count = %d)\n", pid, g_count);
	ret = pthread_create(&thread_id, NULL, thread_func, "Hello Thread");
	if(ret != 0) {
		printf("[%d] error: %d (%d)\n", pid, ret, __LINE__);
		return EXIT_FAILURE;
	}

	printf("[%d] waiting to join with a terminated thread\n", pid);
	ret = pthread_join(thread_id, &thread_ret);
	if(ret != 0) {
		printf("[%d] error: %d (%d)\n", pid, ret, __LINE__);
		return EXIT_FAILURE;
	}
	printf("[%d] thread returned \"%s\" (g_count = %d)\n", pid, (char *)thread_ret, g_count);

	printf("[%d] terminted\n", pid);

	return EXIT_SUCCESS;
}
