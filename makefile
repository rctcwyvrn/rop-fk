CC=gcc
CFLAGS="-fno-stack-protector" "-no-pie" "-lm"

rop: clean
	$(CC) -g -o rop rop-lang.c scanner.c $(CFLAGS)

clean:
	rm -f rop
