#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

pid_t pid;

int main(int argc, char **argv)
{
	//pid_t pid_temp[3];	
	pid_t *pid_temp;
	int i;
	int num_of_child;

	printf("[pid: %d] running %s", pid = getpid(), argv[0]);
	for(i=1; i<argc; i++) {
		printf(" %s", argv[i]);
	}
	printf("\n");

	num_of_child = argc - 2;
	
	/* Implement code */
	
	pid_temp = (pid_t *)malloc(sizeof(pid_t) * num_of_child);
	if (pid_temp == NULL)
		return (2);
	
	pid_t parent_pid = getpid();

	for (int i = 0; i < num_of_child; i++)
	{
		if (getpid() == parent_pid)
			pid_temp[i] = fork();
	}

	for (int i = 0; i < num_of_child; i++)
	{
		if (pid_temp[i] == 0) //child
		{
			printf("[pid: %d] about to run [grep -rn %s %s]\n", (int)getpid(), argv[i + 2], argv[1]);
			execl("/usr/bin/grep", "grep", "-rn", argv[2], argv[1], NULL);
		}
	}
	
	if (getpid() == parent_pid)
	{
		printf("[pid: %d] waiting child's termination\n", (int)getpid());
		for (int i = 0; i < num_of_child; i++)
		{
			waitpid(pid_temp[i], NULL, 0);	
			printf("pid %d bas been terminated with status 0\n", (int)pid_temp[i]);
		}
	}

	/*
	pid_temp[0] = fork();
	if (pid_temp[0] == 0) //first child
	{
		printf("[pid: %d] about to run [grep -rn %s %s]\n", (int)getpid(), argv[2], argv[1]);
		execl("/usr/bin/grep", "grep", "-rn", argv[2], argv[1], NULL);
	}
	else
	{
		pid_temp[1] = fork();
		if (pid_temp[1] == 0) //second child
		{
			printf("[pid: %d] about to run [grep -rn %s %s]\n", (int)getpid(), argv[3], argv[1]);
			execl("/usr/bin/grep", "grep", "-rn", argv[3], argv[1], NULL);
		}
		else
		{
			pid_temp[2] = fork();
			if (pid_temp[2] == 0) //third child
			{
				printf("[pid: %d] about to run [grep -rn %s %s]\n", (int)getpid(), argv[4], argv[1]);
				execl("/usr/bin/grep", "grep", "-rn", argv[4], argv[1], NULL);
			}
			else
			{
				//parent
				printf("[pid: %d] waiting child's termination\n", (int)getpid());
				for (int i = 0; i < 3; i++)
				{
					waitpid(pid_temp[i], NULL, 0);
					printf("pid %ld has been terminated with status 0\n", (long)pid_temp[i]);
				}
		
			}
		}
	}
	*/

	printf("[pid: %d] terminted\n", (int)pid);
	
	free(pid_temp);
	return EXIT_SUCCESS;
}

