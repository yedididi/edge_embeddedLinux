#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
int main(int argc, char **argv)
{
	char *buf = malloc(100);
	if(buf == NULL) return EXIT_FAILURE;

	free(buf);

	strcpy(buf, "Hello Linux");
	printf("buf = %s\n", buf);

	return 0;
}
