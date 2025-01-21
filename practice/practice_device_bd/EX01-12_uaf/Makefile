all:
	aarch64-linux-gnu-gcc -o uaf -fsanitize=address -static-libasan -g uaf.c

clean:
	rm uaf
