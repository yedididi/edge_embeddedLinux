#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_BLOCK_SIZE 4096

int main(int argc, char **argv)
{
	int src_fd, dst_fd, block_size;
	char *src_name, *dst_name;
	char buf[MAX_BLOCK_SIZE];
	unsigned int copied = 0;

	if(argc != 4) {
		printf("usage: %s {src file} {dst file} {block size in bytes}\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("running %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);

	src_name = argv[1];
	dst_name = argv[2];
	block_size = atoi(argv[3]);

	if(block_size > MAX_BLOCK_SIZE) {
		printf("error: block size if too big (maximum is %d bytes (%d)\n", MAX_BLOCK_SIZE, __LINE__);
		return EXIT_FAILURE;
	}

	/* Implement code */
	src_fd = open(src_name, O_RDONLY);
	dst_fd = open(dst_name, O_CREAT | O_RDWR, S_IRWXU);
	while (1)
	{
		memset(buf, 0, block_size);
		int read_ret = read(src_fd, buf, block_size);
		int write_ret = write(dst_fd, buf, read_ret);
				
		if (read_ret == 0)
			break;
		copied += read_ret;
	}

	close(dst_fd);
	close(src_fd);
	printf("%d bytes copied\n", copied);
	return (EXIT_SUCCESS);
}


