#include <stdio.h>
#include <unistd.h>

void func(int num) {
	printf("func %d\n", num);
}

int main(void) {
	int i;

	for(i=0; ; i++) {
		func(i);
		sleep(1);
	}
	return 0;
}
