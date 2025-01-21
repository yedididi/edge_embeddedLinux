all:
	aarch64-linux-gnu-gcc -o leak -fsanitize=address -static-libasan -g leak.c

clean:
	rm leak
