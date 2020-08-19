CC=gcc
CFLAGS="-fno-stack-protector" "-no-pie" "-lm"

rop: clean
	$(CC) -g -o rop rop-lang.c $(CFLAGS)

clean:
	rm -f rop
