#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void leaks(void)
{
	system("leaks a.out");
}
 
int main(int argc, char **argv)
{
	atexit(leaks);
	char *buf = malloc(100);
	if(buf == NULL) return EXIT_FAILURE;

	strcpy(buf, "Hello Linux");
	printf("buf = %s\n", buf);

	return 0;
}
